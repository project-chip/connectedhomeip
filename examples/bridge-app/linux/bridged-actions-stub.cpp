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

#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <vector>

#include "Device.h"
#include "main.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Actions::Attributes;

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
    CHIP_ERROR err = aEncoder.EncodeList([&endpoint](const auto & encoder) -> CHIP_ERROR {
        std::vector<Action *> actionList = GetActionListInfo(endpoint);

        for (auto action : actionList)
        {
            if (action->getIsVisible())
            {
                Actions::Structs::ActionStruct::Type actionStruct = { action->getActionId(),
                                                                      CharSpan::fromCharString(action->getName().c_str()),
                                                                      action->getType(),
                                                                      action->getEndpointListId(),
                                                                      action->getSupportedCommands(),
                                                                      action->getStatus() };
                ReturnErrorOnFailure(encoder.Encode(actionStruct));
            }
        }

        return CHIP_NO_ERROR;
    });
    return err;
}

CHIP_ERROR ActionsAttrAccess::ReadEndpointListAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    std::vector<EndpointListInfo> infoList = GetEndpointListInfo(endpoint);

    CHIP_ERROR err = aEncoder.EncodeList([&infoList](const auto & encoder) -> CHIP_ERROR {
        for (auto info : infoList)
        {
            Actions::Structs::EndpointListStruct::Type endpointListStruct = {
                info.GetEndpointListId(), CharSpan::fromCharString(info.GetName().c_str()), info.GetType(),
                DataModel::List<chip::EndpointId>(info.GetEndpointListData(), info.GetEndpointListSize())
            };
            ReturnErrorOnFailure(encoder.Encode(endpointListStruct));
        }
        return CHIP_NO_ERROR;
    });
    return err;
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
