////
//// Created by tjb3 on 6/9/19.
////
//

#ifndef HEDGEHOG_ABSTRACT_CUDA_TASK_H
#define HEDGEHOG_ABSTRACT_CUDA_TASK_H
#ifdef HH_USE_CUDA
#include <unordered_set>
#include <cuda_runtime.h>
#include "../task/abstract_task.h"

#ifndef checkCudaErrors
#define checkCudaErrors(err) if (!HLOG_ENABLED) {} \
        else __checkCudaErrors(err, __FILE__, __LINE__)

// These are the inline versions for all of the SDK helper functions
inline void __checkCudaErrors(cudaError_t err, [[maybe_unused]]const char *file, [[maybe_unused]]const int line) {
  if (cudaSuccess != err) {
    HLOG(0, "checkCudaErrors() API error = "
        << err
        << "\"" << cudaGetErrorString(err) << " \" from "
        << file << ":" << line);
    cudaDeviceReset();
    exit(42);
  }
}
#endif

template<class TaskOutput, class ... TaskInputs>
class AbstractCUDATask : public AbstractTask<TaskOutput, TaskInputs...> {
 private:
  bool enablePeerAccess_;
  std::unordered_set<int> peerDeviceIds_;
  cudaStream_t stream_;

 public:
  AbstractCUDATask()
      : AbstractTask<TaskOutput, TaskInputs...>("CudaTask", 1, false),
        enablePeerAccess_(true) {}

  AbstractCUDATask(std::string_view const &name)
      : AbstractTask<TaskOutput, TaskInputs...>(name, 1, false),
        enablePeerAccess_(true) {}

  AbstractCUDATask(std::string_view const &name, bool automaticStart, bool enablePeerAccess)
      : AbstractTask<TaskOutput, TaskInputs...>(name, 1, automaticStart),
        enablePeerAccess_(enablePeerAccess) {}

  void setDeviceId(int deviceId) {
    this->core()->deviceId(deviceId);
  }

  void initialize() final {
    int numGpus = 0;

    checkCudaErrors(cudaGetDeviceCount(&numGpus));

//        assert(cudaDeviceId_ < numGpus);

    checkCudaErrors(cudaSetDevice(this->core()->deviceId()));
    checkCudaErrors(cudaStreamCreate(&stream_));

    if (enablePeerAccess_) {
      for (int i = 0; i < numGpus; ++i) {
        if (i != this->deviceId()) {
          int canAccess = 0;
          checkCudaErrors(cudaDeviceCanAccessPeer(&canAccess, this->deviceId(), i));
          if (canAccess) {
            cudaDeviceEnablePeerAccess(i, 0);
            peerDeviceIds_.insert(i);
          }
        }
      }
    }

    this->initializeCuda();
  }

  void shutdown() final {
    this->shutdownCuda();
    checkCudaErrors(cudaStreamDestroy(stream_));
  }

  virtual void initializeCuda() {}
  virtual void shutdownCuda() {}

  bool enablePeerAccess() const {
    return enablePeerAccess_;
  }

  cudaStream_t stream() const {
    return stream_;
  }

  bool hasPeerAccess(int peerDeviceId) {
    return peerDeviceIds_.find(peerDeviceId) != peerDeviceIds_.end();
  }

};

#endif //HH_USE_CUDA

#endif //HEDGEHOG_ABSTRACT_CUDA_TASK_H
