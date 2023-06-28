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
#include <app/clusters/mode-base-server/mode-base-server.h>
#include <app/clusters/on-off-server/on-off-server.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <platform/DiagnosticDataProvider.h>

// using namespace std;
using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using chip::Protocols::InteractionModel::Status;
using BootReasonType = GeneralDiagnostics::BootReasonEnum;
using ModeOptionStructType = chip::app::Clusters::detail::Structs::ModeOptionStruct::Type;
using ModeTagStructType    = chip::app::Clusters::detail::Structs::ModeTagStruct::Type;

namespace chip {
namespace app {
namespace Clusters {
namespace ModeBase {

bool Instance::HasFeature(Feature feature) const
{
    return (mFeature & to_underlying(feature)) != 0;
}

bool Instance::isAliasCluster() const
{
    for (unsigned int AliasedCluster : AliasedClusters)
    {
        if (mClusterId == AliasedCluster)
        {
            return true;
        }
    }
    return false;
}

CHIP_ERROR Instance::Init()
{
    // Check that the cluster ID given is a valid mode base alias cluster ID.
    if (!isAliasCluster())
    {
        ChipLogError(Zcl, "ModeBase: The cluster with ID %lu is not a mode base alias.", long(mClusterId));
        return CHIP_ERROR_INVALID_ARGUMENT; // todo is this the correct error?
    }

    // Check if the cluster has been selected in zap
    if (!emberAfContainsServer(mEndpointId, mClusterId))
    {
        ChipLogError(Zcl, "ModeBase: The cluster with ID %lu was not enabled in zap.", long(mClusterId));
        return CHIP_ERROR_INVALID_ARGUMENT; // todo is this the correct error?
    }

    ReturnErrorOnFailure(chip::app::InteractionModelEngine::GetInstance()->RegisterCommandHandler(this));
    VerifyOrReturnError(registerAttributeAccessOverride(this), CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(AppInit());

    ModeBaseAliasesInstanceMap[mClusterId] = this;

    // StartUp behavior relies on CurrentMode StartUpMode attributes being non-volatile.
    if (!emberAfIsKnownVolatileAttribute(mEndpointId, mClusterId, Attributes::CurrentMode::Id) &&
        !emberAfIsKnownVolatileAttribute(mEndpointId, mClusterId, Attributes::StartUpMode::Id))
    {
        // Read the StartUpMode attribute and set the CurrentMode attribute
        // The StartUpMode attribute SHALL define the desired startup behavior of a
        // device when it is supplied with power and this state SHALL be
        // reflected in the CurrentMode attribute.  The values of the StartUpMode
        // attribute are listed below.

        DataModel::Nullable<uint8_t> startUpMode = GetStartUpMode();
        if (!startUpMode.IsNull())
        {
#ifdef EMBER_AF_PLUGIN_ON_OFF
            // OnMode with Power Up
            // If the On/Off feature is supported and the On/Off cluster attribute StartUpOnOff is present, with a
            // value of On (turn on at power up), then the CurrentMode attribute SHALL be set to the OnMode attribute
            // value when the server is supplied with power, except if the OnMode attribute is null.
            if (emberAfContainsServer(mEndpointId, OnOff::Id) &&
                emberAfContainsAttribute(mEndpointId, OnOff::Id, OnOff::Attributes::StartUpOnOff::Id) &&
                emberAfContainsAttribute(mEndpointId, mClusterId, ModeBase::Attributes::OnMode::Id) &&
                HasFeature(ModeBase::Feature::kOnOff))
            {
                DataModel::Nullable<uint8_t> onMode = GetOnMode();
                bool onOffValueForStartUp           = false;
                if (!emberAfIsKnownVolatileAttribute(mEndpointId, OnOff::Id, OnOff::Attributes::StartUpOnOff::Id) &&
                    OnOffServer::Instance().getOnOffValueForStartUp(mEndpointId, onOffValueForStartUp) == EMBER_ZCL_STATUS_SUCCESS)
                {
                    if (onOffValueForStartUp && !onMode.IsNull())
                    {
                        ChipLogDetail(Zcl, "ModeBase: CurrentMode is overwritten by OnMode");
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
                ChipLogDetail(Zcl, "ModeBase: CurrentMode is ignored for OTA reboot");
                return CHIP_NO_ERROR;
            }

            // Initialise currentMode to 0
            uint8_t currentMode = GetCurrentMode();

            if (startUpMode.Value() != currentMode)
            {
                if (!IsSupportedMode(startUpMode.Value()))
                {
                    ChipLogError(Zcl, "ModeBase: Start-up_mode is not set to a valid mode.");
                    return CHIP_ERROR_INVALID_ARGUMENT; // todo is thins the correct error?
                }

                UpdateCurrentMode(startUpMode.Value());
                ChipLogProgress(Zcl, "ModeBase: Successfully initialized CurrentMode to %u", startUpMode.Value());
            }
        }
    }
    else
    {
        ChipLogProgress(Zcl, "ModeBase: Skipped initializing CurrentMode by StartUpMode because one of them is volatile");
    }

    return CHIP_NO_ERROR;
}

template <typename RequestT, typename FuncT>
void Instance::HandleCommand(HandlerContext & handlerContext, FuncT func)
{
    if (!handlerContext.mCommandHandled && (handlerContext.mRequestPath.mCommandId == RequestT::GetCommandId()))
    {
        RequestT requestPayload;

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

void Instance::handleChangeToMode(HandlerContext & ctx, const Commands::ChangeToMode::DecodableType & commandData)
{
    uint8_t newMode = commandData.newMode;

    Commands::ChangeToModeResponse::Type response;

    if (!IsSupportedMode(newMode))
    {
        ChipLogError(Zcl, "ModeBase: Failed to find the option with mode %u", newMode);
        response.status = static_cast<uint8_t>(StatusCode::kUnsupportedMode);
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        return;
    }

    HandleChangeToMode(newMode, response);

    if (response.status == static_cast<uint8_t>(StatusCode::kSuccess))
    {
        UpdateCurrentMode(newMode);
        ChipLogProgress(Zcl, "ModeBase: handleChangeToMode changed to mode %u", newMode);
    }

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

// This function is called by the interaction model engine when a command destined for this instance is received.
void Instance::InvokeCommand(HandlerContext & handlerContext)
{
    switch (handlerContext.mRequestPath.mCommandId)
    {
    case ModeBase::Commands::ChangeToMode::Id:
        ChipLogDetail(Zcl, "ModeBase: Entering handling ChangeToModeWithStatus");

        HandleCommand<Commands::ChangeToMode::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { handleChangeToMode(ctx, commandData); });
    }
}

// List the commands supported by this instance.
CHIP_ERROR Instance::EnumerateAcceptedCommands(const ConcreteClusterPath & cluster,
                                               CommandHandlerInterface::CommandIdCallback callback, void * context)
{
    callback(ModeBase::Commands::ChangeToMode::Id, context);
    callback(ModeBase::Commands::ChangeToModeResponse::Id, context);
    return CHIP_NO_ERROR;
}

// Implements the read functionality for complex attributes.
CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case Attributes::CurrentMode::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mCurrentMode));
        break;
    case Attributes::StartUpMode::Id:
        if (mStartUpMode.IsNull())
        {
            ReturnErrorOnFailure(aEncoder.EncodeNull());
        }
        else
        {
            ReturnErrorOnFailure(aEncoder.Encode(mStartUpMode));
        }
        break;
    case Attributes::OnMode::Id:
        if (mOnMode.IsNull())
        {
            ReturnErrorOnFailure(aEncoder.EncodeNull());
        }
        else
        {
            ReturnErrorOnFailure(aEncoder.Encode(mOnMode));
        }
        break;
    case Attributes::SupportedModes::Id:
        if (NumberOfModes() == 0)
        {
            aEncoder.EncodeEmptyList();
            return CHIP_NO_ERROR;
        }

        Instance * d   = this;
        CHIP_ERROR err = aEncoder.EncodeList([d](const auto & encoder) -> CHIP_ERROR {
            for (uint8_t i = 0; i < d->NumberOfModes(); i++)
            {
                ModeOptionStructType mode;
                ChipError err1 = CHIP_NO_ERROR;

                // Get the mode label
                char buffer[64];
                MutableCharSpan label(buffer);
                err1 = d->GetModeLabelByIndex(i, label);
                if (err1 != CHIP_NO_ERROR)
                {
                    return err1;
                }
                mode.label = label;

                // Get the mode value
                err1 = d->GetModeValueByIndex(i, mode.mode);
                if (err1 != CHIP_NO_ERROR)
                {
                    return err1;
                }

                // Get the mode tags
                ModeTagStructType tagsBuffer[8];
                List<ModeTagStructType> tags(tagsBuffer);
                err1 = d->GetModeTagsByIndex(i, tags);
                if (err1 != CHIP_NO_ERROR)
                {
                    return err1;
                }
                mode.modeTags = tags;

                ReturnErrorOnFailure(encoder.Encode(mode));
            }
            return CHIP_NO_ERROR;
        });
        ReturnErrorOnFailure(err);
        break;
    }
    return CHIP_NO_ERROR;
}

// Implements checking before attribute writes.
CHIP_ERROR Instance::Write(const ConcreteDataAttributePath & attributePath, AttributeValueDecoder & aDecoder)
{
    DataModel::Nullable<uint8_t> newMode;
    ReturnErrorOnFailure(aDecoder.Decode(newMode));

    if (newMode.IsNull()) // This indicates that the new mode is null.
    {
        switch (attributePath.mAttributeId)
        {
        case ModeBase::Attributes::StartUpMode::Id:
            UpdateStartUpMode(newMode);
            return CHIP_NO_ERROR;
        case ModeBase::Attributes::OnMode::Id:
            UpdateOnMode(newMode);
            return CHIP_NO_ERROR;
        }
    }
    else
    {
        if (!IsSupportedMode(newMode.Value()))
        {
            return StatusIB(Protocols::InteractionModel::Status::InvalidCommand).ToChipError();
        }

        switch (attributePath.mAttributeId)
        {
        case ModeBase::Attributes::StartUpMode::Id:
            UpdateStartUpMode(newMode);
            return CHIP_NO_ERROR;
        case ModeBase::Attributes::OnMode::Id:
            UpdateOnMode(newMode);
            return CHIP_NO_ERROR;
        }
    }

    return StatusIB(Protocols::InteractionModel::Status::InvalidCommand).ToChipError();
}

void Instance::UpdateCurrentMode(uint8_t aNewMode)
{
    uint8_t oldMode = mCurrentMode;
    mCurrentMode    = aNewMode;
    if (mCurrentMode != oldMode)
    {
        // The Administrator Commissioning cluster is always on the root endpoint.
        MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::CurrentMode::Id);
    }
}

void Instance::UpdateStartUpMode(DataModel::Nullable<uint8_t> aNewStartUpMode)
{
    DataModel::Nullable<uint8_t> oldStartUpMode = mStartUpMode;
    mStartUpMode                                = aNewStartUpMode;
    if (mStartUpMode != oldStartUpMode)
    {
        // The Administrator Commissioning cluster is always on the root endpoint.
        MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::StartUpMode::Id);
    }
}

void Instance::UpdateOnMode(DataModel::Nullable<uint8_t> aNewOnMode)
{
    DataModel::Nullable<uint8_t> oldOnMode = mOnMode;
    mOnMode                                = aNewOnMode;
    if (mOnMode != oldOnMode)
    {
        // The Administrator Commissioning cluster is always on the root endpoint.
        MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::OnMode::Id);
    }
}

DataModel::Nullable<uint8_t> Instance::GetStartUpMode() const
{
    return mStartUpMode;
}

DataModel::Nullable<uint8_t> Instance::GetOnMode() const
{
    return mOnMode;
}

uint8_t Instance::GetCurrentMode() const
{
    return mCurrentMode;
}

bool Instance::IsSupportedMode(uint8_t modeValue)
{
    for (uint8_t i = 0; i < NumberOfModes(); i++)
    {
        uint8_t value;
        auto err = GetModeValueByIndex(i, value);
        if (err == CHIP_NO_ERROR)
        {
            if (value == modeValue)
            {
                return true;
            }
        }
        else
        {
            break;
        }
    }
    ChipLogDetail(Zcl, "Cannot find a mode with value %u", modeValue);
    return false;
}

Instance::~Instance()
{
    ModeBaseAliasesInstanceMap.erase(mClusterId);
}

uint8_t Instance::NumberOfModes()
{
    return 0;
}

CHIP_ERROR Instance::GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR Instance::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR Instance::GetModeTagsByIndex(uint8_t modeIndex, List<ModeTagStructType> & modeTags)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

void Instance::HandleChangeToMode(uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    response.status = to_underlying(StatusCode::kSuccess);
}

std::map<uint32_t, Instance *> GetModeBaseInstances()
{
    return ModeBaseAliasesInstanceMap;
}

} // namespace ModeBase
} // namespace Clusters
} // namespace app
} // namespace chip
