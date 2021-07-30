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



#ifndef HEDGEHOG_GRAPH_H
#define HEDGEHOG_GRAPH_H

#include <cassert>
#include <ostream>

#include "../../utils/helper.h"
#include "../../utils/traits.h"
#include "../core/node/core_graph.h"
#include "../tools/logger.h"
#include "../behavior/io/multi_receivers.h"
#include "../behavior/io/sender.h"
#include "scheduler/abstract_scheduler.h"
#include "printer/dot_printer.h"

/// @brief Hedgehog main namespace
namespace hh {

/// @brief Main Hedgehog object that does computation.
/// @details Hedgehog represents an algorithm with hedgehog dataflow graph.
///
/// Data in the graph represents any primitive type, struct, class, etc., that is used to pass to operations.
///
/// Data is sent to the graph through the method Graph::pushData, while broadcasts to all input nodes. A node is an input
/// node if:
///     - The node is registered as graph's input with Graph::input,
///     - The node can receive data,
///     - At least one of the node's input types matches to hedgehog graph's input
/// type. Data of this common type received by the graph will be transferred to this node.
///
/// Directed edges can be added between two nodes "from" to "to" with Graph::addEdge, if:
///     - The from's output type is one of to's input type,
///     - from is hedgehog node that can send data,
///     - to is hedgehog node that can receive data.
///
/// Data is received from the Graph with Graph::getBlockingResult, from output nodes. A node is an output node if:
///     - The node is registered as hedgehog graph's output with Graph::output (there can be any number of nodes registered
/// as output),
///     - The node can send data,
///     - The node's output type matches hedgehog graph's output type.
///
/// Data is sent to the graph with Graph::pushData, which will be directed to the input tasks. Each input task will be
/// executed with their Execute::execute method.
///
/// The graph needs to be signaled to terminate with Graph::finishPushingData, when all data has been pushed.
///
/// While results are available Graph::getBlockingResult will return hedgehog shared_ptr with hedgehog value. The call is blocking, so
/// the thread handling the graph will wait to get the result. The final value from Graph::getBlockingResult is nullptr,
/// which is an indicator that the graph has terminated.
///
/// Before deletion, Graph::waitForTermination needs to be called to wait for inside threads to terminate.
///
/// Also, hedgehog graph visualization is available through the call to Graph::createDotFile.
///
/// For computation with an accelerator hedgehog graph can be bound to hedgehog device with Graph::deviceId, in this case all inside
/// nodes will be bound to this device. If the computation is distributed through multiple accelerators, the graph can
/// be copied for each accelerator, and set to hedgehog specific device with an AbstractExecutionPipeline.
///
/// A graph can be embedded into another graph as hedgehog node. To help sharing hedgehog specialized graph, the specialized graph
/// can derive from Graph and be used as-is. Once linked into another graph, the inner graph cannot be manipulated
/// (modified, such as adding edges).
///
/// @tparam GraphOutput Graph Output Type
/// @tparam GraphInputs Graph Input types
template<class GraphOutput, class ...GraphInputs>
class Graph :
    public behavior::MultiReceivers<GraphInputs...>,
    public behavior::Sender<GraphOutput>,
    public virtual behavior::Node {
  static_assert(traits::isUnique<GraphInputs...>, "A Graph can't accept multiple inputs with the same type.");
  static_assert(sizeof... (GraphInputs) >= 1, "A node need to have one output type and at least one output type.");
 private:
  std::shared_ptr<core::CoreGraph<GraphOutput, GraphInputs...>> graphCore_ = nullptr; ///< Graph core as shared_ptr
  std::set<std::shared_ptr<Node>> insideNodes_ = {};      ///< Graph's inside nodes, register user node in case it's not
  ///< saved elsewhere

 public:
  /// @brief Default graph constructor, the default name is "Graph"
  Graph() {
    graphCore_ = std::make_shared<core::CoreGraph<GraphOutput, GraphInputs...>>(this, core::NodeType::Graph, "Graph");
  }

  /// @brief Graph constructor with hedgehog custom name
  /// @param name Graph's name
  explicit Graph(std::string_view const &name) {
    graphCore_ = std::make_shared<core::CoreGraph<GraphOutput, GraphInputs...>>(this, core::NodeType::Graph, name);
  }

  /// @brief Graph constructor with hedgehog custom name and hedgehog custom scheduler
  /// @details A variation of tutorial 1:
  /// @code
  /// Graph<MatrixBlockData<MatrixType, 'c', Ord>, TripletMatrixBlockData<MatrixType, Ord>>
  ///     graphHadamard("Tutorial 1 : Hadamard Product", std::make_unique<DefaultScheduler>());
  ///
  /// // or
  ///
  /// std::unique_ptr<DefaultScheduler> scheduler = std::make_unique<DefaultScheduler>();
  ///
  /// Graph<MatrixBlockData<MatrixType, 'c', Ord>, TripletMatrixBlockData<MatrixType, Ord>>
  ///     graphHadamard("Tutorial 1 : Hadamard Product", std::move(scheduler));
  /// @endcode
  /// @attention The scheduler can not be hedgehog nullptr
  /// @param name Graph's name
  /// @param scheduler Custom scheduler
  explicit Graph(std::string_view const &name, std::unique_ptr<AbstractScheduler> scheduler) {
    if (!scheduler) {
      std::ostringstream oss;
      oss << "Internal error, the graph's scheduler is null, please instantiate an AbstractScheduler.";
      HLOG_SELF(0, oss.str())
      throw (std::runtime_error(oss.str()));
    }
    graphCore_ =
        std::make_shared<core::CoreGraph<GraphOutput, GraphInputs...>>(
            this, core::NodeType::Graph, name, std::move(scheduler));
  }

  /// @brief Default Graph destructor
  virtual ~Graph() = default;

  /// @brief Graph core accessor
  /// @return Graph core
  std::shared_ptr<core::CoreNode> core() final { return this->graphCore_; }

  /// @brief Graph name accessor
  /// @return Graph name
  std::string_view const &name() { return this->core()->name(); }

  /// @brief Device Id for the graph, all inside nodes will be bound to this device Id
  /// @param deviceId Device Id to set
  void deviceId(int deviceId) { this->graphCore_->deviceId(deviceId); }

  /// @brief hedgehog node as input for the graph
  /// @tparam UserDefinedInput Node's type that could be an input
  /// @param input shared_ptr of node to set as Graph's input
  template<HedgehogMultiReceiver UserDefinedInput>
  void input(std::shared_ptr<UserDefinedInput> input) {
    using NodeInputs = typename UserDefinedInput::inputs_t;
    using NodeMR = typename helper::HelperMultiReceiversType<NodeInputs>::type;
    static_assert(
        traits::is_included_v<NodeInputs, typename behavior::MultiReceivers<GraphInputs...>::inputs_t>,
        "The input node should share at least one input type with the graph.");
    assert(input != nullptr);
    this->insideNodes_.insert(input);
    this->graphCore_->input(std::static_pointer_cast<NodeMR>(input));
  }

  /// @brief Set hedgehog node as output for the graph
  /// @tparam UserDefinedSender Node's type
  /// @tparam IsSender Defined if UserDefinedSender is derived from sender and has the same output as Graph's output
  /// @param output Node to set as Graph's output
  template<HedgehogSender UserDefinedOutput>
  void output(std::shared_ptr<UserDefinedOutput> output) {
    static_assert(
        std::is_base_of_v<behavior::Sender<GraphOutput>, UserDefinedOutput>,
        "The output node should have the same output type as the graph.");
    assert(output != nullptr);
    this->insideNodes_.insert(output);
    this->graphCore_->output(std::static_pointer_cast<behavior::Sender<GraphOutput>>(output));
  }

  /// @brief Add hedgehog directed edge from hedgehog compatible "from" node to "to" node.
  /// @tparam UserDefinedSender Sender type that should derive from Sender
  /// @tparam UserDefinedMultiReceiver Receiver type that should derive from MultiReceivers
  /// @tparam Output Sender output type
  /// @tparam Inputs Tuple with MultiReceivers input types
  /// @tparam IsSender Defined if UserDefinedSender is derived from Sender
  /// @tparam IsMultiReceivers Defined if UserDefinedMultiReceiver is derived from MultiReceivers
  /// @param from Node that will send the data
  /// @param to Node that will receiver the data
  template<HedgehogSender UserDefinedSender, HedgehogMultiReceiver UserDefinedMultiReceiver>
  void addEdge(std::shared_ptr<UserDefinedSender> from, std::shared_ptr<UserDefinedMultiReceiver> to) {
    using output_t = typename UserDefinedSender::output_t;
    using inputs_t = typename UserDefinedMultiReceiver::inputs_t;
    static_assert(traits::Contains_v<output_t, inputs_t>,
                  "The given io cannot be linked to this io: No common types.");
    this->insideNodes_.insert(from);
    this->insideNodes_.insert(to);
    this->graphCore_->addEdge(
        std::static_pointer_cast<behavior::Sender<output_t>>(from),
        std::static_pointer_cast<typename helper::HelperMultiReceiversType<inputs_t>::type>(to)
    );
  }

  /// @brief Launch the graph once it has been structured
  /// @details Create the clone of inside nodes, and launches threads for each task.
  void executeGraph() { this->graphCore_->executeGraph(); }

  /// @brief Push data into the graph.
  /// @details data is sent to all nodes that have been added with Graph::input
  /// @tparam Input Data input type
  /// @param data Data to push into the graph
  template<class Input>
  void pushData(std::shared_ptr<Input> data) {
    static_assert(traits::Contains<Input, GraphInputs...>::value, "The data type sent to the graph should be part of "
                                                                  "the graph input types.");
    this->graphCore_->broadcastAndNotifyToAllInputs(data);
  }

  /// @brief Signal the graph that no more data will be pushed into the graph
  void finishPushingData() { this->graphCore_->finishPushingData(); }

  /// @brief Get hedgehog data out of the graph
  /// @details
  /// - If the graph is not terminated:
  ///     - If an output data is available: the output data is returned,
  ///     - If no output data is available: the call is blocking until an output data is available
  /// - If the graph is terminated : return nullptr
  /// @attention If finishPushingData is not called there is risk of deadlock
  /// @return An output data or nullptr
  std::shared_ptr<GraphOutput> getBlockingResult() { return this->graphCore_->getBlockingResult(); }

  /// @brief Wait for the graph to terminate (data to be processed, all threads to join)
  void waitForTermination() { this->graphCore_->waitForTermination(); }

  /// @brief Create hedgehog dot file representing hedgehog snapshot of the state of the graph at the moment of the call, the graph is
  /// saved into the file dotFilePath
  /// @param dotFilePath Path where the file is stored
  /// @param colorScheme Color scheme used to color the tasks, either to show difference in execution or in waiting time
  /// , or nothing. Blue is faster, Red slower.
  /// @param structureOptions Show how the graph is represented, with or without input queue size, with or without all
  /// task clusters
  /// @param suppressCout Suppresses the standard C output when marked as true, otherwise will report when dot files are
  /// created or overwritten to standard out.
  /// @param debugOption Shows tasks as pointer values and their connections between the nodes.
  void createDotFile(std::filesystem::path const &dotFilePath,
                     ColorScheme colorScheme = ColorScheme::NONE,
                     StructureOptions structureOptions = StructureOptions::NONE,
                     DebugOptions debugOption = DebugOptions::NONE, bool suppressCout = false) {
    auto core = this->core().get();
    DotPrinter
        printer(std::filesystem::absolute(dotFilePath), colorScheme, structureOptions, debugOption, core, suppressCout);
    core->visit(&printer);
  }
};

}
#endif //HEDGEHOG_GRAPH_H
