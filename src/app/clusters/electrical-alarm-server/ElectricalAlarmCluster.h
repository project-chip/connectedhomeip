/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/electrical-alarm-server/electrical-alarm-delegate.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/ElectricalAlarm/AttributeIds.h>
#include <clusters/ElectricalAlarm/ClusterId.h>
#include <clusters/ElectricalAlarm/Commands.h>
#include <clusters/ElectricalAlarm/Enums.h>
#include <clusters/ElectricalAlarm/Events.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalAlarm {

class ElectricalAlarmCluster : public DefaultServerCluster
{
public:
    struct Config
    {
        EndpointId endpointId;
        Delegate & delegate;
        BitMask<Feature> features;
    };

    explicit ElectricalAlarmCluster(const Config & config) :
        DefaultServerCluster({ config.endpointId, ElectricalAlarm::Id }), mDelegate(config.delegate),
        mFeatureFlags(config.features)
    {}

    // DefaultServerCluster overrides
    CHIP_ERROR Startup(ServerClusterContext & context) override;
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                               AttributeValueEncoder & encoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path,
                          ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

    // Alarm Base attribute setters — callable by the application
    // A change in Supported narrows Latch, then Mask, then State.
    Protocols::InteractionModel::Status SetSupportedValue(BitMask<AlarmBitmap> supported);
    // A change in Mask narrows State (drops newly-suppressed active alarms).
    Protocols::InteractionModel::Status SetMaskValue(BitMask<AlarmBitmap> mask);
    Protocols::InteractionModel::Status SetLatchValue(BitMask<AlarmBitmap> latch);
    // When ignoreLatchState=false, latched active bits are preserved and cannot be cleared here.
    Protocols::InteractionModel::Status SetStateValue(BitMask<AlarmBitmap> newState, bool ignoreLatchState = false);
    Protocols::InteractionModel::Status ResetLatchedAlarms(BitMask<AlarmBitmap> alarms);

    // Threshold attribute setters — return UnsupportedAttribute when the feature is absent.
    Protocols::InteractionModel::Status SetOverVoltageThreshold(int64_t value);
    Protocols::InteractionModel::Status SetUnderVoltageThreshold(int64_t value);
    Protocols::InteractionModel::Status SetOverFrequencyThreshold(int64_t value);
    Protocols::InteractionModel::Status SetUnderFrequencyThreshold(int64_t value);
    Protocols::InteractionModel::Status SetOverPowerThreshold(int64_t value);
    Protocols::InteractionModel::Status SetUnderPowerThreshold(int64_t value);
    Protocols::InteractionModel::Status SetOverCurrentThreshold(int64_t value);
    Protocols::InteractionModel::Status SetUnderCurrentThreshold(int64_t value);
    Protocols::InteractionModel::Status SetPowerImportThreshold(int64_t value);
    Protocols::InteractionModel::Status SetPowerExportThreshold(int64_t value);

    // Read-only accessors used by command handlers and tests
    BitMask<AlarmBitmap> GetMask() const { return mMask; }
    BitMask<AlarmBitmap> GetState() const { return mState; }
    BitMask<AlarmBitmap> GetSupported() const { return mSupported; }
    BitMask<AlarmBitmap> GetLatch() const { return mLatch; }

    bool HasFeature(Feature f) const { return mFeatureFlags.Has(f); }

private:
    Protocols::InteractionModel::Status HandleModifyEnabledAlarms(BitMask<AlarmBitmap> mask);
    Protocols::InteractionModel::Status HandleReset(BitMask<AlarmBitmap> alarms);
    Protocols::InteractionModel::Status HandleSetThresholds(const Commands::SetElectricalAlarmThresholds::DecodableType & data);

    void SendNotifyEvent(BitMask<AlarmBitmap> becameActive, BitMask<AlarmBitmap> becameInactive, BitMask<AlarmBitmap> newState,
                         BitMask<AlarmBitmap> mask);

    Delegate & mDelegate;
    const BitMask<Feature> mFeatureFlags;

    // Alarm Base attribute state
    BitMask<AlarmBitmap> mMask;
    BitMask<AlarmBitmap> mState;
    BitMask<AlarmBitmap> mSupported;
    BitMask<AlarmBitmap> mLatch;

    // Threshold attribute state (valid only when the corresponding feature is present)
    int64_t mOverVoltageThreshold   = 0;
    int64_t mUnderVoltageThreshold  = 0;
    int64_t mOverFrequencyThreshold = 0;
    int64_t mUnderFrequencyThreshold  = 0;
    int64_t mOverPowerThreshold     = 0;
    int64_t mUnderPowerThreshold    = 0;
    int64_t mOverCurrentThreshold   = 0;
    int64_t mUnderCurrentThreshold  = 0;
    int64_t mPowerImportThreshold   = 0;
    int64_t mPowerExportThreshold   = 0;
};

} // namespace ElectricalAlarm
} // namespace Clusters
} // namespace app
} // namespace chip
