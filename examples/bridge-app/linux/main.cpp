/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
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

#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

#include <app-common/zap-generated/af-structs.h>

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app/chip-zcl-zpro-codec.h>
#include <app/reporting/reporting.h>
#include <app/util/af-types.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/RandUtils.h>
#include <lib/support/ZclString.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include "Device.h"
#include "Options.h"
#include <app/server/Server.h>

#include <cassert>
#include <iostream>

using namespace chip;
using namespace chip::Credentials;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::DeviceLayer;

static const int kUserLabelSize = 32;
// Current ZCL implementation of Struct uses a max-size array of 254 bytes
static const int kDescriptorAttributeArraySize = 254;
static const int kFixedLabelAttributeArraySize = 254;
// Four attributes in descriptor cluster: DeviceTypeList, ServerList, ClientList, PartsList
static const int kDescriptorAttributeCount          = 4;
static const int kFixedLabelElementsOctetStringSize = 16;

static EndpointId gCurrentEndpointId;
static EndpointId gFirstDynamicEndpointId;
static Device * gDevices[DYNAMIC_ENDPOINT_COUNT];
// Storage for descriptor clusters on dynamic endpoints. The contents of these
// attrs are maintained by the descriptor code (descriptor.cpp). However, since
// all attributes in clusters on dynamic endpoints are external, we must store
// in the app.
static uint8_t gDescriptorAttrStorage[DYNAMIC_ENDPOINT_COUNT][kDescriptorAttributeCount][kDescriptorAttributeArraySize];

// ENDPOINT DEFINITIONS:
// =================================================================================
//
// Endpoint definitions will be reused across multiple endpoints for every instance of the
// endpoint type.
// There will be no intrinsic storage for the endpoint attributes declared here.
// Instead, all attributes will be treated as EXTERNAL, and therefore all reads
// or writes to the attributes must be handled within the emberAfExternalAttributeWriteCallback
// and emberAfExternalAttributeReadCallback functions declared herein. This fits
// the typical model of a bridge, since a bridge typically maintains its own
// state database representing the devices connected to it.

// Device types for dynamic endpoints: TODO Need a generated file from ZAP to define these!
// (taken from chip-devices.xml)
#define DEVICE_TYPE_CHIP_BRIDGE 0x0a0b
// (taken from lo-devices.xml)
#define DEVICE_TYPE_LO_ON_OFF_LIGHT 0x0100

// Device Version for dynamic endpoints:
#define DEVICE_VERSION_DEFAULT 1

// ---------------------------------------------------------------------------
//
// LIGHT ENDPOINT: contains the following clusters:
//   - On/Off
//   - Descriptor
//   - Bridged Device Basic
//   - Fixed Label

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

// ---------------------------------------------------------------------------

int AddDeviceEndpoint(Device * dev, EmberAfEndpointType * ep, uint16_t deviceType)
{
    uint8_t index = 0;
    while (index < DYNAMIC_ENDPOINT_COUNT)
    {
        if (NULL == gDevices[index])
        {
            gDevices[index] = dev;
            EmberAfStatus ret;
            while (1)
            {
                ret = emberAfSetDynamicEndpoint(index, gCurrentEndpointId, ep, deviceType, DEVICE_VERSION_DEFAULT);
                if (ret == EMBER_ZCL_STATUS_SUCCESS)
                {
                    ChipLogProgress(DeviceLayer, "Added device %s to dynamic endpoint %d (index=%d)", dev->GetName(),
                                    gCurrentEndpointId, index);
                    return index;
                }
                else if (ret != EMBER_ZCL_STATUS_DUPLICATE_EXISTS)
                {
                    return -1;
                }
                // Handle wrap condition
                if (++gCurrentEndpointId < gFirstDynamicEndpointId)
                {
                    gCurrentEndpointId = gFirstDynamicEndpointId;
                }
            }
        }
        index++;
    }
    ChipLogProgress(DeviceLayer, "Failed to add dynamic endpoint: No endpoints available!");
    return -1;
}

int RemoveDeviceEndpoint(Device * dev)
{
    uint8_t index = 0;
    while (index < DYNAMIC_ENDPOINT_COUNT)
    {
        if (gDevices[index] == dev)
        {
            EndpointId ep   = emberAfClearDynamicEndpoint(index);
            gDevices[index] = NULL;
            ChipLogProgress(DeviceLayer, "Removed device %s from dynamic endpoint %d (index=%d)", dev->GetName(), ep, index);
            // Silence complaints about unused ep when progress logging
            // disabled.
            UNUSED_VAR(ep);
            return index;
        }
        index++;
    }
    return -1;
}

void EncodeFixedLabel(const char * label, const char * value, uint8_t * buffer, uint16_t length, EmberAfAttributeMetadata * am)
{
    char zclOctetStrBuf[kFixedLabelElementsOctetStringSize];
    uint16_t listCount = 1;
    _LabelStruct labelStruct;

    labelStruct.label = chip::ByteSpan(reinterpret_cast<const uint8_t *>(label), kFixedLabelElementsOctetStringSize);

    strncpy(zclOctetStrBuf, value, sizeof(zclOctetStrBuf));
    labelStruct.value = chip::ByteSpan(reinterpret_cast<uint8_t *>(&zclOctetStrBuf[0]), sizeof(zclOctetStrBuf));

    emberAfCopyList(ZCL_FIXED_LABEL_CLUSTER_ID, am, true, buffer, reinterpret_cast<uint8_t *>(&labelStruct), 1);
    emberAfCopyList(ZCL_FIXED_LABEL_CLUSTER_ID, am, true, buffer, reinterpret_cast<uint8_t *>(&listCount), 0);
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
        MutableByteSpan zclNameSpan(zclName);
        MakeZclCharString(zclNameSpan, dev->GetName());
        emberAfReportingAttributeChangeCallback(dev->GetEndpointId(), ZCL_BRIDGED_DEVICE_BASIC_CLUSTER_ID,
                                                ZCL_USER_LABEL_ATTRIBUTE_ID, CLUSTER_MASK_SERVER, 0, ZCL_CHAR_STRING_ATTRIBUTE_TYPE,
                                                zclNameSpan.data());
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
        uint8_t bufferMemory[254];
        MutableByteSpan zclString(bufferMemory);
        MakeZclCharString(zclString, dev->GetName());
        buffer = zclString.data();
    }
    else
    {
        return EMBER_ZCL_STATUS_FAILURE;
    }

    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus HandleReadOnOffAttribute(Device * dev, chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength)
{
    ChipLogProgress(DeviceLayer, "HandleReadOnOffAttribute: attrId=%d, maxReadLength=%d", attributeId, maxReadLength);

    if ((attributeId == ZCL_ON_OFF_ATTRIBUTE_ID) && (maxReadLength == 1))
    {
        *buffer = dev->IsOn() ? 1 : 0;
    }
    else
    {
        return EMBER_ZCL_STATUS_FAILURE;
    }

    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus HandleWriteOnOffAttribute(Device * dev, chip::AttributeId attributeId, uint8_t * buffer)
{
    ChipLogProgress(DeviceLayer, "HandleWriteOnOffAttribute: attrId=%d", attributeId);

    if ((attributeId == ZCL_ON_OFF_ATTRIBUTE_ID) && (dev->IsReachable()))
    {
        if (*buffer)
        {
            dev->SetOnOff(true);
        }
        else
        {
            dev->SetOnOff(false);
        }
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

    EmberAfStatus ret = EMBER_ZCL_STATUS_FAILURE;

    if ((endpointIndex < DYNAMIC_ENDPOINT_COUNT) && (gDevices[endpointIndex] != NULL))
    {
        Device * dev = gDevices[endpointIndex];

        if (clusterId == ZCL_BRIDGED_DEVICE_BASIC_CLUSTER_ID)
        {
            ret = HandleReadBridgedDeviceBasicAttribute(dev, attributeMetadata->attributeId, buffer, maxReadLength);
        }
        else if (clusterId == ZCL_DESCRIPTOR_CLUSTER_ID)
        {
            ret = HandleReadDescriptorAttribute(endpointIndex, attributeMetadata->attributeId, buffer, maxReadLength);
        }
        else if (clusterId == ZCL_FIXED_LABEL_CLUSTER_ID)
        {
            ret = HandleReadFixedLabelAttribute(dev, attributeMetadata, buffer, maxReadLength);
        }
        else if (clusterId == ZCL_ON_OFF_CLUSTER_ID)
        {
            ret = HandleReadOnOffAttribute(dev, attributeMetadata->attributeId, buffer, maxReadLength);
        }
    }

    return ret;
}

EmberAfStatus emberAfExternalAttributeWriteCallback(EndpointId endpoint, ClusterId clusterId,
                                                    EmberAfAttributeMetadata * attributeMetadata, uint16_t manufacturerCode,
                                                    uint8_t * buffer, int32_t index)
{
    uint16_t endpointIndex = emberAfGetDynamicIndexFromEndpoint(endpoint);

    EmberAfStatus ret = EMBER_ZCL_STATUS_FAILURE;

    // ChipLogProgress(DeviceLayer, "emberAfExternalAttributeWriteCallback: ep=%d", endpoint);

    if (endpointIndex < DYNAMIC_ENDPOINT_COUNT)
    {
        Device * dev = gDevices[endpointIndex];

        if ((dev->IsReachable()) && (clusterId == ZCL_ON_OFF_CLUSTER_ID))
        {
            ret = HandleWriteOnOffAttribute(dev, attributeMetadata->attributeId, buffer);
        }
        else if (clusterId == ZCL_DESCRIPTOR_CLUSTER_ID)
        {
            ret = HandleWriteDescriptorAttribute(endpointIndex, attributeMetadata, buffer, attributeMetadata->size, index);
        }
    }

    return ret;
}

namespace {
void EventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    (void) arg;
    if (event->Type == chip::DeviceLayer::DeviceEventType::kCHIPoBLEConnectionEstablished)
    {
        ChipLogProgress(DeviceLayer, "Receive kCHIPoBLEConnectionEstablished");
    }
}

CHIP_ERROR PrintQRCodeContent()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // If we do not have a discriminator, generate one
    chip::SetupPayload payload;
    uint32_t setUpPINCode;
    uint16_t setUpDiscriminator;
    uint16_t vendorId;
    uint16_t productId;
    std::string result;

    err = ConfigurationMgr().GetSetupPinCode(setUpPINCode);
    SuccessOrExit(err);

    err = ConfigurationMgr().GetSetupDiscriminator(setUpDiscriminator);
    SuccessOrExit(err);

    err = ConfigurationMgr().GetVendorId(vendorId);
    SuccessOrExit(err);

    err = ConfigurationMgr().GetProductId(productId);
    SuccessOrExit(err);

    payload.version       = 0;
    payload.vendorID      = vendorId;
    payload.productID     = productId;
    payload.setUpPINCode  = setUpPINCode;
    payload.discriminator = setUpDiscriminator;

    // Wrap it so SuccessOrExit can work
    {
        chip::QRCodeSetupPayloadGenerator generator(payload);
        err = generator.payloadBase38Representation(result);
        SuccessOrExit(err);
    }

    std::cout << "SetupPINCode: [" << setUpPINCode << "]" << std::endl;
    // There might be whitespace in setup QRCode, add brackets to make it clearer.
    std::cout << "SetupQRCode:  [" << result << "]" << std::endl;

exit:
    if (err != CHIP_NO_ERROR)
    {
        std::cerr << "Failed to generate QR Code: " << ErrorStr(err) << std::endl;
    }
    return err;
}
} // namespace

int main(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Clear out the device database
    memset(gDevices, 0, sizeof(gDevices));

    // Create Mock Devices

    // Define 4 lights
    Device Light1("Light 1", "Office");
    Device Light2("Light 2", "Office");
    Device Light3("Light 3", "Office");
    Device Light4("Light 4", "Den");

    Light1.SetChangeCallback(&HandleDeviceStatusChanged);
    Light2.SetChangeCallback(&HandleDeviceStatusChanged);
    Light3.SetChangeCallback(&HandleDeviceStatusChanged);
    Light4.SetChangeCallback(&HandleDeviceStatusChanged);

    Light1.SetReachable(true);
    Light2.SetReachable(true);
    Light3.SetReachable(true);
    Light4.SetReachable(true);

    // Initialize CHIP

    err = chip::Platform::MemoryInit();
    SuccessOrExit(err);

    err = ParseArguments(argc, argv);
    SuccessOrExit(err);

    err = chip::DeviceLayer::PlatformMgr().InitChipStack();
    SuccessOrExit(err);

    err = PrintQRCodeContent();
    SuccessOrExit(err);

    chip::DeviceLayer::PlatformMgrImpl().AddEventHandler(EventHandler, 0);

    chip::DeviceLayer::ConnectivityMgr().SetBLEDeviceName(nullptr); // Use default device name (CHIP-XXXX)

#if CONFIG_NETWORK_LAYER_BLE
    chip::DeviceLayer::Internal::BLEMgrImpl().ConfigureBle(LinuxDeviceOptions::GetInstance().mBleDevice, false);
#endif

    chip::DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(true);

    // Init ZCL Data Model and CHIP App Server
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

    // Run CHIP

    chip::DeviceLayer::PlatformMgr().RunEventLoop();

exit:
    if (err != CHIP_NO_ERROR)
    {
        std::cerr << "Failed to run Linux Bridge App: " << ErrorStr(err) << std::endl;
        // End the program with non zero error code to indicate a error.
        return 1;
    }
    return 0;
}
