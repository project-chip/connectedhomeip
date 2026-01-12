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
#include <app/persistence/AttributePersistence.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/OnOff/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using chip::Protocols::InteractionModel::Status;

namespace chip::app::Clusters::OnOff {

OnOffLightingCluster::OnOffLightingCluster(EndpointId endpointId, TimerDelegate & timerDelegate,
                                           OnOffEffectDelegate & effectDelegate,
                                           chip::scenes::ScenesIntegrationDelegate * scenesIntegrationDelegate,
                                           BitMask<Feature> featureMap) :
    OnOffCluster(endpointId, timerDelegate, featureMap, { Feature::kLighting, Feature::kDeadFrontBehavior }),
    mEffectDelegate(effectDelegate), mScenesIntegrationDelegate(scenesIntegrationDelegate)
{}

OnOffLightingCluster::~OnOffLightingCluster()
{
    mTimerDelegate.CancelTimer(this);
}

CHIP_ERROR OnOffLightingCluster::Startup(ServerClusterContext & context)
{
    // NOTE: direct base class is NOT called, because we want to give the delegate
    //       the correct on-off clusters setup.
    //
    // NOLINTNEXTLINE(bugprone-parent-virtual-call)
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    AttributePersistence attributePersistence(context.attributeStorage);

    attributePersistence.LoadNativeEndianValue(ConcreteAttributePath(mPath.mEndpointId, Clusters::OnOff::Id, Attributes::OnOff::Id),
                                               mOnOff, false);

    // Load StartUpOnOff. Defaults to NULL (default value for nullable) if not found in storage.
    attributePersistence.LoadNativeEndianValue(
        ConcreteAttributePath(mPath.mEndpointId, Clusters::OnOff::Id, Attributes::StartUpOnOff::Id), mStartUpOnOff, {});

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
        mOnOff = targetState;
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

CHIP_ERROR OnOffLightingCluster::SetOnOffWithTimeReset(bool on)
{
    ReturnErrorOnFailure(SetOnOff(on));

    if (!on && (mOnTime != 0))
    {
        mOnTime = 0;
        NotifyAttributeChanged(Attributes::OnTime::Id);
    }

    if (on && (mOffWaitTime != 0))
    {
        mOffWaitTime = 0;
        NotifyAttributeChanged(Attributes::OffWaitTime::Id);
    }

    return CHIP_NO_ERROR;
}

void OnOffLightingCluster::TimerFired()
{
    bool on      = GetOnOff();
    bool changed = false;

    if (on && mOnTime > 0)
    {
        mOnTime--;
        changed = true;
        if (mOnTime == 0)
        {
            mOffWaitTime = 0;
            // Best effort update logic
            // Since SetOnOff triggers storage writes etc, we just use it.
            LogErrorOnFailure(SetOnOff(false));
            NotifyAttributeChanged(Attributes::OffWaitTime::Id);
            on = false; // Updated state
        }
        NotifyAttributeChanged(Attributes::OnTime::Id);
    }
    else if (!on && mOffWaitTime > 0)
    {
        mOffWaitTime--;
        changed = true;
        NotifyAttributeChanged(Attributes::OffWaitTime::Id);
    }

    // Schedule next tick if needed
    if (changed)
    {
        UpdateTimer();
    }
}

void OnOffLightingCluster::UpdateTimer()
{
    mTimerDelegate.CancelTimer(this);

    bool on = GetOnOff();
    if ((on && mOnTime > 0) || (!on && mOffWaitTime > 0))
    {
        // 100ms = 1/10th second
        LogErrorOnFailure(mTimerDelegate.StartTimer(this, System::Clock::Milliseconds32(100)));
    }
}

DataModel::ActionReturnStatus OnOffLightingCluster::HandleOff()
{
    bool wasOn = GetOnOff();
    ReturnErrorOnFailure(SetOnOff(false));

    if (wasOn && mScenesIntegrationDelegate != nullptr)
    {
        LogErrorOnFailure(mScenesIntegrationDelegate->MakeSceneInvalidForAllFabrics());
    }

    mOnTime = 0;
    NotifyAttributeChanged(Attributes::OnTime::Id);
    UpdateTimer();
    return Status::Success;
}

DataModel::ActionReturnStatus OnOffLightingCluster::HandleOn()
{
    bool wasOff = !GetOnOff();
    ReturnErrorOnFailure(SetOnOff(true));

    if (wasOff && mScenesIntegrationDelegate != nullptr)
    {
        LogErrorOnFailure(mScenesIntegrationDelegate->MakeSceneInvalidForAllFabrics());
    }

    if (mOnTime == 0)
    {
        mOffWaitTime = 0;
        NotifyAttributeChanged(Attributes::OffWaitTime::Id);
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

        mGlobalSceneControl = false;
        NotifyAttributeChanged(Attributes::GlobalSceneControl::Id);

        ReturnErrorOnFailure(SetOnOff(false));

        mOnTime = 0;
        NotifyAttributeChanged(Attributes::OnTime::Id);
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
            ReturnErrorOnFailure(SetOnOff(true));
        }
    }
    else
    {
        ReturnErrorOnFailure(SetOnOff(true));
    }

    mGlobalSceneControl = true;
    NotifyAttributeChanged(Attributes::GlobalSceneControl::Id);

    if (mOnTime == 0)
    {
        mOffWaitTime = 0;
        NotifyAttributeChanged(Attributes::OffWaitTime::Id);
    }
    UpdateTimer();

    return Status::Success;
}

DataModel::ActionReturnStatus OnOffLightingCluster::HandleOnWithTimedOff(chip::TLV::TLVReader & input_arguments)
{
    Commands::OnWithTimedOff::DecodableType commandData;
    ReturnErrorOnFailure(DataModel::Decode(input_arguments, commandData));

    if (commandData.onOffControl.Has(OnOffControlBitmap::kAcceptOnlyWhenOn) && !GetOnOff())
    {
        return Status::Success; // Discard
    }

    if (mOffWaitTime > 0 && !GetOnOff())
    {
        mOffWaitTime = std::min(mOffWaitTime, commandData.offWaitTime);
        NotifyAttributeChanged(Attributes::OffWaitTime::Id);
    }
    else
    {
        mOnTime      = std::max(mOnTime, static_cast<uint16_t>(commandData.onTime));
        mOffWaitTime = commandData.offWaitTime;
        NotifyAttributeChanged(Attributes::OnTime::Id);
        NotifyAttributeChanged(Attributes::OffWaitTime::Id);

        // This command turns the device ON.
        LogErrorOnFailure(SetOnOff(true));
    }

    UpdateTimer();
    return Status::Success;
}

} // namespace chip::app::Clusters::OnOff
