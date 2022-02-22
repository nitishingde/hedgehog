//
// Created by ngs on 2/22/22.
//

#ifndef HEDGEHOG_COMM_H
#define HEDGEHOG_COMM_H


#include <sstream>

/**
 * Assumptions:
 * VarLabels are variables having global scope over the cluster
 * All VarLabels need to be registered in the beginning. Cannot create new VarLabels dynamically (if you do, need to call sync)
 *
 */

namespace comm {
    /**
     * TODO: All thread safe I think, need to confirm
     * @return
     */
    bool isMpiRootPid();
    int getMpiRank();
    int getMpiNumNodes();

    /**
     * RAII
     * Works similar to std::lock_guard
     * Create this object only once.
     * It's lifecycle usually should be the entirety of the process execution.
     * Preferably make a MPI_GlobalLockGuard stack object at the beginning of the main function
     */
    class MPI_GlobalLockGuard {
    public:
        MPI_GlobalLockGuard(int32_t *argc, char ***argv);
        ~MPI_GlobalLockGuard();
    };

    /**
     * An abstract class for serialising/deserialising objects to send using comm::Communicator
     * Do we need it right now? Not really.
     * Future purpose: A way to declare global variables (i.e. scoped over the cluster)
     */
    class VarLabel {
    public:
        std::string name_;
        explicit VarLabel(std::string name);
        ~VarLabel() = default;
        virtual const std::ostream& serialize(std::ostream &oss) const;
    };

    /**
     * A way to send and receive data over network of clusters
     * Thread safe routines
     */
    class Communicator {
    public:
        static void sendMessage(std::ostringstream &message, int32_t destId);
        static std::istringstream recvMessage(const std::string &varName, int32_t srcId);
        static bool hasMessage(const std::string &varName, int32_t srcId);
    };
}


#endif //HEDGEHOG_COMM_H
