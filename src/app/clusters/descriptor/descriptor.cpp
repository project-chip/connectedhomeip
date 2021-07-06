/*
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
 * @brief Implementation for the Descriptor Server Cluster
 ***************************************************************************/

#include <app/common/gen/af-structs.h>
#include <app/common/gen/attribute-id.h>
#include <app/common/gen/attribute-type.h>
#include <app/common/gen/cluster-id.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

using namespace chip;

constexpr const char * kErrorStr = "Descriptor cluster (0x%02x) Error setting '%s' attribute: 0x%02x";

EmberAfStatus writeAttribute(EndpointId endpoint, AttributeId attributeId, uint8_t * buffer, int32_t index = -1)
{
    EmberAfAttributeSearchRecord record;
    record.endpoint         = endpoint;
    record.clusterId        = ZCL_DESCRIPTOR_CLUSTER_ID;
    record.clusterMask      = CLUSTER_MASK_SERVER;
    record.manufacturerCode = EMBER_AF_NULL_MANUFACTURER_CODE;
    record.attributeId      = attributeId;

    // When reading or writing a List attribute the 'index' value could have 3 types of values:
    //  -1: Read/Write the whole list content, including the number of elements in the list
    //   0: Read/Write the number of elements in the list, represented as a uint16_t
    //   n: Read/Write the nth element of the list
    //
    // Since the first 2 bytes of the attribute are used to store the number of elements, elements indexing starts
    // at 1. In order to hide this to the rest of the code of this file, the element index is incremented by 1 here.
    // This also allows calling writeAttribute() with no index arg to mean "write the length".
    return emAfReadOrWriteAttribute(&record, NULL, buffer, 0, true, index + 1);
}

EmberAfStatus writeClientServerAttribute(EndpointId endpoint, bool server)
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

EmberAfStatus writeServerAttribute(EndpointId endpoint)
{
    return writeClientServerAttribute(endpoint, true);
}

EmberAfStatus writeClientAttribute(EndpointId endpoint)
{
    return writeClientServerAttribute(endpoint, false);
}

EmberAfStatus writeDeviceAttribute(EndpointId endpoint, uint16_t index)
{
    EmberAfStatus status    = EMBER_ZCL_STATUS_SUCCESS;
    AttributeId attributeId = ZCL_DEVICE_LIST_ATTRIBUTE_ID;

    uint16_t deviceTypeCount  = 1;
    DeviceTypeId deviceTypeId = emberAfDeviceIdFromIndex(index);
    uint16_t revision         = emberAfDeviceVersionFromIndex(index);

    EmberAfDeviceType deviceType;
    deviceType.type     = deviceTypeId;
    deviceType.revision = revision;

    status = writeAttribute(endpoint, attributeId, (uint8_t *) &deviceType, 0);
    VerifyOrReturnError(status == EMBER_ZCL_STATUS_SUCCESS, status);

    return writeAttribute(endpoint, attributeId, (uint8_t *) &deviceTypeCount);
}

EmberAfStatus writePartsAttribute(EndpointId endpoint)
{
    EmberAfStatus status    = EMBER_ZCL_STATUS_SUCCESS;
    AttributeId attributeId = ZCL_PARTS_LIST_ATTRIBUTE_ID;

    uint16_t partsCount = 0;

    if (endpoint == 0x00)
    {
        for (uint16_t endpointIndex = 1; endpointIndex < emberAfEndpointCount(); endpointIndex++)
        {
            if (emberAfEndpointIndexIsEnabled(endpointIndex))
            {
                EndpointId endpointId = emberAfEndpointFromIndex(endpointIndex);
                status                = writeAttribute(endpoint, attributeId, (uint8_t *) &endpointId, partsCount);
                VerifyOrReturnError(status == EMBER_ZCL_STATUS_SUCCESS, status);
                partsCount++;
            }
        }
    }

    return writeAttribute(endpoint, attributeId, (uint8_t *) &partsCount);
}

void emberAfPluginDescriptorServerInitCallback(void)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    for (uint16_t index = 0; index < emberAfEndpointCount(); index++)
    {
        EndpointId endpoint = emberAfEndpointFromIndex(index);
        if (!emberAfContainsCluster(endpoint, ZCL_DESCRIPTOR_CLUSTER_ID))
        {
            continue;
        }

        if (!emberAfEndpointIndexIsEnabled(index))
        {
            continue;
        }

        status = writeDeviceAttribute(endpoint, index);
        VerifyOrReturn(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, kErrorStr, endpoint, "device", status));

        status = writeServerAttribute(endpoint);
        VerifyOrReturn(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, kErrorStr, endpoint, "server", status));

        status = writeClientAttribute(endpoint);
        VerifyOrReturn(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, kErrorStr, endpoint, "client", status));

        status = writePartsAttribute(endpoint);
        VerifyOrReturn(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, kErrorStr, endpoint, "parts", status));
    }
}
