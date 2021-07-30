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
// Created by Bardakoff, Alexandre (IntlAssoc) on 10/2/20.
//

#ifndef HEDGEHOG_CX_CONCEPT_H
#define HEDGEHOG_CX_CONCEPT_H

#include "traits.h"
#include "../hedgehog_cx/behavior_cx/abstract_node.h"

/// @brief Hedgehog main namespace
namespace hh {

/// @brief Concept to define a compile-time node (CXNode)
/// @tparam StaticHedgehogNode Type to test
template<typename StaticHedgehogNode>
concept HedgehogStaticNode = requires{
  std::is_base_of_v<hh::cx::behavior::AbstractNode, StaticHedgehogNode>;
  typename StaticHedgehogNode::ro_type_t;         // These types are defined in the CXNode abstraction. CXNode should be
  typename StaticHedgehogNode::dynamic_node_t;    // used directly or derived to create hedgehog specialize node.
  typename StaticHedgehogNode::inputs_t;          // AbstractNode should NOT be used directly or derived.
  typename StaticHedgehogNode::output_t;
};

/// @brief Basic definition of an Hedgehog Node
/// @tparam DynamicHedgehogNode Type to test against
template<typename DynamicHedgehogNode>
concept HedgehogNode = std::is_base_of_v<hh::behavior::Node, DynamicHedgehogNode>;

/// @brief Definition if an Hedgehog node can receive data
/// @tparam DynamicHedgehogNode Type to test against
template<typename DynamicHedgehogNode>
concept HedgehogMultiReceiver = HedgehogNode<DynamicHedgehogNode>
    && std::is_base_of_v<
        typename hh::helper::HelperMultiReceiversType<typename DynamicHedgehogNode::inputs_t>::type,
        DynamicHedgehogNode
    >;

/// @brief Definition if asn Hedgehog node can send data
/// @tparam DynamicHedgehogNode Type to test against
template<typename DynamicHedgehogNode>
concept HedgehogSender = HedgehogNode<DynamicHedgehogNode>
    && std::is_base_of_v<
        hh::behavior::Sender<typename DynamicHedgehogNode::output_t>,
        DynamicHedgehogNode
    >;

/// @brief Concept to define a dynamic node that can be connected to other (send and receive data)
/// @tparam DynamicHedgehogNode Type to test
template<typename DynamicHedgehogNode>
concept HedgehogConnectableNode = HedgehogNode<DynamicHedgehogNode>
    && HedgehogMultiReceiver<DynamicHedgehogNode>
    && HedgehogSender<DynamicHedgehogNode>;

/// @brief Concept to define a dynamic graph for the static analysis
/// @details The dynamic Graph's type should have a constructor with only a name
/// (DynamicHedgehogNode(std::string_view const &))
/// @tparam DynamicHedgehogNode Type to test
template<typename DynamicHedgehogNode>
concept HedgehogDynamicGraphForStaticAnalysis = HedgehogConnectableNode<DynamicHedgehogNode>
    // A constructor with only a name(DynamicHedgehogNode(std::string_view const &)) needs to be defined
    && std::is_base_of_v<typename hh::helper::HelperGraphType<typename DynamicHedgehogNode::output_t, typename
    DynamicHedgehogNode::inputs_t>::type, DynamicHedgehogNode>
    && std::is_constructible_v<DynamicHedgehogNode, std::string_view const &>;
}

#endif // HEDGEHOG_CX_CONCEPT_H