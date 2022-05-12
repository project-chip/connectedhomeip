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
 * @brief Routines for the Keypad Input plugin, the
 *server implementation of the Keypad Input cluster.
 *******************************************************************************
 ******************************************************************************/

#include <app/clusters/keypad-input-server/keypad-input-delegate.h>
#include <app/clusters/keypad-input-server/keypad-input-server.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
#include <app/app-platform/ContentAppPlatform.h>
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
#include <app/data-model/Encode.h>
#include <platform/CHIPDeviceConfig.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::KeypadInput;
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
using namespace chip::AppPlatform;
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

static constexpr size_t kKeypadInputDelegateTableSize =
    EMBER_AF_KEYPAD_INPUT_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

// -----------------------------------------------------------------------------
// Delegate Implementation

using chip::app::Clusters::KeypadInput::Delegate;

namespace {

Delegate * gDelegateTable[kKeypadInputDelegateTableSize] = { nullptr };

Delegate * GetDelegate(EndpointId endpoint)
{
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ContentApp * app = ContentAppPlatform::GetInstance().GetContentApp(endpoint);
    if (app != nullptr)
    {
        ChipLogProgress(Zcl, "KeypadInput returning ContentApp delegate for endpoint:%u", endpoint);
        return app->GetKeypadInputDelegate();
    }
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ChipLogProgress(Zcl, "KeypadInput NOT returning ContentApp delegate for endpoint:%u", endpoint);

    uint16_t ep = emberAfFindClusterServerEndpointIndex(endpoint, KeypadInput::Id);
    return ((ep == 0xFFFF || ep >= EMBER_AF_KEYPAD_INPUT_CLUSTER_SERVER_ENDPOINT_COUNT) ? nullptr : gDelegateTable[ep]);
}

bool isDelegateNull(Delegate * delegate, EndpointId endpoint)
{
    if (delegate == nullptr)
    {
        ChipLogProgress(Zcl, "Keypad Input has no delegate set for endpoint:%u", endpoint);
        return true;
    }
    return false;
}
} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace KeypadInput {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t ep = emberAfFindClusterServerEndpointIndex(endpoint, KeypadInput::Id);
    // if endpoint is found and is not a dynamic endpoint
    if (ep != 0xFFFF && ep < EMBER_AF_KEYPAD_INPUT_CLUSTER_SERVER_ENDPOINT_COUNT)
    {
        gDelegateTable[ep] = delegate;
    }
    else
    {
    }
}

bool HasFeature(chip::EndpointId endpoint, KeypadInputFeature feature)
{
    bool hasFeature     = false;
    uint32_t featureMap = 0;

    EmberAfStatus status = Attributes::FeatureMap::Get(endpoint, &featureMap);
    if (EMBER_ZCL_STATUS_SUCCESS == status)
    {
        hasFeature = (featureMap & chip::to_underlying(feature));
    }

    return hasFeature;
}

} // namespace KeypadInput
} // namespace Clusters
} // namespace app
} // namespace chip

// -----------------------------------------------------------------------------
// Matter Framework Callbacks Implementation

bool emberAfKeypadInputClusterSendKeyCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                              const Commands::SendKey::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    auto & keyCode      = commandData.keyCode;
    app::CommandResponseHelper<Commands::SendKeyResponse::Type> responder(command, commandPath);

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);
    {
        delegate->HandleSendKey(responder, keyCode);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfKeypadInputClusterSendKeyCallback error: %s", err.AsString());
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }
    return true;
}

void MatterKeypadInputPluginServerInitCallback() {}
