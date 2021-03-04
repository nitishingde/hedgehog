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
// Created by Bardakoff, Alexandre (IntlAssoc) on 11/9/20.
//

#ifndef HEDGEHOG_CX_SENDER_VISITOR_H
#define HEDGEHOG_CX_SENDER_VISITOR_H

#include "receiver_visitor.h"

/// @brief Hedgehog CX behavior namespace
namespace hh::cx::behavior {

/// @brief Functor used to create an edge by first finding the sender
/// @tparam DynamicVariant Variant type that holds a dynamic node
/// @tparam SpecializedGraph Type of the graph
/// @tparam StaticVariants Type of vector of static variants
template<class DynamicVariant, class SpecializedGraph, class StaticVariants>
struct SenderVisitor {
  size_t const &senderPositionInVariants_; ///< Node's position in vector of variants
  DynamicVariant &dynVariants_; ///< Vector of variants containing the dynamic nodes
  std::shared_ptr<SpecializedGraph> graph_; ///< Graph to set the edge
  std::string_view const &
      senderNodeName_, ///< Name of the sender's name
      receiverNodeName_; ///< Name of the receiver's name
  StaticVariants const &staticVariants_; ///< Vector of variants containing the static nodes
  bool &senderFound_; ///< Mutable bool to return if the sender node has been found

  /// @brief Create the sender functor with context element
  /// @param senderPositionInVariants Position of the sender node in the vector of variants
  /// @param dynVariants Vector of variants containing the dynamic nodes
  /// @param graph Graph to set the edge
  /// @param senderNodeName Name of the sender name
  /// @param receiverNodeName Name of the receiver name
  /// @param staticVariants Vector of variants containing the static nodes
  /// @param senderFound Mutable bool used to inform the sender has been found
  constexpr SenderVisitor(size_t const &senderPositionInVariants,
                          DynamicVariant &dynVariants,
                          std::shared_ptr<SpecializedGraph> graph,
                          std::string_view const &senderNodeName,
                          std::string_view const &receiverNodeName,
                          StaticVariants const &staticVariants,
                          bool &senderFound)
      : senderPositionInVariants_(senderPositionInVariants),
        dynVariants_(dynVariants),
        graph_(graph),
        senderNodeName_(senderNodeName),
        receiverNodeName_(receiverNodeName),
        staticVariants_(staticVariants),
        senderFound_(senderFound) {}

  /// @brief Functor call operator definition
  /// @tparam StaticVariantType Type of the variant that holds the static node
  /// @param staticVariantNode Instance of the variant that holds the static node
  /// @throw std::runtime_error if the static node and the dynamic node have the same name
  template<class StaticVariantType>
  constexpr void operator()(StaticVariantType const &staticVariantNode) {
    if constexpr (!std::is_same_v<StaticVariantType, std::monostate>) {
      using DynamicNodeType = typename StaticVariantType::dynamic_node_t;
      if (staticVariantNode.name() == senderNodeName_) {
        if (auto
            senderNode = std::get_if<std::shared_ptr<DynamicNodeType>>(&(dynVariants_.at(senderPositionInVariants_)))) {

          senderFound_ = true;

          size_t receiverPositionInVariants = 0;
          bool receiverFound = false;
          for (auto staticVariant : staticVariants_) {
            std::visit(
                ReceiverVisitor{dynVariants_.at(receiverPositionInVariants),
                                graph_,
                                receiverNodeName_,
                                *senderNode,
                                receiverFound},
                staticVariant);
            if (receiverFound) { break; }
            else { ++receiverPositionInVariants; }
          }
        } else {
          throw std::runtime_error("Internal error: Miss match in mapping static - dynamic nodes");
        }

      }
    }
  }

};
}
#endif //HEDGEHOG_CX_SENDER_VISITOR_H
