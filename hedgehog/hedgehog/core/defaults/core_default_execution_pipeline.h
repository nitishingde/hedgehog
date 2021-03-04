// NIST-developed software is provided by NIST as hedgehog public service. You may use, copy and distribute copies of the
// software in any medium, provided that you keep intact this entire notice. You may improve, modify and create
// derivative works of the software or any portion of the software, and you may copy and distribute such modifications
// or works. Modified works should carry hedgehog notice stating that you changed the software and should note the date and
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
// operation. This software is not intended to be used in any situation where hedgehog failure could cause risk of injury or
// damage to property. The software developed by NIST employees is not subject to copyright protection within the
// United States.


#ifndef HEDGEHOG_CORE_DEFAULT_EXECUTION_PIPELINE_H
#define HEDGEHOG_CORE_DEFAULT_EXECUTION_PIPELINE_H

#include "../node/execution_pipeline/core_execution_pipeline.h"
#include "../../behavior/switch_rule.h"

/// @brief Hedgehog core namespace
namespace hh::core {

#if defined (__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#endif //__clang//
/// @brief Middle class used to propose hedgehog default definition of CoreExecute::callExecute for DefaultExecutionPipeline
/// @tparam GraphInput Type of data to send to CoreExecute::callExecute
/// @tparam GraphOutput Graph's output type
/// @tparam GraphInputs Graph's inputs type
template<class GraphInput, class GraphOutput, class ...GraphInputs>
class CoreDefaultExecutionPipelineExecute : public virtual CoreExecutionPipeline<GraphOutput, GraphInputs...> {
 public:
  /// @brief CoreDefaultExecutionPipelineExecute constructor
  /// @param name Execution pipeline name
  /// @param executionPipeline User's execution pipeline instance
  /// @param coreBaseGraph Base graph's core
  /// @param numberGraphs Number of graphs i nthe execution pipeline
  /// @param deviceIds Device Ids corresponding of each copy of the graph
  /// @param automaticStart Automatic start property for the graph
  CoreDefaultExecutionPipelineExecute(std::string_view const &name,
                                      AbstractExecutionPipeline<GraphOutput, GraphInputs...> *executionPipeline,
                                      std::shared_ptr<CoreGraph<GraphOutput, GraphInputs...>> coreBaseGraph,
                                      size_t numberGraphs,
                                      std::vector<int> const &deviceIds,
                                      bool automaticStart = false
  ) : CoreExecutionPipeline<GraphOutput, GraphInputs...>(name,
                                                         executionPipeline,
                                                         coreBaseGraph,
                                                         numberGraphs,
                                                         deviceIds,
                                                         automaticStart) {}

  /// @brief Definition of CoreExecute::callExecute for CoreDefaultExecutionPipeline
  /// @details Send the data to the graph if the data should be send
  /// @param data Data to be sent
  void callExecute([[maybe_unused]]std::shared_ptr<GraphInput> data) override {
    for (auto graph : this->epGraphs_) {
      if (this->callSendToGraph<GraphInput>(data, graph->graphId())) {
        std::static_pointer_cast<CoreGraphReceiver<GraphInput>>(graph)->receive(data);
        graph->wakeUp();
      }
    }
  }

 private:
  /// @brief Wrapper to the user-defined SwitchRule::sendToGraph
  /// @tparam Input Data type
  /// @param data Data to be send to the graph
  /// @param graphId Graph Id to test
  /// @return True if the data should be sent to the graph #graphId, else False
  template<class Input>
  bool callSendToGraph(std::shared_ptr<Input> &data, size_t const &graphId) {
    return static_cast<behavior::SwitchRule<Input> *>(this->executionPipeline())->sendToGraph(data, graphId);
  }

};
#if defined (__clang__)
#pragma clang diagnostic pop
#endif //__clang//

/// @brief Node core for the default execution pipeline
/// @tparam GraphOutput Graph's output type
/// @tparam GraphInputs Graph's inputs type
template<class GraphOutput, class ...GraphInputs>
class CoreDefaultExecutionPipeline : public CoreDefaultExecutionPipelineExecute<GraphInputs,
                                                                                GraphOutput,
                                                                                GraphInputs...> ... {
 public:
  using CoreDefaultExecutionPipelineExecute<GraphInputs, GraphOutput, GraphInputs...>::callExecute...;

  /// @brief Node core for the default execution pipeline constructor
  /// @param name Execution pipeline name
  /// @param executionPipeline User's execution pipeline instance
  /// @param coreBaseGraph Base graph's core
  /// @param numberGraphs Number of graphs i nthe execution pipeline
  /// @param deviceIds Device Ids corresponding of each copy of the graph
  /// @param automaticStart Automatic start property for the graph
  CoreDefaultExecutionPipeline(
      std::string_view const &name,
      AbstractExecutionPipeline<GraphOutput, GraphInputs...> *executionPipeline,
      std::shared_ptr<CoreGraph<GraphOutput, GraphInputs...>> coreBaseGraph,
      size_t numberGraphs,
      std::vector<int> const &deviceIds,
      bool automaticStart) :
      CoreNode(name, NodeType::ExecutionPipeline, 1),
      CoreNotifier(name, NodeType::ExecutionPipeline, 1),
      CoreQueueNotifier(name, NodeType::ExecutionPipeline, 1),
      CoreQueueSender<GraphOutput>(name, NodeType::ExecutionPipeline, 1),
      CoreSlot(name, NodeType::ExecutionPipeline, 1),
      CoreReceiver<GraphInputs>(name, NodeType::ExecutionPipeline, 1)...,
      CoreExecutionPipeline<GraphOutput, GraphInputs...>(
      name, executionPipeline, coreBaseGraph, numberGraphs, deviceIds, automaticStart),
      CoreDefaultExecutionPipelineExecute<GraphInputs, GraphOutput, GraphInputs...>(
      name, executionPipeline, coreBaseGraph, numberGraphs, deviceIds, automaticStart)
  ... {}

  /// @brief Constructor used to clone hedgehog CoreDefaultExecutionPipeline
  /// @param rhs CoreDefaultExecutionPipeline to duplicate
  /// @param baseGraph Core graph to set in the different copies of CoreDefaultExecutionPipeline
  CoreDefaultExecutionPipeline(CoreDefaultExecutionPipeline<GraphOutput, GraphInputs...> const &rhs,
                               std::shared_ptr<CoreGraph<GraphOutput, GraphInputs...>> baseGraph) :
      CoreNode(rhs.name(), NodeType::ExecutionPipeline, 1),
      CoreNotifier(rhs.name(), NodeType::ExecutionPipeline, 1),
      CoreQueueNotifier(rhs.name(), NodeType::ExecutionPipeline, 1),
      CoreQueueSender<GraphOutput>(rhs.name(), NodeType::ExecutionPipeline, 1),
      CoreSlot(rhs.name(), NodeType::ExecutionPipeline, 1),
      CoreReceiver<GraphInputs>(rhs.name(), NodeType::ExecutionPipeline, 1)...,
      CoreExecutionPipeline<GraphOutput, GraphInputs...>(
          rhs.name(),
          rhs.executionPipeline(),
          baseGraph,
          rhs.numberGraphs(),
          rhs.deviceIds(),
          rhs.automaticStart()),
      CoreDefaultExecutionPipelineExecute<GraphInputs, GraphOutput, GraphInputs...>(
          rhs.name(),
          rhs.executionPipeline(),
          baseGraph,
          rhs.numberGraphs(),
          rhs.deviceIds(),
          rhs.automaticStart())...
          {  }

  /// @brief Default destructor
  virtual ~CoreDefaultExecutionPipeline() = default;

  /// @brief CoreDefaultExecutionPipeline clone used if it is embedded in another CoreDefaultExecutionPipeline
  /// @return Clone of CoreDefaultExecutionPipeline (this)
  std::shared_ptr<CoreNode> clone() override {
    return std::make_shared<CoreDefaultExecutionPipeline<GraphOutput, GraphInputs...>>(
        *this,
        std::dynamic_pointer_cast<CoreGraph<GraphOutput, GraphInputs...>>(
            this->baseCoreGraph()->clone()
        )
    );
  }

  /// @brief Define how the CoreDefaultExecutionPipeline wait for data
  /// @return True if CoreDefaultExecutionPipeline can terminate, else False
  bool waitForNotification() override {
    //Lock the slot
    std::unique_lock<std::mutex> lock(*(this->slotMutex()));
    HLOG_SELF(2, "Wait for notification")
    // Wait on data or termination
    this->notifyConditionVariable()->wait(lock,
                                          [this]() {
                                            bool receiversEmpty = this->receiversEmpty();
                                            bool callCanTerminate = this->callCanTerminate(false);
                                            HLOG_SELF(2,
                                                      "Check for notification: " << std::boolalpha
                                                                                 << (bool) (!receiversEmpty) << "||"
                                                                                 << std::boolalpha
                                                                                 << (bool) callCanTerminate)
                                            return !receiversEmpty || callCanTerminate;
                                          });
    HLOG_SELF(2, "Notification received")
    return this->callCanTerminate(false);
  }

  /// @brief Post Execute loop for CoreDefaultExecutionPipeline
  void postRun() override {
    this->isActive(false);
#ifndef HH_DISABLE_NVTX_PROFILE
      this->nvtxProfiler()->startRangeShuttingDown();
#endif
    // Disconnect the CoreDefaultExecutionPipeline of all of its inside graph
    for (std::shared_ptr<CoreGraph<GraphOutput, GraphInputs...>> graph : this->epGraphs_) {
      (removeSwitchReceiver<GraphInputs>(graph.get()), ...);
    }
    // Notify all inside graphs
    this->coreSwitch_->notifyAllTerminated();

    // Wait for them to terminate before own termination
    for (std::shared_ptr<CoreGraph<GraphOutput, GraphInputs...>> graph : this->epGraphs_) {
      graph->waitForTermination();
    }
#ifndef HH_DISABLE_NVTX_PROFILE
      this->nvtxProfiler()->endRangeShuttingDown();
#endif
  }

 private:
  /// @brief Remove hedgehog specific Receiver of all input inside graph
  /// @tparam GraphInput Graph's input type to delete
  /// @param coreGraphReceiver Receiver to delete
  template<class GraphInput>
  void removeSwitchReceiver(CoreGraphReceiver<GraphInput> *coreGraphReceiver) {
    for (auto r : coreGraphReceiver->receivers()) {
      static_cast<CoreQueueSender<GraphInput> *>(this->coreSwitch_.get())->removeReceiver(r);
    }
    coreGraphReceiver->removeSender(static_cast<CoreQueueSender<GraphInput> *>(this->coreSwitch_.get()));
  }
};
}
#endif //HEDGEHOG_CORE_DEFAULT_EXECUTION_PIPELINE_H
