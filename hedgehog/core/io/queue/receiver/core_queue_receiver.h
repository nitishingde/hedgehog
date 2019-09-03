//
// Created by anb22 on 5/8/19.
//

#ifndef HEDGEHOG_CORE_QUEUE_RECEIVER_H
#define HEDGEHOG_CORE_QUEUE_RECEIVER_H

#include "../../base/receiver/core_receiver.h"
template<class ...CoreMultiReceiverInputs>
class CoreQueueMultiReceivers;

template<class CoreInput, class ...CoreMultiReceiverInputs>
class CoreQueueReceiver : public virtual CoreReceiver<CoreInput> {
 private:
  std::shared_ptr<std::set<CoreSender<CoreInput> *>> senders_ = nullptr;
  CoreQueueMultiReceivers<CoreMultiReceiverInputs...> & mr_;
  size_t
    queueSize_ = 0,
    maxQueueSize_ = 0;

 public:
  CoreQueueReceiver(std::string_view const &name,
                    NodeType const type,
                    size_t const numberThreads,
                    CoreQueueMultiReceivers<CoreMultiReceiverInputs...> &multiReceiver)
      : CoreReceiver<CoreInput>(name, type, numberThreads), mr_(multiReceiver) {
    HLOG_SELF(0, "Creating CoreQueueReceiver with type: " << (int) type << " and name: " << name)
    senders_ = std::make_shared<std::set<CoreSender<CoreInput> *>>();
  }

  ~CoreQueueReceiver() override {HLOG_SELF(0, "Destructing CoreQueueReceiver")}

  std::shared_ptr<std::set<CoreSender<CoreInput> *>> const &senders() const { return senders_; }

  //Virtual
  size_t queueSize() override { return this->queueSize_; }

  size_t maxQueueSize() override { return this->maxQueueSize_; }

  bool receiverEmpty() final {
    HLOG_SELF(2, "Test queue emptiness")
    return this->queueSize_ == 0;
  }

  void addSender(CoreSender<CoreInput> *sender) final {
    HLOG_SELF(0, "Adding sender " << sender->name() << "(" << sender->id() << ")")
    this->senders_->insert(sender);
  }

  void removeSender(CoreSender<CoreInput> *sender) final {
    HLOG_SELF(0, "Remove sender " << sender->name() << "(" << sender->id() << ")")
    this->senders_->erase(sender);
  }

  void receive(std::shared_ptr<CoreInput> data) final {
    this->queueSlot()->lockUniqueMutex();
    ++this->queueSize_;
    if (this->queueSize() > this->maxQueueSize_) { this->maxQueueSize_ = this->queueSize(); }
    mr_.queue()->push(std::variant<std::shared_ptr<CoreMultiReceiverInputs>...>(data));
    HLOG_SELF(2, "Receives data new queue Size " << this->queueSize())
    this->queueSlot()->unlockUniqueMutex();
  }

  std::set<CoreReceiver<CoreInput> *> receivers() override {
    return {this};
  }

  void copyInnerStructure(CoreQueueReceiver<CoreInput, CoreMultiReceiverInputs...> *rhs) {
    HLOG_SELF(0, "Copy Cluster CoreQueueReceiver information from " << rhs->name() << "(" << rhs->id() << ")")
    this->senders_ = rhs->senders();
  }

  void decrementQueueSize(){
    --this->queueSize_;
  }
};
#endif //HEDGEHOG_CORE_QUEUE_RECEIVER_H
