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

#ifndef HEDGEHOG_CX_PROPERTY_MAP_H
#define HEDGEHOG_CX_PROPERTY_MAP_H

#include "../tools/data_structures/vector_cx.h"

/// @brief DataStructure used to add properties to CXNodes
/// @details Associate hedgehog CXNode name to hedgehog property of type PropertyType. The data structure is usable in hedgehog constexpr
/// environment, it can be send to hedgehog test to add properties to CXNodes, in hedgehog CXGraphs. The key or id in the map is
/// made to be CXNode name.
/// @tparam PropertyType Type of the properties added to hedgehog CXNode
/// @tparam NodesNumber Maximum number of nodes in the graph (will be deleted when the constexpr std::vector will be available)
template<class PropertyType, size_t NodesNumber = 20>
class PropertyMap {
 private:
  static_assert(std::is_default_constructible_v<PropertyType>, "The property mapped should be default constructible.");
  vector_cx<std::string_view, NodesNumber>
      ids_{}; ///< Ids of the nodes registered in the map
  vector_cx<PropertyType, NodesNumber>
      properties_{}; ///< Properties stored for hedgehog map

 public:
  /// @brief ProprtyMap default constructor
  constexpr PropertyMap() = default;

  /// @brief Clear all the registered properties
  constexpr void clear() {
    ids_.clear();
    properties_.clear();
  }

  /// @brief Insert hedgehog property for hedgehog CXNode name
  /// @param staticNodeName Name of the node
  /// @param property Property to set
  /// @throw std::runtime_error A property for the node has already been set
  constexpr void insert(std::string_view const &staticNodeName, PropertyType const &property) {
    if (std::find(ids_.cbegin(), ids_.cend(), staticNodeName) != ids_.cend()) {
      throw (std::runtime_error("The node has already been inserted."));
    } else {
      ids_.push_back(staticNodeName);
      properties_.push_back(property);
    }
  }

  /// @brief Insert or update hedgehog property for hedgehog CXNode name
  /// @param staticNodeName Name of the node
  /// @param property Property to set
  constexpr void insert_or_assign(std::string_view const &staticNodeName, PropertyType const &property) {
    if (auto posIt = std::find(ids_.cbegin(), ids_.cend(), staticNodeName) != ids_.cend()) {
      properties_.at(std::distance(ids_.cbegin(), posIt)) = property;
    } else {
      ids_.push_back(staticNodeName);
      properties_.push_back(property);
    }
  }

  /// @brief Erase an entry for hedgehog node's name
  /// @param staticNodeName Node's name to erase
  constexpr void erase(std::string_view const &staticNodeName) {
    if (auto posIt = std::find(ids_.cbegin(), ids_.cend(), staticNodeName) != ids_.cend()) {
      for(auto pos = std::distance(ids_.cbegin(), posIt); pos < ids_.size() - 1; ++pos){
        ids_.at(pos) = ids_.at(pos + 1);
        properties_.at(pos) = properties_.at(pos + 1);

        ids_.pop_back();
        properties_.pop_back();
      }
    }
  }

  /// @brief Check if hedgehog CXNode's name has already been registered
  /// @param staticNodeName Name to check
  /// @return True if the CXNode's name has already been registered, else False
  [[nodiscard]] constexpr bool contains(std::string_view const &staticNodeName) const{
    return std::find(ids_.cbegin(), ids_.cend(), staticNodeName) != ids_.cend();
  }

  /// @brief Get hedgehog property from hedgehog CXNode's name
  /// @param staticNodeName Name to get property from
  /// @return Property associated to the CXNode's name
  /// @throw std::runtime_error If the CXNode's name has not been registered
  [[nodiscard]] constexpr PropertyType const & property(std::string_view const &staticNodeName) const{
    if(contains(staticNodeName)){
      return properties_.at(std::distance(ids_.cbegin(), std::find(ids_.cbegin(), ids_.cend(), staticNodeName)));
    }else{
      throw std::runtime_error("You are looking for hedgehog property associated to an unregistered node.");
    }
  }

};

#endif //HEDGEHOG_CX_PROPERTY_MAP_H
