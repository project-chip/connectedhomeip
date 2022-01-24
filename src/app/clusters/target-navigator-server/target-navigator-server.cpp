/**
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
 * @brief Routines for the Target Navigator plugin, the
 *server implementation of the Target Navigator cluster.
 *******************************************************************************
 ******************************************************************************/

#include <app/clusters/target-navigator-server/target-navigator-delegate.h>
#include <app/clusters/target-navigator-server/target-navigator-server.h>

#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/data-model/Encode.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::TargetNavigator;

// -----------------------------------------------------------------------------
// Delegate Implementation

using chip::app::Clusters::TargetNavigator::Delegate;

namespace {

Delegate * gDelegateTable[EMBER_AF_TARGET_NAVIGATOR_CLUSTER_SERVER_ENDPOINT_COUNT] = { nullptr };

Delegate * GetDelegate(EndpointId endpoint)
{
    uint16_t ep = emberAfFindClusterServerEndpointIndex(endpoint, chip::app::Clusters::TargetNavigator::Id);
    return (ep == 0xFFFF ? NULL : gDelegateTable[ep]);
}

bool isDelegateNull(Delegate * delegate, EndpointId endpoint)
{
    if (delegate == nullptr)
    {
        ChipLogError(Zcl, "Target Navigator has no delegate set for endpoint:%" PRIu16, endpoint);
        return true;
    }
    return false;
}
} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace TargetNavigator {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t ep = emberAfFindClusterServerEndpointIndex(endpoint, chip::app::Clusters::TargetNavigator::Id);
    if (ep != 0xFFFF)
    {
        gDelegateTable[ep] = delegate;
    }
    else
    {
    }
}

} // namespace TargetNavigator
} // namespace Clusters
} // namespace app
} // namespace chip

// -----------------------------------------------------------------------------
// Attribute Accessor Implementation

namespace {

class TargetNavigatorAttrAccess : public app::AttributeAccessInterface
{
public:
    TargetNavigatorAttrAccess() :
        app::AttributeAccessInterface(Optional<EndpointId>::Missing(), chip::app::Clusters::TargetNavigator::Id)
    {}

    CHIP_ERROR Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadTargetListAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadCurrentTargetAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
};

TargetNavigatorAttrAccess gTargetNavigatorAttrAccess;

CHIP_ERROR TargetNavigatorAttrAccess::Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder)
{
    EndpointId endpoint = aPath.mEndpointId;
    Delegate * delegate = GetDelegate(endpoint);

    switch (aPath.mAttributeId)
    {
    case app::Clusters::TargetNavigator::Attributes::TargetNavigatorList::Id: {
        if (isDelegateNull(delegate, endpoint))
        {
            return aEncoder.EncodeEmptyList();
        }

        return ReadTargetListAttribute(aEncoder, delegate);
    }
    case app::Clusters::TargetNavigator::Attributes::CurrentNavigatorTarget::Id: {
        if (isDelegateNull(delegate, endpoint))
        {
            return CHIP_NO_ERROR;
        }

        return ReadCurrentTargetAttribute(aEncoder, delegate);
    }
    default: {
        break;
    }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR TargetNavigatorAttrAccess::ReadTargetListAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    std::list<Structs::TargetInfo::Type> targetList = delegate->HandleGetTargetList();
    return aEncoder.EncodeList([targetList](const auto & encoder) -> CHIP_ERROR {
        for (const auto & target : targetList)
        {
            ReturnErrorOnFailure(encoder.Encode(target));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR TargetNavigatorAttrAccess::ReadCurrentTargetAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    uint8_t currentTarget = delegate->HandleGetCurrentTarget();
    return aEncoder.Encode(currentTarget);
}

} // anonymous namespace

// -----------------------------------------------------------------------------
// Matter Framework Callbacks Implementation

bool emberAfTargetNavigatorClusterNavigateTargetRequestCallback(app::CommandHandler * command,
                                                                const app::ConcreteCommandPath & commandPath,
                                                                const Commands::NavigateTargetRequest::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    auto & target       = commandData.target;
    auto & data         = commandData.data;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
        Commands::NavigateTargetResponse::Type response = delegate->HandleNavigateTarget(target, data);
        err                                             = command->AddResponseData(commandPath, response);
        SuccessOrExit(err);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfTargetNavigatorClusterNavigateTargetRequestCallback error: %s", err.AsString());
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    return true;
}

void MatterTargetNavigatorPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gTargetNavigatorAttrAccess);
}
