/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/alarm-base-server/Delegate.h>
#include <app/clusters/alarm-base-server/alarm-base-cluster-objects.h>
#include <app/server-cluster/DefaultServerCluster.h>

namespace chip::app::Clusters {

/**
 * AlarmBaseCluster is the code-driven implementation of the Alarm Base cluster.
 *
 * It provides a base implementation for any derivation (e.g. DishwasherAlarm, RefrigeratorAlarm).
 */
class AlarmBaseCluster : public DefaultServerCluster
{
public:
    struct Config
    {
        BitMask<AlarmBase::Feature> feature;
        uint32_t clusterRevision = 0;
        AlarmBase::AlarmMap supported{};
        AlarmBase::AlarmMap latch{};
        bool supportsModifyEnabledAlarms = false;
        AlarmBase::Delegate * delegate   = nullptr;
    };

    AlarmBaseCluster(EndpointId endpointId, ClusterId clusterId, const Config & config);

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    // Application-facing API
    Protocols::InteractionModel::Status GetMaskValue(AlarmBase::AlarmMap * mask) const;
    Protocols::InteractionModel::Status GetLatchValue(AlarmBase::AlarmMap * latch) const;
    Protocols::InteractionModel::Status GetStateValue(AlarmBase::AlarmMap * state) const;
    Protocols::InteractionModel::Status GetSupportedValue(AlarmBase::AlarmMap * supported) const;

    Protocols::InteractionModel::Status SetMaskValue(const AlarmBase::AlarmMap mask);
    Protocols::InteractionModel::Status SetLatchValue(const AlarmBase::AlarmMap latch);
    Protocols::InteractionModel::Status SetSupportedValue(const AlarmBase::AlarmMap supported);
    Protocols::InteractionModel::Status SetStateValue(const AlarmBase::AlarmMap newState, bool ignoreLatchState = false);
    Protocols::InteractionModel::Status ResetLatchedAlarms(const AlarmBase::AlarmMap alarms);

    bool HasResetFeature() const { return mFeature.Has(AlarmBase::Feature::kReset); }

    void SetDelegate(AlarmBase::Delegate * delegate) { mDelegate = delegate; }

private:
    const BitMask<AlarmBase::Feature> mFeature;
    const uint32_t mClusterRevision;
    const bool mSupportsModifyEnabledAlarms;
    AlarmBase::Delegate * mDelegate;

    AlarmBase::AlarmMap mMask{};
    AlarmBase::AlarmMap mLatch{};
    AlarmBase::AlarmMap mState{};
    AlarmBase::AlarmMap mSupported{};

    void SendNotifyEvent(AlarmBase::AlarmMap becameActive, AlarmBase::AlarmMap becameInactive, AlarmBase::AlarmMap newState,
                         AlarmBase::AlarmMap mask);

    std::optional<DataModel::ActionReturnStatus> HandleReset(CommandHandler & commandObj, const ConcreteCommandPath & commandPath,
                                                             AlarmBase::AlarmMap alarms);

    std::optional<DataModel::ActionReturnStatus> HandleModifyEnabledAlarms(CommandHandler & commandObj,
                                                                           const ConcreteCommandPath & commandPath,
                                                                           AlarmBase::AlarmMap mask);
};

} // namespace chip::app::Clusters
