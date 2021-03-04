//
// Created by Bardakoff, Alexandre (IntlAssoc) on 12/4/20.
//

#ifndef HEDGEHOG_TASK_INT_INT_WITH_CAN_TERMINATE_H
#define HEDGEHOG_TASK_INT_INT_WITH_CAN_TERMINATE_H

#include "../../../hedgehog/hedgehog.h"

class TaskIntIntWithCanTerminate : public hh::AbstractTask<int, int> {
 public:
  void execute(std::shared_ptr<int> ptr) override {
    this->addResult(ptr);
  }
  bool canTerminate() override {
    return AbstractTask::canTerminate();
  }
};

#endif //HEDGEHOG_TASK_INT_INT_WITH_CAN_TERMINATE_H
