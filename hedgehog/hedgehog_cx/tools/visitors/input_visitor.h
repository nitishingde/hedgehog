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

#ifndef HEDGEHOG_CX_INPUT_VISITOR_H
#define HEDGEHOG_CX_INPUT_VISITOR_H

#include <string_view>
#include <memory>
#include <variant>

/// @brief Hedgehog CX behavior namespace
namespace hh::cx::behavior {

/// @brief Functor used to set a node as graph's input from a static node hold in a variant
/// @tparam DynamicVariant Variant type that holds a dynamic node to set as input of the graph
/// @tparam SpecializedGraph Type of the graph
template<class DynamicVariant, class SpecializedGraph>
class InputVisitor {
 private:
  DynamicVariant &dynVariant_;                ///< Dynamic Node hold in a variant
  std::shared_ptr<SpecializedGraph> graph_{}; ///< Graph to set the node as input
  std::string_view const &nodeName_;          ///< Name of the node
  bool &inputFound_;                          ///< Mutable bool to return if the input has been found

 public:
  /// @brief Create the input functor with context element
  /// @param dynVariant Dynamic node hold in a variant
  /// @param graph Dynamic graph
  /// @param nodeName Name of the node to set as input
  /// @param inputFound Mutable bool used to inform the input has been found
  constexpr InputVisitor(DynamicVariant &dynVariant, std::shared_ptr<SpecializedGraph> graph,
                         std::string_view const &nodeName, bool &inputFound)
      : dynVariant_(dynVariant), graph_(graph), nodeName_(nodeName), inputFound_(inputFound) {}

  /// @brief Functor call operator definition
  /// @tparam StaticVariantType Type of the variant that holds the static node
  /// @param staticVariantNode Instance of the variant that holds the static node
  /// @throw std::runtime_error if the static node and the dynamic node have the same name
  template<class StaticVariantType>
  constexpr void operator()(StaticVariantType const &staticVariantNode) {
    if constexpr (!std::is_same_v<StaticVariantType, std::monostate>) {
      using DynamicNodeType = typename StaticVariantType::dynamic_node_t;
      if constexpr (traits::is_included_v < typename DynamicNodeType::inputs_t, typename SpecializedGraph::inputs_t >) {
        if (staticVariantNode.name() == nodeName_) {
          if (auto dynNode = std::get_if<std::shared_ptr<DynamicNodeType>>(&(dynVariant_))) {
            inputFound_ = true;
            this->graph_->input(*dynNode);
          } else {
            throw std::runtime_error("Internal error: Miss match in mapping static - dynamic nodes");
          }
        }
      }
    }
  }

};
}
#endif //HEDGEHOG_CX_INPUT_VISITOR_H
