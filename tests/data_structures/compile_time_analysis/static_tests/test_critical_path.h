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
// Created by Bardakoff, Alexandre (IntlAssoc) on 12/4/20.
//

#ifndef HEDGEHOG_TEST_CRITICAL_PATH_H
#define HEDGEHOG_TEST_CRITICAL_PATH_H

#include "../../../hedgehog/hedgehog.h"

template<class GraphType, size_t NodesNumber = 20>
class TestCriticalPath : public hh::cx::CXAbstractTest<GraphType, NodesNumber> {
 private:
  double_t
      maxPathValue_ = 0,
      currentPathValue_ = 0;

  PropertyMap<double, NodesNumber>
      propertyMap_;

  hh::cx::CXGraph<GraphType, NodesNumber> const
      *graph_ = nullptr;

  vector_cx<hh::cx::behavior::AbstractNode const *, NodesNumber>
      criticalVector_{},
      visited_{};

 public:
  constexpr explicit TestCriticalPath(PropertyMap<double, NodesNumber> const &propertyMap)
      : hh::cx::CXAbstractTest<GraphType, NodesNumber>("Critical Path"), propertyMap_(propertyMap) {}

  constexpr ~TestCriticalPath() override = default;

  constexpr void test(hh::cx::CXGraph<GraphType, NodesNumber> const *graph) override {
    graph_ = graph;

    for (auto inputNode : graph->inputNodes()) {
      visitNode(inputNode);
    }

    this->isGraphValid(false);
    this->errorMessage_.push_back("The critical path is:\n\t");
    for (auto node : criticalVector_) {
      this->errorMessage_.push_back(node->name());
      this->errorMessage_.push_back(" -> ");
    }
    this->errorMessage_.pop_back();
  }

 private:
  constexpr void visitNode(hh::cx::behavior::AbstractNode const *node) {
    if (!cxVectorContain(visited_, node)) {
      currentPathValue_ += propertyMap_.property(node->name());
      visited_.push_back(node);

      if (cxVectorContain(graph_->outputNodes(), node)) {
        if (currentPathValue_ > maxPathValue_) {
          maxPathValue_ = currentPathValue_;
          criticalVector_.clear();
          criticalVector_ = visited_;
        }
      }

      for (auto neighbor : graph_->adjacentNodes(node)) {
        visitNode(neighbor);
      }

      currentPathValue_ -= propertyMap_.property(node->name());
      visited_.pop_back();
    }
  }

  template<class T, size_t VECTORSIZE>
  constexpr bool cxVectorContain(vector_cx<T, VECTORSIZE> const &v, T const &val) {
    return std::find(v.cbegin(), v.cend(), val) != v.cend();
  }

};

#endif //HEDGEHOG_TEST_CRITICAL_PATH_H
