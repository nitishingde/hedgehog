//
// Created by ngs on 2/16/22.
//
#include "comm.h"
#include <atomic>
#include <list>
#include <map>
#include <mpi.h>
#include <mutex>
#include <queue>
#include <thread>

namespace comm {
    /**
     * Handles all the sends/recvs logic
     * A daemon thread is spawned to run in the background
     * Each cycle/epoch it
     *  1. syncs the metadata for sends/recvs (blocking)
     *  2. then issues the Isends/Irecvs (non blocking)
     *  3. cleanup after sending/receiving messages (non blocking)
     *
     * Singleton class
     * No need to expose to application
     */
    class DataWarehouse {
    private:
        struct RecvData {
            std::string buffer;
            int32_t srcId;
            MPI_Request request;

            explicit RecvData(std::string buffer, int32_t srcId, MPI_Request request)
                    : buffer(std::move(buffer)), srcId(srcId), request(request) {}

            ~RecvData() = default;
            RecvData(RecvData &other) = delete;
            RecvData& operator=(RecvData &other) = delete;

            RecvData(RecvData &&other) noexcept {
                if (this == &other) return;
                this->buffer = std::move(other.buffer);
                this->srcId = other.srcId;
                this->request = other.request;
            }

            RecvData& operator=(RecvData &&other) noexcept {
                if (this == &other) return *this;
                this->buffer = std::move(other.buffer);
                this->srcId = other.srcId;
                this->request = other.request;

                return *this;
            }
        };

        struct SendData {
            std::ostringstream buffer;
            MPI_Request request;

            explicit SendData(std::ostringstream buffer, MPI_Request request)
                    : buffer(std::move(buffer)), request(request) {}

            ~SendData() = default;
            SendData(SendData &other) = delete;
            SendData& operator=(SendData &other) = delete;

            SendData(SendData &&other) noexcept {
                if(this == &other) return;
                this->buffer = std::move(other.buffer);
                this->request = other.request;
            }

            SendData& operator=(SendData &&other) noexcept {
                if(this == &other) return *this;
                this->buffer = std::move(other.buffer);
                this->request = other.request;

                return *this;
            }
        };

        int32_t numNodes_;
        int32_t nodeId_;
        std::mutex mutex_{};
        std::thread daemonThread_;
        std::atomic_int32_t stopDaemon_ = false;

        // send related
        std::vector<std::queue<std::shared_ptr<SendData>>> sendQueues_;
        std::vector<uint32_t> sendBufferSize_;
        std::list<std::shared_ptr<SendData>> sendTasks_;

        // recv related
        MPI_Win recvMetadataWindow;
        std::vector<uint32_t> recvMetadata_;// buffer for recvMetadataWindow
        std::list<std::shared_ptr<RecvData>> recvTasks_;
        std::vector<std::map<std::string, std::istringstream>> varLabels_;

    private:
        DataWarehouse();
        void init();
        void daemon();
        void syncMetadata();
        void syncMessages();
        void processSendsAndRecvs();
    public:
        ~DataWarehouse();
        static DataWarehouse *getInstance();
        void startDaemon();
        void stopDaemon();
        void sendMessage(std::ostringstream &message, int destId);
        std::istringstream recvMessage(const std::string &varName, int srcId);
        bool hasMessage(const std::string &varName, int srcId);
    };
}

static comm::DataWarehouse *pDataWarehouse = nullptr;
comm::DataWarehouse* comm::DataWarehouse::getInstance() {
    if(pDataWarehouse == nullptr) {
        pDataWarehouse = new comm::DataWarehouse();
    }
    return pDataWarehouse;
}

comm::DataWarehouse::DataWarehouse() {
    numNodes_ = comm::getMpiNumNodes();
    nodeId_ = comm::getMpiNodeId();
    recvMetadataWindow = MPI_WIN_NULL;
    this->init();
}

void comm::DataWarehouse::init() {
    sendQueues_.resize(numNodes_);
    sendBufferSize_.resize(numNodes_, 0);

    varLabels_.resize(numNodes_);
    recvMetadata_.resize(numNodes_, 0);
    MPI_Win_create(
            recvMetadata_.data(),
            MPI_Aint(recvMetadata_.size() * sizeof(decltype(recvMetadata_)::value_type)),
            sizeof(decltype(recvMetadata_)::value_type),
            MPI_INFO_NULL,
            MPI_COMM_WORLD,
            &recvMetadataWindow
    );
}

void comm::DataWarehouse::daemon() {
    while(true) {
        int32_t exitVote = 0;
        MPI_Allreduce(&stopDaemon_, &exitVote, 1, MPI_INT32_T, MPI_SUM, MPI_COMM_WORLD);
        if(exitVote == numNodes_) {// all MPI processes agree to exit
            return;
        }
        /*SCOPED*/{
            std::lock_guard lc(this->mutex_);
            // send metadata
            syncMetadata();

            // send and receive messages
            syncMessages();

            // process sent and received messages
            processSendsAndRecvs();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void comm::DataWarehouse::startDaemon() {
    stopDaemon_ = false;
    daemonThread_ = std::thread(&comm::DataWarehouse::daemon, this);
}

void comm::DataWarehouse::stopDaemon() {
    stopDaemon_ = true;
    daemonThread_.join();
}

void comm::DataWarehouse::syncMetadata() {
    MPI_Win_fence(0, recvMetadataWindow);
    for(int32_t node = 0; node < numNodes_; ++node) {
        if(sendQueues_[node].empty()) continue;

        sendBufferSize_[node] = sendQueues_[node].front()->buffer.str().size();
        if(sendBufferSize_[node]) {//FIXME: unnecessary?
            MPI_Put(
                    &sendBufferSize_[node],
                    1,
                    MPI_UINT32_T,
                    node,
                    nodeId_,
                    1,
                    MPI_UINT32_T,
                    recvMetadataWindow
            );
        }
    }
    MPI_Win_fence(0, recvMetadataWindow);
}

void comm::DataWarehouse::syncMessages() {
    for(int32_t node = 0; node < numNodes_; ++node) {
        if(recvMetadata_[node]) {
            auto recvData = std::make_shared<RecvData>(
                    std::string(recvMetadata_[node], ' '),
                    node,
                    MPI_Request()
            );
            recvTasks_.emplace_back(recvData);

            MPI_Irecv(
                    recvData->buffer.data(),
                    (int)recvData->buffer.size(),
                    MPI_CHAR,
                    node,
                    nodeId_,
                    MPI_COMM_WORLD,
                    &recvData->request
            );
            recvMetadata_[node] = 0;
        }

        if(!sendQueues_[node].empty()) {
            auto sendData = sendQueues_[node].front();
            sendQueues_[node].pop();
            sendTasks_.emplace_back(sendData);
            MPI_Isend(
                    sendData->buffer.str().data(),
                    (int)sendData->buffer.str().size(),
                    MPI_CHAR,
                    node,
                    node,
                    MPI_COMM_WORLD,
                    &sendData->request
            );
            sendBufferSize_[node] = 0;
        }
    }
}

void comm::DataWarehouse::processSendsAndRecvs() {
    int flag;
    MPI_Status status;
    for(auto sendData = sendTasks_.begin(); sendData != sendTasks_.end();) {
        MPI_Test(&(*sendData)->request, &flag, &status);
        if(flag) {
            sendData = sendTasks_.erase(sendData);
        } else {
            sendData++;
        }
    }

    for(auto recvData = recvTasks_.begin(); recvData != recvTasks_.end();) {
        MPI_Test(&(*recvData)->request, &flag, &status);
        if(flag) {
            std::shared_ptr<RecvData> data = *recvData;
            std::istringstream iss(data->buffer);
            std::string name;
            iss >> name;
            iss.seekg(0);
            varLabels_[data->srcId].insert(std::make_pair(std::move(name), std::move(iss)));
            recvData = recvTasks_.erase(recvData);
        }
    }
}

comm::DataWarehouse::~DataWarehouse() {
    pDataWarehouse = nullptr;
}

void comm::DataWarehouse::sendMessage(std::ostringstream &message, int destId) {
    std::lock_guard lg(mutex_);
    sendQueues_[destId].emplace(std::make_shared<SendData>(
            std::move(message),
            MPI_Request()
    ));
}

std::istringstream comm::DataWarehouse::recvMessage(const std::string &varName, int srcId) {
    std::lock_guard lg(mutex_);
    return std::move(varLabels_[srcId].extract(varName).mapped());
}

bool comm::DataWarehouse::hasMessage(const std::string &varName, int srcId) {
    std::lock_guard lg(mutex_);
    return varLabels_[srcId].find(varName) != varLabels_[srcId].end();
}

static int32_t sIsMpiRootPid = -1;
bool comm::isMpiRootPid() {
    if (sIsMpiRootPid == -1) {
        int32_t flag = false;
        if (auto status = MPI_Initialized(&flag); status == MPI_SUCCESS and flag) {
            int32_t processId;
            MPI_Comm_rank(MPI_COMM_WORLD, &processId);
            sIsMpiRootPid = (processId == 0);
        }
    }
    return sIsMpiRootPid;
}

int comm::getMpiNodeId() {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    return rank;
}

int comm::getMpiNumNodes() {
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    return size;
}

comm::MPI_GlobalLockGuard::MPI_GlobalLockGuard(int32_t *argc, char **argv[]) {
    int32_t flag = false;
    if(auto status = MPI_Initialized(&flag); status != MPI_SUCCESS or flag == false) {
        //TODO: Do we need this?
//        int32_t provided;
//        if(MPI_Init_thread(argc, argv, MPI_THREAD_MULTIPLE, &provided) == MPI_SUCCESS) {
        if(MPI_Init(argc, argv) == MPI_SUCCESS) {
            if(isMpiRootPid()) printf("[MPI_GlobalLockGuard] MPI initialized\n");
            comm::DataWarehouse::getInstance()->startDaemon();
        }
    }
}

comm::MPI_GlobalLockGuard::~MPI_GlobalLockGuard() {
    int32_t flag = false;
    if(auto status = MPI_Initialized(&flag); status == MPI_SUCCESS and flag) {
        comm::DataWarehouse::getInstance()->stopDaemon();
        if(MPI_Finalize() == MPI_SUCCESS) {
            if(isMpiRootPid()) printf("[MPI_GlobalLockGuard] MPI exited\n");
            delete comm::DataWarehouse::getInstance();
        }
    }
}

comm::VarLabel::VarLabel(std::string name) : name_(std::move(name)) {}

const std::ostream& comm::VarLabel::serialize(std::ostream &oss) const {
    oss << name_ << " ";
    return oss;
}

void comm::Communicator::sendMessage(std::ostringstream &message, int32_t destId) {
    comm::DataWarehouse::getInstance()->sendMessage(message, destId);
}

std::istringstream comm::Communicator::recvMessage(const std::string &varName, int32_t srcId) {
    return comm::DataWarehouse::getInstance()->recvMessage(varName, srcId);
}

bool comm::Communicator::hasMessage(const std::string &varName, int32_t srcId) {
    return comm::DataWarehouse::getInstance()->hasMessage(varName, srcId);
}
