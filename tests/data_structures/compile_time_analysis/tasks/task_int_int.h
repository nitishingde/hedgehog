//
// Created by Bardakoff, Alexandre (IntlAssoc) on 12/4/20.
//

#ifndef HEDGEHOG_TASK_INT_INT_H
#define HEDGEHOG_TASK_INT_INT_H

#include "../../../hedgehog/hedgehog.h"

class TaskIntInt : public hh::AbstractTask<int, int> {
 public:
  explicit TaskIntInt(std::string_view const &name) : AbstractTask(name) {}
  void execute(std::shared_ptr<int> ptr) override {
    this->addResult(ptr);
  }
};

#endif //HEDGEHOG_TASK_INT_INT_H
