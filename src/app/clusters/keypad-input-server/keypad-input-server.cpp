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

#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
#include <app/app-platform/ContentAppPlatform.h>
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
#include <app/data-model/Encode.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::KeypadInput;
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
using namespace chip::AppPlatform;
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

// -----------------------------------------------------------------------------
// Delegate Implementation

using chip::app::Clusters::KeypadInput::Delegate;

namespace {

Delegate * gDelegateTable[EMBER_AF_KEYPAD_INPUT_CLUSTER_SERVER_ENDPOINT_COUNT] = { nullptr };

Delegate * GetDelegate(EndpointId endpoint)
{
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ContentApp * app = ContentAppPlatform::GetInstance().GetContentApp(endpoint);
    if (app != nullptr)
    {
        ChipLogError(Zcl, "KeypadInput returning ContentApp delegate for endpoint:%" PRIu16, endpoint);
        return app->GetKeypadInputDelegate();
    }
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ChipLogError(Zcl, "KeypadInput NOT returning ContentApp delegate for endpoint:%" PRIu16, endpoint);

    uint16_t ep = emberAfFindClusterServerEndpointIndex(endpoint, KeypadInput::Id);
    return ((ep == 0xFFFF || ep >= EMBER_AF_KEYPAD_INPUT_CLUSTER_SERVER_ENDPOINT_COUNT) ? nullptr : gDelegateTable[ep]);
}

bool isDelegateNull(Delegate * delegate, EndpointId endpoint)
{
    if (delegate == nullptr)
    {
        ChipLogError(Zcl, "Keypad Input has no delegate set for endpoint:%" PRIu16, endpoint);
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

} // namespace KeypadInput
} // namespace Clusters
} // namespace app
} // namespace chip

// -----------------------------------------------------------------------------
// Matter Framework Callbacks Implementation

bool emberAfKeypadInputClusterSendKeyRequestCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                     const Commands::SendKeyRequest::DecodableType & commandData)
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
        ChipLogError(Zcl, "emberAfKeypadInputClusterSendKeyRequestCallback error: %s", err.AsString());
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }
    return true;
}

void MatterKeypadInputPluginServerInitCallback() {}
