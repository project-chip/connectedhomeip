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
#include <lib/support/CodeUtils.h>

using namespace std;
using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

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

void MatterModeSelectPluginServerInitCallback(void)
{
    registerAttributeAccessOverride(&gModeSelectAttrAccess);
}
