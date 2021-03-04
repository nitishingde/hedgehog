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
#ifndef PROTOTYPEGRAPH_VECTOR_H
#define PROTOTYPEGRAPH_VECTOR_H
#pragma once

#include "../algorithm/algorithm.h"
#include "../algorithm/back_insert_iterator.h"

#include <array>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <iostream>

#ifndef DOXYGEN_SHOULD_SKIP_THIS
template<typename Value, std::size_t Size = 5>
class vector_cx {
  using storage_t = std::array<Value, Size>;
 public:
  using iterator = typename storage_t::iterator;
  using const_iterator = typename storage_t::const_iterator;
  using value_type = Value;
  using reference = typename storage_t::reference;
  using const_reference = typename storage_t::const_reference;

  template<typename Itr>
  constexpr vector_cx(Itr begin, const Itr &end) {
    while (begin != end) {
      push_back(*begin);
      ++begin;
    }
  }
  constexpr vector_cx(std::initializer_list<Value> init) : vector_cx(init.begin(), init.end()) {}

  constexpr vector_cx() = default;

  constexpr auto begin() const { return m_data.begin(); }
  constexpr auto begin() { return m_data.begin(); }

  // We would have prefered to use `std::next`, however it does not seem to be
  // enabled for constexpr use for std::array in this version of gcc. As of
  // September 2017 this is fixed in GCC trunk but not in GCC 7.2.
  constexpr auto end() const { return m_data.begin() + m_size; }
  constexpr auto end() { return m_data.begin() + m_size; }

  constexpr auto cbegin() const { return m_data.begin(); }
  constexpr auto cend() const { return m_data.begin() + m_size; }

  constexpr const Value &operator[](const std::size_t t_pos) const {
    return m_data[t_pos];
  }
  constexpr Value &operator[](const std::size_t t_pos) {
    return m_data[t_pos];
  }

  constexpr Value &at(const std::size_t t_pos) {
    if (t_pos >= m_size) {
      // This is allowed in constexpr context, but if the constexpr evaluation
      // hits this exception the compile would fail
      throw std::range_error("Index past end of vector_cx");
    } else {
      return m_data[t_pos];
    }
  }
  constexpr const Value &at(const std::size_t t_pos) const {
    if (t_pos >= m_size) {
      throw std::range_error("Index past end of vector_cx");
    } else {
      return m_data[t_pos];
    }
  }

  constexpr Value &push_back(Value t_v) {
    if (m_size >= Size) {
      throw std::range_error("Index past end of vector_cx");
    } else {
      Value &v = m_data[m_size++];
      v = std::move(t_v);
      return v;
    }
  }

  constexpr Value pop_back() {
    if (m_size == 0) {
      throw std::range_error("There is no element in the vector_cx");
    } else {
      Value v = m_data[--m_size];
      return v;
    }
  }

  constexpr Value pop_front() {
    if (m_size == 0) {
      throw std::range_error("There is no element in the vector_cx");
    } else {
      Value v = m_data[0];
      for (size_t pos = 0; pos < m_size - 1; ++pos) {
        m_data[pos] = m_data[pos + 1];
      }
      m_size--;

      return v;
    }
  }

  constexpr const Value &back() const {
    if (empty()) {
      throw std::range_error("Index past end of vector_cx");
    } else {
      return m_data[m_size - 1];
    }
  }
  constexpr Value &back() {
    if (empty()) {
      throw std::range_error("Index past end of vector_cx");
    } else {
      return m_data[m_size - 1];
    }
  }

  constexpr void remove(const Value &elem) {
    size_t pos = 0;
    for (; pos < m_size; ++pos) {
      if (m_data.at(pos) == elem) {
        break;
      }
    }
    if (pos != m_size) {
      for (; pos < m_size - 1; ++pos) {
        m_data.at(pos) = m_data.at(pos + 1);
      }
      m_size--;
    }
  }

  constexpr auto capacity() const { return Size; }
  constexpr auto size() const { return m_size; }
  constexpr auto empty() const { return m_size == 0; }

  constexpr void clear() { m_size = 0; }

  constexpr const Value *data() const {
    return m_data.data();
  }

  friend std::ostream &operator<<(std::ostream &os, const vector_cx &vector) {
    os << " Vector of size: " << vector.m_size << " ==> ";
    for (size_t e = 0; e < vector.m_size; ++e) {
      std::cout << vector[e] << " ";
    }
    os << "\n";

    return os;
  }

 private:
  storage_t m_data{};
  std::size_t m_size{0};
};

template<typename T, size_t Size1, size_t Size2>
constexpr bool operator==(const vector_cx<T, Size1> &x, const vector_cx<T, Size2> &y) {
  return cx::equal(x.begin(), x.end(), y.begin(), y.end());
}


// Is addition (concatenation) on strings useful? Not sure yet. But it does
// allow us to carry the type information properly.

template<typename Value, std::size_t S1, std::size_t S2>
constexpr auto operator+(vector_cx<Value, S1> a, vector_cx<Value, S2> b) {
  vector_cx<Value, S1 + S2> v;
  copy(a.cbegin(), a.cend(), back_insert_iterator(v));
  copy(b.cbegin(), b.cend(), back_insert_iterator(v));
  return v;
}
#endif //DOXYGEN_SHOULD_SKIP_THIS

#endif //PROTOTYPEGRAPH_VECTOR_H