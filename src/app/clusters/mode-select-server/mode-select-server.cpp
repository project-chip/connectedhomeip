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

/****************************************************************************
 * @file
 * @brief Routines for the Mode Select plugin, the
 *server implementation of the Mode Select Cluster.
 *******************************************************************************
 ******************************************************************************/
#include <cstring>

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
#include <app/clusters/mode-select-server/heap-based-supported-modes-manager.h>
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
    ModeSelectAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), ModeSelectCluster::Id) {}

    CHIP_ERROR Read(const ConcreteAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
};

ModeSelectAttrAccess gModeSelectAttrAccess;

CHIP_ERROR ModeSelectAttrAccess::Read(const ConcreteAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    if (aPath.mClusterId != ModeSelectCluster::Id)
    {
        // We shouldn't have been called at all.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    const ModeSelectCluster::HeapBasedSupportedModesManager & gSupportedModeManager =
        ModeSelectCluster::HeapBasedSupportedModesManager::getHeapBasedSupportedModesManagerInstance();

    if (ModeSelectCluster::Attributes::SupportedModes::Id == aPath.mAttributeId)
    {
        const ModeSelectCluster::HeapBasedSupportedModesManager::IteratorFactory & iteratorFactory =
            *gSupportedModeManager.getIteratorFactory(aPath.mEndpointId);
        CHIP_ERROR err;
        err = aEncoder.EncodeList([iteratorFactory](const TagBoundEncoder & encoder) -> CHIP_ERROR {
            const auto & end = *(iteratorFactory.end());
            for (auto it = *(iteratorFactory.begin()); it != end; ++it)
            {
                emberAfPrintln(EMBER_AF_PRINT_DEBUG, "ModeSelectCluster: dereferencing it");
                emberAfPrintln(EMBER_AF_PRINT_DEBUG, "ModeSelectCluster: it= %p", (void *) it.operator->());
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

bool emberAfModeSelectClusterClusterChangeToModeCallback(
    CommandHandler * commandHandler, const ConcreteCommandPath & commandPath,
    const ModeSelectCluster::Commands::ChangeToMode::DecodableType & commandData)
{
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "ModeSelectCluster: Entering emberAfModeSelectClusterClusterChangeToModeCallback");
    EndpointId endpointId = commandPath.mEndpointId;
    uint8_t newMode       = commandData.newMode;
    // Check that the newMode matches one of the supported options
    const ModeSelectCluster::Structs::ModeOptionStruct::Type * modeOptionPtr;
    const ModeSelectCluster::HeapBasedSupportedModesManager & gSupportedModeManager =
        ModeSelectCluster::HeapBasedSupportedModesManager::getHeapBasedSupportedModesManagerInstance();
    EmberAfStatus checkSupportedModeStatus = gSupportedModeManager.getModeOptionByMode(endpointId, newMode, modeOptionPtr);
    if (EMBER_ZCL_STATUS_SUCCESS != checkSupportedModeStatus)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "ModeSelectCluster: Failed to find the option with mode %" PRIu8, newMode);
        emberAfSendImmediateDefaultResponse(checkSupportedModeStatus);
        return false;
    }
    ModeSelectCluster::Attributes::CurrentMode::Set(endpointId, newMode);
    // TODO: Implement application logic

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
