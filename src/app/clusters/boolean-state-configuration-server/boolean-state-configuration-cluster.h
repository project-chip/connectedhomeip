/**
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/boolean-state-configuration-server/boolean-state-configuration-delegate.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/BooleanStateConfiguration/AttributeIds.h>
#include <clusters/BooleanStateConfiguration/ClusterId.h>

namespace chip::app::Clusters {

class BooleanStateConfigurationCluster : public DefaultServerCluster
{
public:
    using OptionalAttributesSet = OptionalAttributeSet<                     //
        BooleanStateConfiguration::Attributes::DefaultSensitivityLevel::Id, // optional if sensitivity level feature is enabled
        BooleanStateConfiguration::Attributes::AlarmsEnabled::Id,           // optional if VIS or Aud enabled
        BooleanStateConfiguration::Attributes::SensorFault::Id              // fully optional
        >;

    using AlarmModeBitMask   = chip::BitMask<BooleanStateConfiguration::AlarmModeBitmap>;
    using SensorFaultBitMask = chip::BitMask<BooleanStateConfiguration::SensorFaultBitmap>;

    BooleanStateConfigurationCluster(EndpointId endpointId, BitMask<BooleanStateConfiguration::Feature> features,
                                     OptionalAttributesSet optionalAttributes) :
        DefaultServerCluster({ endpointId, BooleanStateConfiguration::Id }), mFeatures(features),
        mOptionalAttributes(optionalAttributes)
    {}

    const BitMask<BooleanStateConfiguration::Feature> & GetFeatures() const { return mFeatures; }

    void SetDelegate(BooleanStateConfiguration::Delegate * delegate) { mDelegate = delegate; }
    BooleanStateConfiguration::Delegate * GetDelegate() const { return mDelegate; }

    Protocols::InteractionModel::Status SetAlarmsActive(AlarmModeBitMask alarms);
    Protocols::InteractionModel::Status SuppressAlarms(AlarmModeBitMask alarms);
    CHIP_ERROR SetCurrentSensitivityLevel(uint8_t level);
    void ClearAllAlarms();
    void EmitSensorFault(SensorFaultBitMask fault);
    Protocols::InteractionModel::Status SetAllEnabledAlarmsActive();

    // ServerClusterInterface/DefaultServerCluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

private:
    const BitMask<BooleanStateConfiguration::Feature> mFeatures;
    const OptionalAttributesSet mOptionalAttributes;
    BooleanStateConfiguration::Delegate * mDelegate = nullptr;

    // attributes that are maintained by this cluster
    uint8_t mCurrentSensitivityLevel    = 0;
    uint8_t mSupportedSensitivityLevels = 2;
    uint8_t mDefaultSensitivityLevel    = 0;
    AlarmModeBitMask mAlarmsActive{ 0 };
    AlarmModeBitMask mAlarmsSuppressed{ 0 };
    AlarmModeBitMask mAlarmsEnabled{ 0 };
    AlarmModeBitMask mAlarmsSupported{ 0 };
    SensorFaultBitMask mSensorFault{ 0 };

    void EmitAlarmsStateChangedEvent();
};

} // namespace chip::app::Clusters
