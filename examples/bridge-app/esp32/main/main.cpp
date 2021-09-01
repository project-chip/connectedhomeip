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

#include "Device.h"
#include "DeviceCallbacks.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app/reporting/reporting.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/ErrorStr.h>

#include <app/server/Server.h>

const char * TAG = "bridge-app";

using namespace ::chip;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceManager;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Platform;

static DeviceCallbacks AppCallback;

static const int kUserLabelSize = 32;
// Current ZCL implementation of Struct uses a max-size array of 254 bytes
static const int kDescriptorAttributeArraySize = 254;
static const int kFixedLabelAttributeArraySize = 254;
// Four attributes in descriptor cluster: DeviceTypeList, ServerList, ClientList, PartsList
static const int kDescriptorAttributeCount          = 4;
static const int kFixedLabelElementsOctetStringSize = 16;

static EndpointId gCurrentEndpointId;
static EndpointId gFirstDynamicEndpointId;
static Device * gDevices[DYNAMIC_ENDPOINT_COUNT]; // number of dynamic endpoints count

// Descriptor attribute storage on dynamic endpoint
static uint8_t gDescriptorAttrStorage[DYNAMIC_ENDPOINT_COUNT][kDescriptorAttributeCount][kDescriptorAttributeArraySize];

// (taken from chip-devices.xml)
#define DEVICE_TYPE_CHIP_BRIDGE 0x0a0b
// (taken from lo-devices.xml)
#define DEVICE_TYPE_LO_ON_OFF_LIGHT 0x0100
// Device Version for dynamic endpoints:
#define DEVICE_VERSION_DEFAULT 1

/* BRIDGED DEVICE ENDPOINT: contains the following clusters:
   - On/Off
   - Descriptor
   - Bridged Device Basic
   - Fixed Label
*/

// Declare On/Off cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(onOffAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(ZCL_ON_OFF_ATTRIBUTE_ID, BOOLEAN, 1, 0) /* on/off */
DECLARE_DYNAMIC_ATTRIBUTE_LIST_END(0x0001);

// Declare Descriptor cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(descriptorAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(ZCL_DEVICE_LIST_ATTRIBUTE_ID, ARRAY, kDescriptorAttributeArraySize, 0),     /* device list */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_SERVER_LIST_ATTRIBUTE_ID, ARRAY, kDescriptorAttributeArraySize, 0), /* server list */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_CLIENT_LIST_ATTRIBUTE_ID, ARRAY, kDescriptorAttributeArraySize, 0), /* client list */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_PARTS_LIST_ATTRIBUTE_ID, ARRAY, kDescriptorAttributeArraySize, 0)   /* parts list */
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END(0x0001);

// Declare Bridged Device Basic information cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(bridgedDeviceBasicAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(ZCL_USER_LABEL_ATTRIBUTE_ID, CHAR_STRING, kUserLabelSize, 0), /* UserLabel */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_REACHABLE_ATTRIBUTE_ID, BOOLEAN, 1, 0)                /* Reachable */
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END(0x0003);

// Declare Fixed Label cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(fixedLabelAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(ZCL_LABEL_LIST_ATTRIBUTE_ID, ARRAY, kFixedLabelAttributeArraySize, 0) /* label list */
DECLARE_DYNAMIC_ATTRIBUTE_LIST_END(0x0001);

// Declare Cluster List for Bridged Light endpoint
DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(bridgedLightClusters)
DECLARE_DYNAMIC_CLUSTER(ZCL_ON_OFF_CLUSTER_ID, onOffAttrs), DECLARE_DYNAMIC_CLUSTER(ZCL_DESCRIPTOR_CLUSTER_ID, descriptorAttrs),
    DECLARE_DYNAMIC_CLUSTER(ZCL_BRIDGED_DEVICE_BASIC_CLUSTER_ID, bridgedDeviceBasicAttrs),
    DECLARE_DYNAMIC_CLUSTER(ZCL_FIXED_LABEL_CLUSTER_ID, fixedLabelAttrs) DECLARE_DYNAMIC_CLUSTER_LIST_END;

// Declare Bridged Light endpoint
DECLARE_DYNAMIC_ENDPOINT(bridgedLightEndpoint, bridgedLightClusters);

CHIP_ERROR AddDeviceEndpoint(Device * dev, EmberAfEndpointType * ep, uint16_t deviceType)
{
    uint8_t index = 0;
    while (index < DYNAMIC_ENDPOINT_COUNT)
    {
        if (NULL == gDevices[index])
        {
            gDevices[index] = dev;
            EmberAfStatus ret;
            ret = emberAfSetDynamicEndpoint(index, gCurrentEndpointId, ep, deviceType, DEVICE_VERSION_DEFAULT);
            if (ret == EMBER_ZCL_STATUS_SUCCESS)
            {
                ChipLogProgress(DeviceLayer, "Added device %s to dynamic endpoint %d (index=%d)", dev->GetName(),
                                gCurrentEndpointId, index);
                gCurrentEndpointId++;
                return CHIP_NO_ERROR;
            }
            else if (ret != EMBER_ZCL_STATUS_DUPLICATE_EXISTS)
            {
                ChipLogProgress(DeviceLayer, "Failed to add dynamic endpoint, Insufficient space");
                return CHIP_ERROR_INTERNAL;
            }
        }
        index++;
    }
    ChipLogProgress(DeviceLayer, "Failed to add dynamic endpoint: No endpoints available!");
    return CHIP_ERROR_INTERNAL;
}

CHIP_ERROR RemoveDeviceEndpoint(Device * dev)
{
    for (uint8_t index = 0; index < DYNAMIC_ENDPOINT_COUNT; index++)
    {
        if (gDevices[index] == dev)
        {
            EndpointId ep   = emberAfClearDynamicEndpoint(index);
            gDevices[index] = NULL;
            ChipLogProgress(DeviceLayer, "Removed device %s from dynamic endpoint %d (index=%d)", dev->GetName(), ep, index);
            // Silence complaints about unused ep when progress logging
            // disabled.
            UNUSED_VAR(ep);
            return CHIP_NO_ERROR;
        }
    }
    return CHIP_ERROR_INTERNAL;
}

// ZCL format -> (len, string)
uint8_t * ToZclCharString(uint8_t * zclString, const char * cString, uint8_t maxLength)
{
    size_t len = strlen(cString);
    if (len > maxLength)
    {
        len = maxLength;
    }
    zclString[0] = static_cast<uint8_t>(len);
    memcpy(&zclString[1], cString, zclString[0]);
    return zclString;
}

// Converted into bytes and mapped the (label, value)
void EncodeFixedLabel(const char * label, const char * value, uint8_t * buffer, uint16_t length, EmberAfAttributeMetadata * am)
{
    uint16_t listCount = 1;
    _LabelStruct labelStruct;

    labelStruct.label = chip::ByteSpan(Uint8::from_const_char(label), strlen(label));
    labelStruct.value = chip::ByteSpan(Uint8::from_const_char(value), strlen(value));

    emberAfCopyList(ZCL_FIXED_LABEL_CLUSTER_ID, am, true, buffer, reinterpret_cast<uint8_t *>(&labelStruct), 1);
    emberAfCopyList(ZCL_FIXED_LABEL_CLUSTER_ID, am, true, buffer, reinterpret_cast<uint8_t *>(&listCount), 0);
}

EmberAfStatus HandleReadBridgedDeviceBasicAttribute(Device * dev, chip::AttributeId attributeId, uint8_t * buffer,
                                                    uint16_t maxReadLength)
{
    ChipLogProgress(DeviceLayer, "HandleReadBridgedDeviceBasicAttribute: attrId=%d, maxReadLength=%d", attributeId, maxReadLength);

    if ((attributeId == ZCL_REACHABLE_ATTRIBUTE_ID) && (maxReadLength == 1))
    {
        *buffer = dev->IsReachable() ? 1 : 0;
    }
    else if ((attributeId == ZCL_USER_LABEL_ATTRIBUTE_ID) && (maxReadLength == 32))
    {
        ToZclCharString(buffer, dev->GetName(), static_cast<uint8_t>(maxReadLength - 1));
    }
    else
    {
        return EMBER_ZCL_STATUS_FAILURE;
    }

    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus HandleReadDescriptorAttribute(uint16_t endpointIndex, chip::AttributeId attributeId, uint8_t * buffer,
                                            uint16_t maxReadLength)
{
    if ((maxReadLength <= kDescriptorAttributeArraySize) && (attributeId < kDescriptorAttributeCount))
    {
        memcpy(buffer, &gDescriptorAttrStorage[endpointIndex][attributeId][0], maxReadLength);
        return EMBER_ZCL_STATUS_SUCCESS;
    }

    return EMBER_ZCL_STATUS_FAILURE;
}

EmberAfStatus HandleReadFixedLabelAttribute(Device * dev, EmberAfAttributeMetadata * am, uint8_t * buffer, uint16_t maxReadLength)
{
    if ((am->attributeId == ZCL_LABEL_LIST_ATTRIBUTE_ID) && (maxReadLength <= kFixedLabelAttributeArraySize))
    {
        EncodeFixedLabel("room", dev->GetLocation(), buffer, maxReadLength, am);

        return EMBER_ZCL_STATUS_SUCCESS;
    }

    return EMBER_ZCL_STATUS_FAILURE;
}

EmberAfStatus HandleReadOnOffAttribute(Device * dev, chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength)
{
    ChipLogProgress(DeviceLayer, "HandleReadOnOffAttribute: attrId=%d, maxReadLength=%d", attributeId, maxReadLength);

    ReturnErrorCodeIf((attributeId != ZCL_ON_OFF_ATTRIBUTE_ID) || (maxReadLength != 1), EMBER_ZCL_STATUS_FAILURE);
    *buffer = dev->IsOn() ? 1 : 0;
    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus HandleWriteOnOffAttribute(Device * dev, chip::AttributeId attributeId, uint8_t * buffer)
{
    ChipLogProgress(DeviceLayer, "HandleWriteOnOffAttribute: attrId=%d", attributeId);

    ReturnErrorCodeIf((attributeId != ZCL_ON_OFF_ATTRIBUTE_ID) || (!dev->IsReachable()), EMBER_ZCL_STATUS_FAILURE);
    dev->SetOnOff(*buffer == 1);
    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus HandleWriteDescriptorAttribute(uint16_t endpointIndex, EmberAfAttributeMetadata * am, uint8_t * buffer,
                                             uint16_t length, int32_t index)
{
    chip::AttributeId attributeId = am->attributeId;

    if (emberAfCopyList(ZCL_DESCRIPTOR_CLUSTER_ID, am, true, &gDescriptorAttrStorage[endpointIndex][attributeId][0], buffer,
                        index) > 0)
    {
        return EMBER_ZCL_STATUS_SUCCESS;
    }

    return EMBER_ZCL_STATUS_FAILURE;
}

EmberAfStatus emberAfExternalAttributeReadCallback(EndpointId endpoint, ClusterId clusterId,
                                                   EmberAfAttributeMetadata * attributeMetadata, uint16_t manufacturerCode,
                                                   uint8_t * buffer, uint16_t maxReadLength, int32_t index)
{
    uint16_t endpointIndex = emberAfGetDynamicIndexFromEndpoint(endpoint);

    if ((endpointIndex < DYNAMIC_ENDPOINT_COUNT) && (gDevices[endpointIndex] != NULL))
    {
        Device * dev = gDevices[endpointIndex];

        if (clusterId == ZCL_BRIDGED_DEVICE_BASIC_CLUSTER_ID)
        {
            return HandleReadBridgedDeviceBasicAttribute(dev, attributeMetadata->attributeId, buffer, maxReadLength);
        }
        else if (clusterId == ZCL_DESCRIPTOR_CLUSTER_ID)
        {
            return HandleReadDescriptorAttribute(endpointIndex, attributeMetadata->attributeId, buffer, maxReadLength);
        }
        else if (clusterId == ZCL_FIXED_LABEL_CLUSTER_ID)
        {
            return HandleReadFixedLabelAttribute(dev, attributeMetadata, buffer, maxReadLength);
        }
        else if (clusterId == ZCL_ON_OFF_CLUSTER_ID)
        {
            return HandleReadOnOffAttribute(dev, attributeMetadata->attributeId, buffer, maxReadLength);
        }
    }

    return EMBER_ZCL_STATUS_FAILURE;
}

EmberAfStatus emberAfExternalAttributeWriteCallback(EndpointId endpoint, ClusterId clusterId,
                                                    EmberAfAttributeMetadata * attributeMetadata, uint16_t manufacturerCode,
                                                    uint8_t * buffer, int32_t index)
{
    uint16_t endpointIndex = emberAfGetDynamicIndexFromEndpoint(endpoint);

    if (endpointIndex < DYNAMIC_ENDPOINT_COUNT)
    {
        Device * dev = gDevices[endpointIndex];

        if ((dev->IsReachable()) && (clusterId == ZCL_ON_OFF_CLUSTER_ID))
        {
            return HandleWriteOnOffAttribute(dev, attributeMetadata->attributeId, buffer);
        }
        else if (clusterId == ZCL_DESCRIPTOR_CLUSTER_ID)
        {
            return HandleWriteDescriptorAttribute(endpointIndex, attributeMetadata, buffer, attributeMetadata->size, index);
        }
    }

    return EMBER_ZCL_STATUS_FAILURE;
}

void HandleDeviceStatusChanged(Device * dev, Device::Changed_t itemChangedMask)
{
    if (itemChangedMask & Device::kChanged_Reachable)
    {
        uint8_t reachable = dev->IsReachable() ? 1 : 0;
        emberAfReportingAttributeChangeCallback(dev->GetEndpointId(), ZCL_BRIDGED_DEVICE_BASIC_CLUSTER_ID,
                                                ZCL_REACHABLE_ATTRIBUTE_ID, CLUSTER_MASK_SERVER, 0, ZCL_BOOLEAN_ATTRIBUTE_TYPE,
                                                &reachable);
    }

    if (itemChangedMask & Device::kChanged_State)
    {
        uint8_t isOn = dev->IsOn() ? 1 : 0;
        emberAfReportingAttributeChangeCallback(dev->GetEndpointId(), ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID,
                                                CLUSTER_MASK_SERVER, 0, ZCL_BOOLEAN_ATTRIBUTE_TYPE, &isOn);
    }

    if (itemChangedMask & Device::kChanged_Name)
    {
        uint8_t zclName[kUserLabelSize];
        ToZclCharString(zclName, dev->GetName(), kUserLabelSize - 1);
        emberAfReportingAttributeChangeCallback(dev->GetEndpointId(), ZCL_BRIDGED_DEVICE_BASIC_CLUSTER_ID,
                                                ZCL_USER_LABEL_ATTRIBUTE_ID, CLUSTER_MASK_SERVER, 0, ZCL_CHAR_STRING_ATTRIBUTE_TYPE,
                                                zclName);
    }
    if (itemChangedMask & Device::kChanged_Location)
    {
        uint8_t buffer[kFixedLabelAttributeArraySize];
        EmberAfAttributeMetadata am = { .attributeId  = ZCL_LABEL_LIST_ATTRIBUTE_ID,
                                        .size         = kFixedLabelAttributeArraySize,
                                        .defaultValue = nullptr };

        EncodeFixedLabel("room", dev->GetLocation(), buffer, sizeof(buffer), &am);

        emberAfReportingAttributeChangeCallback(dev->GetEndpointId(), ZCL_FIXED_LABEL_CLUSTER_ID, ZCL_LABEL_LIST_ATTRIBUTE_ID,
                                                CLUSTER_MASK_SERVER, 0, ZCL_ARRAY_ATTRIBUTE_TYPE, buffer);
    }
}

extern "C" void app_main()
{
    // Initialize the ESP NVS layer.
    esp_err_t err = nvs_flash_init();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "nvs_flash_init() failed: %s", esp_err_to_name(err));
        return;
    }

    CHIP_ERROR chip_err = CHIP_NO_ERROR;

    // bridge will have own database named gDevices.
    // Clear database
    memset(gDevices, 0, sizeof(gDevices));

    // Bridged devices 4
    static Device Light1("Light 1", "Office");
    static Device Light2("Light 2", "Office");
    static Device Light3("Light 3", "Kitchen");
    static Device Light4("Light 4", "Den");

    // Whenever bridged device changes its state
    Light1.SetChangeCallback(&HandleDeviceStatusChanged);
    Light2.SetChangeCallback(&HandleDeviceStatusChanged);
    Light3.SetChangeCallback(&HandleDeviceStatusChanged);
    Light4.SetChangeCallback(&HandleDeviceStatusChanged);

    Light1.SetReachable(true);
    Light2.SetReachable(true);
    Light3.SetReachable(true);
    Light4.SetReachable(true);

    CHIPDeviceManager & deviceMgr = CHIPDeviceManager::GetInstance();

    chip_err = deviceMgr.Init(&AppCallback);
    if (chip_err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "device.Init() failed: %s", ErrorStr(chip_err));
        return;
    }

    InitServer();

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

    // Set starting endpoint id where dynamic endpoints will be assigned, which
    // will be the next consecutive endpoint id after the last fixed endpoint.
    gFirstDynamicEndpointId = static_cast<chip::EndpointId>(
        static_cast<int>(emberAfEndpointFromIndex(static_cast<uint16_t>(emberAfFixedEndpointCount() - 1))) + 1);
    gCurrentEndpointId = gFirstDynamicEndpointId;

    // Disable last fixed endpoint, which is used as a placeholder for all of the
    // supported clusters so that ZAP will generated the requisite code.
    emberAfEndpointEnableDisable(emberAfEndpointFromIndex(static_cast<uint16_t>(emberAfFixedEndpointCount() - 1)), false);

    // Add lights 1..3 --> will be mapped to ZCL endpoints 2, 3, 4
    AddDeviceEndpoint(&Light1, &bridgedLightEndpoint, DEVICE_TYPE_LO_ON_OFF_LIGHT);
    AddDeviceEndpoint(&Light2, &bridgedLightEndpoint, DEVICE_TYPE_LO_ON_OFF_LIGHT);
    AddDeviceEndpoint(&Light3, &bridgedLightEndpoint, DEVICE_TYPE_LO_ON_OFF_LIGHT);

    // Remove Light 2 -- Lights 1 & 3 will remain mapped to endpoints 2 & 4
    RemoveDeviceEndpoint(&Light2);

    // Add Light 4 -- > will be mapped to ZCL endpoint 5
    AddDeviceEndpoint(&Light4, &bridgedLightEndpoint, DEVICE_TYPE_LO_ON_OFF_LIGHT);

    // Re-add Light 2 -- > will be mapped to ZCL endpoint 6
    AddDeviceEndpoint(&Light2, &bridgedLightEndpoint, DEVICE_TYPE_LO_ON_OFF_LIGHT);
}
