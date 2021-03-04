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

#ifndef HEDGEHOG_CX_CX_ABSTRACT_TEST_H
#define HEDGEHOG_CX_CX_ABSTRACT_TEST_H

#include <string_view>
#include <string>
#include "cx_graph.h"

/// @brief Hedgehog CX main namespace
namespace hh::cx {

/// @brief Pure abstract class to define test for the compile-time analysis.
/// @details To define hedgehog test, the method
/// hh::cx::CXAbstractTest<GraphType, NodesNumber>::test(CXGraph<GraphType, NodesNumber> const *graph) needs to
/// be defined. The test will be run by calling the test method. Then, the method isGraphValid() will be invoked to
/// check if the graph passed the test or not. It the graph passed the test it will be considered as "valid". If the
/// graph did not pass the test, it should be marked as "not valid" and the error message will be reported.
/// @tparam GraphType Graph type
/// @tparam NodesNumber Maximum number of nodes in the graph (will be deleted when the constexpr std::vector will be available)
template<HedgehogDynamicGraphForStaticAnalysis GraphType, size_t NodesNumber = 20>
class CXAbstractTest {
 private:
  std::string_view
      testName_; ///< Name of the test

  bool
      isGraphValid_ = false; ///< Validity of the graph against the test

 protected:
  vector_cx<std::string_view, 255>
      errorMessage_; ///< Error message

 public:
  /// @brief Abstract Test
  /// @param testName Name of the test
  constexpr explicit CXAbstractTest(std::string_view const &testName) : testName_(testName), errorMessage_({}) {}
  /// @brief Abstract Test Default destructor
  constexpr virtual ~CXAbstractTest() = default;
  /// @brief Test implementation. This method should test the graph, set if the graph is valid against the test and set
  /// the error message if not.
  /// @param graph Graph to test
  constexpr virtual void test(CXGraph<GraphType, NodesNumber> const *graph) = 0;
  /// @brief Graph validity setter
  /// @param isGraphValid Set if the graph is valid against the test or not
  constexpr void isGraphValid(bool isGraphValid) { isGraphValid_ = isGraphValid; }
  /// @brief Graph validity getter
  /// @return Get if the graph is valid against the test or not
  [[nodiscard]] constexpr bool isGraphValid() const { return isGraphValid_; }
  /// @brief Test name getter
  /// @return Test name
  [[nodiscard]] constexpr std::string_view const &testName() const { return testName_; }
  /// @brief Error message getter. The error message is represented by hedgehog vector of words. These words will be
  /// separated by spaces when reported.
  /// @return Error message
  [[nodiscard]] constexpr vector_cx<std::string_view, 255> const &errorMessage() const { return errorMessage_; }
};

}

#endif //HEDGEHOG_CX_CX_ABSTRACT_TEST_H
