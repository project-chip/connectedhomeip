/*
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

#include "OnOffPlugWithSensors.h"

#include <devices/Types.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;
using chip::Protocols::InteractionModel::Status;

namespace chip::app {

namespace {

constexpr DataModel::DeviceTypeEntry kDeviceTypes[] = {
    Device::Type::kOnOffPlugInUnit,
    Device::Type::kElectricalSensor,
};

constexpr int64_t kMillisecondsPerHour = 3'600'000;

const ElectricalEnergyMeasurement::Structs::MeasurementAccuracyRangeStruct::Type kEnergyAccuracyRange{
    .rangeMin = 0, .rangeMax = 1'000'000'000, .percentMax = MakeOptional(uint8_t{ 100 })
};

const ElectricalEnergyMeasurement::Structs::MeasurementAccuracyStruct::Type kEnergyAccuracy{
    .measurementType  = ElectricalEnergyMeasurement::MeasurementTypeEnum::kElectricalEnergy,
    .measured         = false,
    .minMeasuredValue = 0,
    .maxMeasuredValue = 1'000'000'000,
    .accuracyRanges   = Span(&kEnergyAccuracyRange, 1),
};

const ElectricalPowerMeasurement::Structs::MeasurementAccuracyRangeStruct::Type kPowerAccuracyRange{
    .rangeMin = 0, .rangeMax = OnOffPlugWithSensors::kOnPower_mW, .percentMax = MakeOptional(uint8_t{ 100 })
};

const ElectricalPowerMeasurement::Structs::MeasurementAccuracyRangeStruct::Type kVoltageAccuracyRange{
    .rangeMin = 0, .rangeMax = OnOffPlugWithSensors::kVoltage_mV, .percentMax = MakeOptional(uint8_t{ 100 })
};

const ElectricalPowerMeasurement::Structs::MeasurementAccuracyRangeStruct::Type kCurrentAccuracyRange{
    .rangeMin = 0, .rangeMax = OnOffPlugWithSensors::kOnCurrent_mA, .percentMax = MakeOptional(uint8_t{ 100 })
};

const ElectricalPowerMeasurement::Structs::MeasurementAccuracyStruct::Type kPowerAccuracies[] = {
    {
        .measurementType  = ElectricalPowerMeasurement::MeasurementTypeEnum::kActivePower,
        .measured         = true,
        .minMeasuredValue = 0,
        .maxMeasuredValue = OnOffPlugWithSensors::kOnPower_mW,
        .accuracyRanges   = Span(&kPowerAccuracyRange, 1),
    },
    {
        .measurementType  = ElectricalPowerMeasurement::MeasurementTypeEnum::kVoltage,
        .measured         = true,
        .minMeasuredValue = 0,
        .maxMeasuredValue = OnOffPlugWithSensors::kVoltage_mV,
        .accuracyRanges   = Span(&kVoltageAccuracyRange, 1),
    },
    {
        .measurementType  = ElectricalPowerMeasurement::MeasurementTypeEnum::kActiveCurrent,
        .measured         = true,
        .minMeasuredValue = 0,
        .maxMeasuredValue = OnOffPlugWithSensors::kOnCurrent_mA,
        .accuracyRanges   = Span(&kCurrentAccuracyRange, 1),
    },
};

} // namespace

OnOffPlugWithSensors::OnOffPlugWithSensors(const Context & context) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(kDeviceTypes)), mContext(context)
{}

CHIP_ERROR OnOffPlugWithSensors::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                          EndpointComposition composition)
{
    VerifyOrReturnError(GetEndpointId() == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    if (composition.tagList.empty())
    {
        composition.tagList = mContext.tagList;
    }
    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mLastEnergyUpdate = mContext.timerDelegate.GetCurrentMonotonicTimestamp();

    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mContext.timerDelegate).WithDelegate(&mContext.identifyDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    mScenesTableProvider.SetEndpoint(endpoint);
    mScenesManagementCluster.Create(endpoint,
                                    ScenesManagementCluster::Context{
                                        .groupDataProvider  = &mContext.groupDataProvider,
                                        .fabricTable        = &mContext.fabricTable,
                                        .features           = {},
                                        .sceneTableProvider = mScenesTableProvider,
                                        .supportsCopyScene  = true,
                                    });
    ReturnErrorOnFailure(provider.AddCluster(mScenesManagementCluster.Registration()));

    mOnOffCluster.Create(endpoint,
                         OnOffLightingCluster::Context{
                             .timerDelegate             = mContext.timerDelegate,
                             .effectDelegate            = *this,
                             .scenesIntegrationDelegate = &mScenesManagementCluster.Cluster(),
                         });
    mOnOffCluster.Cluster().AddDelegate(this);
    ReturnErrorOnFailure(provider.AddCluster(mOnOffCluster.Registration()));

    mGroupsCluster.Create(endpoint,
                          GroupsCluster::Context{
                              .groupDataProvider   = mContext.groupDataProvider,
                              .scenesIntegration   = &mScenesManagementCluster.Cluster(),
                              .identifyIntegration = &mIdentifyCluster.Cluster(),
                          });
    ReturnErrorOnFailure(provider.AddCluster(mGroupsCluster.Registration()));

    {
        ScopedSceneTable table(mScenesTableProvider);
        table->RegisterHandler(&mOnOffCluster.Cluster());
    }

    mEnergyMeasurementCluster.Create(EnergyMeasurementClusterT::Config{
        .endpointId         = endpoint,
        .featureFlags       = BitMask<ElectricalEnergyMeasurement::Feature>(ElectricalEnergyMeasurement::Feature::kImportedEnergy,
                                                                      ElectricalEnergyMeasurement::Feature::kCumulativeEnergy),
        .optionalAttributes = EnergyMeasurementClusterT::OptionalAttributesSet(),
        .accuracyStruct     = kEnergyAccuracy,
        .delegate           = *this,
        .timerDelegate      = mContext.timerDelegate,
    });
    ReturnErrorOnFailure(provider.AddCluster(mEnergyMeasurementCluster.Registration()));

    mPowerMeasurementCluster.Create(PowerMeasurementClusterT::Config{
        .endpointId = endpoint,
        .delegate   = *this,
        .features   = BitMask<ElectricalPowerMeasurement::Feature>(ElectricalPowerMeasurement::Feature::kAlternatingCurrent),
        .optionalAttributes = PowerMeasurementClusterT::OptionalAttributesSet()
                                  .Set<ElectricalPowerMeasurement::Attributes::Voltage::Id>()
                                  .Set<ElectricalPowerMeasurement::Attributes::ActiveCurrent::Id>(),
    });
    ReturnErrorOnFailure(provider.AddCluster(mPowerMeasurementCluster.Registration()));

    mPowerTopologyCluster.Create(PowerTopologyClusterT::Config{
        .endpointId = endpoint,
        .delegate   = *this,
        .features   = BitMask<PowerTopology::Feature>(PowerTopology::Feature::kNodeTopology),
    });
    ReturnErrorOnFailure(provider.AddCluster(mPowerTopologyCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();

    mEnergyMeasurementCluster.Cluster().GenerateSnapshots();
    CHIP_ERROR err = mContext.timerDelegate.StartTimer(this, kEnergySnapshotInterval);
    LogErrorOnFailure(err);
    return CHIP_NO_ERROR;
}

void OnOffPlugWithSensors::Unregister(CodeDrivenDataModelProvider & provider)
{
    mContext.timerDelegate.CancelTimer(this);
    UnregisterDescriptor(provider);

    if (mPowerTopologyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mPowerTopologyCluster.Cluster()));
        mPowerTopologyCluster.Destroy();
    }

    if (mPowerMeasurementCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mPowerMeasurementCluster.Cluster()));
        mPowerMeasurementCluster.Destroy();
    }

    if (mEnergyMeasurementCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mEnergyMeasurementCluster.Cluster()));
        mEnergyMeasurementCluster.Destroy();
    }

    if (mGroupsCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mGroupsCluster.Cluster()));
        mGroupsCluster.Destroy();
    }

    if (mOnOffCluster.IsConstructed())
    {
        if (mOnOffCluster.Cluster().IsInList())
        {
            ScopedSceneTable table(mScenesTableProvider);
            table->UnregisterHandler(&mOnOffCluster.Cluster());
        }

        LogErrorOnFailure(provider.RemoveCluster(&mOnOffCluster.Cluster()));
        mOnOffCluster.Cluster().RemoveDelegate(this);
        mOnOffCluster.Destroy();
    }

    if (mScenesManagementCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mScenesManagementCluster.Cluster()));
        mScenesManagementCluster.Destroy();
    }

    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
}

void OnOffPlugWithSensors::OnOffStartup(bool on)
{
    ChipLogProgress(DeviceLayer, "OnOffPlugWithSensors: startup %s", on ? "ON" : "OFF");
    AccumulateEnergy();
    mIsOn = on;
}

void OnOffPlugWithSensors::OnOnOffChanged(bool on)
{
    ChipLogProgress(DeviceLayer, "OnOffPlugWithSensors: turned %s", on ? "ON" : "OFF");
    AccumulateEnergy();
    mIsOn = on;
    if (mPowerMeasurementCluster.IsConstructed())
    {
        mPowerMeasurementCluster.Cluster().MarkAttributeDirty(ElectricalPowerMeasurement::Attributes::ActivePower::Id);
        mPowerMeasurementCluster.Cluster().MarkAttributeDirty(ElectricalPowerMeasurement::Attributes::ActiveCurrent::Id);
    }
    if (mEnergyMeasurementCluster.IsConstructed())
    {
        mEnergyMeasurementCluster.Cluster().GenerateSnapshots();
    }
}

DataModel::ActionReturnStatus OnOffPlugWithSensors::TriggerDelayedAllOff(OnOff::DelayedAllOffEffectVariantEnum variant)
{
    switch (variant)
    {
    case OnOff::DelayedAllOffEffectVariantEnum::kDelayedOffFastFade:
    case OnOff::DelayedAllOffEffectVariantEnum::kNoFade:
    case OnOff::DelayedAllOffEffectVariantEnum::kDelayedOffSlowFade:
        ChipLogProgress(DeviceLayer, "OnOffPlugWithSensors: DelayedAllOff variant %u", static_cast<unsigned>(variant));
        return Status::Success;
    default:
        return Status::ConstraintError;
    }
}

DataModel::ActionReturnStatus OnOffPlugWithSensors::TriggerDyingLight(OnOff::DyingLightEffectVariantEnum variant)
{
    switch (variant)
    {
    case OnOff::DyingLightEffectVariantEnum::kDyingLightFadeOff:
        ChipLogProgress(DeviceLayer, "OnOffPlugWithSensors: DyingLight variant %u", static_cast<unsigned>(variant));
        return Status::Success;
    default:
        return Status::ConstraintError;
    }
}

DataModel::Nullable<int64_t> OnOffPlugWithSensors::GetCumulativeEnergyImported()
{
    AccumulateEnergy();
    return DataModel::MakeNullable(mEnergyImported_mWms / kMillisecondsPerHour);
}

uint8_t OnOffPlugWithSensors::GetNumberOfMeasurementTypes()
{
    return static_cast<uint8_t>(MATTER_ARRAY_SIZE(kPowerAccuracies));
}

CHIP_ERROR OnOffPlugWithSensors::GetAccuracyByIndex(uint8_t index,
                                                    ElectricalPowerMeasurement::Structs::MeasurementAccuracyStruct::Type & value)
{
    VerifyOrReturnError(index < MATTER_ARRAY_SIZE(kPowerAccuracies), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    value = kPowerAccuracies[index];
    return CHIP_NO_ERROR;
}

void OnOffPlugWithSensors::TimerFired()
{
    VerifyOrReturn(mEnergyMeasurementCluster.IsConstructed());
    mEnergyMeasurementCluster.Cluster().GenerateSnapshots();
    CHIP_ERROR err = mContext.timerDelegate.StartTimer(this, kEnergySnapshotInterval);
    LogErrorOnFailure(err);
}

void OnOffPlugWithSensors::AccumulateEnergy()
{
    System::Clock::Timestamp now = mContext.timerDelegate.GetCurrentMonotonicTimestamp();
    if (mIsOn)
    {
        mEnergyImported_mWms += kOnPower_mW * static_cast<int64_t>((now - mLastEnergyUpdate).count());
    }
    mLastEnergyUpdate = now;
}

} // namespace chip::app
