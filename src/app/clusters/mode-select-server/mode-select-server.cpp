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

/**
 *
 *    Copyright (c) 2021 Silicon Labs
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
/****************************************************************************
 * @file
 * @brief Routines for the Mode Select plugin, the
 *server implementation of the Mode Select Cluster.
 *******************************************************************************
 ******************************************************************************/
#include <string>

#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/att-storage.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/command-id.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/clusters/mode-select-server/supported-modes-manager.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <platform/PlatformManager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

class ModeSelectAttrAccess : public chip::app::AttributeAccessInterface
{
public:
    ModeSelectAttrAccess() : chip::app::AttributeAccessInterface(Optional<EndpointId>::Missing(), ModeSelectCluster::Id) {}

    CHIP_ERROR Read(const ConcreteAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
};

ModeSelectCluster::Structs::ModeOptionStruct::Type buildModeOptionStruct(const char * label, size_t labelLength, uint8_t mode,
                                                                         uint32_t semanticTag)
{
    ModeSelectCluster::Structs::ModeOptionStruct::Type option;
    option.label       = ByteSpan((const unsigned char *) label, labelLength);
    option.mode        = mode;
    option.semanticTag = semanticTag;
    return option;
}

ModeSelectAttrAccess gModeSelectAttrAccess;

// TODO: Add as many ModeOptions as necessary.
ModeSelectCluster::Structs::ModeOptionStruct::Type option1 = buildModeOptionStruct("Black", 5, 0, 0);
ModeSelectCluster::Structs::ModeOptionStruct::Type option2 = buildModeOptionStruct("White", 5, 4, 0);
ModeSelectCluster::Structs::ModeOptionStruct::Type option3 = buildModeOptionStruct("Half-and-half", 13, 7, 0);

const ModeSelectCluster::SupportedModesManager & gSupportedModeManager = ModeSelectCluster::SupportedModesManager::Builder()
                                                                             // TODO: Add as many ModeOptions as necessary.
                                                                             .addSupportedMode((unsigned short) 0u, option1)
                                                                             .addSupportedMode((unsigned short) 0u, option2)
                                                                             .addSupportedMode((unsigned short) 0u, option3)
                                                                             .build();

CHIP_ERROR ModeSelectAttrAccess::Read(const ConcreteAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    if (aPath.mClusterId != ModeSelectCluster::Id)
    {
        // We shouldn't have been called at all.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    switch (aPath.mAttributeId)
    {
    case ModeSelectCluster::Attributes::SupportedModes::Id:
        const vector<ModeSelectCluster::Structs::ModeOptionStruct::Type> & supportedOptions =
            gSupportedModeManager.getSupportedModesForEndpoint(aPath.mEndpointId);
        CHIP_ERROR err;
        err = aEncoder.EncodeList([supportedOptions](const TagBoundEncoder & encoder) -> CHIP_ERROR {
            for (vector<ModeSelectCluster::Structs::ModeOptionStruct::Type>::const_iterator it = supportedOptions.begin();
                 it != supportedOptions.end(); ++it)
            {
                const ModeSelectCluster::Structs::ModeOptionStruct::Type & modeOption = *it;
                ReturnErrorOnFailure(encoder.Encode(modeOption));
            }
            return CHIP_NO_ERROR;
        });
        ReturnErrorOnFailure(err);
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "ModeSelectCluster: Successfully read SupportedModes");
    }
    return CHIP_NO_ERROR;
}

} // anonymous namespace

bool emberAfModeSelectClusterClusterChangeToModeCallback(
    chip::app::CommandHandler * commandHandler, const chip::app::ConcreteCommandPath & commandPath,
    const ModeSelectCluster::Commands::ChangeToMode::DecodableType & commandData)
{
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "ModeSelectCluster: Entering emberAfModeSelectClusterClusterChangeToModeCallback");
    EndpointId endpointId = commandPath.mEndpointId;
    uint8_t newMode       = commandData.newMode;
    // Check that the newMode matches one of the supported options
    const ModeSelectCluster::Structs::ModeOptionStruct::Type * modeOptionPtr;
    EmberAfStatus checkSupportedModeStatus = gSupportedModeManager.getModeOptionByMode(endpointId, newMode, modeOptionPtr);
    if (EMBER_ZCL_STATUS_SUCCESS != checkSupportedModeStatus)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "ModeSelectCluster: Failed to find the option with mode %hhu", newMode);
        emberAfSendImmediateDefaultResponse(checkSupportedModeStatus);
        return false;
    }
    // TODO: Implement application logic

    emberAfWriteAttribute(endpointId, ModeSelectCluster::Id, ModeSelectCluster::Attributes::CurrentMode::Id, CLUSTER_MASK_SERVER,
                          &newMode, ZCL_INT8U_ATTRIBUTE_TYPE);

    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "ModeSelectCluster: ChangeToMode successful");
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

void emberAfModeSelectClusterClusterInitCallback(EndpointId endpoint)
{
    static bool attrAccessRegistered = false;
    if (!attrAccessRegistered)
    {
        registerAttributeAccessOverride(&gModeSelectAttrAccess);
        attrAccessRegistered = true;
    }
}
