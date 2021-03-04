//
// Created by Bardakoff, Alexandre (IntlAssoc) on 12/4/20.
//

#ifndef HEDGEHOG_GRAPH_INT_INT_H
#define HEDGEHOG_GRAPH_INT_INT_H

#include "../../../hedgehog/hedgehog.h"

class GraphIntInt : public hh::Graph<int, int> {
 public:
  explicit GraphIntInt(std::string_view const &name) : Graph(name) {}
};

#endif //HEDGEHOG_GRAPH_INT_INT_H
