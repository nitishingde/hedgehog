//
// Created by Bardakoff, Alexandre (IntlAssoc) on 12/4/20.
//

#ifndef HEDGEHOG_TASK_CONST_INT_CONST_INT_H
#define HEDGEHOG_TASK_CONST_INT_CONST_INT_H

#include "hedgehog/hedgehog.h"

class TaskConstIntConstInt : public hh::AbstractTask<int const, int const> {
 public:
  void execute(std::shared_ptr<int const> ptr) override {
    this->addResult(ptr);
  }
};

#endif //HEDGEHOG_TASK_CONST_INT_CONST_INT_H
