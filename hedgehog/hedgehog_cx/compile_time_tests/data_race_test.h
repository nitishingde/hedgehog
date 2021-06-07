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
// Created by Bardakoff, Alexandre (IntlAssoc) on 9/11/20.
//

#ifndef HEDGEHOG_CX_TEST_CONSTNESS_BROADCAST_H
#define HEDGEHOG_CX_TEST_CONSTNESS_BROADCAST_H

#include <cstddef>
#include "../api_cx/cx_abstract_test.h"
#include "../../utils/concept.h"

/// @brief Hedgehog CX test namespace
namespace hh::cx::test {

/// @brief Graph's test to detect potential data races.
/// @details A data race will be detected if:
/// - A non const type is been sent to more than a CXNode
/// - All receivers use the data with a read-write policy
/// @tparam GraphType Graph type
/// @tparam NodesNumber Maximum number of nodes in the graph (will be deleted when the constexpr std::vector will be available)
/// @tparam LengthErrorMessage Maximum length error message (will be deleted when the constexpr std::vector will be
/// available)
template<hh::HedgehogDynamicGraphForStaticAnalysis GraphType, size_t NodesNumber = 20, size_t LengthErrorMessage = 255>
class DataRaceTest : public hh::cx::CXAbstractTest<GraphType, NodesNumber, LengthErrorMessage> {
 public:
  /// @brief Default constructor
  constexpr DataRaceTest() : hh::cx::CXAbstractTest<GraphType, NodesNumber, LengthErrorMessage>("Data races test") {}
  /// @brief Default destructor
  constexpr virtual ~DataRaceTest() = default;
 private:
  /// @brief Run the data race test on the graph
  /// @param graph Graph instance to test
  constexpr virtual void test(hh::cx::CXGraph<GraphType, NodesNumber, LengthErrorMessage> const *graph) {
    this->errorMessage_.push_back("Potential data races found between these nodes:");
    for (auto elem : graph->registeredNodes()) {
      if (!elem->isOutputConst()) {
        auto adjacentNodes = graph->adjacentNodes(elem);
        auto ROEdges = graph->ROEdges(elem);
        if (adjacentNodes.size() > 1 && adjacentNodes.size() != ROEdges.size()) {
          this->errorMessage_.push_back("\n\t");
          this->errorMessage_.push_back(elem->name());
          this->errorMessage_.push_back("->");
          for (auto ROadjacentNode : adjacentNodes) {
            this->errorMessage_.push_back(ROadjacentNode->name());
            this->errorMessage_.push_back("/");
          }
          this->errorMessage_.pop_back();
        }
      }
    }
    if (this->errorMessage_.size() == 1) {
      this->isGraphValid(true);
    } else {
      this->isGraphValid(false);
    }
  }
};
}

#endif //HEDGEHOG_CX_TEST_CONSTNESS_BROADCAST_H
