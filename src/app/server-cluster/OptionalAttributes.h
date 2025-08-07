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

#include <lib/core/DataModelTypes.h>

/// Marks the given cluster/attributeid as settable in
/// `SupportedAttributes<CONTAINER>`
///
/// At this point this is NOT automatic as cluster implementations should
/// define what optional attributes they support
///
/// This should be in the `namespace chip::app`
#define MARK_ATTRIBUTE_SUPPORTED(container, cluster, attribute)                                                                    \
    template <>                                                                                                                    \
    struct IsOptionalAttribute<container, (Clusters::cluster::Attributes::attribute::Id)>                                          \
    {                                                                                                                              \
        static constexpr bool isOptional = true;                                                                                   \
    }

namespace chip {
namespace app {

/// By default we assume all attributes are NOT optional
template <typename CONTAINER, AttributeId ID>
struct IsOptionalAttribute
{
    static constexpr bool isOptional = false;
};

/// It is very common that a class has optional attributes. Such optional attributes
/// need checking for and also affect what attributes are being returned by
/// server cluster implementations
///
/// This class implements the common case where attribute IDs can fit
/// within a 32-bit bitset as most attributes start with low IDs and are incremented
/// by one.
///
/// The implementation of the class generally is a wrapper over a bitset with a
/// `IsSet()` method. Configurations should use the SupportedAttributes<...> class.
class AttributeSet
{
public:
    AttributeSet()                                       = default;
    AttributeSet(const AttributeSet & other)             = default;
    AttributeSet(AttributeSet && other)                  = default;
    AttributeSet & operator=(const AttributeSet & other) = default;
    AttributeSet & operator=(AttributeSet && other)      = default;

    constexpr bool IsSet(AttributeId id) const { return (mSetBits & (1u << id)) != 0; }

    /// Exposes a "force attribute bit set" without extra validation,
    /// so that clusters can enforce specific bits to be set.
    ///
    /// This is NOT intended as a generic set, use `SupportedAttributes` to configure values.
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

/// A specialization of AttributeSet that provides checked
/// calls to `Set`.
///
/// Specifically it requires that attributes are marked as 'known optional' by cluster
/// implementations.
///
/// Example usage:
///
///    // First declare attibutes are optional. Tie them to a class.
///
///    namespace chip::app {
///    namespace Clusters {
///      class GeneralDiagnosticsCluster;
///    } // namespace Clusters
///
///    MARK_ATTRIBUTE_SUPPORTED(Clusters::GeneralDiagnosticsCluster, GeneralDiagnostics, TotalOperationalHours);
///    MARK_ATTRIBUTE_SUPPORTED(Clusters::GeneralDiagnosticsCluster, GeneralDiagnostics, BootReason);
///    MARK_ATTRIBUTE_SUPPORTED(Clusters::GeneralDiagnosticsCluster, GeneralDiagnostics, ActiveHardwareFaults);
///    } // namespace chip::app
///
/// After this, one can:
///
///   SupportedAttributes<GeneralDiagnosticsCluster>()
///      .Set<GeneralDiagnostics::Attributes::TotalOperationalHours::Id>()
///      .Set<GeneralDiagnostics::Attributes::BootReason::Id>();
///
/// Clusters implementaions then can store a
///   Constructor(SupportedAttributes<...> enabled) : mEnabledAttributes(enabled) {...}
///
/// where:
///   const AttributeSet mEnabledAttributes;
///
///
///
template <typename CONTAINER>
class SupportedAttributes : public AttributeSet
{
public:
    SupportedAttributes(const AttributeSet & initialValue) : AttributeSet(initialValue) {}
    SupportedAttributes() = default;

    template <uint32_t ATTRIBUTE_ID>
    constexpr SupportedAttributes & Set(bool value = true)
    {
        static_assert(ATTRIBUTE_ID < 32, "Cluster attribute bits supports attributes up to 31");
        static_assert(IsOptionalAttribute<CONTAINER, ATTRIBUTE_ID>::isOptional, "attribute MUST be optional");
        (void) AttributeSet::Set(ATTRIBUTE_ID, value);
        return *this;
    }
};

} // namespace app
} // namespace chip
