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

#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Descriptor::Attributes;

constexpr const char * kErrorStr = "Descriptor cluster (0x%02x) Error setting '%s' attribute: 0x%02x";

namespace {

class DescriptorAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the Descriptor cluster on all endpoints.
    DescriptorAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), Descriptor::Id) {}

    CHIP_ERROR Read(const ConcreteAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadPartsAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadDeviceAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadClientServerAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder, bool server);
};

CHIP_ERROR DescriptorAttrAccess::ReadPartsAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (endpoint == 0x00)
    {
        err = aEncoder.EncodeList([](const TagBoundEncoder & encoder) -> CHIP_ERROR {
            for (uint16_t index = 0; index < emberAfEndpointCount(); index++)
            {
                if (emberAfEndpointIndexIsEnabled(index))
                {
                    EndpointId endpointId = emberAfEndpointFromIndex(index);
                    if (endpointId == 0)
                        continue;

                    ReturnErrorOnFailure(encoder.Encode(endpointId));
                }
            }

            return CHIP_NO_ERROR;
        });
    }
    else
    {
        err = aEncoder.Encode(DataModel::List<EndpointId>());
    }

    return err;
}

CHIP_ERROR DescriptorAttrAccess::ReadDeviceAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = aEncoder.EncodeList([&endpoint](const TagBoundEncoder & encoder) -> CHIP_ERROR {
        Descriptor::Structs::DeviceType::Type deviceStruct;
        uint16_t index = emberAfIndexFromEndpoint(endpoint);

        deviceStruct.type     = emberAfDeviceIdFromIndex(index);
        deviceStruct.revision = emberAfDeviceVersionFromIndex(index);
        return encoder.Encode(deviceStruct);
    });

    return err;
}

CHIP_ERROR DescriptorAttrAccess::ReadClientServerAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder, bool server)
{
    CHIP_ERROR err = aEncoder.EncodeList([&endpoint, server](const TagBoundEncoder & encoder) -> CHIP_ERROR {
        uint16_t clusterCount = emberAfClusterCount(endpoint, server);

        for (uint8_t clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++)
        {
            EmberAfCluster * cluster = emberAfGetNthCluster(endpoint, clusterIndex, server);
            ReturnErrorOnFailure(encoder.Encode(cluster->clusterId));
        }

        return CHIP_NO_ERROR;
    });

    return err;
}

DescriptorAttrAccess gAttrAccess;

CHIP_ERROR DescriptorAttrAccess::Read(const ConcreteAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == Descriptor::Id);

    switch (aPath.mAttributeId)
    {
    case DeviceList::Id: {
        return ReadDeviceAttribute(aPath.mEndpointId, aEncoder);
    }
    case ServerList::Id: {
        return ReadClientServerAttribute(aPath.mEndpointId, aEncoder, true);
    }
    case ClientList::Id: {
        return ReadClientServerAttribute(aPath.mEndpointId, aEncoder, false);
    }
    case PartsList::Id: {
        return ReadPartsAttribute(aPath.mEndpointId, aEncoder);
    }
    default: {
        break;
    }
    }
    return CHIP_NO_ERROR;
}
} // anonymous namespace

EmberAfStatus writeAttribute(EndpointId endpoint, AttributeId attributeId, uint8_t * buffer, int32_t index = -1)
{
    EmberAfAttributeSearchRecord record;
    record.endpoint         = endpoint;
    record.clusterId        = Descriptor::Id;
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
    AttributeId attributeId = server ? Descriptor::Attributes::ServerList::Id : Descriptor::Attributes::ClientList::Id;

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
    AttributeId attributeId = Descriptor::Attributes::DeviceList::Id;

    uint16_t deviceTypeCount  = 1;
    DeviceTypeId deviceTypeId = emberAfDeviceIdFromIndex(index);
    uint16_t revision         = emberAfDeviceVersionFromIndex(index);

    DeviceType deviceType;
    deviceType.type     = deviceTypeId;
    deviceType.revision = revision;

    status = writeAttribute(endpoint, attributeId, (uint8_t *) &deviceType, 0);
    VerifyOrReturnError(status == EMBER_ZCL_STATUS_SUCCESS, status);

    return writeAttribute(endpoint, attributeId, (uint8_t *) &deviceTypeCount);
}

EmberAfStatus writePartsAttribute(EndpointId endpoint)
{
    EmberAfStatus status    = EMBER_ZCL_STATUS_SUCCESS;
    AttributeId attributeId = Descriptor::Attributes::PartsList::Id;

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

void MatterDescriptorPluginServerInitCallback(void)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

#if CHIP_CLUSTER_CONFIG_ENABLE_COMPLEX_ATTRIBUTE_READ
    static bool attrAccessRegistered = false;

    if (!attrAccessRegistered)
    {
        registerAttributeAccessOverride(&gAttrAccess);
        attrAccessRegistered = true;
    }
#endif

    /*
       To prevent reporting from being broken, we still need to keep following part in case
       CHIP_CLUSTER_CONFIG_ENABLE_COMPLEX_ATTRIBUTE_READ is true. The old setup has the emberAfPluginDescriptorServerInitCallback
       called in emberAfEndpointEnableDisable which updates the stored values for the new topology.
    */
    for (uint16_t index = 0; index < emberAfEndpointCount(); index++)
    {
        if (!emberAfEndpointIndexIsEnabled(index))
        {
            continue;
        }

        EndpointId endpoint = emberAfEndpointFromIndex(index);
        if (!emberAfContainsCluster(endpoint, Descriptor::Id))
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
