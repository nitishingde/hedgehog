// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the
// software in any medium, provided that you keep intact this entire notice. You may improve, modify and create
// derivative works of the software or any portion of the software, and you may copy and distribute such modifications
// or works. Modified works should carry a notice stating that you changed the software and should note the date and
// nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the
// source of the software. NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND,
// EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR
// WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE
// CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS
// THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE. You
// are solely responsible for determining the appropriateness of using and distributing the software and you assume
// all risks associated with its use, including but not limited to the risks and costs of program errors, compliance
// with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of 
// operation. This software is not intended to be used in any situation where a failure could cause risk of injury or
// damage to property. The software developed by NIST employees is not subject to copyright protection within the
// United States.



#ifndef HEDGEHOG_CORE_NODE_H
#define HEDGEHOG_CORE_NODE_H

#include <iostream>
#include <sstream>
#include <memory>
#include <map>
#include <set>
#include <chrono>
#include <cmath>
#include <vector>

#include "../../api/printer/abstract_printer.h"
#include "../../behavior/node.h"
#include "../../tools/logger.h"

/// @brief Hedgehog core namespace
namespace hh::core {

/// @brief Hedgehog node's type
enum struct NodeType {Graph, Task, StateManager, Sink, Source, ExecutionPipeline, Switch};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
/// @brief CoreSlot forward declaration
class CoreSlot;
#endif //DOXYGEN_SHOULD_SKIP_THIS

/// @brief Main Hedgehog core abstraction
class CoreNode {
 private:
  bool
      isInside_ = false, ///< True if the node is inside hedgehog graph, else False
      hasBeenRegistered_ = false, ///< True if the node has been registered into hedgehog graph, else False
      isCudaRelated_ = false, ///< True if the node is related with CUDA, else False
      isInCluster_ = false, ///< True if the node is in cluster, else False
      isActive_ = false; ///< True if the node is active, else False

  int threadId_ = 0; ///< Thread id, used to debug only

  size_t
      numberThreads_ = 1, ///< Number of threads associated to the node
      numberReceivedElements_ = 0; ///< Number of element received by the node

  std::string_view name_ = ""; ///< Node name

  NodeType const type_; ///< Node type

  CoreNode
      *belongingNode_ = nullptr, ///< Pointer to the belonging node, hedgehog graph, does not store memory, just for reference
      *coreClusterNode_ = nullptr; ///< Pointer to the main cluster node, does not store memory, just for reference

  std::shared_ptr<std::multimap<CoreNode *, std::shared_ptr<CoreNode>>>
      insideNodes_ = nullptr; ///< Map of inside nodes [Main Cluster Node -> Node]

  std::chrono::nanoseconds
      creationDuration_ = std::chrono::nanoseconds::zero(), ///< Node creation duration
      executionDuration_ = std::chrono::nanoseconds::zero(), ///< Node execution duration
      perElementExecutionDuration_ = std::chrono::nanoseconds::zero(), ///< Node per element duration
      waitDuration_ = std::chrono::nanoseconds::zero(), ///< Node wait duration
      memoryWaitDuration_ = std::chrono::nanoseconds::zero(); ///< Node memory wait duration

  std::chrono::time_point<std::chrono::system_clock> const
      creationTimeStamp_ = std::chrono::system_clock::now(); ///< Node creation timestamp

  std::chrono::time_point<std::chrono::system_clock>
      startExecutionTimeStamp_ = std::chrono::system_clock::now(); ///< Node begin execution timestamp

 public:
  /// @brief Deleted default constructor
  CoreNode() = delete;

  /// @brief Core node only constructor
  /// @param name Node name
  /// @param type Node type
  /// @param numberThreads Node number of threads
  CoreNode(std::string_view const &name, NodeType const type, size_t numberThreads)
      : isActive_(false), name_(name), type_(type), coreClusterNode_(this) {
    numberThreads_ = numberThreads == 0 ? 1 : numberThreads;
    HLOG_SELF(0,
              "Creating CoreNode with type: " << (int) type << ", name: " << name << " and number of Threads: "
                                              << this->numberThreads_)
    this->insideNodes_ = std::make_shared<std::multimap<CoreNode *, std::shared_ptr<CoreNode>>>();
  }

  /// @brief Default virtual destructor
  virtual ~CoreNode() {
    HLOG_SELF(0, "Destructing CoreNode")
  }

  /// @brief Virtual constructor for copy
  /// @return A copy of the current node
  virtual std::shared_ptr<CoreNode> clone() = 0;  // Virtual constructor (copying)

  // Accessor

  /// @brief Unique Id accessor
  /// @return Unique Id
  [[nodiscard]] virtual std::string id() const {
    std::stringstream ss{};
    ss << "x" << this;
    return ss.str();
  }

  /// @brief Input node ids [nodeId, nodeIdCluster] accessor
  /// @return  Input node ids [nodeId, nodeIdCluster]
  [[nodiscard]] virtual std::vector<std::pair<std::string, std::string>> ids() const {
    return {{this->id(), this->coreClusterNode()->id()}};
  }

  /// @brief Node name accessor
  /// @return Node name
  [[nodiscard]] std::string_view const &name() const { return name_; }

  /// @brief Node type accessor
  /// @return Node type
  [[nodiscard]] NodeType type() const { return type_; }

  /// @brief Node inside property accessor
  /// @return Node inside property
  [[nodiscard]] bool isInside() const { return isInside_; }

  /// @brief Node registration property accessor
  /// @return Node registration property
  [[nodiscard]] bool hasBeenRegistered() const { return hasBeenRegistered_; }

  /// @brief Main cluster core node link to this node accessor
  /// @return Main cluster core node link to this node
  [[nodiscard]] CoreNode *coreClusterNode() const { return coreClusterNode_; }

  /// @brief Thread id accessor
  /// @return Thread id
  [[nodiscard]] int threadId() const { return threadId_; }

  /// @brief Number of threads associated accessor
  /// @return Number of threads associated
  [[nodiscard]] size_t numberThreads() const { return numberThreads_; }

  /// @brief Belonging node accessor
  /// @return Belonging node
  [[nodiscard]] CoreNode *belongingNode() const { return belongingNode_; }

  /// @brief Inside node accessor
  /// @return Inside node
  [[nodiscard]] std::shared_ptr<std::multimap<CoreNode *,
                                              std::shared_ptr<CoreNode>>> const &insideNodes() const {
    return insideNodes_;
  }

  /// @brief Inside nodes accessor
  /// @return Inside nodes
  [[nodiscard]] std::shared_ptr<std::multimap<CoreNode *, std::shared_ptr<CoreNode>>> &insideNodes() {
    return insideNodes_;
  }

  /// @brief Execution time accessor
  /// @return Execution time
  [[nodiscard]] std::chrono::nanoseconds const &executionTime() const { return executionDuration_; }

  /// @brief Total time per element during execute
  /// @return Total time per element during execute
  [[nodiscard]] std::chrono::nanoseconds const &perElementExecutionTime() const {
    return perElementExecutionDuration_;
  }

  /// @brief Wait time accessor
  /// @return Wait time
  [[nodiscard]] std::chrono::nanoseconds const &waitTime() const { return waitDuration_; }

  /// @brief Memory wait time accessor
  /// @return Memory wait time
  [[nodiscard]] std::chrono::nanoseconds const &memoryWaitTime() const {
    return memoryWaitDuration_; }

  /// @brief Execution time per element accessor
  /// @return Execution time per element
  [[nodiscard]] std::chrono::nanoseconds executionTimePerElement() const {
    if(this->numberReceivedElements() == 0){ std::chrono::nanoseconds::zero(); }
    return this->perElementExecutionTime() / this->numberReceivedElements();
  }

  /// @brief In cluster property accessor
  /// @return In cluster property
  [[nodiscard]] bool isInCluster() const { return this->isInCluster_; }

  /// @brief Is active property accessor
  /// @return Is active property
  [[nodiscard]] bool isActive() const { return isActive_; }

  /// @brief Is related to CUDA, used to have hedgehog green background on the dot file
  /// @return True if CUDA related, else False
  [[nodiscard]] bool isCudaRelated() const { return isCudaRelated_; }

  /// @brief Test if hedgehog memory manager is attached to the node, by default not
  /// @return True if there is hedgehog memory manager attached, else False
  [[nodiscard]] virtual bool hasMemoryManagerAttached() const {return false;}

  /// @brief Graph id accessor
  /// @return Graph id
  [[nodiscard]] virtual int graphId() { return this->belongingNode()->graphId(); }

  /// @brief Device id accessor
  /// @return Device id
  [[nodiscard]] virtual int deviceId() { return this->belongingNode()->deviceId(); }

  /// @brief Maximum execution time accessor
  /// @return Maximum execution time
  [[nodiscard]] virtual std::chrono::nanoseconds maxExecutionTime() const {
    return this->executionDuration_;
  }

  /// @brief Minimum execution time accessor
  /// @return Minimum execution time
  [[nodiscard]] virtual std::chrono::nanoseconds minExecutionTime() const {
    return this->executionDuration_;
  }

  /// @brief Maximum waiting time accessor
  /// @return Maximum waiting time
  [[nodiscard]] virtual std::chrono::nanoseconds maxWaitTime() const { return this->waitDuration_; }

  /// @brief Minimum waiting time accessor
  /// @return Minimum waiting time
  [[nodiscard]] virtual std::chrono::nanoseconds minWaitTime() const { return this->waitDuration_; }

  /// @brief Creation timestamp accessor
  /// @return Creation timestamp
  [[nodiscard]] std::chrono::time_point<std::chrono::system_clock> const &creationTimeStamp() const {
    return creationTimeStamp_;
  }

  /// @brief Execution start timestamp accessor
  /// @return Execution start timestamp
  [[nodiscard]] std::chrono::time_point<std::chrono::system_clock> const &startExecutionTimeStamp() const {
    return startExecutionTimeStamp_;
  }

  /// @brief Creation duration accessor
  /// @return Creation duration
  [[nodiscard]] std::chrono::nanoseconds const &creationDuration() const { return creationDuration_; }

  /// @brief Execution duration accessor
  /// @return Execution duration
  [[nodiscard]] std::chrono::nanoseconds const &executionDuration() const {
    return executionDuration_;
  }

  /// @brief Number elements accessor
  /// @return Number of received elements
  [[nodiscard]] size_t numberReceivedElements() const {return numberReceivedElements_; }

  /// @brief Compute and return the mean execution time for all tasks in the node cluster
  /// @return The mean execution time for all tasks in the node cluster
  [[nodiscard]] std::chrono::nanoseconds meanExecTimeCluster() const {
    auto ret = this->executionTime();
    if (this->isInCluster()) {
      std::chrono::nanoseconds sum = std::chrono::nanoseconds::zero();
      for (auto it = this->belongingNode()->insideNodes()->equal_range(this->coreClusterNode()).first;
           it != this->belongingNode()->insideNodes()->equal_range(this->coreClusterNode()).second; ++it) {
        sum += it->second->executionTime();
      }
      ret = sum / this->numberThreads();
    }
    return ret;
  }

  /// @brief Compute and return the mean wait time for all tasks in the node cluster
  /// @return The mean wait time for all tasks in the node cluster
  [[nodiscard]] std::chrono::nanoseconds meanWaitTimeCluster() const {
    auto ret = this->waitTime();
    if (this->isInCluster()) {
      std::chrono::nanoseconds sum = std::chrono::nanoseconds::zero();
      for (auto it = this->belongingNode()->insideNodes()->equal_range(this->coreClusterNode()).first;
           it != this->belongingNode()->insideNodes()->equal_range(this->coreClusterNode()).second; ++it) {
        sum += it->second->waitTime();
      }
      ret = sum / this->numberThreads();
    }
    return ret;
  }

  /// @brief Compute and return the mean memory wait time for all tasks in the node cluster
  /// @return The mean memory wait time for all tasks in the node cluster
  [[nodiscard]] std::chrono::nanoseconds meanMemoryWaitTimeCluster() const {
    auto ret = this->memoryWaitTime();
    if (this->isInCluster()) {
      std::chrono::nanoseconds sum = std::chrono::nanoseconds::zero();
      for (auto it = this->belongingNode()->insideNodes()->equal_range(this->coreClusterNode()).first;
           it != this->belongingNode()->insideNodes()->equal_range(this->coreClusterNode()).second; ++it) {
        sum += it->second->memoryWaitTime();
      }
      ret = sum / this->numberThreads();
    }
    return ret;
  }

  /// @brief Compute and return the mean execution time per received element for all tasks in the node cluster
  /// @return The mean execution time per received element for all tasks in the node cluster
  [[nodiscard]] std::chrono::nanoseconds meanExecTimePerElementCluster() const {
    auto ret = this->executionTimePerElement();
    if (this->isInCluster()) {
      std::chrono::nanoseconds sum = std::chrono::nanoseconds::zero();
      for (auto it = this->belongingNode()->insideNodes()->equal_range(this->coreClusterNode()).first;
           it != this->belongingNode()->insideNodes()->equal_range(this->coreClusterNode()).second; ++it) {
        sum += it->second->executionTimePerElement();
      }
      ret = sum / this->numberThreads();
    }
    return ret;
  }

  /// @brief Compute and return the mean number of Elements received per task for all tasks in the node cluster
  /// @return mean number of Elements received per task for all tasks in the node cluster
  [[nodiscard]] double meanNumberElementsReceivedCluster() const {
    double ret = this->numberReceivedElements();
    if (this->isInCluster()) {
      double sum = 0;
      for (auto it = this->belongingNode()->insideNodes()->equal_range(this->coreClusterNode()).first;
           it != this->belongingNode()->insideNodes()->equal_range(this->coreClusterNode()).second; ++it) {
        sum += it->second->numberReceivedElements();
      }
      ret = sum / this->numberThreads();
    }
    return ret;
  }

  /// @brief Compute and return the standard deviation execution time for all tasks in the node cluster
  /// @return The standard deviation execution time for all tasks in the node cluster
  [[nodiscard]] std::chrono::nanoseconds stdvExecTimeCluster() const {
    double ret = 0;
    if (this->isInCluster()) {
      auto mean = this->meanExecTimeCluster();
      for (auto it = this->belongingNode()->insideNodes()->equal_range(this->coreClusterNode()).first;
           it != this->belongingNode()->insideNodes()->equal_range(this->coreClusterNode()).second; ++it) {
        ret += std::pow(it->second->executionTime().count() - mean.count(), 2);
      }
      ret /= (this->numberThreads() - 1);
      ret = std::sqrt(ret);
    }
    return std::chrono::nanoseconds((int64_t)(ret));
  }

  /// @brief Compute and return the standard deviation wait time for all tasks in the node cluster
  /// @return The standard deviation wait time for all tasks in the node cluster
  [[nodiscard]] std::chrono::nanoseconds stdvWaitTimeCluster() const {
    double ret = 0;
    if (this->isInCluster()) {
      auto mean = this->meanWaitTimeCluster().count();//, meanSquare = mean * mean;
      for (auto it = this->belongingNode()->insideNodes()->equal_range(this->coreClusterNode()).first;
           it != this->belongingNode()->insideNodes()->equal_range(this->coreClusterNode()).second; ++it) {
        ret += std::pow(it->second->waitTime().count() - mean, 2) ;
      }
      ret /= (this->numberThreads() - 1);
      ret = std::sqrt(ret);
    }
    return std::chrono::nanoseconds((int64_t)(ret));
  }

  /// @brief Compute and return the standard deviation memory wait time for all tasks in the node cluster
  /// @return The standard deviation memory wait time for all tasks in the node cluster
  [[nodiscard]] std::chrono::nanoseconds stdvMemoryWaitTimeCluster() const {
    double ret = 0;
    if (this->isInCluster()) {
      auto mean = this->meanMemoryWaitTimeCluster().count();//, meanSquare = mean * mean;
      for (auto it = this->belongingNode()->insideNodes()->equal_range(this->coreClusterNode()).first;
           it != this->belongingNode()->insideNodes()->equal_range(this->coreClusterNode()).second; ++it) {
        ret += std::pow(it->second->memoryWaitTime().count() - mean, 2);
      }
      ret /= (this->numberThreads() - 1);
      ret = std::sqrt(ret);
    }
    return std::chrono::nanoseconds((int64_t)(ret));
  }

  /// @brief Compute and return the standard deviation execution per received element time for all tasks in the node
  /// cluster
  /// @return The standard deviation execution time per received element for all tasks in the node cluster
  [[nodiscard]] std::chrono::nanoseconds stdvExecPerElementTimeCluster() const {
    double ret = 0;
    if (this->isInCluster()) {
      auto
        mean = this->meanExecTimePerElementCluster().count();
      for (auto it = this->belongingNode()->insideNodes()->equal_range(this->coreClusterNode()).first;
           it != this->belongingNode()->insideNodes()->equal_range(this->coreClusterNode()).second; ++it) {
        ret += std::pow(it->second->executionTimePerElement().count() - mean, 2) ;
      }
      ret /= (this->numberThreads() - 1);
      ret = std::sqrt(ret);
    }
    return std::chrono::nanoseconds((int64_t)(ret));
  }

  /// @brief Compute and return the standard deviation number of Elements received per task for all tasks in the node
  /// cluster
  /// @return The standard deviation number of Elements received per task for all tasks in the node cluster
  [[nodiscard]] double stdvNumberElementsReceivedCluster() const {
    double ret = 0;
    if (this->isInCluster()) {
      auto mean = this->meanNumberElementsReceivedCluster();
      for (auto it = this->belongingNode()->insideNodes()->equal_range(this->coreClusterNode()).first;
           it != this->belongingNode()->insideNodes()->equal_range(this->coreClusterNode()).second; ++it) {
        ret += std::pow(it->second->numberReceivedElements() - mean, 2) ;
      }
      ret /= (this->numberThreads() - 1);
      ret = std::sqrt(ret);
    }
    return ret;
  }

  /// @brief Compute and return the min and max wait time for all tasks in the node cluster
  /// @return The min and max mean wait time for all tasks in the node cluster
  [[nodiscard]] std::pair<std::chrono::nanoseconds, std::chrono::nanoseconds> minmaxWaitTimeCluster() const {
    std::chrono::nanoseconds min = std::chrono::nanoseconds::max();
    std::chrono::nanoseconds max = std::chrono::nanoseconds::min();
    std::chrono::nanoseconds val = std::chrono::nanoseconds::zero();
    if (this->isInCluster()) {
      for (auto it = this->belongingNode()->insideNodes()->equal_range(this->coreClusterNode()).first;
           it != this->belongingNode()->insideNodes()->equal_range(this->coreClusterNode()).second; ++it) {
        val = it->second->waitTime();
        if (val < min) { min = val; }
        if (val > max) { max = val; }
      }
    } else {
      min = this->meanWaitTimeCluster();
      max = min;
    }
    return {min, max};
  }

  /// @brief Compute and return the min and max memory wait time for all tasks in the node cluster
  /// @return The min and max memory mean memory wait time for all tasks in the node cluster
  [[nodiscard]] std::pair<std::chrono::nanoseconds, std::chrono::nanoseconds> minmaxMemoryWaitTimeCluster() const {
    std::chrono::nanoseconds min = std::chrono::nanoseconds::max();
    std::chrono::nanoseconds max = std::chrono::nanoseconds::min();
    std::chrono::nanoseconds val = std::chrono::nanoseconds::zero();
    if (this->isInCluster()) {
      for (auto it = this->belongingNode()->insideNodes()->equal_range(this->coreClusterNode()).first;
           it != this->belongingNode()->insideNodes()->equal_range(this->coreClusterNode()).second; ++it) {
        val = it->second->memoryWaitTime();
        if (val < min) { min = val; }
        if (val > max) { max = val; }
      }
    } else {
      min = this->meanMemoryWaitTimeCluster();
      max = min;
    }
    return {min, max};
  }

  /// @brief Compute and return the min and max execution time for all tasks in the node cluster
  /// @return The min and max execution execution time for all tasks in the node cluster
  [[nodiscard]] std::pair<std::chrono::nanoseconds, std::chrono::nanoseconds> minmaxExecTimeCluster() const {
    std::chrono::nanoseconds min = std::chrono::nanoseconds::max();
    std::chrono::nanoseconds max = std::chrono::nanoseconds::min();
    std::chrono::nanoseconds val = std::chrono::nanoseconds::zero();
    if (this->isInCluster()) {
      for (auto it = this->belongingNode()->insideNodes()->equal_range(this->coreClusterNode()).first;
           it != this->belongingNode()->insideNodes()->equal_range(this->coreClusterNode()).second; ++it) {
        val = it->second->executionTime();
        if (val < min) { min = val; }
        if (val > max) { max = val; }
      }
    } else {
      min = this->meanExecTimeCluster();
      max = min;
    }

    return {min, max};
  }

  /// @brief Compute and return the min and max execution time per element for all tasks in the node cluster
  /// @return The min and max execution wait time for all tasks in the node cluster
  [[nodiscard]] std::pair<std::chrono::nanoseconds, std::chrono::nanoseconds> minmaxExecTimePerElementCluster() const {
    std::chrono::nanoseconds min = std::chrono::nanoseconds::max();
    std::chrono::nanoseconds max = std::chrono::nanoseconds::min();
    std::chrono::nanoseconds val = std::chrono::nanoseconds::zero();
    if (this->isInCluster()) {
      for (auto it = this->belongingNode()->insideNodes()->equal_range(this->coreClusterNode()).first;
           it != this->belongingNode()->insideNodes()->equal_range(this->coreClusterNode()).second; ++it) {
        val = it->second->executionTimePerElement();
        if (val < min) { min = val; }
        if (val > max) { max = val; }
      }
    } else {
      min = this->executionTimePerElement();
      max = min;
    }
    return {min, max};
  }


  /// @brief Compute and return the min and max number of Elements received per task for all tasks in the node cluster
  /// @return The min and max number of Elements received per task for all tasks in the node cluster
  [[nodiscard]] std::pair<size_t, size_t> minmaxNumberElementsReceivedCluster() const {
    size_t
        min = std::numeric_limits<size_t>::max(),
        max = 0,
        val = 0;
    if (this->isInCluster()) {
      for (auto it = this->belongingNode()->insideNodes()->equal_range(this->coreClusterNode()).first;
           it != this->belongingNode()->insideNodes()->equal_range(this->coreClusterNode()).second; ++it) {
        val = it->second->numberReceivedElements();
        if (val < min) { min = val; }
        if (val > max) { max = val; }
      }
    } else {
      min = this->executionTimePerElement().count();
      max = min;
    }
    return {min, max};
  }

  /// @brief Compute and return the number of active nodes in hedgehog cluster
  /// @return The number of active nodes in hedgehog cluster
  [[nodiscard]] size_t numberActiveThreadInCluster() const {
    size_t ret = 0;
    if (this->isInCluster()) {
      for (auto it = this->belongingNode()->insideNodes()->equal_range(this->coreClusterNode()).first;
           it != this->belongingNode()->insideNodes()->equal_range(this->coreClusterNode()).second; ++it) {
        ret += it->second->isActive() ? 1 : 0;
      }
    } else {
      ret = this->isActive() ? 1 : 0;
    }
    return ret;
  }

  /// @brief Extra printing information accessor
  /// @return Extra printing information
  [[nodiscard]] virtual std::string extraPrintingInformation() { return node()->extraPrintingInformation(); }

  // Setter
  /// @brief Execution timestamp setter
  /// @param startExecutionTimeStamp Execution timestamp to set
  void startExecutionTimeStamp(
      std::chrono::time_point<std::chrono::system_clock> const &startExecutionTimeStamp) {
    startExecutionTimeStamp_ = startExecutionTimeStamp;
  }

  /// @brief Device id setter
  /// @param deviceId Device id
  virtual void deviceId(int deviceId) { this->belongingNode()->deviceId(deviceId); }

  /// @brief Set the node as inside, (inside hedgehog graph)
  virtual void setInside() { this->isInside_ = true; }

  /// @brief Set the task as part of hedgehog cluster
  void setInCluster() { this->isInCluster_ = true; }

  /// @brief Set the thread id
  /// @param threadId Thread id to set
  void threadId(uint8_t threadId) { threadId_ = threadId; }

  /// @brief Set the main cluster node to associate to this node
  /// @param coreClusterNode Main cluster node to associate to this node
  void coreClusterNode(CoreNode *coreClusterNode) { coreClusterNode_ = coreClusterNode; }

  /// @brief Name node setter
  /// @param name Name node to set
  void name(std::string_view const &name) { name_ = name; }

  /// @brief Number of threads setter
  /// @param numberThreads Number of threads
  void numberThreads(size_t numberThreads) { numberThreads_ = numberThreads; }

  /// @brief Belonging node setter
  /// @param belongingNode Belonging node
  void belongingNode(CoreNode *belongingNode) { belongingNode_ = belongingNode; }

  /// @brief Has been registered property setter
  /// @param hasBeenRegistered Has been registered property
  void hasBeenRegistered(bool hasBeenRegistered) { hasBeenRegistered_ = hasBeenRegistered; }

  /// @brief Is active property setter
  /// @param isActive Is active property
  void isActive(bool isActive) { isActive_ = isActive; }

  /// @brief Is CUDA related property setter
  /// @param isCudaRelated CUDA related property to set
  void isCudaRelated(bool isCudaRelated) { isCudaRelated_ = isCudaRelated; }

  /// @brief Set the node as being inside another one
  /// @param isInside True if the node is inside another one, else False
  void isInside(bool isInside) { isInside_ = isInside; }

  /// @brief Creation duration setter
  /// @param creationDuration Creation duration to set
  void creationDuration(std::chrono::nanoseconds const &creationDuration) {
    creationDuration_ = creationDuration;
  }

  /// @brief Execution duration setter
  /// @param executionDuration Execution duration
  void executionDuration(std::chrono::nanoseconds const &executionDuration) {
    executionDuration_ = executionDuration;
  }

  /// @brief Add wait for memory duration to total duration
  /// @param memoryWait Duration to add to the memory duration
  void incrementWaitForMemoryDuration(std::chrono::nanoseconds const &memoryWait) {
    this->memoryWaitDuration_ += memoryWait;
  }

  /// @brief Increment the number of element received for the node
  void incrementNumberReceivedElements() { ++this->numberReceivedElements_; }

  // Virtual
  /// @brief Method defining what to do before the run
  virtual void preRun() {}

  /// @brief Run method, main execution
  virtual void run() {}

  /// @brief Method defining what to do after the run
  virtual void postRun() {}

  /// @brief Define how to create hedgehog cluster for the node, by default do nothing
  virtual void createCluster(std::shared_ptr<std::multimap<CoreNode *, std::shared_ptr<CoreNode>>> &) {};

  /// @brief Define what is done when the thread is joined
  virtual void joinThreads() {}

  /// @brief Duplicate all of the edges from this to its copy duplicateNode
  /// @param duplicateNode Node to connect
  /// @param correspondenceMap Correspondence  map from base node to copy
  virtual void duplicateEdge(
      CoreNode *duplicateNode, std::map<CoreNode *, std::shared_ptr<CoreNode>> &correspondenceMap) = 0;

  /// @brief User's node accessor
  /// @return User's node
  virtual behavior::Node *node() = 0;

  /// @brief Abstract visit method for printing mechanism
  /// @param printer Printer visitor to print the node
  virtual void visit(AbstractPrinter *printer) = 0;

  /// @brief Slots accessor for the node
  /// @return Slots from the node
  virtual std::set<CoreSlot *> getSlots() = 0;

  // Public method
  /// @brief Remove hedgehog node from the registered inside nodes
  /// @param coreNode Node to remove from the inside nodes
  void removeInsideNode(CoreNode *coreNode) {
    HLOG_SELF(0, "Remove inside node " << coreNode->id() << ")")
    this->insideNodes()->erase(coreNode);
  }


  /// @brief Copy inner structure from rhs nodes to this
  /// @param rhs Node to copy to this
  void copyInnerStructure(CoreNode *rhs) {
    this->isInside_ = rhs->isInside();
    this->belongingNode_ = rhs->belongingNode();
    this->hasBeenRegistered_ = rhs->hasBeenRegistered();
    this->isInCluster_ = rhs->isInCluster();
    this->isCudaRelated_ = rhs->isCudaRelated();
    this->numberThreads_ = rhs->numberThreads();
    this->coreClusterNode(rhs->coreClusterNode());
  }

 protected:
  /// @brief Add hedgehog node to the inside nodes
  /// @param coreNode Node to add to the inside nodes
  void addUniqueInsideNode(const std::shared_ptr<CoreNode> &coreNode) {
    HLOG_SELF(0, "Add InsideNode " << coreNode->name() << "(" << coreNode->id() << ")")
    if (insideNodes_->find(coreNode.get()) == insideNodes_->end()) {
      coreNode->belongingNode(this);
      coreNode->hasBeenRegistered(true);
      insideNodes_->insert({coreNode.get(), coreNode});
    }
  }

  /// @brief Increment wait duration
  /// @param wait Duration to add to the wait duration
  void incrementWaitDuration(std::chrono::nanoseconds const &wait) { this->waitDuration_ += wait; }

  /// @brief Increment execution duration
  /// @param exec Duration to add to the execution duration
  void incrementExecutionDuration(std::chrono::nanoseconds const &exec) {
    this->executionDuration_ += exec;
  }

  /// @brief Increment per element execution duration
  /// @param exec Duration to add to the per element execution duration
  void incrementPerElementExecutionDuration(std::chrono::nanoseconds const &exec) {
    this->perElementExecutionDuration_ += exec;
  }
};

}
#endif //HEDGEHOG_CORE_NODE_H
