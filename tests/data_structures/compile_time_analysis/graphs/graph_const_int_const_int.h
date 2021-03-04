//
// Created by Bardakoff, Alexandre (IntlAssoc) on 12/4/20.
//

#ifndef HEDGEHOG_GRAPH_CONST_INT_CONST_INT_H
#define HEDGEHOG_GRAPH_CONST_INT_CONST_INT_H

#include "../../../hedgehog/hedgehog.h"

class GraphConstIntConstInt : public hh::Graph<int const, int const> {
 public:
  GraphConstIntConstInt(std::string_view const &name) : Graph(name) {}
};

#endif //HEDGEHOG_GRAPH_CONST_INT_CONST_INT_H
