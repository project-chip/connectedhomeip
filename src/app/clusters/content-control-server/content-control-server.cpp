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

#include <app/clusters/content-control-server/content-control-delegate.h>
#include <app/clusters/content-control-server/content-control-server.h>

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
using namespace chip::app::Clusters::ContentControl;
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
using namespace chip::AppPlatform;
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
using chip::NodeId;
using chip::app::LogEvent;
using chip::app::Clusters::ContentControl::Delegate;
using chip::Protocols::InteractionModel::Status;
using RemainingScreenTimeExpiredEvent = chip::app::Clusters::ContentControl::Events::RemainingScreenTimeExpired::Type;

static constexpr size_t kContentControlDeletageTableSize =
    MATTER_DM_CONTENT_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;
static_assert(kContentControlDeletageTableSize <= kEmberInvalidEndpointIndex, "ContentControl Delegate table size error");

// -----------------------------------------------------------------------------
// Delegate Implementation

namespace {

Delegate * gDelegateTable[kContentControlDeletageTableSize] = { nullptr };

Delegate * GetDelegate(EndpointId endpoint)
{
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ContentApp * app = ContentAppPlatform::GetInstance().GetContentApp(endpoint);
    if (app != nullptr)
    {
        ChipLogProgress(Zcl, "ContentControl returning ContentApp delegate for endpoint:%u", endpoint);
        return app->GetContentControlDelegate();
    }
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ChipLogProgress(Zcl, "ContentControl NOT returning ContentApp delegate for endpoint:%u", endpoint);

    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, ContentControl::Id, MATTER_DM_CONTENT_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kContentControlDeletageTableSize ? nullptr : gDelegateTable[ep]);
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
namespace ContentControl {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, ContentControl::Id, MATTER_DM_CONTENT_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    // if endpoint is found
    if (ep < kContentControlDeletageTableSize)
    {
        gDelegateTable[ep] = delegate;
    }
    else
    {
    }
}

} // namespace ContentControl
} // namespace Clusters
} // namespace app
} // namespace chip

// -----------------------------------------------------------------------------
// Matter Framework Callbacks Implementation

/**
 * @brief Content Control Cluster UpdatePIN Command callback (from client)
 */
bool emberAfContentControlClusterUpdatePINCallback(
    chip::app::CommandHandler * command, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::ContentControl::Commands::UpdatePIN::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    Status status       = Status::Success;

    auto & oldPin = commandData.oldPIN;
    auto & newPin = commandData.newPIN;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);
    {
        delegate->HandleUpdatePIN(oldPin, newPin);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfContentControlClusterUpdatePINCallback error: %s", err.AsString());
        status = Status::Failure;
    }

    command->AddStatus(commandPath, status);
    return true;
}

/**
 * @brief Content Control Cluster ResetPIN Command callback (from client)
 */
bool emberAfContentControlClusterResetPINCallback(
    chip::app::CommandHandler * command, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::ContentControl::Commands::ResetPIN::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;

    app::CommandResponseHelper<Commands::ResetPINResponse::Type> responder(command, commandPath);

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);
    {
        delegate->HandleResetPIN(responder);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfContentControlClusterResetPINCallback error: %s", err.AsString());
    }

    // If isDelegateNull, no one will call responder, so HasSentResponse will be false
    if (!responder.HasSentResponse())
    {
        command->AddStatus(commandPath, Status::Failure);
    }

    return true;
}

/**
 * @brief Content Control Cluster Enable Command callback (from client)
 */
bool emberAfContentControlClusterEnableCallback(
    chip::app::CommandHandler * command, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::ContentControl::Commands::Enable::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    Status status       = Status::Success;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);
    {
        delegate->HandleEnable();
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfContentControlClusterEnableCallback error: %s", err.AsString());
        status = Status::Failure;
    }

    command->AddStatus(commandPath, status);
    return true;
}

/**
 * @brief Content Control Cluster Disable Command callback (from client)
 */
bool emberAfContentControlClusterDisableCallback(
    chip::app::CommandHandler * command, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::ContentControl::Commands::Disable::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    Status status       = Status::Success;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);
    {
        delegate->HandleDisable();
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfContentControlClusterDisableCallback error: %s", err.AsString());
        status = Status::Failure;
    }

    command->AddStatus(commandPath, status);
    return true;
}

/**
 * @brief Content Control Cluster AddBonusTime Command callback (from client)
 */
bool emberAfContentControlClusterAddBonusTimeCallback(
    chip::app::CommandHandler * command, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::ContentControl::Commands::AddBonusTime::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    Status status       = Status::Success;

    auto & pinCode   = commandData.PINCode;
    auto & bonusTime = commandData.bonusTime;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);
    {
        delegate->HandleAddBonusTime(pinCode, bonusTime);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfContentControlClusterAddBonusTimeCallback error: %s", err.AsString());
        status = Status::Failure;
    }

    command->AddStatus(commandPath, status);
    return true;
}

/**
 * @brief Content Control Cluster SetScreenDailyTime Command callback (from client)
 */
bool emberAfContentControlClusterSetScreenDailyTimeCallback(
    chip::app::CommandHandler * command, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::ContentControl::Commands::SetScreenDailyTime::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    Status status       = Status::Success;

    auto & screenTime = commandData.screenTime;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);
    {
        delegate->HandleSetScreenDailyTime(screenTime);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfContentControlClusterSetScreenDailyTimeCallback error: %s", err.AsString());
        status = Status::Failure;
    }

    command->AddStatus(commandPath, status);
    return true;
}

/**
 * @brief Content Control Cluster BlockUnratedContent Command callback (from client)
 */
bool emberAfContentControlClusterBlockUnratedContentCallback(
    chip::app::CommandHandler * command, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::ContentControl::Commands::BlockUnratedContent::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    Status status       = Status::Success;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);
    {
        delegate->HandleBlockUnratedContent();
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfContentControlClusterBlockUnratedContentCallback error: %s", err.AsString());
        status = Status::Failure;
    }

    command->AddStatus(commandPath, status);
    return true;
}

/**
 * @brief Content Control Cluster UnblockUnratedContent Command callback (from client)
 */
bool emberAfContentControlClusterUnblockUnratedContentCallback(
    chip::app::CommandHandler * command, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::ContentControl::Commands::UnblockUnratedContent::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    Status status       = Status::Success;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);
    {
        delegate->HandleUnblockUnratedContent();
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfContentControlClusterUnblockUnratedContentCallback error: %s", err.AsString());
        status = Status::Failure;
    }

    command->AddStatus(commandPath, status);
    return true;
}

/**
 * @brief Content Control Cluster SetOnDemandRatingThreshold Command callback (from client)
 */
bool emberAfContentControlClusterSetOnDemandRatingThresholdCallback(
    chip::app::CommandHandler * command, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::ContentControl::Commands::SetOnDemandRatingThreshold::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    Status status       = Status::Success;

    auto & rating = commandData.rating;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);
    {
        delegate->HandleSetOnDemandRatingThreshold(rating);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfContentControlClusterSetOnDemandRatingThresholdCallback error: %s", err.AsString());
        status = Status::Failure;
    }

    command->AddStatus(commandPath, status);
    return true;
}

/**
 * @brief Content Control Cluster SetScheduledContentRatingThreshold Command callback (from client)
 */
bool emberAfContentControlClusterSetScheduledContentRatingThresholdCallback(
    chip::app::CommandHandler * command, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::ContentControl::Commands::SetScheduledContentRatingThreshold::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    Status status       = Status::Success;

    auto & rating = commandData.rating;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);
    {
        delegate->HandleSetScheduledContentRatingThreshold(rating);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfContentControlClusterSetScheduledContentRatingThresholdCallback error: %s", err.AsString());
        status = Status::Failure;
    }

    command->AddStatus(commandPath, status);
    return true;
}

// -----------------------------------------------------------------------------
// Plugin initialization

void MatterContentControlPluginServerInitCallback() {}
