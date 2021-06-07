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
// Created by Bardakoff, Alexandre (IntlAssoc) on 8/20/20.
//

#ifndef HEDGEHOG_CX_CX_GRAPH_H
#define HEDGEHOG_CX_CX_GRAPH_H
#include "../tools/data_structures/vector_cx.h"
#include "../../utils/concept.h"

#include "cx_node.h"

/// @brief Hedgehog CX main namespace
namespace hh::cx {

#ifndef DOXYGEN_SHOULD_SKIP_THIS
template<HedgehogDynamicGraphForStaticAnalysis, size_t, size_t>
class Defroster;

template<HedgehogDynamicGraphForStaticAnalysis, size_t, size_t>
class CXAbstractTest;
#endif //DOXYGEN_SHOULD_SKIP_THIS

/// @brief Compile time graph representation.
/// @details A CXGraph is used to bundle hedgehog set of CXNode or CXGraph (graph-composition), to test. It represents hedgehog
/// dynamic Hedgehog Graph and its inputs / output type are the one of the dynamic node.
/// The methods input/output/addEdge follow the same restriction than the ones on the dynamic library.
/// The graph is tested against different CXAbstractTest. It is possible to transform hedgehog CXGraph to hedgehog hh::Graph,
/// through hedgehog defroster. The defroster will run the compile_time_tests when constructed and extract all information concerning the
/// CXGraph structure.
/// @attention The number of test is limited to 255 until std::vector is available
/// @attention The dynamic Graph of type GraphType, should have hedgehog constructor as the following:
/// GraphType(std::string_view const &)
/// @tparam GraphType Graph type
/// @tparam NodesNumber Maximum number of nodes in the graph (will be deleted when the constexpr std::vector will be available)
/// @tparam LengthErrorMessage Maximum length error message (will be deleted when the constexpr std::vector will be
/// available)
template<HedgehogDynamicGraphForStaticAnalysis GraphType, size_t NodesNumber = 20, size_t LengthErrorMessage = 255>
class CXGraph : public CXNode<GraphType> {
 private:
  /// @brief Defroster friend declaration
  /// @relates Defroster<GraphType, NodesNumber>
  friend Defroster<GraphType, NodesNumber, LengthErrorMessage>;

  vector_cx<hh::cx::behavior::AbstractNode const *, NodesNumber>
      registeredNodes_{}, ///< Registered nodes, position is an ID for the adjacency matrix, the names, the RO edges...
      inputNodes_{}, ///< Nodes declared as inputs
      outputNodes_{}; ///< Nodes declared as outputs

  std::array<std::array<bool, NodesNumber>, NodesNumber>
      adjacencyMatrix_{}, ///< Adjacency Matrix
      ROEdges_{}; ///< Edges declared as Read Only

  vector_cx<CXAbstractTest<GraphType, NodesNumber, LengthErrorMessage> *, 255>
      tests_{}; ///< Test associated

 public:
  /// @brief CXGraph Constructor
  /// @param name CXGraph name, used to build the hh::Graph
  constexpr explicit CXGraph(std::string_view const &name) : CXNode<GraphType>(name) {}

  /// @brief CXGraph destructor
  constexpr virtual ~CXGraph() {
    for (auto & test : tests_) { test = nullptr; }
    for (auto & inputNode : inputNodes_) { inputNode = nullptr; }
    for (auto & outputNode : outputNodes_) { outputNode = nullptr; }
    for (auto & node : registeredNodes_) { node = nullptr; }
  }

  /// @brief Registered nodes accessor
  /// @return A const reference to the vector of registered nodes
  [[nodiscard]] constexpr vector_cx<hh::cx::behavior::AbstractNode const *, NodesNumber> const &
  registeredNodes() const {
    return registeredNodes_;
  }

  /// @brief Get the number of registered nodes
  /// @return Number of registered nodes
  [[nodiscard]] constexpr size_t numberNodesRegistered() const { return registeredNodes_.size(); }

  /// @brief Accessor to the input nodes
  /// @return The input nodes
  [[nodiscard]] constexpr vector_cx<hh::cx::behavior::AbstractNode const *, NodesNumber> const &inputNodes() const {
    return inputNodes_;
  }

  /// @brief Accessor to the output nodes
  /// @return The output nodes
  [[nodiscard]] constexpr vector_cx<hh::cx::behavior::AbstractNode const *, NodesNumber> const &outputNodes() const {
    return outputNodes_;
  }

  /// @brief Set hedgehog CXNode as input of the CXGraph. Register the node if needs be.
  /// @attention The CXNode should share at least one input type defined in the InputNode type, with the input type
  /// of the CXGraph defined in the GraphType
  /// @tparam InputNode Type of the input node
  /// @param inputNode Instance of the input node
  template<HedgehogStaticNode InputNode>
  constexpr void input(InputNode const &inputNode) {
    static_assert(traits::is_included_v<typename InputNode::inputs_t, typename GraphType::inputs_t>,
                  "The node can not be an input node, at least one of its input types should be the same of the graph input "
                  "types");
    registerNode(&inputNode);
    if (std::find(inputNodes_.cbegin(), inputNodes_.cend(), &inputNode) == inputNodes_.cend()) {
      inputNodes_.push_back(&inputNode);
    }
  }

  /// @brief Set hedgehog CXNode as output of the CXGraph. Register the node if needs be.
  /// @attention The CXNode should have the same output type defined in the OutputNode type, with the output type
  /// of the CXGraph defined in the GraphType
  /// @tparam OutputNode Type of the output node
  /// @param outputNode Instance of the output node
  template<HedgehogStaticNode OutputNode>
  requires std::is_same_v<typename OutputNode::output_t, typename GraphType::output_t>
  constexpr void output(OutputNode const &outputNode) {
    static_assert(std::is_same_v<typename OutputNode::output_t, typename GraphType::output_t>,
                  "The node can not be an output node, its output type should be the same of the graph output type");
    registerNode(&outputNode);
    if (std::find(outputNodes_.cbegin(), outputNodes_.cend(), &outputNode) == outputNodes_.cend()) {
      outputNodes_.push_back(&outputNode);
    }
  }

  /// @brief Create an edge between two CXNodes.
  /// @attention The SenderNode's output type should be part of the ReceiverNode's input types
  /// @tparam SenderNode Type of the sender node
  /// @tparam ReceiverNode Type of the receiver node
  /// @param sender Sender node instance
  /// @param receiver Receiver node instance
  template<HedgehogStaticNode SenderNode, HedgehogStaticNode ReceiverNode>
  requires std::is_base_of_v<hh::cx::behavior::AbstractNode, ReceiverNode>
  constexpr void addEdge(SenderNode const &sender, ReceiverNode const &receiver) {
    using sender_output_t = typename SenderNode::output_t;
    using receiver_inputs_t = typename ReceiverNode::inputs_t;
    using ro_receiver_inputs_t = typename ReceiverNode::ro_type_t;
    static_assert(traits::Contains_v<sender_output_t, receiver_inputs_t>,
                  "The given io cannot be linked to this io: No common types.");
    registerNode(&sender);
    registerNode(&receiver);
    adjacencyMatrix_.at(nodeId(&sender)).at(nodeId(&receiver)) = true;
    if constexpr(traits::Contains_v<sender_output_t, ro_receiver_inputs_t>){
      ROEdges_.at(nodeId(&sender)).at(nodeId(&receiver)) = true;
    }
  }

  /// @brief Add hedgehog test, to test the graph against
  /// @tparam UserTest Test type, should derive from the CXAbstractTest
  /// @param test instance of the test to add
  template<class UserTest>
  requires std::is_base_of_v<CXAbstractTest<GraphType, NodesNumber, LengthErrorMessage>, UserTest>
  constexpr void addTest(UserTest &test) {
    // Add the test if not already added
    if (std::find(tests_.cbegin(), tests_.cend(), &test) == tests_.cend()) {
      tests_.push_back(&test);
    }
  }

  /// @brief Test if there is an edge between the SenderNode and the ReceiverNode
  /// @tparam SenderNode Type of the sender node
  /// @tparam ReceiverNode Type of the receiver node
  /// @param sender Instance of the sender node
  /// @param receiver Instance of the receiver node
  /// @return True if there is an edge between the SenderNode and ReceiverNode, else False
  template<class SenderNode, class ReceiverNode>
  requires
  std::is_base_of_v<hh::cx::behavior::AbstractNode, SenderNode>
      && std::is_base_of_v<hh::cx::behavior::AbstractNode, ReceiverNode>
  constexpr bool isLinked(SenderNode *sender, ReceiverNode *receiver) const {
    auto
        idSender = nodeId(sender),
        idReceiver = nodeId(receiver);

    if (idSender == NodesNumber || idReceiver == NodesNumber) { return false; }
    return adjacencyMatrix_[idSender][idReceiver];
  }

  /// @brief Test if there is an edge between the id of the SenderNode and the id of the ReceiverNode
  /// @param idSender Sender's id
  /// @param idReceiver Receiver's id
  /// @return True if there is an edge between the SenderNode and ReceiverNode, else False
  /// @throw std::runtime_error if the node's ids have not been found
  [[nodiscard]] constexpr bool isLinked(size_t idSender, size_t idReceiver) const {
    if (idSender >= NodesNumber || idReceiver >= NodesNumber) {
      throw (std::runtime_error("The nodes you are trying to test do not exist in the graph."));
    }
    return adjacencyMatrix_[idSender][idReceiver];
  }

  /// @brief Get node's id
  /// @tparam Node Type of node
  /// @param node Node instance
  /// @return Node's id
  /// @throw std::runtime_error if the node has not been found
  template<class Node>
  requires std::is_base_of_v<hh::cx::behavior::AbstractNode, Node>
  constexpr size_t nodeId(Node *node) const {
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

  /// @brief Get hedgehog node from its id
  /// @param id Id of the node
  /// @return A pointer to AbstractNode linked to the id
  [[nodiscard]] constexpr hh::cx::behavior::AbstractNode const *node(size_t id) const {
    if (id >= registeredNodes_.size()) {
      throw (std::runtime_error("The node you are requesting does not exist."));
    } else {
      return registeredNodes_.at(id);
    }
  }

  /// @brief Get all the adjacent nodes to hedgehog node
  /// @param origin Sender node to get the adjacent list from
  /// @return  A vector of pointers to hh::cx::behavior::AbstractNode, that receives data from the origin node
  [[nodiscard]] constexpr vector_cx<hh::cx::behavior::AbstractNode const *, NodesNumber>
  adjacentNodes (hh::cx::behavior::AbstractNode const *origin) const {
    vector_cx<hh::cx::behavior::AbstractNode const *, NodesNumber> adjacentNodes{};
    auto senderId = nodeId(origin);
    for (size_t receiverId = 0; receiverId < NodesNumber; ++receiverId) {
      if (isLinked(senderId, receiverId)) {
        adjacentNodes.push_back(node(receiverId));
      }
    }

    return adjacentNodes;
  }

  /// @brief Get all the adjacent nodes id to hedgehog node id
  /// @param originId Sender node id to get the adjacent list from
  /// @return  A vector of ids, that receives data from the origin node id
  [[nodiscard]] constexpr vector_cx<size_t, NodesNumber> adjacentNodesId(size_t const originId) const {
    vector_cx<size_t, NodesNumber> neighborsId;
    if (originId < registeredNodes_.size()) {
      for (size_t receiverId = 0; receiverId < NodesNumber; ++receiverId) {
        if (isLinked(originId, receiverId)) {
          neighborsId.push_back(receiverId);
        }
      }
    } else {
      throw (std::runtime_error("The node you are trying to get neighbors from does not exist in the graph."));
    }
  }

  /// @brief Get all the receiving nodes which used the types sent only in hedgehog read only fashion
  /// @param origin Sender node to get the read only edges list from
  /// @return  A vector of pointers to hh::cx::behavior::AbstractNode, that receives data from the origin node and
  /// use the data in hedgehog read only fashion
  [[nodiscard]] constexpr vector_cx<hh::cx::behavior::AbstractNode const *, NodesNumber>
  ROEdges(hh::cx::behavior::AbstractNode const *origin) const {
    vector_cx<hh::cx::behavior::AbstractNode const *, NodesNumber> ROEdges{};
    auto senderId = nodeId(origin);
    for (size_t receiverId = 0; receiverId < NodesNumber; ++receiverId) {
      if (senderId >= NodesNumber || receiverId >= NodesNumber) {
        throw (std::runtime_error("The nodes you are trying to test do not exist in the graph."));
      }else if (ROEdges_.at(senderId).at(receiverId)) {
        ROEdges.push_back(node(receiverId));
      }
    }
    return ROEdges;
  }

 private:
  /// @brief Register hedgehog node
  /// @details A node is registered if it has not already been registered and if no other node with the same name has
  /// been registered.
  /// @param node Node to register
  constexpr void registerNode(hh::cx::behavior::AbstractNode const *node) {
    if (std::find(registeredNodes_.cbegin(), registeredNodes_.cend(), node) == registeredNodes_.cend()) {
      validateName(node);
      registeredNodes_.push_back(node);
    }
  }

  /// @brief Validate the name of the node to register
  /// @param node Node to check name
  /// @throw std::runtime_error if another node with the same name has already been registered
  constexpr void validateName(hh::cx::behavior::AbstractNode const *node) {
    if (std::any_of(registeredNodes_.cbegin(), registeredNodes_.cend(),
                    [&node](auto const & registeredNode){return node->name() == registeredNode->name();})) {
      throw std::runtime_error("Another node with the same name has already been registered.");
    }
  }

};
}
#endif //HEDGEHOG_CX_CX_GRAPH_H
