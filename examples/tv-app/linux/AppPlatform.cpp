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

/**
 * @file Contains shell commands for a commissionee (eg. end device) related to commissioning.
 */

#include "AppPlatform.h"
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <inttypes.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/DataModelTypes.h>
#include <lib/shell/Commands.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ZclString.h>
#include <platform/CHIPDeviceLayer.h>

// #include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/cluster-id.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/Command.h>
#include <app/util/af.h>

#include "AppPlatform.h"

#include <cstdio>
#include <platform/CHIPDeviceLayer.h>

// #include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/cluster-id.h>

using namespace chip;
using namespace chip::AppPlatform;

static EndpointId gCurrentEndpointId;
static EndpointId gFirstDynamicEndpointId;
static ContentApp * gContentApps[DYNAMIC_ENDPOINT_COUNT];

// Device Version for dynamic endpoints:
#define DEVICE_VERSION_DEFAULT 1

#define ZCL_DESCRIPTOR_CLUSTER_REVISION (1u)
#define ZCL_APPLICATION_BASIC_CLUSTER_REVISION (1u)

int AddContentApp(ContentApp * app, EmberAfEndpointType * ep, uint16_t deviceType)
{
    ChipLogProgress(DeviceLayer, "Adding device %s ", app->GetApplicationName());
    uint8_t index = 0;
    // check if already loaded
    while (index < DYNAMIC_ENDPOINT_COUNT)
    {
        if (gContentApps[index] == app)
        {
            ChipLogProgress(DeviceLayer, "Already added");
            return index;
        }
        index++;
    }

    index = 0;
    while (index < DYNAMIC_ENDPOINT_COUNT)
    {
        ChipLogProgress(DeviceLayer, "Adding device 1");
        if (NULL == gContentApps[index])
        {
            ChipLogProgress(DeviceLayer, "Adding device 2");
            gContentApps[index] = app;
            EmberAfStatus ret;
            while (1)
            {
                ret = emberAfSetDynamicEndpoint(index, gCurrentEndpointId, ep, deviceType, DEVICE_VERSION_DEFAULT);
                if (ret == EMBER_ZCL_STATUS_SUCCESS)
                {
                    ChipLogProgress(DeviceLayer, "Added device %s to dynamic endpoint %d (index=%d)", app->GetApplicationName(),
                                    gCurrentEndpointId, index);
                    app->SetEndpointId(gCurrentEndpointId);
                    return index;
                }
                else if (ret != EMBER_ZCL_STATUS_DUPLICATE_EXISTS)
                {
                    ChipLogProgress(DeviceLayer, "Adding device 3 ret=%d", ret);
                    return -1;
                }
                // Handle wrap condition
                if (++gCurrentEndpointId < gFirstDynamicEndpointId)
                {
                    ChipLogProgress(DeviceLayer, "Adding device 4");
                    gCurrentEndpointId = gFirstDynamicEndpointId;
                }
            }
        }
        index++;
    }
    ChipLogProgress(DeviceLayer, "Failed to add dynamic endpoint: No endpoints available!");
    return -1;
}

int RemoveContentApp(ContentApp * app)
{
    uint8_t index = 0;
    while (index < DYNAMIC_ENDPOINT_COUNT)
    {
        if (gContentApps[index] == app)
        {
            EndpointId ep       = emberAfClearDynamicEndpoint(index);
            gContentApps[index] = NULL;
            ChipLogProgress(DeviceLayer, "Removed device %s from dynamic endpoint %d (index=%d)", app->GetApplicationName(), ep,
                            index);
            // Silence complaints about unused ep when progress logging
            // disabled.
            UNUSED_VAR(ep);
            return index;
        }
        index++;
    }
    return -1;
}

EmberAfStatus HandleReadApplicationBasicAttribute(ContentApp * app, chip::AttributeId attributeId, uint8_t * buffer,
                                                  uint16_t maxReadLength)
{
    ChipLogProgress(DeviceLayer, "HandleReadApplicationBasicAttribute: attrId=%d, maxReadLength=%d", attributeId, maxReadLength);

    if ((attributeId == ZCL_APPLICATION_VENDOR_NAME_ATTRIBUTE_ID) && (maxReadLength == 32))
    {
        uint8_t bufferMemory[254];
        MutableByteSpan zclString(bufferMemory);
        MakeZclCharString(zclString, app->GetVendorName());
        buffer = zclString.data();
    }
    else if ((attributeId == ZCL_APPLICATION_VENDOR_ID_ATTRIBUTE_ID) && (maxReadLength == 1))
    {
        *(uint16_t *) buffer = app->GetVendorId();
    }
    else if ((attributeId == ZCL_APPLICATION_NAME_ATTRIBUTE_ID) && (maxReadLength == 32))
    {
        uint8_t bufferMemory[254];
        MutableByteSpan zclString(bufferMemory);
        MakeZclCharString(zclString, app->GetApplicationName());
        buffer = zclString.data();
    }
    else if ((attributeId == ZCL_APPLICATION_PRODUCT_ID_ATTRIBUTE_ID) && (maxReadLength == 1))
    {
        *(uint16_t *) buffer = app->GetProductId();
    }
    else if ((attributeId == ZCL_APPLICATION_STATUS_ATTRIBUTE_ID) && (maxReadLength == 1))
    {
        *buffer = (uint8_t) app->GetApplicationStatus();
    }
    else if ((attributeId == ZCL_CLUSTER_REVISION_SERVER_ATTRIBUTE_ID) && (maxReadLength == 2))
    {
        *buffer = (uint16_t) ZCL_APPLICATION_BASIC_CLUSTER_REVISION;
    }
    else
    {
        return EMBER_ZCL_STATUS_FAILURE;
    }

    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus HandleWriteApplicationBasicAttribute(ContentApp * app, chip::AttributeId attributeId, uint8_t * buffer)
{
    ChipLogProgress(DeviceLayer, "HandleWriteApplicationBasicAttribute: attrId=%d", attributeId);

    if (attributeId == ZCL_APPLICATION_STATUS_ATTRIBUTE_ID)
    {
        if (*buffer)
        {
            app->SetApplicationStatus(EMBER_ZCL_APPLICATION_BASIC_STATUS_ACTIVE_VISIBLE_FOCUS);
        }
        else
        {
            app->SetApplicationStatus(EMBER_ZCL_APPLICATION_BASIC_STATUS_ACTIVE_VISIBLE_NOT_FOCUS);
        }
    }
    else
    {
        return EMBER_ZCL_STATUS_FAILURE;
    }

    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus emberAfExternalAttributeReadCallback(EndpointId endpoint, ClusterId clusterId,
                                                   EmberAfAttributeMetadata * attributeMetadata, uint16_t manufacturerCode,
                                                   uint8_t * buffer, uint16_t maxReadLength, int32_t index)
{
    uint16_t endpointIndex = emberAfGetDynamicIndexFromEndpoint(endpoint);

    ChipLogProgress(DeviceLayer, "emberAfExternalAttributeReadCallback endpoint %d ", endpointIndex);

    EmberAfStatus ret = EMBER_ZCL_STATUS_FAILURE;

    if ((endpointIndex < DYNAMIC_ENDPOINT_COUNT) && (gContentApps[endpointIndex] != NULL))
    {
        ContentApp * app = gContentApps[endpointIndex];

        ChipLogProgress(DeviceLayer, "Read Attribute for device %s cluster %d attribute=%d)", app->GetApplicationName(), clusterId,
                        attributeMetadata->attributeId);

        if (clusterId == ZCL_APPLICATION_BASIC_CLUSTER_ID)
        {
            ret = HandleReadApplicationBasicAttribute(app, attributeMetadata->attributeId, buffer, maxReadLength);
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
        ContentApp * app = gContentApps[endpointIndex];

        if (clusterId == ZCL_APPLICATION_BASIC_CLUSTER_ID)
        {
            ret = HandleWriteApplicationBasicAttribute(app, attributeMetadata->attributeId, buffer);
        }

        // if ((dev->IsReachable()) && (clusterId == ZCL_APPLICATION_BASIC_CLUSTER_ID))
        // {
        //     ret = HandleWriteApplicationBasicAttribute(static_cast<DeviceOnOff *>(dev), attributeMetadata->attributeId, buffer);
        // }
    }

    return ret;
}

// bool emberAfAccountLoginClusterGetSetupPINCallback(
//     chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
//     const chip::app::Clusters::AccountLogin::Commands::GetSetupPIN::DecodableType & commandData);

// END DYNAMIC ENDPOINTS
// =================================================================================

namespace chip {
namespace AppPlatform {

// BEGIN DYNAMIC ENDPOINTS
// =================================================================================

static const int kNameSize = 32;

// Current ZCL implementation of Struct uses a max-size array of 254 bytes
static const int kDescriptorAttributeArraySize = 254;
static const int kFixedLabelAttributeArraySize = 254;

// Device types for dynamic endpoints: TODO Need a generated file from ZAP to define these!
// (taken from chip-devices.xml)
#define DEVICE_TYPE_CONTENT_APP 0x0024

// ---------------------------------------------------------------------------
//
// CONTENT APP ENDPOINT: contains the following clusters:
//   - Descriptor
//   - Application Basic

// Declare Descriptor cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(descriptorAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(ZCL_DEVICE_LIST_ATTRIBUTE_ID, ARRAY, kDescriptorAttributeArraySize, 0),     /* device list */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_SERVER_LIST_ATTRIBUTE_ID, ARRAY, kDescriptorAttributeArraySize, 0), /* server list */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_CLIENT_LIST_ATTRIBUTE_ID, ARRAY, kDescriptorAttributeArraySize, 0), /* client list */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_PARTS_LIST_ATTRIBUTE_ID, ARRAY, kDescriptorAttributeArraySize, 0),  /* parts list */
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Application Basic information cluster attributes
// TODO: add missing attributes once schema is updated
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(applicationBasicAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(ZCL_APPLICATION_VENDOR_NAME_ATTRIBUTE_ID, CHAR_STRING, kNameSize, 0), /* VendorName */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_APPLICATION_VENDOR_ID_ATTRIBUTE_ID, INT16U, 1, 0),            /* VendorID */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_APPLICATION_NAME_ATTRIBUTE_ID, CHAR_STRING, kNameSize, 0),    /* ApplicationName */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_APPLICATION_PRODUCT_ID_ATTRIBUTE_ID, INT16U, 1, 0),           /* ProductID */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_APPLICATION_STATUS_ATTRIBUTE_ID, INT8U, 1, 0),                /* ApplicationStatus */
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Keypad Input cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(keypadInputAttrs)
DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Application Launcher cluster attributes
// TODO: add missing attributes once schema is updated
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(applicationLauncherAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(ZCL_APPLICATION_LAUNCHER_LIST_ATTRIBUTE_ID, ARRAY, kDescriptorAttributeArraySize, 0), /* catalog list */
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Account Login cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(accountLoginAttrs)
DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Content Launcher cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(contentLauncherAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(ZCL_CONTENT_LAUNCHER_ACCEPTS_HEADER_ATTRIBUTE_ID, ARRAY, kDescriptorAttributeArraySize,
                          0), /* accept header list */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_CONTENT_LAUNCHER_SUPPORTED_STREAMING_TYPES_ATTRIBUTE_ID, BITMAP32, 1,
                              0), /* streaming protocols */
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Media Playback cluster attributes
// TODO: add missing attributes once schema is updated
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(mediaPlaybackAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(ZCL_MEDIA_PLAYBACK_STATE_ATTRIBUTE_ID, ENUM8, 1, 0),                          /* current state */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_MEDIA_PLAYBACK_START_TIME_ATTRIBUTE_ID, EPOCH_US, 1, 0),              /* start time */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_MEDIA_PLAYBACK_DURATION_ATTRIBUTE_ID, INT64U, 1, 0),                  /* duration */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_MEDIA_PLAYBACK_PLAYBACK_SPEED_ATTRIBUTE_ID, SINGLE, 1, 0),            /* playback speed */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_MEDIA_PLAYBACK_PLAYBACK_SEEK_RANGE_END_ATTRIBUTE_ID, INT64U, 1, 0),   /* seek range end */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_MEDIA_PLAYBACK_PLAYBACK_SEEK_RANGE_START_ATTRIBUTE_ID, INT64U, 1, 0), /* seek range start */
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Target Navigator cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(targetNavigatorAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(ZCL_TARGET_NAVIGATOR_LIST_ATTRIBUTE_ID, ARRAY, kDescriptorAttributeArraySize, 0), /* target list */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_TARGET_NAVIGATOR_CURRENT_TARGET_ATTRIBUTE_ID, INT8U, 1, 0),               /* current target */
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Channel cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(channelAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(ZCL_TV_CHANNEL_LIST_ATTRIBUTE_ID, ARRAY, kDescriptorAttributeArraySize, 0), /* channel list */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_TV_CHANNEL_LINEUP_ATTRIBUTE_ID, STRUCT, 1, 0),                      /* lineup */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_TV_CHANNEL_CURRENT_CHANNEL_ATTRIBUTE_ID, STRUCT, 1, 0),             /* current channel */
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Cluster List for Content App endpoint
DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(contentAppClusters)
DECLARE_DYNAMIC_CLUSTER(ZCL_DESCRIPTOR_CLUSTER_ID, descriptorAttrs),
    DECLARE_DYNAMIC_CLUSTER(ZCL_APPLICATION_BASIC_CLUSTER_ID, applicationBasicAttrs),
    DECLARE_DYNAMIC_CLUSTER(ZCL_APPLICATION_BASIC_CLUSTER_ID, keypadInputAttrs),
    DECLARE_DYNAMIC_CLUSTER(ZCL_APPLICATION_BASIC_CLUSTER_ID, applicationLauncherAttrs),
    DECLARE_DYNAMIC_CLUSTER(ZCL_APPLICATION_BASIC_CLUSTER_ID, accountLoginAttrs),
    DECLARE_DYNAMIC_CLUSTER(ZCL_APPLICATION_BASIC_CLUSTER_ID, contentLauncherAttrs),
    DECLARE_DYNAMIC_CLUSTER(ZCL_APPLICATION_BASIC_CLUSTER_ID, mediaPlaybackAttrs),
    DECLARE_DYNAMIC_CLUSTER(ZCL_APPLICATION_BASIC_CLUSTER_ID, targetNavigatorAttrs),
    DECLARE_DYNAMIC_CLUSTER(ZCL_APPLICATION_BASIC_CLUSTER_ID, channelAttrs) DECLARE_DYNAMIC_CLUSTER_LIST_END;

// Declare Content App endpoint
DECLARE_DYNAMIC_ENDPOINT(contentAppEndpoint, contentAppClusters);

ContentApp::ContentApp(const char * szVendorName, uint16_t vendorId, const char * szApplicationName, uint16_t productId,
                       const char * szApplicationVersion)
{
    SetApplicationName(szApplicationName);
    SetVendorName(szApplicationName);
    SetVendorId(vendorId);
    SetProductId(productId);
    SetApplicationVersion(szApplicationVersion);

    mEndpointId = 0;
}

void ContentApp::SetApplicationName(const char * szApplicationName)
{
    ChipLogProgress(DeviceLayer, "ContentApp[%s]: Application Name=\"%s\"", szApplicationName, szApplicationName);

    strncpy(mApplicationName, szApplicationName, sizeof(mApplicationName));
}

void ContentApp::SetVendorName(const char * szVendorName)
{
    ChipLogProgress(DeviceLayer, "ContentApp[%s]: Vendor Name=\"%s\"", mApplicationName, szVendorName);

    strncpy(mVendorName, szVendorName, sizeof(mVendorName));
}

void ContentApp::SetApplicationVersion(const char * szApplicationVersion)
{
    ChipLogProgress(DeviceLayer, "ContentApp[%s]: Application Version=\"%s\"", mApplicationName, szApplicationVersion);

    strncpy(mApplicationVersion, szApplicationVersion, sizeof(mApplicationVersion));
}

bool ContentApp::AccountLogin_Login(const char * tempAccountId, uint32_t setupPin)
{
    ChipLogProgress(DeviceLayer, "ContentApp[%s]: AccountLogin_Login TempAccountId=\"%s\" SetupPIN=\"%d\"", mApplicationName,
                    tempAccountId, setupPin);
    return (setupPin == mAccountLogin_SetupPIN);
}

uint32_t ContentApp::AccountLogin_GetSetupPIN(const char * tempAccountId)
{
    ChipLogProgress(DeviceLayer, "ContentApp[%s]: AccountLogin_GetSetupPIN TempAccountId=\"%s\" returning setuppin=%d",
                    mApplicationName, tempAccountId, mAccountLogin_SetupPIN);
    return mAccountLogin_SetupPIN;
}

ContentApp App1("Vendor1", 1, "App1", 11, "Version1");
ContentApp App2("Vendor2", 2, "App2", 22, "Version2");
ContentApp App3("Vendor3", 9050, "App3", 22, "Version3");

void SetupAppPlatform()
{
    // initialize test pin codes
    App2.AccountLogin_SetSetupPIN(34567890);
    App3.AccountLogin_SetSetupPIN(20202021);

    // Clear out the device database
    memset(gContentApps, 0, sizeof(gContentApps));

    // Set starting endpoint id where dynamic endpoints will be assigned, which
    // will be the next consecutive endpoint id after the last fixed endpoint.
    gFirstDynamicEndpointId = static_cast<chip::EndpointId>(
        static_cast<int>(emberAfEndpointFromIndex(static_cast<uint16_t>(emberAfFixedEndpointCount() - 1))) + 1);
    gCurrentEndpointId = gFirstDynamicEndpointId;

    {
        for (int i = 0; i < emberAfFixedEndpointCount(); i++)
        {
            ChipLogProgress(DeviceLayer, "endpoint index=%d, id=%d", i,
                            static_cast<chip::EndpointId>(static_cast<int>(emberAfEndpointFromIndex(static_cast<uint16_t>(i)))));
        }
    }

    if (gCurrentEndpointId < emberAfFixedEndpointCount())
    {
        gCurrentEndpointId = emberAfFixedEndpointCount();
    }

    ChipLogProgress(DeviceLayer, "emberAfFixedEndpointCount()=%d gCurrentEndpointId=%d", emberAfFixedEndpointCount(),
                    gCurrentEndpointId);

    // Disable last fixed endpoint, which is used as a placeholder for all of the
    // supported clusters so that ZAP will generated the requisite code.
    // emberAfEndpointEnableDisable(emberAfEndpointFromIndex(static_cast<uint16_t>(emberAfFixedEndpointCount() - 1)), false);
}

ContentApp * GetLoadContentAppByVendorId(uint16_t vendorId)
{
    if (App1.GetVendorId() == vendorId)
    {
        AddContentApp(&App1, &contentAppEndpoint, DEVICE_TYPE_CONTENT_APP);
        return &App1;
    }
    if (App2.GetVendorId() == vendorId)
    {
        AddContentApp(&App2, &contentAppEndpoint, DEVICE_TYPE_CONTENT_APP);
        return &App2;
    }
    if (App3.GetVendorId() == vendorId)
    {
        AddContentApp(&App3, &contentAppEndpoint, DEVICE_TYPE_CONTENT_APP);
        return &App3;
    }
    ChipLogProgress(DeviceLayer, "GetLoadContentAppByVendorId() - vendor %d not found ", vendorId);

    return nullptr;
}

ContentApp * GetContentAppByEndpointId(chip::EndpointId id)
{
    if (App1.GetEndpointId() == id)
    {
        return &App1;
    }
    if (App2.GetEndpointId() == id)
    {
        return &App2;
    }
    if (App3.GetEndpointId() == id)
    {
        return &App3;
    }
    ChipLogProgress(DeviceLayer, "GetContentAppByEndpointId() - endpoint %d not found ", id);

    return nullptr;
}

} // namespace AppPlatform
} // namespace chip
