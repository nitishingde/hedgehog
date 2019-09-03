//
// Created by anb22 on 5/8/19.
//

#ifndef HEDGEHOG_CORE_QUEUE_MULTI_RECEIVERS_H
#define HEDGEHOG_CORE_QUEUE_MULTI_RECEIVERS_H
#include <variant>

#include "../../base/receiver/core_multi_receivers.h"
#include "core_queue_slot.h"
#include "core_queue_receiver.h"

template<class ...NodeInputs>
class CoreQueueMultiReceivers
    : public CoreMultiReceivers<NodeInputs...>, public CoreQueueSlot, public CoreQueueReceiver<NodeInputs, NodeInputs...> ... {

 private:
  std::shared_ptr<std::queue<std::variant<std::shared_ptr<NodeInputs>...>>> queue_{};

 public:
  explicit
  CoreQueueMultiReceivers(std::string_view const &name, NodeType const type, size_t const numberThreads) :
      CoreNode(name, type, numberThreads),
      CoreSlot(name, type, numberThreads),
      CoreReceiver<NodeInputs>(name, type, numberThreads)...,
      CoreMultiReceivers<NodeInputs...>(name, type, numberThreads),
      CoreQueueSlot(name, type, numberThreads),
      CoreQueueReceiver<NodeInputs, NodeInputs...>(name, type, numberThreads, *this)...{
    queue_ = std::make_shared<std::queue<std::variant<std::shared_ptr<NodeInputs>...>>>();
    HLOG_SELF(0, "Creating CoreQueueMultiReceivers with type: " << (int) type << " and name: " << name)
  }

  ~CoreQueueMultiReceivers() override {
    HLOG_SELF(0, "Destructing CoreQueueMultiReceivers")
  }

  bool receiversEmpty() final {
    HLOG_SELF(2, "Test all destinations empty")
    return queue_->empty();
  }

  size_t totalQueueSize() final {
    return (static_cast<CoreReceiver<NodeInputs> *>(this)->queueSize() + ...);
  }

  std::shared_ptr<std::queue<std::variant<std::shared_ptr<NodeInputs>...>>> const &queue() const {
    return queue_;
  }

  std::set<CoreSlot *> getSlots() final { return {this}; }

  CoreQueueSlot *queueSlot() final { return this; };

  void copyInnerStructure(CoreQueueMultiReceivers<NodeInputs...> *rhs) {
    HLOG_SELF(0, "Copy Cluster information from " << rhs->name() << "(" << rhs->id() << ")")
    (CoreQueueReceiver<NodeInputs, NodeInputs...>::copyInnerStructure(static_cast<CoreQueueReceiver < NodeInputs, NodeInputs... >*>(rhs)),...);
    CoreQueueSlot::copyInnerStructure(rhs);
    this->queue_ = rhs->queue_;
  }

  void removeForAllSenders(CoreNode *coreNode) override {
	(this->removeForAllSendersConditional<NodeInputs>(coreNode),...);
  }

 private:
  template <class Input>
  void removeForAllSendersConditional(CoreNode* coreNode){
	if(auto temp = dynamic_cast<CoreQueueSender<Input>*>(coreNode)){
	  static_cast<CoreReceiver<Input>*>(this)->removeSender(temp);
	}
  }

};

#endif //HEDGEHOG_CORE_QUEUE_MULTI_RECEIVERS_H
