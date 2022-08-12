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

#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include "DeviceCallbacks.h"

static const char * TAG = "bridge-devicecallbacks";

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::app::Clusters;
using namespace ::chip::app::Clusters::Actions::Attributes;
using namespace ::chip::Inet;
using namespace ::chip::System;

void AppDeviceCallbacks::PostAttributeChangeCallback(EndpointId endpointId, ClusterId clusterId, AttributeId attributeId,
                                                     uint8_t type, uint16_t size, uint8_t * value)
{
    ESP_LOGI(TAG, "PostAttributeChangeCallback - Cluster ID: '0x%04x', EndPoint ID: '0x%02x', Attribute ID: '0x%04x'", clusterId,
             endpointId, attributeId);
    ESP_LOGI(TAG, "Current free heap: %d\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
}

namespace {

class ActionsAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the Actions cluster on all endpoints.
    ActionsAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), Actions::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    static constexpr uint16_t ClusterRevision = 1;

    CHIP_ERROR ReadActionListAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadEndpointListAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadSetupUrlAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadClusterRevision(EndpointId endpoint, AttributeValueEncoder & aEncoder);
};

constexpr uint16_t ActionsAttrAccess::ClusterRevision;

CHIP_ERROR ActionsAttrAccess::ReadActionListAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    // Just return an empty list
    return aEncoder.EncodeEmptyList();
}

CHIP_ERROR ActionsAttrAccess::ReadEndpointListAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    // Just return an empty list
    return aEncoder.EncodeEmptyList();
}

CHIP_ERROR ActionsAttrAccess::ReadSetupUrlAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    const char SetupUrl[] = "https://example.com";
    return aEncoder.Encode(chip::CharSpan::fromCharString(SetupUrl));
}

CHIP_ERROR ActionsAttrAccess::ReadClusterRevision(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(ClusterRevision);
}

ActionsAttrAccess gAttrAccess;

CHIP_ERROR ActionsAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == Actions::Id);

    switch (aPath.mAttributeId)
    {
    case ActionList::Id:
        return ReadActionListAttribute(aPath.mEndpointId, aEncoder);
    case EndpointLists::Id:
        return ReadEndpointListAttribute(aPath.mEndpointId, aEncoder);
    case SetupURL::Id:
        return ReadSetupUrlAttribute(aPath.mEndpointId, aEncoder);
    case ClusterRevision::Id:
        return ReadClusterRevision(aPath.mEndpointId, aEncoder);
    default:
        break;
    }
    return CHIP_NO_ERROR;
}
} // anonymous namespace

void MatterActionsPluginServerInitCallback(void)
{
    registerAttributeAccessOverride(&gAttrAccess);
}
