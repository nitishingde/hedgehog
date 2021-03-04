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

#ifndef HEDGEHOG_CX_ABSTRACT_NODE_H
#define HEDGEHOG_CX_ABSTRACT_NODE_H

#include <string_view>
#include <iostream>
#include <type_traits>
#include "../tools/data_structures/vector_cx.h"

/// @brief Hedgehog CX behavior namespace
namespace hh::cx::behavior {

/// @brief CXNode abstraction. Used to get hedgehog uniform type to store all CXNode in the CXGraph for the compile-time
/// analysis.
/// @attention It should NOT be used or derived by an end user. Use CXNode instead.
class AbstractNode {
 private:
  std::string_view name_; ///< Name of the node
 public:
  /// @brief AbstractNode constructor
  /// @param name Node's name
  constexpr AbstractNode(std::string_view const &name) : name_(name) {}

  /// @brief Name accessor
  /// @return Node's name
  [[nodiscard]] constexpr std::string_view const &name() const { return name_; }

  /// @brief Test if the output type is cont
  /// @return True if the output type is const, else False
  [[nodiscard]] constexpr virtual bool isOutputConst() const = 0;

  /// @brief Test if the canTerminate method has been overloaded in the dynamic node type
  /// @return True, if canTerminate method has been overloaded in the dynamic node type, else False
  [[nodiscard]] constexpr virtual bool isCanTerminateOverloaded() const { return  false;};
};

}

#endif //HEDGEHOG_CX_ABSTRACT_NODE_H
