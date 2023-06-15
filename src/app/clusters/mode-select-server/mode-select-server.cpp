/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/InteractionModelEngine.h>
#include <app/clusters/mode-select-server/mode-select-server.h>
#include <app/clusters/on-off-server/on-off-server.h>
#include <app/util/attribute-storage.h>
#include <platform/DiagnosticDataProvider.h>

// using namespace std;
using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ModeSelect;
using chip::Protocols::InteractionModel::Status;

using BootReasonType = GeneralDiagnostics::BootReasonEnum;

namespace chip {
namespace app {
namespace Clusters {
namespace ModeSelect {

// todo find a cleaner solution by modifying the zap generated code.
EmberAfStatus Instance::GetFeature(uint32_t * value) const
{
    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    EmberAfStatus status =
        emberAfReadAttribute(endpointId, clusterId, ModeSelect::Attributes::FeatureMap::Id, readable, sizeof(temp));
    VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return EMBER_ZCL_STATUS_CONSTRAINT_ERROR;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

EmberAfStatus Instance::SetFeatureMap(uint32_t value) const
{
    using Traits = NumericAttributeTraits<uint32_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return EMBER_ZCL_STATUS_CONSTRAINT_ERROR;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpointId, clusterId, ModeSelect::Attributes::FeatureMap::Id, writable,
                                 ZCL_BITMAP32_ATTRIBUTE_TYPE);
}

EmberAfStatus Instance::GetCurrentMode(uint8_t * value) const
{
    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    EmberAfStatus status =
        emberAfReadAttribute(endpointId, clusterId, ModeSelect::Attributes::CurrentMode::Id, readable, sizeof(temp));
    VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return EMBER_ZCL_STATUS_CONSTRAINT_ERROR;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

EmberAfStatus Instance::SetCurrentMode(uint8_t value) const
{
    using Traits = NumericAttributeTraits<uint8_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return EMBER_ZCL_STATUS_CONSTRAINT_ERROR;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpointId, clusterId, ModeSelect::Attributes::CurrentMode::Id, writable,
                                 ZCL_INT8U_ATTRIBUTE_TYPE);
}

EmberAfStatus Instance::GetOnMode(DataModel::Nullable<uint8_t> & value) const
{
    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp;
    uint8_t * readable   = Traits::ToAttributeStoreRepresentation(temp);
    EmberAfStatus status = emberAfReadAttribute(endpointId, clusterId, ModeSelect::Attributes::OnMode::Id, readable, sizeof(temp));
    VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == status, status);
    if (Traits::IsNullValue(temp))
    {
        value.SetNull();
    }
    else
    {
        value.SetNonNull(Traits::StorageToWorking(temp));
    }
    return status;
}

EmberAfStatus Instance::SetOnMode(uint8_t value) const
{
    using Traits = NumericAttributeTraits<uint8_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ true, value))
    {
        return EMBER_ZCL_STATUS_CONSTRAINT_ERROR;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpointId, clusterId, ModeSelect::Attributes::OnMode::Id, writable, ZCL_INT8U_ATTRIBUTE_TYPE);
}

EmberAfStatus Instance::SetOnModeNull() const
{
    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType value;
    Traits::SetNull(value);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(value);
    return emberAfWriteAttribute(endpointId, clusterId, ModeSelect::Attributes::OnMode::Id, writable, ZCL_INT8U_ATTRIBUTE_TYPE);
}

EmberAfStatus Instance::GetStartUpMode(DataModel::Nullable<uint8_t> & value) const
{
    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    EmberAfStatus status =
        emberAfReadAttribute(endpointId, clusterId, ModeSelect::Attributes::StartUpMode::Id, readable, sizeof(temp));
    VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == status, status);
    if (Traits::IsNullValue(temp))
    {
        value.SetNull();
    }
    else
    {
        value.SetNonNull(Traits::StorageToWorking(temp));
    }
    return status;
}

EmberAfStatus Instance::SetStartUpMode(uint8_t value) const
{
    using Traits = NumericAttributeTraits<uint8_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ true, value))
    {
        return EMBER_ZCL_STATUS_CONSTRAINT_ERROR;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpointId, clusterId, ModeSelect::Attributes::StartUpMode::Id, writable,
                                 ZCL_INT8U_ATTRIBUTE_TYPE);
}

EmberAfStatus Instance::SetStartUpModeNull() const
{
    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType value;
    Traits::SetNull(value);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(value);
    return emberAfWriteAttribute(endpointId, clusterId, ModeSelect::Attributes::StartUpMode::Id, writable,
                                 ZCL_INT8U_ATTRIBUTE_TYPE);
}

std::map<uint32_t, Instance*> Instance::ModeSelectAliasesInstanceMap;

CHIP_ERROR Instance::Init()
{
    // Check that the cluster ID given is a valid mode select alias cluster ID.
    if (!std::any_of(AliasedClusters.begin(), AliasedClusters.end(), [this](ClusterId i){return i == clusterId;}))
    {
        ChipLogError(Zcl, "ModeSelect: The cluster with ID %lu is not a mode select alias.", long(clusterId));
        return CHIP_ERROR_INVALID_ARGUMENT; // todo is this the correct error?
    }

    // Check if the cluster has been selected in zap
    if (!emberAfContainsServer(endpointId, clusterId)) {
        ChipLogError(Zcl, "ModeSelect: The cluster with ID %lu was not enabled in zap.", long(clusterId));
        return CHIP_ERROR_INVALID_ARGUMENT; // todo is this the correct error?
    }

    ReturnErrorOnFailure(chip::app::InteractionModelEngine::GetInstance()->RegisterCommandHandler(this));
    VerifyOrReturnError(registerAttributeAccessOverride(this), CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(delegate->Init());

    ModeSelectAliasesInstanceMap[clusterId] = this;

    // StartUp behavior relies on CurrentMode StartUpMode attributes being non-volatile.
    if (!emberAfIsKnownVolatileAttribute(endpointId, clusterId, Attributes::CurrentMode::Id) &&
        !emberAfIsKnownVolatileAttribute(endpointId, clusterId, Attributes::StartUpMode::Id))
    {
        // Read the StartUpMode attribute and set the CurrentMode attribute
        // The StartUpMode attribute SHALL define the desired startup behavior of a
        // device when it is supplied with power and this state SHALL be
        // reflected in the CurrentMode attribute.  The values of the StartUpMode
        // attribute are listed below.

        DataModel::Nullable<uint8_t> startUpMode;
        EmberAfStatus status = GetStartUpMode(startUpMode);
        if (status == EMBER_ZCL_STATUS_SUCCESS && !startUpMode.IsNull())
        {
#ifdef EMBER_AF_PLUGIN_ON_OFF
            // OnMode with Power Up
            // If the On/Off feature is supported and the On/Off cluster attribute StartUpOnOff is present, with a
            // value of On (turn on at power up), then the CurrentMode attribute SHALL be set to the OnMode attribute
            // value when the server is supplied with power, except if the OnMode attribute is null.
            if (emberAfContainsServer(endpointId, OnOff::Id) &&
                emberAfContainsAttribute(endpointId, OnOff::Id, OnOff::Attributes::StartUpOnOff::Id) &&
                emberAfContainsAttribute(endpointId, ModeSelect::Id, ModeSelect::Attributes::OnMode::Id) &&
                HasFeature(ModeSelectFeature::kDeponoff))
            {
                DataModel::Nullable<uint8_t> onMode;
                bool onOffValueForStartUp = false;
                if (GetOnMode(onMode) == EMBER_ZCL_STATUS_SUCCESS &&
                    !emberAfIsKnownVolatileAttribute(endpointId, OnOff::Id, OnOff::Attributes::StartUpOnOff::Id) &&
                    OnOffServer::Instance().getOnOffValueForStartUp(endpointId, onOffValueForStartUp) == EMBER_ZCL_STATUS_SUCCESS)
                {
                    if (onOffValueForStartUp && !onMode.IsNull())
                    {
                        ChipLogDetail(Zcl, "ModeSelect: CurrentMode is overwritten by OnMode");
                        // it is overwritten by the on/off cluster.
                        return CHIP_NO_ERROR;
                    }
                }
            }
#endif // EMBER_AF_PLUGIN_ON_OFF

            BootReasonType bootReason = BootReasonType::kUnspecified;
            CHIP_ERROR error          = DeviceLayer::GetDiagnosticDataProvider().GetBootReason(bootReason);

            if (error != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Unable to retrieve boot reason: %" CHIP_ERROR_FORMAT, error.Format());
                // We really only care whether the boot reason is OTA.  Assume it's not.
                bootReason = BootReasonType::kUnspecified;
            }
            if (bootReason == BootReasonType::kSoftwareUpdateCompleted)
            {
                ChipLogDetail(Zcl, "ModeSelect: CurrentMode is ignored for OTA reboot");
                return CHIP_NO_ERROR;
            }

            // Initialise currentMode to 0
            uint8_t currentMode = 0;
            status              = GetCurrentMode(&currentMode);

            if ((status == EMBER_ZCL_STATUS_SUCCESS) && (startUpMode.Value() != currentMode))
            {
                status = SetCurrentMode(startUpMode.Value());
                if (status != EMBER_ZCL_STATUS_SUCCESS)
                {
                    ChipLogError(Zcl, "ModeSelect: Error initializing CurrentMode, EmberAfStatus code 0x%02x", status);
                    // todo return some error
                }
                else
                {
                    ChipLogProgress(Zcl, "ModeSelect: Successfully initialized CurrentMode to %u", startUpMode.Value());
                }
            }
        }
    }
    else
    {
        ChipLogProgress(Zcl, "ModeSelect: Skipped initializing CurrentMode by StartUpMode because one of them is volatile");
    }

    return CHIP_NO_ERROR;
}

template <typename RequestT, typename FuncT>
void Instance::HandleCommand(HandlerContext & handlerContext, FuncT func)
{
    if (!handlerContext.mCommandHandled && (handlerContext.mRequestPath.mCommandId == RequestT::GetCommandId()))
    {
        RequestT requestPayload;

        //
        // If the command matches what the caller is looking for, let's mark this as being handled
        // even if errors happen after this. This ensures that we don't execute any fall-back strategies
        // to handle this command since at this point, the caller is taking responsibility for handling
        // the command in its entirety, warts and all.
        //
        handlerContext.SetCommandHandled();

        if (DataModel::Decode(handlerContext.mPayload, requestPayload) != CHIP_NO_ERROR)
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath,
                                                     Protocols::InteractionModel::Status::InvalidCommand);
            return;
        }

        func(handlerContext, requestPayload);
    }
}

void Instance::HandleChangeToMode(HandlerContext & ctx, const Commands::ChangeToMode::DecodableType & commandData)
{
    uint8_t newMode = commandData.newMode;

    if (!delegate->IsSupportedMode(newMode))
    {
        ChipLogError(Zcl, "ModeSelect: Failed to find the option with mode %u", newMode);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
        return;
    }

    Status checkIsChangeToThisModeAllowed = delegate->HandleChangeToMode(newMode);
    if (Status::Success != checkIsChangeToThisModeAllowed)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, checkIsChangeToThisModeAllowed);
        return;
    }

    SetCurrentMode(newMode);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Success);
    ChipLogProgress(Zcl, "ModeSelect: ChangeToMode changed to mode %u", newMode);
}

void Instance::HandleChangeToModeWithStatus(HandlerContext & ctx,
                                            const Commands::ChangeToModeWithStatus::DecodableType & commandData)
{
    uint8_t newMode = commandData.newMode;

    ModeSelect::Commands::ChangeToModeResponse::Type response;

    if (!delegate->IsSupportedMode(newMode))
    {
        ChipLogError(Zcl, "ModeSelect: Failed to find the option with mode %u", newMode);
        response.status = static_cast<uint8_t>(ModeSelect::ChangeToModeResponseStatus::kUnsupportedMode);
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        return;
    }

    delegate->HandleChangeToModeWitheStatus(newMode, response);

    if (response.status == static_cast<uint8_t>(ChangeToModeResponseStatus::kSuccess))
    {
        SetCurrentMode(newMode);
        ChipLogProgress(Zcl, "ModeSelect: HandleChangeToModeWitheStatus changed to mode %u", newMode);
    }

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

// This function is called by the interaction model engine when a command destined for this instance is received.
void Instance::InvokeCommand(HandlerContext & handlerContext)
{
    switch (handlerContext.mRequestPath.mCommandId)
    {
    case ModeSelect::Commands::ChangeToMode::Id:
        ChipLogDetail(Zcl, "ModeSelect: Entering handling ChangeToMode");

        HandleCommand<Commands::ChangeToMode::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleChangeToMode(ctx, commandData); });
        break;
    case ModeSelect::Commands::ChangeToModeWithStatus::Id:
        ChipLogDetail(Zcl, "ModeSelect: Entering handling ChangeToModeWithStatus");

        HandleCommand<Commands::ChangeToModeWithStatus::DecodableType>(
            handlerContext,
            [this](HandlerContext & ctx, const auto & commandData) { HandleChangeToModeWithStatus(ctx, commandData); });
    }
}


bool Instance::HasFeature(ModeSelectFeature feature) const
{
    bool success;
    uint32_t featureMap;
    success = (GetFeature(&featureMap) == EMBER_ZCL_STATUS_SUCCESS);

    return success && ((featureMap & to_underlying(feature)) != 0);
}

// List the commands supported by this instance.
CHIP_ERROR Instance::EnumerateAcceptedCommands(const ConcreteClusterPath & cluster,
                                               CommandHandlerInterface::CommandIdCallback callback, void * context)
{
    callback(ModeSelect::Commands::ChangeToMode::Id, context);
    if (HasFeature(ModeSelect::ModeSelectFeature::kExt))
    {
        callback(ModeSelect::Commands::ChangeToModeWithStatus::Id, context);
        callback(ModeSelect::Commands::ChangeToModeResponse::Id, context);
    }

    return CHIP_NO_ERROR;
}

// Implements the read functionality for non-standard attributes.
CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case Attributes::SupportedModes::Id:
        if (delegate->NumberOfModes() == 0)
        {
            aEncoder.EncodeEmptyList();
            return CHIP_NO_ERROR;
        }

        Delegate *d = delegate;
        CHIP_ERROR err = aEncoder.EncodeList([d](const auto & encoder) -> CHIP_ERROR {
            for (uint8_t i = 0; i < d->NumberOfModes(); i++)
            {
                ModeOptionStructType mode;
                ChipError err1 = CHIP_NO_ERROR;

                // Get the mode label
                char buffer[64];
                MutableCharSpan label(buffer);
                err1 = d->getModeLabelByIndex(i, label);
                if (err1 != CHIP_NO_ERROR) {
                    return err1;
                }
                mode.label = label;

                // Get the mode value
                err1 = d->getModeValueByIndex(i, mode.mode);
                if (err1 != CHIP_NO_ERROR) {
                    return err1;
                }

                // Get the mode tags
                SemanticTagStructType tagsBuffer[8];
                List<SemanticTagStructType> tags(tagsBuffer);
                err1 = d->getModeTagsByIndex(i, tags);
                if (err1 != CHIP_NO_ERROR) {
                    return err1;
                }
                mode.semanticTags = tags;

                ReturnErrorOnFailure(encoder.Encode(mode));
            }
            return CHIP_NO_ERROR;
        });
        ReturnErrorOnFailure(err);
    }
    return CHIP_NO_ERROR;
}

// Implements checking before attribute writes.
CHIP_ERROR Instance::Write(const ConcreteDataAttributePath & attributePath, AttributeValueDecoder & aDecoder)
{
    EmberAfStatus result;

    if (aDecoder.WillDecodeNull()) // This indicates that the new mode is null.
    {
        switch (attributePath.mAttributeId)
        {
        case ModeSelect::Attributes::StartUpMode::Id:
            result = SetStartUpModeNull();
            break;
        case ModeSelect::Attributes::OnMode::Id:
            result = SetOnModeNull();
            break;
        }
    }
    else
    {
        uint8_t newMode;
        ReturnErrorOnFailure(aDecoder.Decode(newMode));

        if (!delegate->IsSupportedMode(newMode))
        {
            return StatusIB(Protocols::InteractionModel::Status::InvalidCommand).ToChipError();
        }

        switch (attributePath.mAttributeId)
        {
        case ModeSelect::Attributes::StartUpMode::Id:
            result = SetStartUpMode(newMode);
            break;
        case ModeSelect::Attributes::OnMode::Id:
            result = SetOnMode(newMode);
            break;
        }
    }

    if (result == EMBER_ZCL_STATUS_SUCCESS)
    {
        return CHIP_NO_ERROR;
    }

    return StatusIB(Protocols::InteractionModel::Status::InvalidCommand).ToChipError();
}

} // namespace ModeSelect
} // namespace Clusters
} // namespace app
} // namespace chip

