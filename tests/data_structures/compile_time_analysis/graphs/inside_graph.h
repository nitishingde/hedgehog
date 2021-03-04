//
// Created by Bardakoff, Alexandre (IntlAssoc) on 12/7/20.
//

#ifndef HEDGEHOG_INSIDE_GRAPH_H
#define HEDGEHOG_INSIDE_GRAPH_H
#include "../../../hedgehog/hedgehog.h"

class InsideGraph : public hh::Graph<int, int> {
 public:
  explicit InsideGraph(std::string_view const &name) : Graph(name) {
    auto node = std::make_shared<TaskIntInt>("Inside Task");
    this->input(node);
    this->output(node);
  }
};

#endif //HEDGEHOG_INSIDE_GRAPH_H
