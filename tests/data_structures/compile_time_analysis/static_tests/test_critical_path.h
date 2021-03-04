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
