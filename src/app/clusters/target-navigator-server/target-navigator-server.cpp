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
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
#include <app/app-platform/ContentAppPlatform.h>
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
#include <app/data-model/Encode.h>
#include <app/util/attribute-storage.h>
#include <platform/CHIPDeviceConfig.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::TargetNavigator;
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
using namespace chip::AppPlatform;
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

static constexpr size_t kTargetNavigatorDelegateTableSize =
    EMBER_AF_TARGET_NAVIGATOR_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

// -----------------------------------------------------------------------------
// Delegate Implementation

using chip::app::Clusters::TargetNavigator::Delegate;

namespace {

Delegate * gDelegateTable[kTargetNavigatorDelegateTableSize] = { nullptr };

Delegate * GetDelegate(EndpointId endpoint)
{
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ContentApp * app = ContentAppPlatform::GetInstance().GetContentApp(endpoint);
    if (app != nullptr)
    {
        ChipLogProgress(Zcl, "TargetNavigator returning ContentApp delegate for endpoint:%u", endpoint);
        return app->GetTargetNavigatorDelegate();
    }
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ChipLogProgress(Zcl, "TargetNavigator NOT returning ContentApp delegate for endpoint:%u", endpoint);

    uint16_t ep = emberAfFindClusterServerEndpointIndex(endpoint, TargetNavigator::Id);
    return ((ep == 0xFFFF || ep >= EMBER_AF_TARGET_NAVIGATOR_CLUSTER_SERVER_ENDPOINT_COUNT) ? nullptr : gDelegateTable[ep]);
}

bool isDelegateNull(Delegate * delegate, EndpointId endpoint)
{
    if (delegate == nullptr)
    {
        ChipLogProgress(Zcl, "Target Navigator has no delegate set for endpoint:%u", endpoint);
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
    uint16_t ep = emberAfFindClusterServerEndpointIndex(endpoint, TargetNavigator::Id);
    // if endpoint is found and is not a dynamic endpoint
    if (ep != 0xFFFF && ep < EMBER_AF_TARGET_NAVIGATOR_CLUSTER_SERVER_ENDPOINT_COUNT)
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
    TargetNavigatorAttrAccess() : app::AttributeAccessInterface(Optional<EndpointId>::Missing(), TargetNavigator::Id) {}

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
    case app::Clusters::TargetNavigator::Attributes::TargetList::Id: {
        if (isDelegateNull(delegate, endpoint))
        {
            return aEncoder.EncodeEmptyList();
        }

        return ReadTargetListAttribute(aEncoder, delegate);
    }
    case app::Clusters::TargetNavigator::Attributes::CurrentTarget::Id: {
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
    return delegate->HandleGetTargetList(aEncoder);
}

CHIP_ERROR TargetNavigatorAttrAccess::ReadCurrentTargetAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    uint8_t currentTarget = delegate->HandleGetCurrentTarget();
    return aEncoder.Encode(currentTarget);
}

} // anonymous namespace

// -----------------------------------------------------------------------------
// Matter Framework Callbacks Implementation

bool emberAfTargetNavigatorClusterNavigateTargetCallback(app::CommandHandler * command,
                                                         const app::ConcreteCommandPath & commandPath,
                                                         const Commands::NavigateTarget::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    auto & target       = commandData.target;
    auto & data         = commandData.data;
    app::CommandResponseHelper<Commands::NavigateTargetResponse::Type> responder(command, commandPath);

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
        delegate->HandleNavigateTarget(responder, target, data.HasValue() ? data.Value() : CharSpan());
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfTargetNavigatorClusterNavigateTargetCallback error: %s", err.AsString());
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    return true;
}

void MatterTargetNavigatorPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gTargetNavigatorAttrAccess);
}
