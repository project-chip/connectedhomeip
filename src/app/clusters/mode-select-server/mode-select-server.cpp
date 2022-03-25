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

#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/att-storage.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/clusters/mode-select-server/supported-modes-manager.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <app/util/error-mapping.h>
#include <app/util/util.h>
#include <lib/support/CodeUtils.h>

using namespace std;
using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ModeSelect;
using namespace chip::Protocols;

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
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "ModeSelect: Entering emberAfModeSelectClusterChangeToModeCallback");
    EndpointId endpointId = commandPath.mEndpointId;
    uint8_t newMode       = commandData.newMode;
    // Check that the newMode matches one of the supported options
    const ModeSelect::Structs::ModeOptionStruct::Type * modeOptionPtr;
    EmberAfStatus checkSupportedModeStatus =
        ModeSelect::getSupportedModesManager()->getModeOptionByMode(endpointId, newMode, &modeOptionPtr);
    if (EMBER_ZCL_STATUS_SUCCESS != checkSupportedModeStatus)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "ModeSelect: Failed to find the option with mode %u", newMode);
        emberAfSendImmediateDefaultResponse(checkSupportedModeStatus);
        return false;
    }
    ModeSelect::Attributes::CurrentMode::Set(endpointId, newMode);
    // TODO: Implement application logic

    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "ModeSelect: ChangeToMode successful");
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
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
        EmberAfStatus status = Attributes::StartUpMode::Get(endpointId, startUpMode);
        if (status == EMBER_ZCL_STATUS_SUCCESS && !startUpMode.IsNull())
        {
            // Initialise currentMode to 0
            uint8_t currentMode = 0;
            status              = Attributes::CurrentMode::Get(endpointId, &currentMode);
            if (status == EMBER_ZCL_STATUS_SUCCESS && startUpMode.Value() != currentMode)
            {
                status = Attributes::CurrentMode::Set(endpointId, startUpMode.Value());
                if (status != EMBER_ZCL_STATUS_SUCCESS)
                {
                    ChipLogError(Zcl, "ModeSelect: Error initializing CurrentMode, EmberAfStatus code 0x%02x", status);
                }
                else
                {
                    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "ModeSelect: Successfully initialized CurrentMode to %u",
                                   startUpMode.Value());
                }
            }
        }
    }
    else
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG,
                       "ModeSelect: Skipped initializing CurrentMode by StartUpMode because one of them is volatile");
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
    return emberAfIsNonVolatileAttribute(endpointId, ModeSelect::Id, Attributes::CurrentMode::Id, true) &&
        emberAfIsNonVolatileAttribute(endpointId, ModeSelect::Id, Attributes::StartUpMode::Id, true);
}

} // namespace

void MatterModeSelectPluginServerInitCallback(void)
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
    const ModeSelect::Structs::ModeOptionStruct::Type * modeOptionPtr;
    EmberAfStatus checkSupportedModeStatus =
        ModeSelect::getSupportedModesManager()->getModeOptionByMode(endpointId, newMode, &modeOptionPtr);
    if (EMBER_ZCL_STATUS_SUCCESS != checkSupportedModeStatus)
    {
        const InteractionModel::Status returnStatus = ToInteractionModelStatus(checkSupportedModeStatus);
        return returnStatus;
    }
    return InteractionModel::Status::Success;
}
