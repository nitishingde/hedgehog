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
#ifndef PROTOTYPEGRAPH_MOD_SEQ_H
#define PROTOTYPEGRAPH_MOD_SEQ_H

#pragma once

#ifndef DOXYGEN_SHOULD_SKIP_THIS
namespace cx
{
  template <class InputIt, class OutputIt>
  constexpr OutputIt copy(InputIt first, InputIt last,
                          OutputIt d_first)
  {
    while (first != last) {
      *d_first++ = *first++;
    }
    return d_first;
  }

  template <class InputIt, class OutputIt, class UnaryPredicate>
  constexpr OutputIt copy_if(InputIt first, InputIt last,
                             OutputIt d_first, UnaryPredicate pred)
  {
    while (first != last) {
      if (pred(*first)) {
        *d_first++ = *first;
      }
      ++first;
    }
    return d_first;
  }

  template <class InputIt, class Size, class OutputIt>
  constexpr OutputIt copy_n(InputIt first, Size count, OutputIt result)
  {
    if (count > 0) {
      *result++ = *first;
      for (Size i = 1; i < count; ++i) {
        *result++ = *++first;
      }
    }
    return result;
  }

  template <class BidirIt1, class BidirIt2>
  constexpr BidirIt2 copy_backward(BidirIt1 first, BidirIt1 last, BidirIt2 d_last)
  {
    while (first != last) {
      *(--d_last) = *(--last);
    }
    return d_last;
  }

  template <class InputIt, class OutputIt>
  constexpr OutputIt move(InputIt first, InputIt last, OutputIt d_first)
  {
    while (first != last) {
      *d_first++ = std::move(*first++);
    }
    return d_first;
  }

  template <class BidirIt1, class BidirIt2>
  constexpr BidirIt2 move_backward(BidirIt1 first,
                                   BidirIt1 last,
                                   BidirIt2 d_last)
  {
    while (first != last) {
      *(--d_last) = std::move(*(--last));
    }
    return d_last;
  }

  template <class ForwardIt, class T>
  constexpr void fill(ForwardIt first, ForwardIt last, const T& value)
  {
    for (; first != last; ++first) {
      *first = value;
    }
  }

  template <class OutputIt, class Size, class T>
  constexpr OutputIt fill_n(OutputIt first, Size count, const T& value)
  {
    for (Size i = 0; i < count; i++) {
      *first++ = value;
    }
    return first;
  }

}
#endif //DOXYGEN_SHOULD_SKIP_THIS

#endif //PROTOTYPEGRAPH_MOD_SEQ_H