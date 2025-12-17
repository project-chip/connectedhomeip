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

#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/ThreadNetworkDiagnostics/Enums.h>
#include <platform/DeviceInfoProvider.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/GeneralFaults.h>

namespace chip::app::Clusters {

namespace ThreadNetworkDiagnostics {

// OptionalAttributeSet supports attributes IDs < 32.
// This cluster needs a version of OptionalAttributeSet that supports attributes IDs < 64.
// Let's have an OptionalAttributeSet64 that is a specialization of AttributeSet64 for this cluster.
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
/// within a 64-bit bitset as most attributes start with low IDs and are incremented
/// by one.
///
/// NOTE: this will NOT work for all possible attributes/clusters, only for clusters
///       whose optionnal attributes all have IDs under 64.
///
/// The implementation of the class generally is a wrapper over a bitset with a
/// `IsSet()` method. Configurations should use the OptionalAttributeSet<...> class.
class AttributeSet64 : public AttributeSet
{
public:
    explicit AttributeSet64(uint64_t initialValue) : mSetBits(initialValue) {}

    AttributeSet64()                                         = default;
    AttributeSet64(const AttributeSet64 & other)             = default;
    AttributeSet64(AttributeSet64 && other)                  = default;
    AttributeSet64 & operator=(const AttributeSet64 & other) = default;
    AttributeSet64 & operator=(AttributeSet64 && other)      = default;

    // Checks if an attribute ID is set.
    //
    // NOTE: this does NOT validate that the ID is < 64 because all the Set functions
    //       generally are asserted on this (forceset as well as subclasses) and the
    //       initial value contructor uses a uint64_t bitmask as well.
    //
    // This MUST be called with id < 64.
    constexpr bool IsSet(AttributeId id) const { return (mSetBits & (1ULL << id)) != 0; }

    /// Exposes a "force attribute bit set" without extra validation,
    /// so that clusters can enforce specific bits to be set.
    ///
    /// This is NOT intended as a generic set, use `OptionalAttributeSet64` to configure values.
    template <AttributeId id>
    constexpr AttributeSet64 & ForceSet()
    {
        static_assert(id < 64, "Attribute ID must be settable");
        return Set(id, true);
    }

protected:
    constexpr AttributeSet64 & Set(AttributeId id, bool value = true)
    {
        if (value)
        {
            mSetBits |= (1ULL << id);
        }
        else
        {
            mSetBits &= ~(1ULL << id);
        }
        return *this;
    }

private:
    uint64_t mSetBits = 0;
};

/// A specialization of AttributeSet64 that provides checked calls to `Set`.
///
/// Specifically it requires that attributes are declared as part of the template
/// parameter pack.
///
/// NOTE: this will NOT work for all possible attributes/clusters, only for clusters
///       whose optional attributes all have IDs under 64. Static asserts are in place
///       to ensure that arguments to the template are valid.
///
/// Example usage:
///
///    namespace chip::app::Clusters::GeneralDiagnostics {
///
///    using OptionalAttributeSet64 = chip::app::OptionalAttributeSet64<
///        Attributes::TotalOperationalHours::Id,
///        Attributes::BootReason::Id,
///        Attributes::ActiveHardwareFaults::Id
///    >;
///
///    } // namespace chip::app::Clusters::GeneralDiagnostics
///
/// After this, one can:
///
///   GeneralDiagnostics::OptionalAttributeSet64()
///      .Set<GeneralDiagnostics::Attributes::TotalOperationalHours::Id>()
///      .Set<GeneralDiagnostics::Attributes::BootReason::Id>();
///
/// Cluster implementations can then store a
///   Constructor(const GeneralDiagnostics::OptionalAttributeSet64& optionalAttributeSet64) :
///   mOptionalAttributeSet64(optionalAttributeSet64) {...}
///
/// where:
///   const AttributeSet64 mOptionalAttributeSet64;
template <AttributeId... OptionalAttributeIds>
class OptionalAttributeSet64 : public AttributeSet64
{
public:
    explicit OptionalAttributeSet64(uint64_t initialValue) : AttributeSet64(initialValue & All()) {}
    OptionalAttributeSet64(const AttributeSet64 & initialValue) : AttributeSet64(initialValue) {}
    OptionalAttributeSet64() = default;

    template <AttributeId ATTRIBUTE_ID>
    constexpr OptionalAttributeSet64 & Set(bool value = true)
    {
        static_assert(ATTRIBUTE_ID < 64, "Cluster attribute bits supports attributes up to 63");
        static_assert(Internal::IsOneOf<ATTRIBUTE_ID, OptionalAttributeIds...>::value, "attribute MUST be optional");
        (void) AttributeSet64::Set(ATTRIBUTE_ID, value);
        return *this;
    }

    static constexpr uint64_t All()
    {
        if constexpr (sizeof...(OptionalAttributeIds) == 0)
        {
            return 0;
        }
        else
        {
            return ((1ULL << OptionalAttributeIds) | ...);
        }
    }
};

} // namespace ThreadNetworkDiagnostics

class ThreadNetworkDiagnosticsCluster : public DefaultServerCluster, public DeviceLayer::ThreadDiagnosticsDelegate
{
public:
    // NOTE: this set is smaller than the full optional attributes supported by time synchronization
    //       as other attributes are controlled by feature flags
    using OptionalAttributeSet =
        ThreadNetworkDiagnostics::OptionalAttributeSet64<ThreadNetworkDiagnostics::Attributes::ActiveTimestamp::Id,
                                                         ThreadNetworkDiagnostics::Attributes::PendingTimestamp::Id,
                                                         ThreadNetworkDiagnostics::Attributes::Delay::Id>;

    struct StartupConfiguration
    {
        ThreadNetworkDiagnostics::Attributes::ActiveTimestamp::TypeInfo::Type activeTs;
        ThreadNetworkDiagnostics::Attributes::PendingTimestamp::TypeInfo::Type pendingTs;
        ThreadNetworkDiagnostics::Attributes::Delay::TypeInfo::Type delay;
    };

    ThreadNetworkDiagnosticsCluster(EndpointId endpointId, const BitFlags<ThreadNetworkDiagnostics::Feature> features,
                                    const OptionalAttributeSet & optionalAttributeSet, const StartupConfiguration & config);

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown() override;

    void OnConnectionStatusChanged(ThreadNetworkDiagnostics::ConnectionStatusEnum newConnectionStatus) override;
    void OnNetworkFaultChanged(const DeviceLayer::GeneralFaults<DeviceLayer::kMaxNetworkFaults> & previous,
                               const DeviceLayer::GeneralFaults<DeviceLayer::kMaxNetworkFaults> & current) override;

private:
    const BitFlags<ThreadNetworkDiagnostics::Feature> mFeatures;
    const OptionalAttributeSet mOptionalAttributeSet;

    ThreadNetworkDiagnostics::Attributes::ActiveTimestamp::TypeInfo::Type mActiveTs;
    ThreadNetworkDiagnostics::Attributes::PendingTimestamp::TypeInfo::Type mPendingTs;
    ThreadNetworkDiagnostics::Attributes::Delay::TypeInfo::Type mDelay;

    std::optional<DataModel::ActionReturnStatus>
    HandleResetCounts(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                      const ThreadNetworkDiagnostics::Commands::ResetCounts::DecodableType & commandData);
};

} // namespace chip::app::Clusters
