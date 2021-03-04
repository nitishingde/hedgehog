// NIST-developed software is provided by NIST as hedgehog public service. You may use, copy and distribute copies of the
// software in any medium, provided that you keep intact this entire notice. You may improve, modify and create
// derivative works of the software or any portion of the software, and you may copy and distribute such modifications
// or works. Modified works should carry hedgehog notice stating that you changed the software and should note the date and
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
// operation. This software is not intended to be used in any situation where hedgehog failure could cause risk of injury or
// damage to property. The software developed by NIST employees is not subject to copyright protection within the
// United States.


#ifndef HEDGEHOG_POOL_H
#define HEDGEHOG_POOL_H

#include <memory>
#include <deque>
#include <algorithm>
#include <cassert>
#include <mutex>
#include <condition_variable>

/// @brief Hedgehog behavior namespace
namespace hh::behavior {

/// @brief Pool that is used by the memory manager
/// @tparam ManagedData Type stored in the pool
template<class ManagedData>
class Pool {
 private:
  size_t capacity_ = 1; ///< Pool capacity, maximum size
  std::deque<std::shared_ptr<ManagedData>> queue_ = {}; ///< Container used to store pool's data
  std::mutex mutex_ = {}; ///< Pool mutex to protect pool access
  std::unique_ptr<std::condition_variable>
      conditionVariable_ = std::make_unique<std::condition_variable>(); ///< Condition variable to wait on data for
  ///< empty queue

 public:
  /// @brief Pool constructor defining capacity
  /// @details If the capacity given is 0, it set to 1
  /// @param capacity Pool capacity
  explicit Pool(size_t const &capacity) {
    capacity_ = capacity == 0 ? 1 : capacity;
    queue_ = std::deque<std::shared_ptr<ManagedData>>(capacity_);
  }

  /// @brief Queue container accessor
  /// @attention Not protected with mutex
  /// @return Queue container
  std::deque<std::shared_ptr<ManagedData>> const &queue() const { return queue_; }

  /// @brief Begin iterator accessor for the queue's container
  /// @attention Not protected with mutex
  /// @return Begin iterator for the queue's container
  typename std::deque<std::shared_ptr<ManagedData>>::iterator begin() { return this->queue_.begin(); }

  /// @brief End iterator accessor for the queue's container
  /// @attention Not protected with mutex
  /// @return End iterator for the queue's container
  typename std::deque<std::shared_ptr<ManagedData>>::iterator end() { return this->queue_.end(); }

  /// @brief Queue size accessor
  /// @attention Protected with mutex
  /// @return Queue size
  size_t size() {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
  }

  /// @brief Emptiness queue accessor
  /// @attention Not protected with mutex
  /// @return True if queue empty, else False
  bool empty() { return queue_.empty(); }

  /// @brief Capacity accessor
  /// @attention Not protected with mutex
  /// @return Queue accessor
  [[nodiscard]] size_t capacity() const { return capacity_; }

  /// @brief Push back data into the pool
  /// @exception std::runtime_error Queue overflow, too much push_back
  /// @param data Data to push back
  void push_back(std::shared_ptr<ManagedData> const &data) {
    std::lock_guard<std::mutex> lock(mutex_);
    this->queue_.push_back(data);
    if (this->queue_.size() > capacity_) {
      std::ostringstream oss;
      oss << "The queue is overflowing, the same data " << data << " has been returned to the memory manager too many "
          << "times: "
          << __FUNCTION__;
      HLOG_SELF(0, oss.str())
      throw (std::runtime_error(oss.str()));
    }
    conditionVariable_->notify_one();
  }

  /// @brief Return and pop the fist element from the queue, it the queue is empty wait for an element to come back
  /// @return The queue front element
  std::shared_ptr<ManagedData> pop_front() {
    std::unique_lock<std::mutex> lock(mutex_);
    std::shared_ptr<ManagedData> ret = nullptr;
    // Wait if the queue is empty
    conditionVariable_->wait(lock, [this]() { return !queue_.empty(); });
    ret = queue_.front();
    queue_.pop_front();
    conditionVariable_->notify_one();
    return ret;
  }

};
}

#endif //HEDGEHOG_POOL_H
