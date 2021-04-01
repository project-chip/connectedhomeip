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

/****************************************************************************
 * @file
 * @brief Implementation for the Descriptor Server Cluster
 ***************************************************************************/

#include "af.h"
#include <app/util/attribute-storage.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

#include "gen/af-structs.h"
#include "gen/attribute-id.h"
#include "gen/attribute-type.h"
#include "gen/cluster-id.h"

using namespace chip;

constexpr const char * kErrorStr = "Descriptor cluster (0x%02x) Error setting '%s' attribute: 0x%02x";

EmberAfStatus readWriteAttribute(uint8_t endpoint, AttributeId attributeId, bool write, uint8_t * buffer, int32_t index)
{
    EmberAfAttributeSearchRecord record;
    record.endpoint         = endpoint;
    record.clusterId        = ZCL_DESCRIPTOR_CLUSTER_ID;
    record.clusterMask      = CLUSTER_MASK_SERVER;
    record.manufacturerCode = EMBER_AF_NULL_MANUFACTURER_CODE;
    record.attributeId      = attributeId;

    return emAfReadOrWriteAttribute(&record, NULL, (uint8_t *) buffer, 0, write, index + 1);
}

EmberAfStatus writeAttribute(uint8_t endpoint, AttributeId attributeId, uint8_t * buffer, int32_t index = -1)
{
    return readWriteAttribute(endpoint, attributeId, true, buffer, index);
}

EmberAfStatus readAttribute(uint8_t endpoint, AttributeId attributeId, uint8_t * buffer, int32_t index = -1)
{
    return readWriteAttribute(endpoint, attributeId, false, buffer, index);
}

EmberAfStatus writeClientServerAttribute(uint8_t endpoint, bool server)
{
    EmberAfStatus status    = EMBER_ZCL_STATUS_SUCCESS;
    AttributeId attributeId = server ? ZCL_SERVER_LIST_ATTRIBUTE_ID : ZCL_CLIENT_LIST_ATTRIBUTE_ID;

    uint16_t clusterCount = emberAfClusterCount(endpoint, server);

    EmberAfCluster * cluster;
    for (uint8_t clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++)
    {
        cluster = emberAfGetNthCluster(endpoint, clusterIndex, server);
        status  = writeAttribute(endpoint, attributeId, (uint8_t *) &cluster->clusterId, clusterIndex);
        VerifyOrReturnError(status == EMBER_ZCL_STATUS_SUCCESS, status);
    }

    return writeAttribute(endpoint, attributeId, (uint8_t *) &clusterCount);
}

EmberAfStatus writeServerAttribute(uint8_t endpoint)
{
    return writeClientServerAttribute(endpoint, true);
}

EmberAfStatus writeClientAttribute(uint8_t endpoint)
{
    return writeClientServerAttribute(endpoint, false);
}

EmberAfStatus writeDeviceAttribute(uint8_t endpoint)
{
    EmberAfStatus status    = EMBER_ZCL_STATUS_SUCCESS;
    AttributeId attributeId = ZCL_DEVICE_LIST_ATTRIBUTE_ID;

    uint16_t deviceTypeCount = 0;
    uint8_t startIndex       = endpoint;
    uint8_t stopIndex        = (endpoint == 0 ? emberAfEndpointCount() : static_cast<uint8_t>(endpoint + 1));

    EmberAfDeviceType deviceType;
    for (uint8_t endpointIndex = startIndex; endpointIndex < stopIndex; endpointIndex++)
    {
        DeviceTypeId deviceTypeId = emberAfDeviceIdFromIndex(endpointIndex);
        uint16_t revision         = emberAfDeviceVersionFromIndex(endpointIndex);

        bool duplicate = false;
        for (uint8_t entryIndex = 0; entryIndex < (endpointIndex - endpoint); entryIndex++)
        {
            status = readAttribute(endpoint, attributeId, (uint8_t *) &deviceType, endpointIndex - endpoint);
            VerifyOrReturnError(status == EMBER_ZCL_STATUS_SUCCESS, status);

            if (deviceType.type == deviceTypeId)
            {
                duplicate = true;
                break;
            }
        }

        if (duplicate == false)
        {
            deviceType.type     = deviceTypeId;
            deviceType.revision = revision;

            status = writeAttribute(endpoint, attributeId, (uint8_t *) &deviceType, endpointIndex - endpoint);
            VerifyOrReturnError(status == EMBER_ZCL_STATUS_SUCCESS, status);
            deviceTypeCount++;
        }
    }

    return writeAttribute(endpoint, attributeId, (uint8_t *) &deviceTypeCount);
}

EmberAfStatus writePartsAttribute(uint8_t endpoint)
{
    EmberAfStatus status    = EMBER_ZCL_STATUS_SUCCESS;
    AttributeId attributeId = ZCL_PARTS_LIST_ATTRIBUTE_ID;

    uint16_t partsCount = 0;

    for (uint8_t endpointIndex = 0; endpointIndex < emberAfEndpointCount(); endpointIndex++)
    {
        EndpointId endpointId = emberAfEndpointFromIndex(endpointIndex);

        // The Descriptor Cluster instance on Endpoint 0 (zero) SHALL have a Parts attribute that lists all endpoint instances on a
        // node instance.
        // All local endpoints required by the device type(s) in the Device attribute SHALL be included in the Parts list.
        if (endpoint == 0x00 || (emberAfDeviceIdFromIndex(endpoint) == emberAfDeviceIdFromIndex(endpointIndex)))
        {
            status = writeAttribute(endpoint, attributeId, (uint8_t *) &endpointId, partsCount);
            VerifyOrReturnError(status == EMBER_ZCL_STATUS_SUCCESS, status);
            partsCount++;
        }
    }

    return writeAttribute(endpoint, attributeId, (uint8_t *) &partsCount);
}

void emberAfPluginDescriptorServerInitCallback(void)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    for (uint8_t index = 0; index < emberAfEndpointCount(); index++)
    {
        EndpointId endpoint = emberAfEndpointFromIndex(index);
        if (!emberAfContainsCluster(endpoint, ZCL_DESCRIPTOR_CLUSTER_ID))
        {
            continue;
        }

        status = writeDeviceAttribute(endpoint);
        VerifyOrReturn(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, kErrorStr, endpoint, "device", status));

        status = writeServerAttribute(endpoint);
        VerifyOrReturn(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, kErrorStr, endpoint, "server", status));

        status = writeClientAttribute(endpoint);
        VerifyOrReturn(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, kErrorStr, endpoint, "client", status));

        status = writePartsAttribute(endpoint);
        VerifyOrReturn(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, kErrorStr, endpoint, "parts", status));
    }
}
