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

#include "AppImpl.h"

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/Command.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <app/util/af.h>
#include <cstdio>
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

using namespace chip;
using namespace chip::AppPlatform;

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

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

ContentAppImpl::ContentAppImpl(const char * szVendorName, uint16_t vendorId, const char * szApplicationName, uint16_t productId,
                               const char * szApplicationVersion)
{
    mApplicationBasic.SetApplicationName(szApplicationName);
    mApplicationBasic.SetVendorName(szApplicationName);
    mApplicationBasic.SetVendorId(vendorId);
    mApplicationBasic.SetProductId(productId);
    mApplicationBasic.SetApplicationVersion(szApplicationVersion);
}

void ApplicationBasicImpl::SetApplicationName(const char * szApplicationName)
{
    ChipLogProgress(DeviceLayer, "ApplicationBasic[%s]: Application Name=\"%s\"", szApplicationName, szApplicationName);

    strncpy(mApplicationName, szApplicationName, sizeof(mApplicationName));
}

void ApplicationBasicImpl::SetVendorName(const char * szVendorName)
{
    ChipLogProgress(DeviceLayer, "ApplicationBasic[%s]: Vendor Name=\"%s\"", mApplicationName, szVendorName);

    strncpy(mVendorName, szVendorName, sizeof(mVendorName));
}

void ApplicationBasicImpl::SetApplicationVersion(const char * szApplicationVersion)
{
    ChipLogProgress(DeviceLayer, "ApplicationBasic[%s]: Application Version=\"%s\"", mApplicationName, szApplicationVersion);

    strncpy(mApplicationVersion, szApplicationVersion, sizeof(mApplicationVersion));
}

bool AccountLoginImpl::Login(const char * tempAccountId, uint32_t setupPin)
{
    ChipLogProgress(DeviceLayer, "AccountLogin: Login TempAccountId=\"%s\" SetupPIN=\"%d\"", tempAccountId, setupPin);
    return (setupPin == mSetupPIN);
}

uint32_t AccountLoginImpl::GetSetupPIN(const char * tempAccountId)
{
    ChipLogProgress(DeviceLayer, "AccountLogin: GetSetupPIN TempAccountId=\"%s\" returning setuppin=%d", tempAccountId, mSetupPIN);
    return mSetupPIN;
}

ContentAppFactoryImpl::ContentAppFactoryImpl()
{
    mContentApps[1].GetAccountLogin()->SetSetupPIN(34567890);
    mContentApps[2].GetAccountLogin()->SetSetupPIN(20202021);
}

ContentApp * ContentAppFactoryImpl::LoadContentAppByVendorId(uint16_t vendorId)
{
    for (unsigned int i = 0; i < sizeof(mContentApps); i++)
    {
        ContentAppImpl app = mContentApps[i];
        if (app.GetApplicationBasic()->GetVendorId() == vendorId)
        {
            AppPlatform::GetInstance().AddContentApp(&app, &contentAppEndpoint, DEVICE_TYPE_CONTENT_APP);
            return &mContentApps[i];
        }
    }
    ChipLogProgress(DeviceLayer, "LoadContentAppByVendorId() - vendor %d not found ", vendorId);

    return nullptr;
}

} // namespace AppPlatform
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
