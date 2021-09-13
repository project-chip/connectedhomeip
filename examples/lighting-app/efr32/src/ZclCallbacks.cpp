/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 * @file
 *   This file implements the handler for data model messages.
 */

#include "AppConfig.h"
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

#include "LightingManager.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/util/af-types.h>

using namespace ::chip;
using namespace ::chip::app::Clusters;
using namespace ::chip::DeviceLayer;

void emberAfPostAttributeChangeCallback(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId, uint8_t mask,
                                        uint16_t manufacturerCode, uint8_t type, uint16_t size, uint8_t * value)
{
    if (clusterId == OnOff::Id)
    {
        if (attributeId != OnOff::Attributes::Ids::OnOff)
        {
            ChipLogError(Zcl, "ON OFF attribute ID: " ChipLogFormatMEI " Type: %" PRIu8 " Value: %" PRIu16 ", length %" PRIu16,
                         ChipLogValueMEI(attributeId), type, *value, size);
            return;
        }

        LightMgr().InitiateAction(AppEvent::kEventType_Light, *value ? LightingManager::ON_ACTION : LightingManager::OFF_ACTION);
    }
    else if (clusterId == LevelControl::Id)
    {
        ChipLogProgress(Zcl,
                        "Level Control attribute ID: " ChipLogFormatMEI " Type: %" PRIu8 " Value: %" PRIu16 ", length %" PRIu16,
                        ChipLogValueMEI(attributeId), type, *value, size);

        // WIP Apply attribute change to Light
    }
    else if (clusterId == ColorControl::Id)
    {
        ChipLogProgress(Zcl,
                        "Color Control attribute ID: " ChipLogFormatMEI " Type: %" PRIu8 " Value: %" PRIu16 ", length %" PRIu16,
                        ChipLogValueMEI(attributeId), type, *value, size);

        // WIP Apply attribute change to Light
    }
    else if (clusterId == OnOffSwitchConfiguration::Id)
    {
        ChipLogProgress(Zcl,
                        "OnOff Switch Configuration attribute ID: " ChipLogFormatMEI " Type: %" PRIu8 " Value: %" PRIu16
                        ", length %" PRIu16,
                        ChipLogValueMEI(attributeId), type, *value, size);

        // WIP Apply attribute change to Light
    }
}

EmberAfStatus HandleReadEthernetNetworkDiagnosticsAttribute(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength)
{
    EmberAfStatus ret = EMBER_ZCL_STATUS_FAILURE;

    switch (attributeId)
    {
    case ZCL_PACKET_RX_COUNT_ATTRIBUTE_ID:
        if (maxReadLength == sizeof(uint64_t))
        {
            uint64_t packetRxCount;

            if (ConnectivityMgr().GetEthPacketRxCount(packetRxCount) == CHIP_NO_ERROR)
            {
                memcpy(buffer, &packetRxCount, maxReadLength);
                ret = EMBER_ZCL_STATUS_SUCCESS;
            }
        }
        break;
    case ZCL_PACKET_TX_COUNT_ATTRIBUTE_ID:
        if (maxReadLength == sizeof(uint64_t))
        {
            uint64_t packetTxCount;

            if (ConnectivityMgr().GetEthPacketTxCount(packetTxCount) == CHIP_NO_ERROR)
            {
                memcpy(buffer, &packetTxCount, maxReadLength);
                ret = EMBER_ZCL_STATUS_SUCCESS;
            }
        }
        break;
    case ZCL_TX_ERR_COUNT_ATTRIBUTE_ID:
        if (maxReadLength == sizeof(uint64_t))
        {
            uint64_t txErrCount;

            if (ConnectivityMgr().GetEthTxErrCount(txErrCount) == CHIP_NO_ERROR)
            {
                memcpy(buffer, &txErrCount, maxReadLength);
                ret = EMBER_ZCL_STATUS_SUCCESS;
            }
        }
        break;
    case ZCL_COLLISION_COUNT_ATTRIBUTE_ID:
        if (maxReadLength == sizeof(uint64_t))
        {
            uint64_t collisionCount;

            if (ConnectivityMgr().GetEthCollisionCount(collisionCount) == CHIP_NO_ERROR)
            {
                memcpy(buffer, &collisionCount, maxReadLength);
                ret = EMBER_ZCL_STATUS_SUCCESS;
            }
        }
        break;
    case ZCL_ETHERNET_OVERRUN_COUNT_ATTRIBUTE_ID:
        if (maxReadLength == sizeof(uint64_t))
        {
            uint64_t overrunCount;

            if (ConnectivityMgr().GetEthOverrunCount(overrunCount) == CHIP_NO_ERROR)
            {
                memcpy(buffer, &overrunCount, maxReadLength);
                ret = EMBER_ZCL_STATUS_SUCCESS;
            }
        }
        break;
    default:
        ChipLogProgress(Zcl, "Unhandled attribute ID: %ld", attributeId);
        break;
    }

    return ret;
}

EmberAfStatus emberAfExternalAttributeReadCallback(EndpointId endpoint, ClusterId clusterId,
                                                   EmberAfAttributeMetadata * attributeMetadata, uint16_t manufacturerCode,
                                                   uint8_t * buffer, uint16_t maxReadLength, int32_t index)
{
    EmberAfStatus ret = EMBER_ZCL_STATUS_FAILURE;

    ChipLogProgress(Zcl,
                    "emberAfExternalAttributeReadCallback - Cluster ID: '0x%04lx', EndPoint ID: '0x%02x', Attribute ID: '0x%04lx'",
                    clusterId, endpoint, attributeMetadata->attributeId);

    switch (clusterId)
    {
    case ZCL_ETHERNET_NETWORK_DIAGNOSTICS_CLUSTER_ID:
        ret = HandleReadEthernetNetworkDiagnosticsAttribute(attributeMetadata->attributeId, buffer, maxReadLength);
        break;
    default:
        ChipLogError(Zcl, "Unhandled cluster ID: %ld", clusterId);
        break;
    }

    return ret;
}

/** @brief OnOff Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 * TODO Issue #3841
 * emberAfOnOffClusterInitCallback happens before the stack initialize the cluster
 * attributes to the default value.
 * The logic here expects something similar to the deprecated Plugins callback
 * emberAfPluginOnOffClusterServerPostInitCallback.
 *
 */
void emberAfOnOffClusterInitCallback(EndpointId endpoint)
{
    // TODO: implement any additional Cluster Server init actions
}
