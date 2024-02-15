/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
 * @brief Routines for the Application Launcher plugin, the
 *server implementation of the Application Launcher cluster.
 *******************************************************************************
 ******************************************************************************/

#include <app/clusters/content-app-observer/content-app-observer-delegate.h>
#include <app/clusters/content-app-observer/content-app-observer.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/EventLogging.h>
#include <app/util/af.h>
#include <app/util/config.h>
#include <platform/CHIPDeviceConfig.h>

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
#include <app/app-platform/ContentAppPlatform.h>
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ContentAppObserver;
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
using namespace chip::AppPlatform;
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
using chip::app::Clusters::ContentAppObserver::Delegate;
using chip::Protocols::InteractionModel::Status;

static constexpr size_t kContentAppObserverDeletageTableSize =
    MATTER_DM_CONTENT_APP_OBSERVER_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;
static_assert(kContentAppObserverDeletageTableSize <= kEmberInvalidEndpointIndex, "ContentAppObserver Delegate table size error");

// -----------------------------------------------------------------------------
// Delegate Implementation

namespace {

Delegate * gDelegateTable[kContentAppObserverDeletageTableSize] = { nullptr };

Delegate * GetDelegate(EndpointId endpoint)
{
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ContentApp * app = ContentAppPlatform::GetInstance().GetContentApp(endpoint);
    if (app != nullptr)
    {
        ChipLogProgress(Zcl, "ContentAppObserver returning ContentApp delegate for endpoint:%u", endpoint);
        return app->GetContentAppObserverDelegate();
    }
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ChipLogProgress(Zcl, "ContentAppObserver NOT returning ContentApp delegate for endpoint:%u", endpoint);

    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, ContentAppObserver::Id,
                                                       MATTER_DM_CONTENT_APP_OBSERVER_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kContentAppObserverDeletageTableSize ? nullptr : gDelegateTable[ep]);
}

bool isDelegateNull(Delegate * delegate, EndpointId endpoint)
{
    if (delegate == nullptr)
    {
        ChipLogProgress(Zcl, "Content App Observer has no delegate set for endpoint:%u", endpoint);
        return true;
    }
    return false;
}
} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace ContentAppObserver {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, ContentAppObserver::Id,
                                                       MATTER_DM_CONTENT_APP_OBSERVER_CLUSTER_SERVER_ENDPOINT_COUNT);
    // if endpoint is found
    if (ep < kContentAppObserverDeletageTableSize)
    {
        gDelegateTable[ep] = delegate;
    }
    else
    {
    }
}

} // namespace ContentAppObserver
} // namespace Clusters
} // namespace app
} // namespace chip

// -----------------------------------------------------------------------------
// Matter Framework Callbacks Implementation

/**
 * @brief Content App Observer Cluster ContentAppMessage Command callback (from client)
 */
bool emberAfContentAppObserverClusterContentAppMessageCallback(
    chip::app::CommandHandler * command, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::ContentAppObserver::Commands::ContentAppMessage::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;

    auto & data         = commandData.data;
    auto & encodingHint = commandData.encodingHint;

    app::CommandResponseHelper<Commands::ContentAppMessageResponse::Type> responder(command, commandPath);

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);
    {
        delegate->HandleContentAppMessage(responder, data, encodingHint);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfContentAppObserverClusterContentAppMessageCallback error: %s", err.AsString());
    }

    // If isDelegateNull, no one will call responder, so HasSentResponse will be false
    if (!responder.HasSentResponse())
    {
        command->AddStatus(commandPath, Status::Failure);
    }

    return true;
}

// -----------------------------------------------------------------------------
// Plugin initialization

void MatterContentAppObserverPluginServerInitCallback() {}
