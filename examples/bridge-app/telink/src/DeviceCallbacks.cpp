/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::app::Clusters;
using namespace ::chip::app::Clusters::Actions::Attributes;
using namespace ::chip::Inet;
using namespace ::chip::System;

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
