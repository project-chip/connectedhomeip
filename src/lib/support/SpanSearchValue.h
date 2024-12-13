/*
 *    Copyright (c) 2024 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#pragma once

#include <cstddef>
#include <lib/support/Span.h>

#include <optional>

namespace chip {

/// represents a wrapper around a type `T` that contains internal
/// `Span<...>` values of other sub-types. It allows searching within the container sub-spans
/// to create new containers.
///
/// The use case is that we very often search within nested containers, like "find-endpoint" + "find-cluster" + "find-attribute"
/// and we generally only care if "does the last element exist or not"
///
/// A typical example of the way this class is used looks like this:
///
///    SpanSearchValue container(somePointer);
///
///    const AcceptedCommandData * value =
///           container
///              .Find<ByEndpoint>(path.mEndpointId, mEndpointIndexHint)
///              .Find<ByServerCluster>(path.mClusterId, mServerClusterHint)
///              .Find<ByAcceptedCommand>(path.mCommandId, mAcceptedCommandHint)
///              .Value();
///
/// Where a `ByFoo` structure looks like:
///
///    struct ByFoo {
///      using Key  = int;              // the KEY inside a type
///      using Type = SomeValueType;    // The type that is indexed by `Key`
///
///      /// Allows getting the "Span of Type" from an underlying structure.
///      /// A `SpanSearchValue<Foo>` will require a `GetSpan(Foo&)`
///      static Span<Type> GetSpan(ContainerType & data) { /* return ... */ }
///
///      /// Checks that the `Type` value has the given `Key` or not
///      static bool HasKey(const Key & id, const Type & instance) { /* return "instance has key id" */ }
///    }
///
/// Where we define:
///    - how to get a "span of sub-elements" for an object (`GetSpan`)
///    - how to determine if a given sub-element has the "correct key"
template <typename T>
class SpanSearchValue
{
public:
    SpanSearchValue() : mValue(nullptr) {}
    SpanSearchValue(std::nullptr_t) : mValue(nullptr) {}
    explicit SpanSearchValue(T * value) : mValue(value) {}

    /// Returns nullptr if such an element does not exist or non-null valid value if the element exists
    T * Value() const { return mValue; }

    /// Gets the first element of `TYPE::Type`
    template <typename TYPE>
    SpanSearchValue<typename TYPE::Type> First(unsigned & indexHint)
    {
        // if no value, searching more also yields no value
        VerifyOrReturnValue(mValue != nullptr, nullptr);

        Span<typename TYPE::Type> value_span = TYPE::GetSpan(*mValue);
        VerifyOrReturnValue(!value_span.empty(), nullptr);

        // found it, save the hint
        indexHint = 0;
        return SpanSearchValue<typename TYPE::Type>(&value_span[0]);
    }

    /// Find the value corresponding to `key`
    template <typename TYPE>
    SpanSearchValue<typename TYPE::Type> Find(typename TYPE::Key key, unsigned & indexHint)
    {
        VerifyOrReturnValue(mValue != nullptr, nullptr);

        Span<typename TYPE::Type> value_span = TYPE::GetSpan(*mValue);

        if (!FindIndexUsingHint(key, value_span, indexHint, TYPE::HasKey))
        {
            return nullptr;
        }

        return SpanSearchValue<typename TYPE::Type>(&value_span[indexHint]);
    }

    /// Finds the value that occurs after `key` in the underlying collection.
    template <typename TYPE>
    SpanSearchValue<typename TYPE::Type> Next(typename TYPE::Key key, unsigned & indexHint)
    {
        VerifyOrReturnValue(mValue != nullptr, nullptr);

        Span<typename TYPE::Type> value_span = TYPE::GetSpan(*mValue);

        if (!FindIndexUsingHint(key, value_span, indexHint, TYPE::HasKey))
        {
            return nullptr;
        }

        VerifyOrReturnValue((indexHint + 1) < value_span.size(), nullptr);

        indexHint++;
        return SpanSearchValue<typename TYPE::Type>(&value_span[indexHint]);
    }

private:
    T * mValue = nullptr; // underlying value, NULL if such a value does not exist

    /// Search for the index where `needle` is located inside `haystack`
    ///
    /// using `haystackValueMatchesNeedle` to find if a given haystack value matches the given needle
    ///
    /// `in_out_hint` contains a start search point at the start and will contain the found index
    /// location (if found) at the end.
    ///
    /// Returns true on success (index found) false on failure (index not found). If returning
    /// false, the value of `in_out_hint` is unchanged
    template <typename N, typename H>
    static bool FindIndexUsingHint(const N & needle, Span<H> haystack, unsigned & in_out_hint,
                                   bool (*haystackValueMatchesNeedle)(const N &, const typename std::remove_const<H>::type &))
    {
        // search starts at `hint` rather than 0
        const unsigned haystackSize = static_cast<unsigned>(haystack.size());
        unsigned checkIndex         = (in_out_hint < haystackSize) ? in_out_hint : 0;

        for (unsigned i = 0; i < haystackSize; i++, checkIndex++)
        {
            if (haystackValueMatchesNeedle(needle, haystack[checkIndex % haystackSize]))
            {
                in_out_hint = checkIndex % haystackSize;
                return true;
            }
        }

        return false;
    }
};

} // namespace chip
