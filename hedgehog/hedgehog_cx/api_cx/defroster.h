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
// Created by Bardakoff, Alexandre (IntlAssoc) on 11/20/20.
//

#ifndef HEDGEHOG_CX_DEFROSTER_H
#define HEDGEHOG_CX_DEFROSTER_H

#include <sstream>
#include "../../utils/concept.h"
#include "../tools/visitors/input_visitor.h"
#include "../tools/visitors/output_visitor.h"
#include "../tools/visitors/sender_visitor.h"

/// @brief Hedgehog CX main namespace
namespace hh::cx {

#ifndef DOXYGEN_SHOULD_SKIP_THIS
/// @brief Graph representation used during the compile-time analysis.
/// @tparam GraphType Dynamic graph type represented by the CXGraph
/// @tparam NodesNumber Maximum number of nodes in the graph (will be deleted when the constexpr std::vector will be available)
/// @tparam LengthErrorMessage Maximum length error message (will be deleted when the constexpr std::vector will be
/// available)
template<HedgehogDynamicGraphForStaticAnalysis GraphType, size_t NodesNumber, size_t LengthErrorMessage>
class CXGraph;
#endif //DOXYGEN_SHOULD_SKIP_THIS

/// @brief The defroster has an analyser role during the compile time analysis and will convert the compile-time
/// representation to the Hedgehog dynamic Graph. At construction, the structural information from the CXGraph will
/// be extracted and the compile_time_tests will be invoked. The validity of the graph and the error message will be updated
/// consequently. Finally, the defroster is used to convert its internal representation of the CXGraph to hedgehog dynamic
/// Hedgehog graph.
/// @tparam GraphType Graph type
/// @tparam NodesNumber Maximum number of nodes in the graph (will be deleted when the constexpr std::vector will be available)
/// @tparam LengthErrorMessage Maximum length error message (will be deleted when the constexpr std::vector will be
/// available)
template<HedgehogDynamicGraphForStaticAnalysis GraphType, size_t NodesNumber = 20, size_t LengthErrorMessage = 255>
class Defroster {
 private:
  bool
      isValid_ = true; ///< Validity of the graph after the test ran

  std::string_view
      graphName_{}; ///< Graph's name to be tested

  vector_cx<std::pair<std::string_view, vector_cx<std::string_view, LengthErrorMessage>>, 255>
      errorMessages_; ///< List of error messages from the different compile_time_tests

  vector_cx<std::string_view, NodesNumber>
      registeredNodesName_{}, ///< Registered nodes' name, position used as id for the adjacencyMatrix
      inputNodesName_{}, ///< Input node's name
      outputNodesName_{}; ///< Output node's name

  std::array<std::array<bool, NodesNumber>, NodesNumber>
      adjacencyMatrix_{}; ///< Graph's adjacency matrix

 public:
  /// @brief Defroster constructor. At construction, the structural information from the CXGraph will
  /// be extracted and the compile_time_tests will be invoked. The validity of the graph and the error message will be updated
  /// consequently.
  /// @param g The Graph to defrost
  constexpr explicit Defroster(CXGraph<GraphType, NodesNumber, LengthErrorMessage> const &g) : adjacencyMatrix_(g.adjacencyMatrix_){
    graphName_ = g.name();

    // Copy all the internal representation
    for (auto inputNode : g.inputNodes_) { inputNodesName_.push_back(inputNode->name()); }
    for (auto outputNode : g.outputNodes_) { outputNodesName_.push_back(outputNode->name()); }
    for (auto node : g.registeredNodes()) { registeredNodesName_.push_back(node->name()); }

    // Run the compile_time_tests and update the graph status and the error message
    for (auto test : g.tests_) {
      if (test) {
        test->test(&g);
        if (!test->isGraphValid()) {
          isValid_ = false;
          errorMessages_.push_back({test->testName(), test->errorMessage()});
        }
      }
    }
  }

  /// @brief Getter to the graph validity against the compile_time_tests.
  /// @return True if the graph passed all the compile_time_tests, False if errors were founds (i.e. Cycles / Data races found)
  [[nodiscard]] constexpr bool isGraphValid() const { return isValid_; }

  /// @brief Report the error messages from the compile_time_tests.
  /// @return A formatted std::string with all the error messages concatenated.
  [[nodiscard]] std::string report() const {
    std::ostringstream oss;
    oss << "In graph " << graphName_ << ":\n";
    for (auto[testName, errorMessage] : errorMessages_) {
      oss << "\t" << testName << ": ";
      for (auto message : errorMessage) {
        oss << message << " ";
      }
      oss << "\n";
    }
    return oss.str();
  }

  /// @brief Convert hedgehog representation of the CXGraph to hedgehog dynamic hh::Graph thanks to the nodes instances in parameter.
  /// @details "The convert function need an even number of arguments, following the pattern: instanceCXNode1,
  /// instanceDynamicNode1, instanceCXNode2, instanceDynamicNode2..."
  /// @tparam Args Types of the mapped node.
  /// @param args The parameters should follow the pattern: instanceCXNode1, instanceDynamicNode1, instanceCXNode2,
  /// instanceDynamicNode2 ect.. Their numbers should be even.
  /// @return A dynamic hh::Graph created from the CXGraph structure and the mapped dynamic instances.
  template<class ...Args>
  [[nodiscard]] constexpr auto convert(Args const &... args) const {
    validateAll(args...);
    auto[staticNodes, dynamicNodes] = splitter(args...);
    return generateDynamicGraph(staticNodes, dynamicNodes);
  }

 private:
  /// @brief Validate the instances to be mapped
  /// @tparam Args Types of the mapped node
  /// @param args The parameters should follow the pattern: instanceCXNode1, instanceDynamicNode1, instanceCXNode2,
  /// instanceDynamicNode2 ect.. Their numbers should be even.
  template<class ...Args>
  constexpr void validateAll(Args const &... args) const {
    static_assert(sizeof...(Args) % 2 == 0,
                  "The map function need an even number of arguments, following the pattern: instanceCXNode1, "
                  "instanceDynamicNode1, instanceCXNode2, instanceDynamicNode2...");
    validate(args...);
  }

  /// @brief Validate hedgehog pair of hedgehog static node of type StaticNodeType and hedgehog dynamic node of type DynamicNodeType.
  /// @tparam StaticNodeType Type of hedgehog static node. It should be compliant with the HedgehogStaticNode concept.
  /// @tparam DynamicNodeType Type of hedgehog dynamic node. It should be compliant with the HedgehogConnectableNode concept.
  /// @tparam Args Following type to be validated
  /// @param args Instances to be validated
  template<HedgehogStaticNode StaticNodeType, HedgehogConnectableNode DynamicNodeType, class ...Args>
  constexpr void
  validate(StaticNodeType const &, std::shared_ptr<DynamicNodeType> const &, Args const &... args) const {
    static_assert(
        std::is_same_v<typename StaticNodeType::dynamic_node_t, DynamicNodeType>,
        "The dynamic node type should correspond to the type associated with the static node."
    );
    // Validate the next pair
    if constexpr (sizeof...(Args) > 0) { validate(args...); }
  }

  /// @brief Create hedgehog pair of tuple, the first containing the static node, the second containing the dynamic node
  /// @tparam StaticNodeType Type of the static node
  /// @tparam DynamicNodeType Type of the dynamic node
  /// @param staticNode Instance of the static node
  /// @param dynamicNode Instance of the dynamic node
  /// @return A pair of tuple, the first containing the StaticNodeType, the second containing the DynamicNodeType
  template<class StaticNodeType, class DynamicNodeType>
  constexpr auto splitter(StaticNodeType staticNode, DynamicNodeType dynamicNode) const {
    return std::make_pair(std::tuple<StaticNodeType>(staticNode), std::tuple<DynamicNodeType>(dynamicNode));
  }

  /// @brief Create hedgehog pair, the first contains all the static node, the second contains all the dynamic node
  /// @tparam StaticNodeType Type of the static node
  /// @tparam DynamicNodeType Type of the dynamic node
  /// @tparam Args Following types to be split
  /// @param staticNode Current instance of the static node
  /// @param dynamicNode Current instance of the dynamic node
  /// @param args Following instances to be split
  /// @return A pair of tuples containing for the first element all the static nodes, for the second element all the
  /// dynamic nodes
  template<class StaticNodeType, class DynamicNodeType, class... Args>
  constexpr auto splitter(StaticNodeType staticNode, DynamicNodeType dynamicNode, Args... args) const {
    auto[prevStaticTuples, prevDynamicTuples] = splitter(args...);
    return std::make_pair(
        std::tuple_cat(std::tuple<StaticNodeType>(staticNode), prevStaticTuples),
        std::tuple_cat(std::tuple<DynamicNodeType>(dynamicNode), prevDynamicTuples));
  }


  /// @brief Create hedgehog vector of variants from hedgehog tuple
  /// @tparam Position Position in the tuple
  /// @tparam Tuple Tuple type to transform to hedgehog vector of variants
  /// @tparam VectorVariant Vector of variants type to construct
  /// @param tup Tuple to transform to hedgehog vector of variants
  /// @param vectorVariant Vector of variants to construct
  template<size_t Position = 0, class Tuple, class VectorVariant>
  void populateVariants(Tuple const &tup, VectorVariant &vectorVariant) const {
    if constexpr (Position < std::tuple_size_v<Tuple>) {
      vectorVariant.emplace_back(std::get<Position>(tup));
      populateVariants<Position + 1, Tuple, VectorVariant>(tup, vectorVariant);
    }
  }

  /// @brief Generate the dynamic graph from the vectors of tuples of static nodes and dynamic nodes.
  /// @tparam StaticTuple Vector of tuple static nodes type
  /// @tparam DynamicTuple Vector of tuple dynamic nodes type
  /// @param staticTuple Vector of tuple static nodes
  /// @param dynamicTuple Vector of tuple dynamic nodes
  /// @return The Hedgehog dynamic graph
  template<class StaticTuple, class DynamicTuple>
  std::shared_ptr<GraphType> generateDynamicGraph(StaticTuple &staticTuple, DynamicTuple &dynamicTuple) const {
    size_t
        positionInVariants = 0;
    bool
        senderFound = false,
        inputFound = false,
        outputFound = false;

    auto graph = std::make_shared<GraphType>(graphName_);
    std::vector<typename hh::helper::getUniqueVariantFromTuple<StaticTuple>::type>
        staticVariants;
    std::vector<typename hh::helper::getUniqueVariantFromTuple<DynamicTuple>::type>
        dynamicVariants;

    // Transform the tuples to the variants
    populateVariants(staticTuple, staticVariants);
    populateVariants(dynamicTuple, dynamicVariants);

    // Set the input nodes
    for (auto &inputNodeName : inputNodesName_) {
      positionInVariants = 0;
      inputFound = false;
      for (auto staticVariant : staticVariants) {
        std::visit(
            hh::cx::behavior::InputVisitor{ dynamicVariants.at(positionInVariants), graph, inputNodeName, inputFound},
            staticVariant);
        if (inputFound) { break; }
        else { ++positionInVariants; }
      }
    }

    // Set the output nodes
    for (auto &outputNodeName : outputNodesName_) {
      positionInVariants = 0;
      outputFound = false;
      for (auto staticVariant : staticVariants) {
        std::visit(hh::cx::behavior::OutputVisitor{dynamicVariants.at(positionInVariants), graph, outputNodeName,
                                                   outputFound},
                   staticVariant);
        ++positionInVariants;
      }
    }

    // Set the edges
    for (size_t posSender = 0; posSender < registeredNodesName_.size(); ++posSender) {
      std::string_view senderName = registeredNodesName_.at(posSender);
      for (size_t posReceiver = 0; posReceiver < registeredNodesName_.size(); ++posReceiver) {
        if (adjacencyMatrix_.at(posSender).at(posReceiver)) {
          std::string_view receiverName = registeredNodesName_.at(posReceiver);
          senderFound = false;
          positionInVariants = 0;
          for (auto staticVariant : staticVariants) {
            std::visit(
                hh::cx::behavior::SenderVisitor(positionInVariants,
                                                dynamicVariants,
                                                graph,
                                                senderName,
                                                receiverName,
                                                staticVariants,
                                                senderFound),
                staticVariant);
            if (senderFound) { break; }
            ++positionInVariants;
          }
        }
      }
    }
    return graph;
  }

};
}

#endif //HEDGEHOG_CX_DEFROSTER_H
