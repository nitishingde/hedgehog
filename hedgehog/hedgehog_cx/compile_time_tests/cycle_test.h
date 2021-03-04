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
//
// Created by Bardakoff, Alexandre (IntlAssoc) on 11/17/20.
//

#ifndef HEDGEHOG_CX_CYCLE_TEST_H
#define HEDGEHOG_CX_CYCLE_TEST_H

#include "../api_cx/cx_abstract_test.h"
#include "../tools/data_structures/stack_cx.h"

/// @brief Hedgehog CX test namespace
namespace hh::cx::test {

/// @brief Detect cycles in the graph. Hedgehog accepts cycles in graphs, a custom canTerminate method need to be
/// overloaded in a cycle's node to define custom rule for termination.
/// @details Two algorithms are used to detect cycles:
/// - Tarjan algorithm to detect strongly connected components
/// (https://en.wikipedia.org/wiki/Tarjan%27s_strongly_connected_components_algorithm)
/// - Johnson algorithm to detect all elementary circuits from the connected components (https://doi.org/10.1137/0204007)
/// A cycle will be filtered if the canTerminate method has been defined in one of the node in the cycle.
/// @tparam GraphType Graph type
/// @tparam NodesNumber Maximum number of nodes in the graph (will be deleted when the constexpr std::vector will be available)
/// @tparam MaximumNumberCycles Maximum number of cycles that can be found (will be deleted when the constexpr std::vector will be available)
template<hh::HedgehogDynamicGraphForStaticAnalysis GraphType, size_t NodesNumber = 20, size_t MaximumNumberCycles = 100>
class CycleTest : public hh::cx::CXAbstractTest<GraphType, NodesNumber> {
  /// @brief Simple representation of a subgraph used to detect cycles
  /// @attention Use with caution, less check are made to build the graph, are they are already made in the CXGraph
  class SimpleSubGraph {
    /// @brief Type definition for a pointer to const AbstractNode
    using pointer_const_abstract_node = hh::cx::behavior::AbstractNode const *;

    vector_cx<pointer_const_abstract_node, NodesNumber>
        registeredNodes_{}; ///< Nodes registered in the subgraph

    std::array<std::array<bool, NodesNumber>, NodesNumber>
        adjacencyMatrix_{}; ///< Adjacency's matrix in subgraph

   public:
    /// @brief Default constructor
    constexpr SimpleSubGraph() = default;

    /// @brief Create a subgraph from a graph. Add only nodes from a minimum node ID.
    /// @param graph Graph to extract nodes from.
    /// @param minNodeId Minimum id of nodes extracted
    constexpr SimpleSubGraph(CXGraph<GraphType, NodesNumber> const *graph, size_t minNodeId) {
      for (size_t nodeId = minNodeId; nodeId < graph->numberNodesRegistered(); ++nodeId) {
        registeredNodes_.push_back(graph->node(nodeId));
      }

      for (size_t nodeIdSrc = minNodeId; nodeIdSrc < graph->numberNodesRegistered(); ++nodeIdSrc) {
        for (size_t nodeIdDest = minNodeId; nodeIdDest < graph->numberNodesRegistered(); ++nodeIdDest) {
          if (graph->isLinked(nodeIdSrc, nodeIdDest)) {
            adjacencyMatrix_.at(nodeId(graph->node(nodeIdSrc))).at(nodeId(graph->node(nodeIdDest))) = true;
          }
        }
      }
    }

    /// @brief Default destructor
    virtual ~SimpleSubGraph() = default;

    /// @brief Registered Nodes accessor
    /// @return Registered Nodes
    [[nodiscard]] constexpr vector_cx<pointer_const_abstract_node, NodesNumber> const & registeredNodes() const {
      return registeredNodes_;
    }

    /// @brief Get all adjacent nodes from an origin node
    /// @param origin Origin node to get adjacent nodes from
    /// @return A vector of nodes that receives information the origin node
    [[nodiscard]] constexpr vector_cx<pointer_const_abstract_node, NodesNumber>
    adjacentNodes (pointer_const_abstract_node origin) const {
      vector_cx<pointer_const_abstract_node, NodesNumber> adjacentNodes{};
      auto senderId = nodeId(origin);
      for (size_t receiverId = 0; receiverId < NodesNumber; ++receiverId) {
        if (isLinked(senderId, receiverId)) {
          adjacentNodes.push_back(node(receiverId));
        }
      }
      return adjacentNodes;
    }

    /// @brief Number of registered nodes accessor
    /// @return The number of nodes accessed
    [[nodiscard]] constexpr size_t numberNodesRegistered() const { return registeredNodes_.size(); }

    /// @brief Test if an edge exists between a sender node and a receiver node
    /// @param sender Sender node
    /// @param receiver Receiver node
    /// @return True if an edge exists between a sender node and a receiver node, else False
    [[nodiscard]] constexpr bool isLinked(pointer_const_abstract_node sender, pointer_const_abstract_node receiver) const {
      auto
          idSender = nodeId(sender),
          idReceiver = nodeId(receiver);

      if (idSender == NodesNumber || idReceiver == NodesNumber) { return false; }
      return adjacencyMatrix_[idSender][idReceiver];
    }

    /// @brief Test if an edge exists between a sender node id and a receiver node id
    /// @param idSender Sender node id
    /// @param idReceiver Receiver node id
    /// @return True if an edge exists between a sender node id and a receiver node id, else False
    /// @throw std::runtime_error Try to access nodes that have not been registered
    [[nodiscard]] constexpr bool isLinked(size_t idSender, size_t idReceiver) const {
      if (idSender >= NodesNumber || idReceiver >= NodesNumber) {
        throw (std::runtime_error("The nodes you are trying to test do not exist in the graph."));
      }
      return adjacencyMatrix_[idSender][idReceiver];
    }

    /// @brief Get a subgraph node id from a node pointer
    /// @param node Node pointer to get an id from
    /// @return The subgraph node id corresponding to the node
    [[nodiscard]]  constexpr size_t nodeId(pointer_const_abstract_node node) const {
      if (std::find(registeredNodes_.cbegin(), registeredNodes_.cend(), node) == registeredNodes_.cend()) {
        throw (std::runtime_error("The node you are trying to get does not exist in the graph."));
      }
      size_t nodeId = 0;
      for (auto it = registeredNodes_.cbegin(); it != registeredNodes_.cend(); ++it) {
        if (*it == node) { return nodeId; }
        else { ++nodeId; }
      }
      return nodeId;
    }

    /// @brief Get a node pointer from an id
    /// @param id Get the node corresponding to the id
    /// @return A node pointercorresponding to the id
    [[nodiscard]] constexpr pointer_const_abstract_node node(size_t id) const {
      if (id >= registeredNodes_.size()) {
        throw (std::runtime_error("The node you are requesting does not exist."));
      } else {
        return registeredNodes_.at(id);
      }
    }

    /// @brief Add an edge between a sender node and a receiver node
    /// @param sender Sender node
    /// @param receiver Receiver node
    constexpr void addEdge(pointer_const_abstract_node sender, pointer_const_abstract_node receiver) {
      registerNode(sender);
      registerNode(receiver);
      adjacencyMatrix_.at(nodeId(sender)).at(nodeId(receiver)) = true;
    }
   private:
    /// @brief Register a node in the subgraph if it has not already been registered
    /// @param node Node to register
    constexpr void registerNode(pointer_const_abstract_node node) {
      if (std::find(registeredNodes_.cbegin(), registeredNodes_.cend(), node) == registeredNodes_.cend()) {
        registeredNodes_.push_back(node);
      }
    }
  };

 private:
  std::array<size_t, NodesNumber>
      tarjanNodeNumbers_{}, ///< Number associated to the node for the Tarjan algorithm
      tarjanLowLink_{}; ///< Low link property associated to the node for the Tarjan algorithm

  std::array<bool, NodesNumber>
      tarjanIsOnStack_{}; ///< Direct accessor to test node presence in a stack for the Tarjan algorithm

  stack_cx<hh::cx::behavior::AbstractNode const *, NodesNumber>
      tarjanStack_{}, ///< Stack used for the the Tarjan algorithm
      johnsonStack_{}; ///< Stack used for the the Johnson algorithm

  vector_cx<vector_cx<hh::cx::behavior::AbstractNode const *, NodesNumber>, MaximumNumberCycles>
      tarjanConnectedComponent_{}, ///< List of connected components
      johnsonCycles_{}; ///< List of cycles found

  std::array<bool, NodesNumber>
      johnsonBlockedSetArray_{}; ///< List of blocked nodes for the the Johnson algorithm

  std::array<vector_cx<hh::cx::behavior::AbstractNode const *, NodesNumber>, NodesNumber>
      johnsonBlockedMap_{}; ///< Map of nodes to unblock if a node is unblocked for the the Johnson algorithm

 public:
  /// @brief Default constructor
  constexpr CycleTest() : hh::cx::CXAbstractTest<GraphType, NodesNumber>("Johnson") {}
  /// @brief Default destructor
  constexpr virtual ~CycleTest() = default;

  /// @brief Test a graph to detect cycles
  /// @param graph Graph to test
  constexpr virtual void test(CXGraph<GraphType, NodesNumber> const *graph) {
    findAllCycles(graph);
    removeCyclesWhereNodesRedefineCanTerminate();
    if (johnsonCycles_.empty()) {
      this->isGraphValid(true);
    } else {
      this->isGraphValid(false);
      this->errorMessage_.push_back("Cycles found, the canTerminate() method needs to be defined for each of these "
                                    "cycles.");
      for (const auto cycle : johnsonCycles_) {
        this->errorMessage_.push_back("\n\t");
        auto origin = cycle.at(0);
        for (const auto node : cycle) {
          this->errorMessage_.push_back(node->name());
          this->errorMessage_.push_back(" -> ");
        }
        this->errorMessage_.push_back(origin->name());
      }
    }
  }

 private:
  /// @brief Find all cycles in the graph
  /// @param graph Graph to get the cycles from
  constexpr void findAllCycles(CXGraph<GraphType, NodesNumber> const *graph) {
    size_t startIndex = 0;
    while (startIndex < graph->numberNodesRegistered()) {
      initTarjan();
      SimpleSubGraph subGraph(graph, startIndex);
      tarjanAllStrongConnect(subGraph);

      auto[leastIndex, minSubGraph] = leastIndexStrongConnectedComponents(subGraph);

      if (leastIndex == NodesNumber) {
        break;
      } else {
        initJohnson();
        auto node = subGraph.node(leastIndex);
        findCyclesInSCC(node, node, minSubGraph);
        ++startIndex;
      }
    }
  }

  /// @brief Initialize the data structures for the Tarjan Algorithm
  constexpr void initTarjan() {
    for (size_t id = 0; id < NodesNumber; ++id) {
      tarjanNodeNumbers_ = {};
      tarjanLowLink_ = {};
    }
    for (auto & elem : tarjanIsOnStack_) { elem = false; }
    while (!tarjanStack_.empty()) { tarjanStack_.pop(); }
    while (!tarjanConnectedComponent_.empty()) { tarjanConnectedComponent_.pop_back(); }
  }

  /// @brief Initialize the data structures for the Johnson Algorithm
  constexpr void initJohnson() {
    while (!johnsonStack_.empty()) { johnsonStack_.pop(); }
    for (size_t nodeId = 0; nodeId < NodesNumber; ++nodeId) {
      johnsonBlockedMap_.at(nodeId) = {};
      johnsonBlockedSetArray_.at(nodeId) = false;
    }
  }

  /// @brief Find all the cycles in a subgraph
  /// @param startNode Cycle start node
  /// @param currentNode Cycle current node
  /// @param subGraph Subgraph used
  /// @return True if cycles are found, else False
  constexpr bool findCyclesInSCC(
      hh::cx::behavior::AbstractNode const *startNode,
      hh::cx::behavior::AbstractNode const *currentNode,
      SimpleSubGraph const &subGraph) {
    bool cycleFound = false;
    johnsonStack_.push(currentNode);
    johnsonBlockedSetArray_.at(subGraph.nodeId(currentNode)) = true;

    for (auto neighbor :  subGraph.adjacentNodes(currentNode)) {
      if (neighbor == startNode) {
        vector_cx<hh::cx::behavior::AbstractNode const *, NodesNumber> cycle{};
        auto tempStack(johnsonStack_);
        while (!tempStack.empty()) { cycle.push_back(tempStack.pop()); }
        std::reverse(cycle.begin(), cycle.end());
        if (std::none_of(johnsonCycles_.cbegin(), johnsonCycles_.cend(),
                         [&cycle](auto &storedCycle) { return cycle == storedCycle; })) {
          johnsonCycles_.push_back(cycle);
        }
        cycleFound = true;
      } else if (!johnsonBlockedSetArray_.at(subGraph.nodeId(neighbor))) {
        cycleFound |= findCyclesInSCC(startNode, neighbor, subGraph);
      }
    }

    if (cycleFound) { unblock(currentNode, subGraph);}
    else {
      for (auto neighbors : subGraph.adjacentNodes(currentNode)) {
        auto nodes = johnsonBlockedMap_.at(subGraph.nodeId(neighbors));
        if (!cxVectorContain(nodes, currentNode)) {
          nodes.push_back(currentNode);
          johnsonBlockedMap_.at(subGraph.nodeId(neighbors)) = nodes;
        }
      }
    }

    johnsonStack_.pop();
    return cycleFound;
  }

  /// @brief Get the least index and the subgraph containing the least index
  /// @param subGraph Subgraph to get the node from
  /// @return The least index and the subgraph containing the least index
  constexpr std::pair<size_t, SimpleSubGraph> leastIndexStrongConnectedComponents(SimpleSubGraph const &subGraph) {
    size_t min = NodesNumber;
    vector_cx<hh::cx::behavior::AbstractNode const *, NodesNumber> minSCC;

    for (auto connectedComponent : tarjanConnectedComponent_) {
      if (connectedComponent.size() == 1) {
        auto node = connectedComponent.at(0);
        if (subGraph.isLinked(node, node)) {
          johnsonCycles_.push_back({node});
        }
      } else {
        for (auto node : connectedComponent) {
          if (subGraph.nodeId(node) < min) {
            min = subGraph.nodeId(node);
            minSCC = connectedComponent;
          }
        }
      }
    }

    if (min == NodesNumber) { return {NodesNumber, SimpleSubGraph()}; }
    else {
      SimpleSubGraph minSubGraph;

      vector_cx<size_t, MaximumNumberCycles> minIndexes{};
      for (auto sender : subGraph.registeredNodes()) {
        for (auto receiver : subGraph.registeredNodes()) {
          if (subGraph.isLinked(sender, receiver)) {
            if (cxVectorContain(minSCC, sender) && cxVectorContain(minSCC, sender)) {
              minSubGraph.addEdge(sender, receiver);
              minIndexes.push_back(subGraph.nodeId(sender));
              minIndexes.push_back(subGraph.nodeId(receiver));
            }
          }
        }
      }
      return {*std::min_element(minIndexes.cbegin(), minIndexes.cend()), minSubGraph};
    }
  }

  /// @brief Generate all the strong connected components
  /// @param subGraph Subgraph to get the strong connected components from
  constexpr void tarjanAllStrongConnect(SimpleSubGraph const &subGraph) {
    size_t num = 1;
    for (size_t nodeId = 0; nodeId < subGraph.numberNodesRegistered(); ++nodeId) {
      if (tarjanNodeNumbers_[nodeId] == 0) { tarjanStrongConnect(num, nodeId, subGraph); }
    }
  }

  /// @brief Tarjan algorithm to get the string connected components from a subgraph
  /// @param num Number used in the Tarjan algorithm
  /// @param nodeId Id of the current node
  /// @param subGraph Subgraph to extract the connected components from
  constexpr void tarjanStrongConnect(size_t &num, size_t &nodeId, SimpleSubGraph const &subGraph) {
    auto &node = subGraph.registeredNodes().at(nodeId);
    tarjanNodeNumbers_[nodeId] = num;
    tarjanLowLink_[nodeId] = num;
    tarjanIsOnStack_[nodeId] = true;
    tarjanStack_.push(node);
    num += 1;

    for (auto &neighbor : subGraph.adjacentNodes(node)) {
      size_t neighborId = subGraph.nodeId(neighbor);
      if (tarjanNodeNumbers_[neighborId] == 0) {
        tarjanStrongConnect(num, neighborId, subGraph);
        tarjanLowLink_[nodeId] = std::min(tarjanLowLink_[nodeId], tarjanLowLink_[neighborId]);
      } else if (tarjanIsOnStack_[neighborId]) {
        tarjanLowLink_[nodeId] = std::min(tarjanLowLink_[nodeId], tarjanNodeNumbers_[neighborId]);
      }
    }

    if (tarjanLowLink_[nodeId] == tarjanNodeNumbers_[nodeId]) {
      vector_cx<hh::cx::behavior::AbstractNode const *, NodesNumber> component;
      hh::cx::behavior::AbstractNode const *nodeStack = nullptr;
      do {
        nodeStack = tarjanStack_.pop();
        tarjanIsOnStack_[subGraph.nodeId(nodeStack)] = false;
        component.push_back(nodeStack);
      } while (node != nodeStack);
      tarjanConnectedComponent_.push_back(component);
    }
  }

  /// @brief Helper to test if a vector contain an element
  /// @tparam VectorType Element type in vector
  /// @tparam VectorSize Vector size
  /// @param v Vector instance
  /// @param val Value to look for
  /// @return True if val is found in v, else False
  template<class VectorType, size_t VectorSize>
  [[nodiscard]] constexpr bool cxVectorContain(vector_cx<VectorType, VectorSize> const &v, VectorType const &val) {
    return std::find(v.cbegin(), v.cend(), val) != v.cend();
  }

  /// @brief Unblock a node, and all cascading nodes found in johnsonBlockedMap
  /// @param node Node to unblock
  /// @param subGraph Subgraph considered
  constexpr void unblock(hh::cx::behavior::AbstractNode const *node, SimpleSubGraph const &subGraph) {
    johnsonBlockedSetArray_.at(subGraph.nodeId(node)) = false;
    for (auto nodeBlocked : johnsonBlockedMap_.at(subGraph.nodeId(node))) {
      if (johnsonBlockedSetArray_.at(subGraph.nodeId(nodeBlocked))) {
        unblock(nodeBlocked, subGraph);
      }
    }
    johnsonBlockedMap_.at(subGraph.nodeId(node)) = {};
  }

  /// @brief Remove the cycles in which a node overload the canTerminate method
  constexpr void removeCyclesWhereNodesRedefineCanTerminate() {
    bool found = false;
    vector_cx<vector_cx<hh::cx::behavior::AbstractNode const *, NodesNumber>, MaximumNumberCycles> temp;
    for (auto cycle : johnsonCycles_) {
      found = std::any_of(cycle.cbegin(), cycle.cend(), [](auto const &node) { return node->isCanTerminateOverloaded(); });
      if (!found) { temp.push_back(cycle); }
    }
    johnsonCycles_ = temp;
  }

};

}

#endif //HEDGEHOG_CX_CYCLE_TEST_H
