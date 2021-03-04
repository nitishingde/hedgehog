// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the
// software in any medium, provided that you keep intact this entire notice. You may improve, modify and create
// derivative works of the software or any portion of the software, and you may copy and distribute such modifications
// or works. Modified works should carry a notice stating that you changed the software and should note the date and
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
// operation. This software is not intended to be used in any situation where a failure could cause risk of injury or
// damage to property. The software developed by NIST employees is not subject to copyright protection within the
// United States.
//
// Created by anb22 on 10/30/18.
//

#ifndef PROTOTYPEGRAPH_STACK_H
#define PROTOTYPEGRAPH_STACK_H

#include <array>
#include <iostream>
#include "../algorithm/algorithm.h"

#ifndef DOXYGEN_SHOULD_SKIP_THIS
template <typename Value, std::size_t Capacity = 5>
class stack_cx{
  using storage_t = std::array<Value, Capacity>;
  storage_t
    _data{};
  std::size_t
    _size{0};

 public:
  using iterator = typename storage_t::iterator;
  using const_iterator = typename storage_t::const_iterator;
  using value_type = Value;
  using reference = typename storage_t::reference;
  using const_reference = typename storage_t::const_reference;

  template<typename Itr>
  constexpr stack_cx(Itr begin, const Itr &end)
  {
    while (begin != end) {
      push(*begin);
      ++begin;
    }
  }

  constexpr stack_cx(std::initializer_list<Value> init)
  : stack_cx(init.begin(), init.end()) {}

  constexpr stack_cx(const stack_cx & s){
    _size = s.size();
    cx::copy_n(s._data.begin(), _size , _data.begin());
  }
  constexpr stack_cx(stack_cx && s) noexcept{
    _size = std::move(s._size);
    _data = std::move(s._data);
  }

  constexpr stack_cx() = default;

  constexpr auto capacity() const { return Capacity; }
  constexpr auto size() const { return _size; }
  constexpr auto empty() const { return _size == 0; }
  constexpr const Value* data() const { return _data.data(); }

  constexpr void push(Value t_v) {
    if (_size >= Capacity) {
      throw std::range_error("Index past end of vector_cx");
    } else {
      reference v = _data[_size++];
      v = std::move(t_v);
    }
  }

  constexpr reference top() {
    if(empty()){
      throw std::range_error("Top called on empty stack_cx");
    } else {
      return *(std::next(_data.begin(), _size - 1));
    }
  }

  constexpr const_reference top() const {
    if(empty()){
      throw std::range_error("Top called on empty stack_cx");
    } else {
      return std::next(_data.begin(), _size - 1);
    }
  }

  constexpr reference pop() {
    if(empty()){
      throw std::range_error("Pop called on empty stack_cx");
    } else {
      reference v = top();
      _size --;
      return v;
    }
  }

  friend std::ostream &operator<<(std::ostream &os, const stack_cx &stack) {
    os << "stack_cx with " << stack.size() << " elements" << std::endl << std::flush;
    auto stackCopie(stack);
    for (size_t size = 0; size < stack.size(); ++size ) {
      os << *(stackCopie.pop()) << std::endl << std::flush;
    }
    return os;
  }
};
#endif //DOXYGEN_SHOULD_SKIP_THIS

#endif //PROTOTYPEGRAPH_STACK_H
