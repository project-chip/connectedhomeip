/*
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
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

#include <cstdint>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Internal {

// Compile-time helper to check if an ID is in a list of IDs.
template <AttributeId T, AttributeId... Ts>
struct IsOneOf;

template <AttributeId T, AttributeId Head, AttributeId... Tail>
struct IsOneOf<T, Head, Tail...>
{
    static constexpr bool value = (T == Head) || IsOneOf<T, Tail...>::value;
};

template <AttributeId T>
struct IsOneOf<T>
{
    static constexpr bool value = false;
};

} // namespace Internal

/// It is very common that a class has optional attributes. Such optional attributes
/// need checking for and also affect what attributes are being returned by
/// server cluster implementations
///
/// This class implements the common case where attribute IDs can fit
/// within a 32-bit bitset as most attributes start with low IDs and are incremented
/// by one.
///
/// NOTE: this will NOT work for all possible attributes/clusters, only for clusters
///       whose optionnal attributes all have IDs under 32.
///
/// The implementation of the class generally is a wrapper over a bitset with a
/// `IsSet()` method. Configurations should use the OptionalAttributeSet<...> class.
class AttributeSet
{
public:
    explicit AttributeSet(uint32_t initialValue) : mSetBits(initialValue) {}

    AttributeSet()                                       = default;
    AttributeSet(const AttributeSet & other)             = default;
    AttributeSet(AttributeSet && other)                  = default;
    AttributeSet & operator=(const AttributeSet & other) = default;
    AttributeSet & operator=(AttributeSet && other)      = default;

    // Checks if an attribute ID is set.
    //
    // NOTE: this does NOT validate that the ID is < 32 because all the Set functions
    //       generally are asserted on this (forceset as well as subclasses) and the
    //       initial value contructor uses a uint32_t bitmask as well.
    //
    // This MUST be called with id < 32.
    constexpr bool IsSet(AttributeId id) const { return (mSetBits & (1u << id)) != 0; }

    /// Exposes a "force attribute bit set" without extra validation,
    /// so that clusters can enforce specific bits to be set.
    ///
    /// This is NOT intended as a generic set, use `OptionalAttributeSet` to configure values.
    template <AttributeId id>
    constexpr AttributeSet & ForceSet()
    {
        static_assert(id < 32, "Attribute ID must be settable");
        return Set(id, true);
    }

protected:
    constexpr AttributeSet & Set(AttributeId id, bool value = true)
    {
        if (value)
        {
            mSetBits |= static_cast<uint32_t>((static_cast<uint32_t>(1) << id));
        }
        else
        {
            mSetBits &= ~static_cast<uint32_t>((static_cast<uint32_t>(1) << id));
        }
        return *this;
    }

private:
    uint32_t mSetBits = 0;
};

/// A specialization of AttributeSet that provides checked calls to `Set`.
///
/// Specifically it requires that attributes are declared as part of the template
/// parameter pack.
///
/// NOTE: this will NOT work for all possible attributes/clusters, only for clusters
///       whose optional attributes all have IDs under 32. Static asserts are in place
///       to ensure that arguments to the template are valid.
///
/// Example usage:
///
///    namespace chip::app::Clusters::GeneralDiagnostics {
///
///    using OptionalAttributeSet = chip::app::OptionalAttributeSet<
///        Attributes::TotalOperationalHours::Id,
///        Attributes::BootReason::Id,
///        Attributes::ActiveHardwareFaults::Id
///    >;
///
///    } // namespace chip::app::Clusters::GeneralDiagnostics
///
/// After this, one can:
///
///   GeneralDiagnostics::OptionalAttributeSet()
///      .Set<GeneralDiagnostics::Attributes::TotalOperationalHours::Id>()
///      .Set<GeneralDiagnostics::Attributes::BootReason::Id>();
///
/// Cluster implementations can then store a
///   Constructor(const GeneralDiagnostics::OptionalAttributeSet& optionalAttributeSet) :
///   mOptionalAttributeSet(optionalAttributeSet) {...}
///
/// where:
///   const AttributeSet mOptionalAttributeSet;
template <AttributeId... OptionalAttributeIds>
class OptionalAttributeSet : public AttributeSet
{
public:
    explicit OptionalAttributeSet(uint32_t initialValue) : AttributeSet(initialValue & All()) {}
    OptionalAttributeSet(const AttributeSet & initialValue) : AttributeSet(initialValue) {}
    OptionalAttributeSet() = default;

    template <uint32_t ATTRIBUTE_ID>
    constexpr OptionalAttributeSet & Set(bool value = true)
    {
        static_assert(ATTRIBUTE_ID < 32, "Cluster attribute bits supports attributes up to 31");
        static_assert(Internal::IsOneOf<ATTRIBUTE_ID, OptionalAttributeIds...>::value, "attribute MUST be optional");
        (void) AttributeSet::Set(ATTRIBUTE_ID, value);
        return *this;
    }

    static constexpr uint32_t All()
    {
        if constexpr (sizeof...(OptionalAttributeIds) == 0)
        {
            return 0;
        }
        else
        {
            return ((1U << OptionalAttributeIds) | ...);
        }
    }
};

} // namespace app
} // namespace chip
