#include "hedgehog/hedgehog.h"
#include <iostream>

// Class that represents a communication queue
class CommQueue {
    int commId_ {};
    std::queue<char *> queue_ {};
    std::mutex mutex_ {};
public:
    explicit CommQueue(int commId) : commId_(commId) {}
    virtual ~CommQueue() = default;

    char *popFront() {
        std::lock_guard<std::mutex> lc(this->mutex_);
        auto element = queue_.front();
        assert(element != nullptr);
        queue_.pop();
        return element;
    }

    void push(char *data) {
        std::lock_guard<std::mutex> lc(this->mutex_);
        queue_.push(data);
    }

    bool hasElem() {
        std::lock_guard<std::mutex> lc(this->mutex_);
        return !queue_.empty();
    }

    [[nodiscard]] int commId() const {
        return commId_;
    }

};

// Class that represents a communicator sending/receiving data between graphs, each 'node' has its own communication queue
class Communicator {
//    std::unordered_map<int, std::shared_ptr<CommQueue>> commQueueMap;
    std::vector<std::shared_ptr<CommQueue>> commQueues_;

public:
    Communicator() = default;
    virtual ~Communicator() = default;


    void initComm(int numNodes) {
        commQueues_.resize(numNodes);
        for (int id = 0; id < numNodes; ++id) {
            auto commQueue = std::make_shared<CommQueue>(id);
//            commQueueMap.insert({{id, commQueue}});
            commQueues_[id] = commQueue;

        }
    }

    void sendMessage(std::ostringstream const &oss, int srcId, int destId) {
        size_t size = oss.str().size() + 1;
//        std::cout << srcId << " sending to " << destId << " " << size << " bytes" << std::endl;

        auto const &destComm = commQueues_[destId];
        char *cstr = new char[size];
        std::strcpy(cstr, oss.str().c_str());
        destComm->push(cstr);
    }

    bool hasMessage(int srcId) {
        return commQueues_[srcId]->hasElem();
    }

    std::istringstream receiveMessage(int srcId) {
        auto &srcComm = commQueues_[srcId];
        assert(srcComm->hasElem());

        auto buffer = srcComm->popFront();
        std::istringstream iss((std::string(buffer)));
        return iss;
    }

};

// A block of memory, contains information about the source and destination for the data (based on decomposition in main)
template<class MatrixType>
class CommMatrixBlock {
    static_assert(std::is_arithmetic_v<MatrixType>, "The template parameter MatrixType should be an arithmetic value.");
    int const
            dest_;
    int const src_;

    size_t const
            indexRow_ = 0,
            indexCol_ = 0,
            height_ = 0,
            width_ = 0;

    std::unique_ptr<std::vector<MatrixType>>
            data_;

public:
    CommMatrixBlock(int const src,
                    int const dest,
                    size_t const indexRow,
                    size_t const indexCol,
                    size_t const height,
                    size_t const width)
            : src_(src), dest_(dest), indexRow_(indexRow), indexCol_(indexCol), height_(height), width_(width) {
        data_ = std::make_unique<std::vector<MatrixType>>(height_ * width_, 0);
    }

    CommMatrixBlock(CommMatrixBlock<MatrixType> &&toMove) noexcept:
            src_(toMove.src_), dest_(toMove.dest_),
            indexRow_(toMove.indexRow_), indexCol_(toMove.indexRow_),
            height_(toMove.height_), width_(toMove.width_) {
        data_.swap(toMove.data_);
    }

    virtual ~CommMatrixBlock() = default;
    [[nodiscard]] int src() const { return src_; }
    [[nodiscard]] int dest() const { return dest_; }
    [[nodiscard]] size_t indexRow() const { return indexRow_; }
    [[nodiscard]] size_t indexCol() const { return indexCol_; }
    [[nodiscard]] size_t height() const { return height_; }
    [[nodiscard]] size_t width() const { return width_; }
    [[nodiscard]] std::vector<MatrixType> &data() const { return *data_; }

    friend std::ostream &operator<<(std::ostream &os, CommMatrixBlock const &block) {
        os << "src_: " << block.src_ << " dest_: " << block.dest_ << " indexRow_: " << block.indexRow_ << " indexCol_: " << block.indexCol_ << " height_: "
           << block.height_ << " width_: " << block.width_ << "\nData:\n";
        for (size_t row = 0; row < block.height_; ++row) {
            std::copy_n(block.data_->begin(), block.width_, std::ostream_iterator<MatrixType>(os, " "));
            os << "\n";
        }
        return os;
    }

    std::ostream const &serialize(std::ostream &ostream) const {
        ostream << src_ << " " << dest_ << " " << indexRow_ << " " << indexCol_ << " " << height_ << " " << width_;
        std::for_each(data().begin(), data().end(), [&ostream](MatrixType const &data) { ostream << " " << data; });
        // Use null terminated string to aid with defining length of the data
        ostream << '\0';
        return ostream;
    }

    static std::shared_ptr<CommMatrixBlock<MatrixType>> deserialize(std::istream &istream) {
        int src;
        int dst;
        size_t
                indexRow,
                indexCol,
                height,
                width;

        istream >> src >> dst >> indexRow >> indexCol >> height >> width;

        auto output = std::make_shared<CommMatrixBlock<MatrixType>>(src, dst, indexRow, indexCol, height, width);

        for (auto &data: output->data()) { istream >> data; }
        return std::move(output);
    }
};

// State for data that is 'local' to a graph (destination == id)
template<class MatrixType>
class LocalMatrixState : public hh::AbstractState<CommMatrixBlock<MatrixType>, CommMatrixBlock<MatrixType>> {
private:
    int id_;

public:
    LocalMatrixState(int id) : id_(id) {}

    ~LocalMatrixState() override = default;

    void execute(std::shared_ptr<CommMatrixBlock<MatrixType>> ptr) override {
        if (ptr->dest() == id_){
            this->push(ptr);
        }
    }
};

// State for data that is 'remote' to a graph (destination != id)
template<class MatrixType>
class RemoteMatrixState : public hh::AbstractState<CommMatrixBlock<MatrixType>, CommMatrixBlock<MatrixType>> {
private:
    int id_;

public:
    RemoteMatrixState(int id) : id_(id) {}

    ~RemoteMatrixState() override = default;

    void execute(std::shared_ptr<CommMatrixBlock<MatrixType>> ptr) override {
        if (ptr->dest() != id_){
            this->push(ptr);
        }
    }
};

// Receive task, that receives data from the communicator based on the id of the task
template<class MatrixType>
class CommReceiveTask : public hh::AbstractTask<CommMatrixBlock<MatrixType>, void> {
private:
    size_t const
            numberBlocksToReceive_;

    // TODO: Determine way to receive special message that shows "done"
    size_t numberElemRcv_ = 0;
    std::shared_ptr<Communicator> communicator_;
    int id_;

public:

    explicit CommReceiveTask(int id, std::shared_ptr<Communicator> communicator, size_t numberBlocksToReceive) :
            hh::AbstractTask<CommMatrixBlock<MatrixType>, void>("Comm Receiver task", 1, true),
            numberBlocksToReceive_(numberBlocksToReceive), id_(id), communicator_(communicator) {};

    ~CommReceiveTask() override = default;

    void execute([[maybe_unused]]std::shared_ptr<void> ptr) override {
        using namespace std::chrono_literals;
        for (size_t rcv = 0; rcv < numberBlocksToReceive_; ++rcv) {

            while (!communicator_->hasMessage(this->id_)) {
                std::this_thread::sleep_for(50ms);
            }


            std::istringstream iss = communicator_->receiveMessage(this->id_);
            numberElemRcv_++;

//            std::cout << id_ << " Received data: " << rcv << " out of " << numberBlocksToReceive_ << std::endl;

            this->addResult(CommMatrixBlock<MatrixType>::deserialize(iss));
        }
    }

    [[nodiscard]] std::string extraPrintingInformation() const override {
        std::ostringstream oss;
        oss << "Got " << numberElemRcv_ << " piece(s) of data\n";
        return oss.str();
    }

};

// Send task to send data to a destination id to the communicator
template<class MatrixType>
class CommSendTask : public hh::AbstractTask<void, CommMatrixBlock<MatrixType>> {
    int id_;
    std::shared_ptr<Communicator> communicator_;
public:
    explicit CommSendTask(int id, std::shared_ptr<Communicator> communicator) :
            hh::AbstractTask<void, CommMatrixBlock<MatrixType>>("Comm Sender task"), id_(id),
            communicator_(communicator) {}
    ~CommSendTask() override = default;

    void execute(std::shared_ptr<CommMatrixBlock<MatrixType>> block) override {
        std::ostringstream oss;

        block->serialize(oss);

        assert(this->id_ == block->src());

        communicator_->sendMessage(oss, this->id_, block->dest());
    }

};

// Task to print the final result
template<class MatrixType>
class PrintBlockTask : public hh::AbstractTask<void, CommMatrixBlock<MatrixType>> {
    int id_;
public:
    PrintBlockTask(int id) : hh::AbstractTask<void, CommMatrixBlock<MatrixType>>("PrintTask"), id_(id) {}
    void execute([[maybe_unused]]std::shared_ptr<CommMatrixBlock<MatrixType>> ptr) override {
        std::cout << this->id_ << " print" << std::endl;
        std::cout << *ptr << std::endl;
    }
};

// Task that adds a scalar to each element in the matrix block
template<class MatrixType>
class AddTask : public hh::AbstractTask<CommMatrixBlock<MatrixType>, CommMatrixBlock<MatrixType>> {
    MatrixType const value_;
public:
    AddTask(size_t numberThreads, MatrixType const value) :
            hh::AbstractTask<CommMatrixBlock<MatrixType>, CommMatrixBlock<MatrixType>>("AddTask", numberThreads),
            value_(value) {}

    void execute(std::shared_ptr<CommMatrixBlock<MatrixType>> ptr) override {
//        std::cout << "Adding " << *ptr << std::endl;
        std::transform(ptr->data().begin(), ptr->data().end(), ptr->data().begin(),
                       [this](MatrixType &val) { return val + this->value_; });
        this->addResult(ptr);
    }

    std::shared_ptr<hh::AbstractTask<CommMatrixBlock<MatrixType>, CommMatrixBlock<MatrixType>>> copy() override {
        return std::make_shared<AddTask<MatrixType>>(this->numberThreads(), this->value_);
    }
};

// Routine that decomposes the data to setup source and destinations for each data
template<class MatrixType>
std::vector<std::shared_ptr<CommMatrixBlock<MatrixType>>> decomposeData(int numRows, int numCols, int numNodes, int height, int width) {
    std::vector<std::shared_ptr<CommMatrixBlock<MatrixType>>> retData;

    int destNode = 0;
    int srcNode = numNodes-1;

    for (int r = 0; r < numRows; ++r) {
        for (int c = 0; c < numCols; ++c) {

            auto block = std::make_shared<CommMatrixBlock<MatrixType>>(srcNode, destNode, r, c, height, width);
            retData.push_back(block);

            srcNode--;
            if (srcNode < 0) {
                srcNode = numNodes-1;
            }

            destNode++;

            if (destNode == numNodes) {
                destNode = 0;
            }
        }
    }

    return retData;
}

// Creates the hedgehog graphs with send/receive communication across multiple graphs
template<class MatrixType>
std::shared_ptr<hh::Graph<void, CommMatrixBlock<MatrixType>>> createGraph(int id, std::shared_ptr<Communicator> communicator, size_t numDataReceived) {
    auto graph = std::make_shared<hh::Graph<void, CommMatrixBlock<MatrixType>>>("NodeGraph");

    auto localState = std::make_shared<LocalMatrixState<MatrixType>>(id);
    auto remoteState = std::make_shared<RemoteMatrixState<MatrixType>>(id);

    auto localStateManager = std::make_shared<hh::StateManager<CommMatrixBlock<MatrixType>, CommMatrixBlock<MatrixType>>>(localState);
    auto remoteStateManager = std::make_shared<hh::StateManager<CommMatrixBlock<MatrixType>, CommMatrixBlock<MatrixType>>>(remoteState);

    auto accumulateTask = std::make_shared<AddTask<MatrixType>>(1, 42);
    auto printTask = std::make_shared<PrintBlockTask<MatrixType>>(id);

    auto receiverTask = std::make_shared<CommReceiveTask<MatrixType>>(id, communicator, numDataReceived);
    auto sendTask = std::make_shared<CommSendTask<MatrixType>>(id, communicator);

    graph->input(localStateManager);
    graph->input(remoteStateManager);

    graph->addEdge(localStateManager, accumulateTask);
    graph->addEdge(remoteStateManager, sendTask);
    graph->addEdge(receiverTask, accumulateTask);

    graph->addEdge(accumulateTask, printTask);

    return graph;
}

int main(int argc, char **argv) {
    using MatrixType = float;

    int numNodes = 5;
    int gridWidth = 5;
    int gridHeight = 5;
    int blockWidth = 2;
    int blockHeight = 2;

    auto communicator = std::make_shared<Communicator>();
    communicator->initComm(numNodes);

    std::vector<std::shared_ptr<hh::Graph<void, CommMatrixBlock<MatrixType>>>> graphs(numNodes);

    // Decompose data to have source and destinations
    auto dataVector = decomposeData<MatrixType>(gridHeight, gridWidth, numNodes, blockHeight, blockWidth);

    // Calculate how many data would be received for each node (used to terminate)
    std::vector<int> numDataReceived(numNodes);
    for (auto data : dataVector) {
        if (data->src() != data->dest()) {
            numDataReceived[data->dest()]++;
        }
    }

    // Create the hedgehog graphs
    for (int nodeId = 0; nodeId < numNodes; ++nodeId) {
        graphs[nodeId] = createGraph<MatrixType>(nodeId, communicator, numDataReceived[nodeId]);
    }

    // Execute the hedgehog graphs
    for (auto graph : graphs) {
        graph->executeGraph();
    }

    // Push data into the hedgehog graphs (index into graphs vector represents node, so only send to srcNode)
    for (auto data : dataVector) {
        int srcNode = data->src();
        graphs[srcNode]->pushData(data);
    }

    // Indicate each graph is done receiving data from main
    for (auto graph : graphs) {
        graph->finishPushingData();
    }

    // Wait for graphs to finish executing
    for (auto graph : graphs) {
        graph->waitForTermination();
    }

    std::cout << "Finished" << std::endl;

    return 0;
}
