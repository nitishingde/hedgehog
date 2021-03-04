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

#ifndef HEDGEHOG_CX_CX_NODE_H
#define HEDGEHOG_CX_CX_NODE_H

#include "../behavior_cx/abstract_node.h"
#include "../../utils/concept.h"
#include "../../utils/helper.h"

/// @brief Hedgehog CX main namespace
namespace hh::cx {

/// @brief Compile time node representation.
/// @details It represents hedgehog Hedgehog dynamic node of type HedgehogDynamicNodeType. For the data races test, it is
/// possible to declare some input types as read only. It can be set as input or output of the graph and an edge can
/// be set between two nodes.
/// @attention In hedgehog graph, all CXNodes should have unique names
/// @tparam HedgehogDynamicNodeType Type of dynamic node
/// @tparam InputTypesRO Input types designed as Read only
template<HedgehogConnectableNode HedgehogDynamicNodeType, class ...InputTypesRO>
class CXNode : public hh::cx::behavior::AbstractNode {
 public:
  using ro_type_t = std::tuple<InputTypesRO...>; ///< Types designed as read only
  using dynamic_node_t = HedgehogDynamicNodeType; ///< Dynamic node type represented byt he CXNode
  using inputs_t = typename HedgehogDynamicNodeType::inputs_t; ///< Node's inputs type
  using output_t = typename HedgehogDynamicNodeType::output_t; ///< Node's output type
  static_assert(
  (traits::Contains_v<InputTypesRO, inputs_t> && ...), "The inputs set as RO should be in the input node list.");

  /// @brief CXNode constructor
  /// @param name Unique name of the CXNode
  explicit constexpr CXNode(std::string_view const &name) : AbstractNode(name) {};

  /// @brief CXNode destructor
  virtual ~CXNode() = default;

  /// @brief Test if the output type is const
  /// @return True if the output type is const, else False
  [[nodiscard]] constexpr bool isOutputConst() const final {
    return std::is_const_v<typename HedgehogDynamicNodeType::output_t>;
  }

  /// @brief Test if the canTerminate method has been overloaded in the dynamic node type
  /// @return True, if canTerminate method has been overloaded in the dynamic node type, else False
  [[nodiscard]] constexpr bool isCanTerminateOverloaded() const final {
    if constexpr (
        std::is_base_of_v<
            typename helper::HelperAbstractTaskType<typename HedgehogDynamicNodeType::output_t,
                                                    typename HedgehogDynamicNodeType::inputs_t>::type,
            HedgehogDynamicNodeType>) {
      using BaseTaskType =
      typename helper::HelperAbstractTaskType<
          typename HedgehogDynamicNodeType::output_t,
          typename HedgehogDynamicNodeType::inputs_t>::type;

      if constexpr (!std::is_same_v<decltype(&BaseTaskType::canTerminate), decltype(&dynamic_node_t::canTerminate)>) {
        return true;
      }
    }
    return false;
  };
};

}

#endif //HEDGEHOG_CX_CX_NODE_H
