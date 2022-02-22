#include "hedgehog/hedgehog.h"
#include <iostream>
#include "comm.h"

// A block of memory, contains information about the source and destination for the data (based on decomposition in main)
template<class MatrixType>
class CommMatrixBlock: public comm::VarLabel {
    static_assert(std::is_arithmetic_v<MatrixType>, "The template parameter MatrixType should be an arithmetic value.");
    int const
            src_,
            dest_;

    size_t const
            indexRow_ = 0,
            indexCol_ = 0,
            height_ = 0,
            width_ = 0;

    std::unique_ptr<std::vector<MatrixType>>
            data_;

public:
    CommMatrixBlock(std::string name,
                    int const src,
                    int const dest,
                    size_t const indexRow,
                    size_t const indexCol,
                    size_t const height,
                    size_t const width)
            : comm::VarLabel(name), src_(src), dest_(dest), indexRow_(indexRow), indexCol_(indexCol), height_(height), width_(width) {
        data_ = std::make_unique<std::vector<MatrixType>>(height_ * width_, comm::getMpiNodeId());
    }

    CommMatrixBlock(CommMatrixBlock<MatrixType> &&toMove) noexcept:
            src_(toMove.src_), dest_(toMove.dest_),
            indexRow_(toMove.indexRow_), indexCol_(toMove.indexRow_),
            height_(toMove.height_), width_(toMove.width_) {
        data_.swap(toMove.data_);
        this->name_ = std::move(toMove.name_);
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
        comm::VarLabel::serialize(ostream);
        ostream << src_ << " " << dest_ << " " << indexRow_ << " " << indexCol_ << " " << height_ << " " << width_;
        std::for_each(data().begin(), data().end(), [&ostream](MatrixType const &data) { ostream << " " << data; });
        // Use null terminated string to aid with defining length of the data
        ostream << '\0';
        return ostream;
    }

    static std::shared_ptr<CommMatrixBlock<MatrixType>> deserialize(std::istream &istream) {
        std::string name;
        int src;
        int dst;
        size_t
                indexRow,
                indexCol,
                height,
                width;

        istream >> name >> src >> dst >> indexRow >> indexCol >> height >> width;

        auto output = std::make_shared<CommMatrixBlock<MatrixType>>(name, src, dst, indexRow, indexCol, height, width);

        for (auto &data: output->data()) { istream >> data; }
        return output;
    }
};

// State for data that is 'local' to a graph (destination == id)
template<class MatrixType>
class LocalMatrixState : public hh::AbstractState<CommMatrixBlock<MatrixType>, CommMatrixBlock<MatrixType>> {
private:
    int nodeId_;

public:
    LocalMatrixState() : nodeId_(comm::getMpiNodeId()) {}

    ~LocalMatrixState() override = default;

    void execute(std::shared_ptr<CommMatrixBlock<MatrixType>> ptr) override {
        if (ptr->dest() == nodeId_){
            this->push(ptr);
        }
    }
};

// State for data that is 'remote' to a graph (destination != id)
template<class MatrixType>
class RemoteMatrixState : public hh::AbstractState<CommMatrixBlock<MatrixType>, CommMatrixBlock<MatrixType>> {
private:
    int nodeId_;

public:
    RemoteMatrixState() : nodeId_(comm::getMpiNodeId()) {}

    ~RemoteMatrixState() override = default;

    void execute(std::shared_ptr<CommMatrixBlock<MatrixType>> ptr) override {
        if (ptr->dest() != nodeId_){
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
    int srcId_;

public:

    explicit CommReceiveTask(int srcId, size_t numberBlocksToReceive) :
            hh::AbstractTask<CommMatrixBlock<MatrixType>, void>("Comm Receiver task", 1, true),
            numberBlocksToReceive_(numberBlocksToReceive), srcId_(srcId) {};

    ~CommReceiveTask() override = default;

    void execute([[maybe_unused]]std::shared_ptr<void> ptr) override {
        using namespace std::chrono_literals;
        for (size_t rcv = 0; rcv < numberBlocksToReceive_; ++rcv) {
            auto varName = "block"+std::to_string(rcv)+std::to_string(srcId_);
            while (!comm::Communicator::hasMessage(varName, this->srcId_)) {
                std::this_thread::sleep_for(50ms);
            }
            std::istringstream iss = comm::Communicator::recvMessage(varName, this->srcId_);
            numberElemRcv_++;
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
public:
    explicit CommSendTask() :
            hh::AbstractTask<void, CommMatrixBlock<MatrixType>>("Comm Sender task") {}
    ~CommSendTask() override = default;

    void execute(std::shared_ptr<CommMatrixBlock<MatrixType>> block) override {
        std::ostringstream oss;
        block->serialize(oss);
        comm::Communicator::sendMessage(oss, block->dest());
    }
};

// Task to print the final result
template<class MatrixType>
class PrintBlockTask : public hh::AbstractTask<void, CommMatrixBlock<MatrixType>> {
    int nodeId_;
public:
    PrintBlockTask() : hh::AbstractTask<void, CommMatrixBlock<MatrixType>>("PrintTask"), nodeId_(comm::getMpiNodeId()) {}
    void execute([[maybe_unused]]std::shared_ptr<CommMatrixBlock<MatrixType>> ptr) override {
        std::cout << this->nodeId_ << " print" << std::endl;
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
std::vector<std::shared_ptr<CommMatrixBlock<MatrixType>>> decomposeData(int numRows, int numCols, int height, int width) {
    std::vector<std::shared_ptr<CommMatrixBlock<MatrixType>>> retData;

    int srcNode = comm::getMpiNodeId();
    int destNode = numCols-srcNode-1;
    for (int r = 0, c = srcNode; r < numRows; ++r) {
        auto varName = "block"+std::to_string(r)+std::to_string(c);
        auto block = std::make_shared<CommMatrixBlock<MatrixType>>(varName, srcNode, destNode, r, c, height, width);
        retData.push_back(block);
    }

    return retData;
}

// Creates the hedgehog graphs with send/receive communication across multiple graphs
template<class MatrixType>
std::shared_ptr<hh::Graph<void, CommMatrixBlock<MatrixType>>> createGraph(size_t numDataReceived) {
    int rank = comm::getMpiNodeId();
    int numNodes = comm::getMpiNumNodes();
    int srcNode = numNodes-rank-1;

    auto graph = std::make_shared<hh::Graph<void, CommMatrixBlock<MatrixType>>>("NodeGraph");

    auto localState = std::make_shared<LocalMatrixState<MatrixType>>();
    auto remoteState = std::make_shared<RemoteMatrixState<MatrixType>>();

    auto localStateManager = std::make_shared<hh::StateManager<CommMatrixBlock<MatrixType>, CommMatrixBlock<MatrixType>>>(localState);
    auto remoteStateManager = std::make_shared<hh::StateManager<CommMatrixBlock<MatrixType>, CommMatrixBlock<MatrixType>>>(remoteState);

    auto accumulateTask = std::make_shared<AddTask<MatrixType>>(1, 42);
    auto printTask = std::make_shared<PrintBlockTask<MatrixType>>();

    auto receiverTask = std::make_shared<CommReceiveTask<MatrixType>>(srcNode, numDataReceived);
    auto sendTask = std::make_shared<CommSendTask<MatrixType>>();

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
    comm::MPI_GlobalLockGuard mpiGlobalLockGuard(&argc, &argv);

    int gridWidth = comm::getMpiNumNodes();
    int gridHeight = 5;
    int blockWidth = 2;
    int blockHeight = 2;

    std::shared_ptr<hh::Graph<void, CommMatrixBlock<MatrixType>>> graph;

    // Decompose data to have source and destinations
    auto dataVector = decomposeData<MatrixType>(gridHeight, gridWidth, blockHeight, blockWidth);

    // Calculate how many data would be received for each node (used to terminate)
    int numDataReceived = 0;
    for (auto data : dataVector) {
        if (data->src() != data->dest()) {
            numDataReceived++;
        }
    }

    // Create the hedgehog graphs
    graph = createGraph<MatrixType>(numDataReceived);

    // Execute the hedgehog graphs
    graph->executeGraph();

    // Push data into the hedgehog graphs (index into graphs vector represents node, so only send to srcNode)
    for (auto data : dataVector) {
        graph->pushData(data);
    }

    // Indicate each graph is done receiving data from main
    graph->finishPushingData();

    // Wait for graphs to finish executing
    graph->waitForTermination();

    std::cout << "Finished" << std::endl;

    return 0;
}
