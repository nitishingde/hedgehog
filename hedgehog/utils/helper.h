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


#ifndef HEDGEHOG_CX_HELPER_H
#define HEDGEHOG_CX_HELPER_H

#include <tuple>
#include <variant>

#include "../hedgehog/core/io/base/receiver/core_multi_receivers.h"

/// @brief Hedgehog main namespace
namespace hh {

/// @brief Hedgehog behavior namespace
namespace behavior {
#ifndef DOXYGEN_SHOULD_SKIP_THIS
/// @brief Forward declaration of MultiReceivers
/// @tparam Inputs Inputs type of MultiReceivers
template<class ...Inputs>
class MultiReceivers;

/// @brief Forward declaration of Sender
/// @tparam Output Output Type of sender
template<class Output>
class Sender;
#endif // DOXYGEN_SHOULD_SKIP_THIS
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
/// @brief Forward declaration of Graph
/// @tparam GraphOutput Output type of Graph
/// @tparam GraphInputs Inputs types of Graph
template<class GraphOutput, class ...GraphInputs>
class Graph;

template<class TaskOutput, class ...TaskInputs>
class AbstractTask;
#endif // DOXYGEN_SHOULD_SKIP_THIS

/// Helper used in hedgehog
namespace helper {
/// @brief Base definition of HelperMultiReceiversType
/// @tparam Inputs Tuple of input types
template<class Inputs>
struct HelperMultiReceiversType;
/// @brief Used helper to get the type of hedgehog MultiReceivers for inputs from hedgehog tuple of Input types
/// @tparam Inputs MultiReceivers inputs
template<class ...Inputs>
struct HelperMultiReceiversType<std::tuple<Inputs...>> {
  using type = behavior::MultiReceivers<Inputs...>; ///< Type of the MultiReceivers
};

/// @brief Base definition of HelperGraphType
/// @tparam Output Graph Output
/// @tparam Inputs Tuple of Graph input types
template<class GraphOutput, class ...GraphInputs>
struct HelperGraphType;

/// @brief Used helper to get the type of hedgehog MultiReceivers for inputs from hedgehog tuple of Input types
/// @tparam Inputs MultiReceivers inputs
template<class GraphOutput, class ...GraphInputs>
struct HelperGraphType<GraphOutput, std::tuple<GraphInputs...>> {
  using type = Graph<GraphOutput, GraphInputs...>; ///< Type of the MultiReceivers
};

/// @brief Base definition of HelperCoreMultiReceiversType
/// @tparam Inputs Tuple of input types
template<class Inputs>
struct HelperCoreMultiReceiversType;
/// @brief Used helper to get the type of hedgehog CoreMultiReceivers for inputs from hedgehog tuple of Input types
/// @tparam Inputs CoreMultiReceivers inputs
template<class ...Inputs>
struct HelperCoreMultiReceiversType<std::tuple<Inputs...>> {
  using type = core::CoreMultiReceivers<Inputs...>; ///< Type of the CoreMultiReceivers
};

/// @brief Base definition of HelperCoreMultiReceiversType
/// @tparam Inputs Tuple of input types
template<class Inputs, class Output>
struct HelperAbstractTaskType;
/// @brief Used helper to get the type of hedgehog CoreMultiReceivers for inputs from hedgehog tuple of Input types
/// @tparam Inputs CoreMultiReceivers inputs
template<class Output, class ...Inputs>
struct HelperAbstractTaskType<Output, std::tuple<Inputs...>> {
  using type = hh::AbstractTask<Output, Inputs...>; ///< Type of the CoreMultiReceivers
};

/// @brief Base definition of getUniqueVariantFromTuple
/// @tparam Tuple Tuple of types
template<class Tuple>
class getUniqueVariantFromTuple;

/// @brief Transform a tuple to a variant of unique types
/// @tparam AllTypes All the types contained in the tuple
template<class ...AllTypes>
class getUniqueVariantFromTuple<std::tuple<AllTypes...>> {
  class MakeUnique {
   private:
    /// @brief Create a tuple with unique type
    /// @tparam Current Current type analysed
    /// @tparam Others Other type remaining in the tuple
    /// @return A tuple with unique types as pointers
    template<class Current, class ... Others>
    static auto makeUniqueTuple() {
      if constexpr (sizeof...(Others) == 0) {
        return std::tuple<Current *>{};
      } else {
        if constexpr (std::disjunction_v<std::is_same<Current, Others>...>) {
          return makeUniqueTuple<Others...>();
        } else {
          return std::tuple_cat(std::tuple<Current *>{}, makeUniqueTuple<Others...>());
        }
      }
    }

    /// @brief Create a variant from a tuple of pointer of types
    /// @tparam TupleType Type of the tuple
    /// @return A variant containing all types (+ std::monostate)
    template<class ... TupleType>
    static auto makeVariant(std::tuple<TupleType...>) {
      return (std::variant<std::monostate, std::remove_pointer_t<TupleType>...>());
    }

   public:
    /// @brief Type of tuple with uniques pointer types
    using type = decltype(makeUniqueTuple<AllTypes...>());
    /// @brief Type of variant with uniques pointer
    using variant_type = decltype(makeVariant(std::declval<type>()));
  };

 public:
  /// @brief Direct accessor to the variant type with unique types
  using type = typename MakeUnique::variant_type;
};
}
}

#endif //HEDGEHOG_CX_HELPER_H
