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



#ifndef HEDGEHOG_ABSTRACT_EXECUTION_PIPELINE_H
#define HEDGEHOG_ABSTRACT_EXECUTION_PIPELINE_H

#include <numeric>
#include "graph.h"
#include "../core/defaults/core_default_execution_pipeline.h"

/// @brief Hedgehog main namespace
namespace hh {

/// @brief Node interface made for duplicating hedgehog graph, for example in the case of multi-GPU computation.
/// @details The duplicated graph will run concurrently, and their inside nodes is also duplicated. Each graph will be
/// distinguished thanks to hedgehog device ID (deviceId), from 0, the original to n the number of duplications minus one.
///
/// Also, it is used as tool to enable multi-GPU computation, each graph will be bounded to hedgehog specific device id
/// accessible from the graph's inside nodes.
///
/// @attention The template parameters GraphOutput and GraphInputs have to exactly match to the managed graph.
/// @attention There is no default constructor, because hedgehog graph is mandatory.
///
/// The code below is from Hedgehog tutorial 5.
/// @code
/// template<class MatrixType>
/// class MultiGPUExecPipeline : public AbstractExecutionPipeline<
///     MatrixBlockData<MatrixType, 'p', Order::Column>,
///                              MatrixBlockData<MatrixType, 'hedgehog', Order::Column>,
///                              MatrixBlockData<MatrixType, 'b', Order::Column>> {
/// private:
///     size_t
///         numberGraphDuplication_ = 0;
///
///     public:
///     MultiGPUExecPipeline(std::shared_ptr<
///         Graph<MatrixBlockData<MatrixType, 'p', Order::Column>,
///         MatrixBlockData<MatrixType, 'hedgehog', Order::Column>,
///                          MatrixBlockData<MatrixType, 'b', Order::Column>>> const &graph,
///     size_t const &numberGraphDuplications,
///         std::vector<int> const &deviceIds) : AbstractExecutionPipeline<
///         MatrixBlockData<MatrixType, 'p', Order::Column>,
///     MatrixBlockData<MatrixType, 'hedgehog', Order::Column>,
///     MatrixBlockData<MatrixType, 'b', Order::Column>>("Cuda Execution Pipeline", graph, numberGraphDuplications,
///     deviceIds), numberGraphDuplication_(numberGraphDuplications) {}
///     virtual ~MultiGPUExecPipeline() = default;
///
///     bool sendToGraph(
///         [[maybe_unused]]std::shared_ptr<MatrixBlockData<MatrixType, 'hedgehog', Order::Column>> &data,
///     [[maybe_unused]]size_t const &graphId) override {
///     return data->colIdx() % numberGraphDuplication_ == graphId;
///     }
///
///     bool sendToGraph(
///         [[maybe_unused]]std::shared_ptr<MatrixBlockData<MatrixType, 'b', Order::Column>> &data,
///     [[maybe_unused]]size_t const &graphId) override {
///     return data->rowIdx() % numberGraphDuplication_ == graphId;
///     }
/// };
///
/// // Main
/// // GPU Graph
/// auto cudaMatrixMultiplication = std::make_shared<CUDAComputationGraph<MatrixType>>(n, m, p, blockSize,
/// numberThreadProduct);
///
/// // Execution Pipeline
/// auto executionPipeline =
///     std::make_shared<MultiGPUExecPipeline<MatrixType>>(cudaMatrixMultiplication, deviceIds.size(), deviceIds);
///
/// @endcode
///
/// A data send to hedgehog specialized AbstractExecutionPipeline, before reaching hedgehog graph will first go through
/// SwitchRule::sendToGraph that will send it to the graph's copy or not.
///
/// @par Virtual functions
/// SwitchRule::sendToGraph for each input data type <br>
///
/// @tparam GraphOutput Graph output data type
/// @tparam GraphInputs Graph inputs data type
template<class GraphOutput, class ...GraphInputs>
class AbstractExecutionPipeline
    : public behavior::MultiReceivers<GraphInputs...>,
      public behavior::Sender<GraphOutput>,
      public behavior::SwitchRule<GraphInputs> ... {
  static_assert(traits::isUnique<GraphInputs...>, "A node can't accept multiple inputs with the same type.");
  static_assert(sizeof... (GraphInputs) >= 1, "A node need to have one output type and at least one output type.");
 private:
  std::shared_ptr<Graph<GraphOutput, GraphInputs...>>
      graph_ = nullptr; ///< Original Graph that will be duplicated

  std::shared_ptr<core::CoreDefaultExecutionPipeline<GraphOutput, GraphInputs...>>
      coreExecutionPipeline_ = nullptr; ///< Execution Pipeline core

 public:

  /// @brief Deleted default constructor, an execution pipeline need to be constructed with hedgehog compatible graph.
  AbstractExecutionPipeline() = delete;

  /// @brief Constructor to set an execution pipeline with hedgehog graph and the total number of graphs.
  /// @details If iota is set to true, the device ids are initialized from 0 to numberGraphDuplications - 1, if iota is
  /// set to false, the device Ids are all set to 0.
  /// @param graph Inside Base Graph
  /// @param numberGraphDuplications Total number of graphs inside the execution pipeline
  /// (numberGraphDuplications - 1 duplications)
  /// @param iota If true the device Id are set from 0 to numberGraphDuplications - 1 (using std::iota), else all device
  /// Ids are set to 0.
  AbstractExecutionPipeline(std::shared_ptr<Graph<GraphOutput, GraphInputs...>> graph,
                            size_t const &numberGraphDuplications,
                            bool iota = false) {
    std::vector<int> deviceIds(numberGraphDuplications, 0);
    if (iota) { std::iota(deviceIds.begin(), deviceIds.end(), 0); }
    graph_ = graph;
    coreExecutionPipeline_ = std::make_shared<core::CoreDefaultExecutionPipeline<GraphOutput, GraphInputs...>>(
        "AbstractExecutionPipeline", this,
        std::dynamic_pointer_cast<core::CoreGraph<GraphOutput, GraphInputs...>>(graph->core()),
        numberGraphDuplications, deviceIds, false);
  }

  /// @brief Constructor to set an execution pipeline with hedgehog graph, the total number of graphs, the device Ids, and set
  /// the execution to automatically start.
  /// @param graph Inside Base Graph
  /// @param numberGraphDuplications Total number of graphs inside the execution pipeline
  /// (numberGraphDuplications - 1 duplications)
  /// @param deviceIds Vector of Ids, corresponding to device Ids for each inside graphs
  /// @param automaticStart If True, the execution pipeline will be automatically started by sending nullptr to the
  /// SwitchRule::sendToGraph, else not
  AbstractExecutionPipeline(std::shared_ptr<Graph<GraphOutput, GraphInputs...>> graph,
                            size_t const &numberGraphDuplications,
                            std::vector<int> const deviceIds, bool automaticStart = false)
      : graph_(graph),
        coreExecutionPipeline_(std::make_shared<core::CoreDefaultExecutionPipeline<GraphOutput, GraphInputs...>>(
            "AbstractExecutionPipeline",
            this,
            std::dynamic_pointer_cast<core::CoreGraph<GraphOutput, GraphInputs...>>(graph->core()),
            numberGraphDuplications,
            deviceIds, automaticStart)) {}

  /// @brief Constructor to set an execution pipeline with hedgehog graph, the total number of graphs, the device Ids and set
  /// the execution to automatically start.
  /// @param name Execution pipeline's name
  /// @param graph Inside Base Graph
  /// @param numberGraphDuplications Total number of graphs inside the execution pipeline
  /// (numberGraphDuplications - 1 duplications)
  /// @param deviceIds Vector of Ids, corresponding to device Ids for each inside graphs
  /// @param automaticStart If True, the execution pipeline will be automatically started by sending nullptr to the
  /// SwitchRule::sendToGraph, else not
  AbstractExecutionPipeline(std::string_view const &name,
                            std::shared_ptr<Graph<GraphOutput, GraphInputs...>> const &graph,
                            size_t const &numberGraphDuplications,
                            std::vector<int> const &deviceIds,
                            bool automaticStart = false)
      : graph_(graph),
        coreExecutionPipeline_(std::make_shared<core::CoreDefaultExecutionPipeline<GraphOutput, GraphInputs...>>(
            name,
            this,
            std::dynamic_pointer_cast<core::CoreGraph<GraphOutput, GraphInputs...>>(graph->core()),
            numberGraphDuplications,
            deviceIds,
            automaticStart)
        ) {}

  /// @brief  Default destructor
  virtual ~AbstractExecutionPipeline() = default;

  /// @brief Execution Pipeline accessor core
  /// @return Execution Pipeline's core
  std::shared_ptr<core::CoreNode> core() override { return this->coreExecutionPipeline_; }

  /// @brief Inner graph accessor
  /// @return Inner graph
  std::shared_ptr<Graph<GraphOutput, GraphInputs...>> const &graph() const { return graph_; }
};
}

#endif //HEDGEHOG_ABSTRACT_EXECUTION_PIPELINE_H
