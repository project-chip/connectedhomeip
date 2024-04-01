/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/clusters/mode-select-server/supported-modes-manager.h>
#include <app/util/af.h>
#include <app/util/att-storage.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <app/util/odd-sized-integers.h>
#include <app/util/util.h>
#include <lib/support/CodeUtils.h>
#include <platform/DiagnosticDataProvider.h>
#include <tracing/macros.h>

#ifdef MATTER_DM_PLUGIN_ON_OFF
#include <app/clusters/on-off-server/on-off-server.h>
#endif // MATTER_DM_PLUGIN_ON_OFF

using namespace std;
using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ModeSelect;
using namespace chip::Protocols;
using chip::Protocols::InteractionModel::Status;

using BootReasonType = GeneralDiagnostics::BootReasonEnum;

static InteractionModel::Status verifyModeValue(const EndpointId endpointId, const uint8_t newMode);

namespace {

inline bool areStartUpModeAndCurrentModeNonVolatile(EndpointId endpoint);

class ModeSelectAttrAccess : public AttributeAccessInterface
{
public:
    ModeSelectAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), ModeSelect::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
};

ModeSelectAttrAccess gModeSelectAttrAccess;

CHIP_ERROR ModeSelectAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == ModeSelect::Id);

    const ModeSelect::SupportedModesManager * gSupportedModeManager = ModeSelect::getSupportedModesManager();

    if (ModeSelect::Attributes::SupportedModes::Id == aPath.mAttributeId)
    {
        const ModeSelect::SupportedModesManager::ModeOptionsProvider modeOptionsProvider =
            gSupportedModeManager->getModeOptionsProvider(aPath.mEndpointId);
        if (modeOptionsProvider.begin() == nullptr)
        {
            aEncoder.EncodeEmptyList();
            return CHIP_NO_ERROR;
        }
        CHIP_ERROR err;
        err = aEncoder.EncodeList([modeOptionsProvider](const auto & encoder) -> CHIP_ERROR {
            const auto * end = modeOptionsProvider.end();
            for (auto * it = modeOptionsProvider.begin(); it != end; ++it)
            {
                auto & modeOption = *it;
                ReturnErrorOnFailure(encoder.Encode(modeOption));
            }
            return CHIP_NO_ERROR;
        });
        ReturnErrorOnFailure(err);
    }
    return CHIP_NO_ERROR;
}

} // anonymous namespace

bool emberAfModeSelectClusterChangeToModeCallback(CommandHandler * commandHandler, const ConcreteCommandPath & commandPath,
                                                  const ModeSelect::Commands::ChangeToMode::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("ChangeToMode", "ModeSelect");
    ChipLogProgress(Zcl, "ModeSelect: Entering emberAfModeSelectClusterChangeToModeCallback");
    EndpointId endpointId = commandPath.mEndpointId;
    uint8_t newMode       = commandData.newMode;
    // Check that the newMode matches one of the supported options
    const ModeSelect::Structs::ModeOptionStruct::Type * modeOptionPtr;
    Status checkSupportedModeStatus =
        ModeSelect::getSupportedModesManager()->getModeOptionByMode(endpointId, newMode, &modeOptionPtr);
    if (Status::Success != checkSupportedModeStatus)
    {
        ChipLogProgress(Zcl, "ModeSelect: Failed to find the option with mode %u", newMode);
        commandHandler->AddStatus(commandPath, checkSupportedModeStatus);
        return true;
    }
    ModeSelect::Attributes::CurrentMode::Set(endpointId, newMode);

    ChipLogProgress(Zcl, "ModeSelect: ChangeToMode successful");
    commandHandler->AddStatus(commandPath, Status::Success);
    return true;
}

/**
 * Callback for Mode Select Cluster Server Initialization.
 * Enabled in src/app/zap-templates/templates/app/helper.js
 * @param endpointId    id of the endpoint that is being initialized
 */
void emberAfModeSelectClusterServerInitCallback(EndpointId endpointId)
{
    // StartUp behavior relies on CurrentMode StartUpMode attributes being non-volatile.
    if (areStartUpModeAndCurrentModeNonVolatile(endpointId))
    {
        // Read the StartUpMode attribute and set the CurrentMode attribute
        // The StartUpMode attribute SHALL define the desired startup behavior of a
        // device when it is supplied with power and this state SHALL be
        // reflected in the CurrentMode attribute.  The values of the StartUpMode
        // attribute are listed below.

        DataModel::Nullable<uint8_t> startUpMode;
        Status status = Attributes::StartUpMode::Get(endpointId, startUpMode);
        if (status == Status::Success && !startUpMode.IsNull())
        {
#ifdef MATTER_DM_PLUGIN_ON_OFF
            // OnMode with Power Up
            // If the On/Off feature is supported and the On/Off cluster attribute StartUpOnOff is present, with a
            // value of On (turn on at power up), then the CurrentMode attribute SHALL be set to the OnMode attribute
            // value when the server is supplied with power, except if the OnMode attribute is null.
            if (emberAfContainsServer(endpointId, OnOff::Id) &&
                emberAfContainsAttribute(endpointId, OnOff::Id, OnOff::Attributes::StartUpOnOff::Id) &&
                emberAfContainsAttribute(endpointId, ModeSelect::Id, ModeSelect::Attributes::OnMode::Id))
            {
                Attributes::OnMode::TypeInfo::Type onMode;
                bool onOffValueForStartUp = false;
                if (Attributes::OnMode::Get(endpointId, onMode) == Status::Success &&
                    !emberAfIsKnownVolatileAttribute(endpointId, OnOff::Id, OnOff::Attributes::StartUpOnOff::Id) &&
                    OnOffServer::Instance().getOnOffValueForStartUp(endpointId, onOffValueForStartUp) == Status::Success)
                {
                    if (onOffValueForStartUp && !onMode.IsNull())
                    {
                        ChipLogProgress(Zcl, "ModeSelect: CurrentMode is overwritten by OnMode");
                        return;
                    }
                }
            }
#endif // MATTER_DM_PLUGIN_ON_OFF

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
                ChipLogProgress(Zcl, "ModeSelect: CurrentMode is ignored for OTA reboot");
                return;
            }

            // Initialise currentMode to 0
            uint8_t currentMode = 0;
            status              = Attributes::CurrentMode::Get(endpointId, &currentMode);

            if ((status == Status::Success) && (startUpMode.Value() != currentMode))
            {
                status = Attributes::CurrentMode::Set(endpointId, startUpMode.Value());
                if (status != Status::Success)
                {
                    ChipLogError(Zcl, "ModeSelect: Error initializing CurrentMode, Status code 0x%02x", to_underlying(status));
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
}

namespace {

/**
 * Checks if StartUpMode and CurrentMode are non-volatile.
 * @param endpointId    id of the endpoint to check
 * @return true if both attributes are non-volatile; false otherwise.
 */
inline bool areStartUpModeAndCurrentModeNonVolatile(EndpointId endpointId)
{
    return !emberAfIsKnownVolatileAttribute(endpointId, ModeSelect::Id, Attributes::CurrentMode::Id) &&
        !emberAfIsKnownVolatileAttribute(endpointId, ModeSelect::Id, Attributes::StartUpMode::Id);
}

} // namespace

void MatterModeSelectPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gModeSelectAttrAccess);
}

/**
 * Callback for Mode Select Cluster Server Pre Attribute Changed
 * Enabled in src/app/zap-templates/templates/app/helper.js
 * @param attributePath Concrete attribute path to be changed
 * @param attributeType Attribute type
 * @param size          Attribute size
 * @param value         Attribute value
 */
InteractionModel::Status MatterModeSelectClusterServerPreAttributeChangedCallback(const ConcreteAttributePath & attributePath,
                                                                                  EmberAfAttributeType attributeType, uint16_t size,
                                                                                  uint8_t * value)
{
    const EndpointId endpointId = attributePath.mEndpointId;
    InteractionModel::Status result;

    switch (attributePath.mAttributeId)
    {
    case ModeSelect::Attributes::StartUpMode::Id:
        result = verifyModeValue(endpointId, *value);
        break;
    case ModeSelect::Attributes::OnMode::Id:
        result = verifyModeValue(endpointId, *value);
        break;
    default:
        result = InteractionModel::Status::Success;
    }

    return result;
}

/**
 * Checks the new mode against the endpoint's supported modes.
 * @param endpointId    endpointId of the endpoint
 * @param newMode       value of the new mode
 * @return              Success status if the value is valid; InvalidValue otherwise.
 */
static InteractionModel::Status verifyModeValue(const EndpointId endpointId, const uint8_t newMode)
{
    if (NumericAttributeTraits<uint8_t>::IsNullValue(newMode)) // This indicates that the new mode is null.
    {
        return InteractionModel::Status::Success;
    }
    const ModeSelect::Structs::ModeOptionStruct::Type * modeOptionPtr;
    return ModeSelect::getSupportedModesManager()->getModeOptionByMode(endpointId, newMode, &modeOptionPtr);
}
