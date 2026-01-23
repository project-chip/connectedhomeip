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

#include <app/clusters/on-off-server/OnOffLightingCluster.h>

#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model/Nullable.h>
#include <app/persistence/AttributePersistence.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/OnOff/AttributeIds.h>
#include <clusters/OnOff/ClusterId.h>
#include <clusters/OnOff/Enums.h>
#include <clusters/OnOff/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters::OnOff;
using chip::Protocols::InteractionModel::Status;

namespace chip::app::Clusters {

OnOffLightingCluster::OnOffLightingCluster(EndpointId endpointId, const Context & context) :
    OnOffCluster(endpointId,
                 {
                     .timerDelegate = context.timerDelegate,
                     .featureMap    = context.featureMap,
                     .defaults =
                         OnOffCluster::Defaults{
                             .onOff = context.defaults.onOff,
                         },
                 },
                 { Feature::kLighting, Feature::kDeadFrontBehavior }),
    mEffectDelegate(context.effectDelegate), mScenesIntegrationDelegate(context.scenesIntegrationDelegate),
    mStartUpOnOff(context.defaults.startupOnOff), mStartupType(context.startupType)
{}

OnOffLightingCluster::~OnOffLightingCluster()
{
    mTimerDelegate.CancelTimer(this);
}

CHIP_ERROR OnOffLightingCluster::Startup(ServerClusterContext & context)
{
    // NOTE: direct base class is NOT called, because we want to give the delegate
    //       the correct on-off cluster setup.
    //
    // NOLINTNEXTLINE(bugprone-parent-virtual-call)
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    AttributePersistence attributePersistence(context.attributeStorage);

    attributePersistence.LoadNativeEndianValue(ConcreteAttributePath(mPath.mEndpointId, Clusters::OnOff::Id, Attributes::OnOff::Id),
                                               mOnOff, mOnOff);

    // Load StartUpOnOff. Defaults to "unchanged" (i.e. constructor default) if missing from storage.
    attributePersistence.LoadNativeEndianValue(
        ConcreteAttributePath(mPath.mEndpointId, Clusters::OnOff::Id, Attributes::StartUpOnOff::Id), mStartUpOnOff, mStartUpOnOff);

    if (mStartupType != StartupType::kOTA)
    {
        // Apply StartUpOnOff behavior
        if (!mStartUpOnOff.IsNull())
        {
            bool targetState = GetOnOff();
            switch (mStartUpOnOff.Value())
            {
            case StartUpOnOffEnum::kOff:
                targetState = false;
                break;
            case StartUpOnOffEnum::kOn:
                targetState = true;
                break;
            case StartUpOnOffEnum::kToggle:
                targetState = !targetState;
                break;
            default:
                // Invalid value, keep previous
                break;
            }
            if (mOnOff != targetState)
            {
                // If startup value modified the state, make sure we also persist it.
                // In practice this means "toggle" will flip it on every reboot.
                mOnOff = targetState;
                LogErrorOnFailure(mContext->attributeStorage.WriteValue(
                    ConcreteAttributePath(mPath.mEndpointId, Clusters::OnOff::Id, Attributes::OnOff::Id),
                    ByteSpan(reinterpret_cast<const uint8_t *>(&mOnOff), sizeof(mOnOff))));
            }
        }
    }

    for (auto & delegate : mDelegates)
    {
        delegate.OnOffStartup(mOnOff);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR OnOffLightingCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                  ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    static constexpr DataModel::AcceptedCommandEntry kLightingCommands[] = {
        Commands::Off::kMetadataEntry,
        Commands::On::kMetadataEntry,
        Commands::Toggle::kMetadataEntry,
        Commands::OffWithEffect::kMetadataEntry,
        Commands::OnWithRecallGlobalScene::kMetadataEntry,
        Commands::OnWithTimedOff::kMetadataEntry,
    };
    return builder.ReferenceExisting(kLightingCommands);
}

CHIP_ERROR OnOffLightingCluster::Attributes(const ConcreteClusterPath & path,
                                            ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    // Base attributes
    ReturnErrorOnFailure(OnOffCluster::Attributes(path, builder));

    static const DataModel::AttributeEntry kLightingAttributes[] = {
        Attributes::GlobalSceneControl::kMetadataEntry,
        Attributes::OnTime::kMetadataEntry,
        Attributes::OffWaitTime::kMetadataEntry,
        Attributes::StartUpOnOff::kMetadataEntry,
    };

    return listBuilder.Append(Span<const DataModel::AttributeEntry>(kLightingAttributes), {});
}

DataModel::ActionReturnStatus OnOffLightingCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                  AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Attributes::GlobalSceneControl::Id:
        return encoder.Encode(mGlobalSceneControl);
    case Attributes::OnTime::Id:
        return encoder.Encode(mOnTime);
    case Attributes::OffWaitTime::Id:
        return encoder.Encode(mOffWaitTime);
    case Attributes::StartUpOnOff::Id:
        return encoder.Encode(mStartUpOnOff);
    default:
        return OnOffCluster::ReadAttribute(request, encoder);
    }
}

DataModel::ActionReturnStatus OnOffLightingCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                   AttributeValueDecoder & decoder)
{
    return NotifyAttributeChangedIfSuccess(request.path.mAttributeId, WriteImpl(request, decoder));
}

DataModel::ActionReturnStatus OnOffLightingCluster::WriteImpl(const DataModel::WriteAttributeRequest & request,
                                                              AttributeValueDecoder & decoder)
{
    switch (request.path.mAttributeId)
    {
    case Attributes::OnTime::Id: {
        uint16_t value;
        ReturnErrorOnFailure(decoder.Decode(value));
        VerifyOrReturnValue(mOnTime != value, DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp);
        mOnTime = value;
        UpdateTimer();
        return Status::Success;
    }
    case Attributes::OffWaitTime::Id: {
        uint16_t value;
        ReturnErrorOnFailure(decoder.Decode(value));
        VerifyOrReturnValue(mOffWaitTime != value, DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp);
        mOffWaitTime = value;
        UpdateTimer();
        return Status::Success;
    }
    case Attributes::StartUpOnOff::Id: {
        AttributePersistence persistence(mContext->attributeStorage);
        return persistence.DecodeAndStoreNativeEndianValue(request.path, decoder, mStartUpOnOff);
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedWrite;
    }
}

std::optional<DataModel::ActionReturnStatus> OnOffLightingCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                 chip::TLV::TLVReader & input_arguments,
                                                                                 CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case Commands::Off::Id:
        return HandleOff();
    case Commands::On::Id:
        return HandleOn();
    case Commands::Toggle::Id:
        return HandleToggle();
    case Commands::OffWithEffect::Id:
        return HandleOffWithEffect(request, input_arguments);
    case Commands::OnWithRecallGlobalScene::Id:
        return HandleOnWithRecallGlobalScene(request);
    case Commands::OnWithTimedOff::Id:
        return HandleOnWithTimedOff(input_arguments);
    default:
        return Status::UnsupportedCommand;
    }
}

void OnOffLightingCluster::SetOnTime(uint16_t value)
{
    VerifyOrReturn(SetAttributeValue(mOnTime, value, Attributes::OnTime::Id));
    UpdateTimer();
}

void OnOffLightingCluster::SetOffWaitTime(uint16_t value)
{
    VerifyOrReturn(SetAttributeValue(mOffWaitTime, value, Attributes::OffWaitTime::Id));
    UpdateTimer();
}

CHIP_ERROR OnOffLightingCluster::SetStartupOnOff(DataModel::Nullable<OnOff::StartUpOnOffEnum> value)
{
    VerifyOrReturnValue(SetAttributeValue(mStartUpOnOff, value, Attributes::StartUpOnOff::Id), CHIP_NO_ERROR);

    if (mContext != nullptr)
    {
        NumericAttributeTraits<OnOff::StartUpOnOffEnum>::StorageType storageValue;
        DataModel::NullableToStorage(mStartUpOnOff, storageValue);

        ReturnErrorOnFailure(
            mContext->attributeStorage.WriteValue({ mPath.mEndpointId, OnOff::Id, Attributes::StartUpOnOff::Id },
                                                  { reinterpret_cast<const uint8_t *>(&storageValue), sizeof(storageValue) }));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR OnOffLightingCluster::SetOnOffWithTimeReset(bool on)
{
    ReturnErrorOnFailure(SetOnOff(on));

    // Spec: On receipt of a level control cluster command that:
    //   - causes the OnOff attribute to be set to FALSE
    // the server SHALL set the OnTime attribute to 0
    //
    // Note that here we only update things if mOnTime is not already 0
    if (!on && (mOnTime != 0))
    {
        mOnTime = 0;
        UpdateTimer();
        NotifyAttributeChanged(Attributes::OnTime::Id);
    }

    // Spec: On receipt of a level control cluster command that:
    //   - causes the OnOff attribute to be set to TRUE
    //   - if the value of the OnTime attribute is equal to 0
    // the server SHALL set the OffWaitTime attribute to 0
    //
    // Note that here we only update things if mOffWaitTime is not already 0
    if (on && (mOnTime == 0) && (mOffWaitTime != 0))
    {
        mOffWaitTime = 0;
        UpdateTimer();
        NotifyAttributeChanged(Attributes::OffWaitTime::Id);
    }

    return CHIP_NO_ERROR;
}

void OnOffLightingCluster::TimerFired()
{
    if (GetOnOff())
    {
        // we are on, see if we have to decrease the timer
        VerifyOrReturn(mOnTime > 0);

        // TIMED_ON state: we decrement OnTime  to see if we need to turn off
        mOnTime--;
        NotifyAttributeChanged(Attributes::OnTime::Id);

        // If timer is not yet 0, update the timer and keep going. Otherwise move to off state.
        VerifyOrReturn(mOnTime == 0, UpdateTimer());

        // transition TIMED_ON to OFF - clear off wait time and turn off
        SetOffWaitTime(0);
        LogErrorOnFailure(SetOnOff(false));
    }
    else
    {
        VerifyOrReturn(mOffWaitTime > 0);

        mOffWaitTime--;
        NotifyAttributeChanged(Attributes::OffWaitTime::Id);
        UpdateTimer();
    }
}

void OnOffLightingCluster::UpdateTimer()
{
    mTimerDelegate.CancelTimer(this);

    bool on = GetOnOff();

    const bool needTimer = [&]() {
        // Timer is needed if we did not reach 0 AND if value is not 0xFFFF (according to the spec):
        //    "... attribute is decremented every 1/10th second, unless its value equals 0xFFFF"
        if (on)
        {
            return (mOnTime != 0) && (mOnTime != 0xFFFF);
        }

        // not on, check off conditions
        return (mOffWaitTime != 0) && (mOffWaitTime != 0xFFFF);
    }();

    VerifyOrReturn(needTimer);

    // 100ms = 1/10th second
    LogErrorOnFailure(mTimerDelegate.StartTimer(this, System::Clock::Milliseconds32(100)));
}

DataModel::ActionReturnStatus OnOffLightingCluster::HandleOff()
{
    bool wasOn = GetOnOff();
    ReturnErrorOnFailure(SetOnOffFromCommand(false));

    if (wasOn && mScenesIntegrationDelegate != nullptr)
    {
        LogErrorOnFailure(mScenesIntegrationDelegate->MakeSceneInvalidForAllFabrics());
    }

    SetAttributeValue<uint16_t>(mOnTime, 0, Attributes::OnTime::Id);
    UpdateTimer();
    return Status::Success;
}

DataModel::ActionReturnStatus OnOffLightingCluster::HandleOn()
{
    bool wasOff = !GetOnOff();
    ReturnErrorOnFailure(SetOnOffFromCommand(true));

    // Spec requirement:
    //   This attribute SHALL be set to TRUE after the reception of a command which
    //   causes the OnOff attribute to be set to TRUE;
    SetAttributeValue(mGlobalSceneControl, true, Attributes::GlobalSceneControl::Id);

    if (wasOff && mScenesIntegrationDelegate != nullptr)
    {
        LogErrorOnFailure(mScenesIntegrationDelegate->MakeSceneInvalidForAllFabrics());
    }

    if (mOnTime == 0)
    {
        SetAttributeValue<uint16_t>(mOffWaitTime, 0, Attributes::OffWaitTime::Id);
    }
    UpdateTimer();
    return Status::Success;
}

DataModel::ActionReturnStatus OnOffLightingCluster::HandleToggle()
{
    bool on = GetOnOff();
    if (on)
    {
        return HandleOff();
    }

    return HandleOn();
}

DataModel::ActionReturnStatus OnOffLightingCluster::HandleOffWithEffect(const DataModel::InvokeRequest & request,
                                                                        chip::TLV::TLVReader & input_arguments)
{

    Commands::OffWithEffect::DecodableType commandData;
    ReturnErrorOnFailure(DataModel::Decode(input_arguments, commandData));

    if (mGlobalSceneControl)
    {
        if (mScenesIntegrationDelegate != nullptr)
        {
            LogErrorOnFailure(mScenesIntegrationDelegate->StoreCurrentGlobalScene(request.subjectDescriptor->fabricIndex));
        }

        DataModel::ActionReturnStatus status =
            mEffectDelegate.TriggerEffect(commandData.effectIdentifier, commandData.effectVariant);

        VerifyOrReturnValue(status.IsSuccess(), status);

        SetAttributeValue(mGlobalSceneControl, false, Attributes::GlobalSceneControl::Id);

        ReturnErrorOnFailure(SetOnOffFromCommand(false));

        SetAttributeValue<uint16_t>(mOnTime, 0, Attributes::OnTime::Id);

        UpdateTimer();
        return Status::Success;
    }

    return HandleOff();
}

DataModel::ActionReturnStatus OnOffLightingCluster::HandleOnWithRecallGlobalScene(const DataModel::InvokeRequest & request)
{
    if (mGlobalSceneControl)
    {
        return Status::Success; // Discard
    }

    if (mScenesIntegrationDelegate != nullptr)
    {
        CHIP_ERROR err = mScenesIntegrationDelegate->RecallGlobalScene(request.subjectDescriptor->fabricIndex);
        if (err != CHIP_NO_ERROR)
        {
            // Spec requirement:
            //   - If the GlobalSceneControl attribute is FALSE, the OnWithRecallGlobalScene command SHALL recall the global
            //     scene... If the scene cannot be recalled... it SHALL set the OnOff attribute to TRUE
            // Log and proceed to turning on.
            ChipLogError(Zcl, "Failed to recall global scene: %" CHIP_ERROR_FORMAT, err.Format());
            ReturnErrorOnFailure(SetOnOffFromCommand(true));
        }
    }
    else
    {
        ReturnErrorOnFailure(SetOnOffFromCommand(true));
    }

    SetAttributeValue(mGlobalSceneControl, true, Attributes::GlobalSceneControl::Id);

    if (mOnTime == 0)
    {
        SetAttributeValue<uint16_t>(mOffWaitTime, 0, Attributes::OffWaitTime::Id);
    }
    UpdateTimer();

    return Status::Success;
}

DataModel::ActionReturnStatus OnOffLightingCluster::HandleOnWithTimedOff(chip::TLV::TLVReader & input_arguments)
{
    Commands::OnWithTimedOff::DecodableType commandData;
    ReturnErrorOnFailure(DataModel::Decode(input_arguments, commandData));

    VerifyOrReturnError(commandData.onTime <= 0xFFFE, Status::ConstraintError);
    VerifyOrReturnError(commandData.offWaitTime <= 0xFFFE, Status::ConstraintError);

    if (commandData.onOffControl.Has(OnOffControlBitmap::kAcceptOnlyWhenOn) && !GetOnOff())
    {
        return Status::Success; // Discard
    }

    const bool wasOn = GetOnOff();

    // we have to turn the device on EXCEPT if we already in a TIMED_OFF state
    if (mOffWaitTime == 0 && !GetOnOff())
    {
        // NOTE: this sets directly the ONOFF without the "from command" because the state
        //       transitions are handled by OnWithTimedOff command directly
        LogErrorOnFailure(SetOnOff(true));

        // Spec requirement:
        //   This attribute SHALL be set to TRUE after the reception of a command which
        //   causes the OnOff attribute to be set to TRUE;
        SetAttributeValue(mGlobalSceneControl, true, Attributes::GlobalSceneControl::Id);
    }

    // state transition logic between ON/OFF/TIMED_ON/TIMED_OFF
    if (wasOn)
    {
        // we are in a form of ON state. Determine which.
        if (mOnTime > 0)
        {
            // TIMED_ON state, stay here
            SetOnTime(std::max(mOnTime, static_cast<uint16_t>(commandData.onTime)));
        }
        else
        {
            // ON -> TIMED_ON transition
            SetOnTime(commandData.onTime);
        }

        // in both cases, OffWaitTime stays to the user input value
        SetOffWaitTime(commandData.offWaitTime);
    }
    else
    {
        // We are in an off state, determine which.
        if (mOffWaitTime > 0)
        {
            // TIMED_OFF already, we IGNORE the OnTime (according to spec diagram)
            SetOffWaitTime(std::min(mOffWaitTime, commandData.offWaitTime));
        }
        else
        {
            // OFF -> TIMED_ON transition, keep user values
            SetOnTime(commandData.onTime);
            SetOffWaitTime(commandData.offWaitTime);
        }
    }

    // since times changed, make sure we update states as needed
    UpdateTimer();

    return Status::Success;
}

CHIP_ERROR OnOffLightingCluster::SetOnOffFromCommand(bool on)
{
    // if no change, do not attempt any transition
    VerifyOrReturnValue(GetOnOff() != on, CHIP_NO_ERROR);

    if (on)
    {
        // device turned on. we stop waiting for on again
        SetOffWaitTime(0);
    }
    else
    {
        // device turned off. Stop waiting to turn off after a timer
        SetOnTime(0);
    }

    return SetOnOff(on);
}

} // namespace chip::app::Clusters
