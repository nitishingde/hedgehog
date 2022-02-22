//
// Created by ngs on 2/16/22.
//

#include <stdlib.h>

#include "comm.h"
#include <chrono>
#include <thread>

class CommData: public comm::VarLabel {
public:
    int32_t data;

    explicit CommData(std::string name, int32_t data) : comm::VarLabel(std::move(name)), data(data) {}
    std::ostream &serialize(std::ostream &ostream) const override {
        comm::VarLabel::serialize(ostream);
        ostream << data;
        return ostream;
    }

    static CommData deserialize(std::istringstream istream) {
        std::string name;
        int32_t data;
        istream >> name >> data;
        return CommData(name, data);
    };
};

int main(int argc, char* argv[]) {
    comm::MPI_GlobalLockGuard globalLockGuard(&argc, &argv);

    auto rank = comm::getMpiRank();
    auto numNodes = comm::getMpiNumNodes();

    int32_t srcId = (numNodes+rank-1)%numNodes;
    int32_t destId = (rank+1)%numNodes;

    auto send = CommData("rank" + std::to_string(rank), rank);
    auto recvVarName = "rank" + std::to_string(srcId);

    auto oss = std::ostringstream();
    send.serialize(oss);
    comm::Communicator::sendMessage(oss, destId);
    while(!comm::Communicator::hasMessage(recvVarName, srcId)) {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(100ms);
    }
    auto recv = CommData::deserialize(comm::Communicator::recvMessage(recvVarName, srcId));
    printf("[Process %d] data = %d\n", rank, recv.data);

    return EXIT_SUCCESS;
}
