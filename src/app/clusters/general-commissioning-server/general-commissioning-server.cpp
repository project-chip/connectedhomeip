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
 * @brief Routines for the Content Launch plugin, the
 *server implementation of the Content Launch cluster.
 *******************************************************************************
 ******************************************************************************/

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandler.h>
#include <app/CommandHandlerInterface.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteCommandPath.h>
#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/ConfigurationManager.h>
#include <platform/DeviceControlServer.h>
#include <tracing/macros.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::GeneralCommissioning;
using namespace chip::app::Clusters::GeneralCommissioning::Attributes;
using namespace chip::DeviceLayer;
using Transport::SecureSession;
using Transport::Session;

#define CheckSuccess(expr, code)                                                                                                   \
    do                                                                                                                             \
    {                                                                                                                              \
        if (!::chip::ChipError::IsSuccess(expr))                                                                                   \
        {                                                                                                                          \
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::code, #expr);                     \
            return;                                                                                                                \
        }                                                                                                                          \
    } while (false)

namespace {

class GeneralCommissioningGlobalInstance : public AttributeAccessInterface, public CommandHandlerInterface
{
public:
    // Register for the GeneralCommissioning cluster on all endpoints.
    GeneralCommissioningGlobalInstance() :
        AttributeAccessInterface(Optional<EndpointId>::Missing(), GeneralCommissioning::Id),
        CommandHandlerInterface(Optional<EndpointId>::Missing(), GeneralCommissioning::Id)
    {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadIfSupported(CHIP_ERROR (ConfigurationManager::*getter)(uint8_t &), AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadBasicCommissioningInfo(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadSupportsConcurrentConnection(AttributeValueEncoder & aEncoder);

    void InvokeCommand(HandlerContext & ctx) override;

    void HandleArmFailSafe(HandlerContext & ctx, const Commands::ArmFailSafe::DecodableType & commandData);
    void HandleCommissioningComplete(HandlerContext & ctx, const Commands::CommissioningComplete::DecodableType & commandData);
    void HandleSetRegulatoryConfig(HandlerContext & ctx, const Commands::SetRegulatoryConfig::DecodableType & commandData);
};

GeneralCommissioningGlobalInstance gGeneralCommissioningInstance;

CHIP_ERROR GeneralCommissioningGlobalInstance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    if (aPath.mClusterId != GeneralCommissioning::Id)
    {
        // We shouldn't have been called at all.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    switch (aPath.mAttributeId)
    {
    case RegulatoryConfig::Id: {
        return ReadIfSupported(&ConfigurationManager::GetRegulatoryLocation, aEncoder);
    }
    case LocationCapability::Id: {
        return ReadIfSupported(&ConfigurationManager::GetLocationCapability, aEncoder);
    }
    case BasicCommissioningInfo::Id: {
        return ReadBasicCommissioningInfo(aEncoder);
    }
    case SupportsConcurrentConnection::Id: {
        return ReadSupportsConcurrentConnection(aEncoder);
    }
    default: {
        break;
    }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR GeneralCommissioningGlobalInstance::ReadIfSupported(CHIP_ERROR (ConfigurationManager::*getter)(uint8_t &),
                                                               AttributeValueEncoder & aEncoder)
{
    uint8_t data;
    CHIP_ERROR err = (DeviceLayer::ConfigurationMgr().*getter)(data);
    if (err == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
    {
        data = 0;
    }
    else if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    return aEncoder.Encode(data);
}

CHIP_ERROR GeneralCommissioningGlobalInstance::ReadBasicCommissioningInfo(AttributeValueEncoder & aEncoder)
{
    BasicCommissioningInfo::TypeInfo::Type basicCommissioningInfo;

    // TODO: The commissioner might use the critical parameters in BasicCommissioningInfo to initialize
    // the CommissioningParameters at the beginning of commissioning flow.
    basicCommissioningInfo.failSafeExpiryLengthSeconds  = CHIP_DEVICE_CONFIG_FAILSAFE_EXPIRY_LENGTH_SEC;
    basicCommissioningInfo.maxCumulativeFailsafeSeconds = CHIP_DEVICE_CONFIG_MAX_CUMULATIVE_FAILSAFE_SEC;
    static_assert(CHIP_DEVICE_CONFIG_MAX_CUMULATIVE_FAILSAFE_SEC >= CHIP_DEVICE_CONFIG_FAILSAFE_EXPIRY_LENGTH_SEC,
                  "Max cumulative failsafe seconds must be larger than failsafe expiry length seconds");

    return aEncoder.Encode(basicCommissioningInfo);
}

CHIP_ERROR GeneralCommissioningGlobalInstance::ReadSupportsConcurrentConnection(AttributeValueEncoder & aEncoder)
{
    SupportsConcurrentConnection::TypeInfo::Type supportsConcurrentConnection;

    // TODO: The commissioner might use the critical parameters in BasicCommissioningInfo to initialize
    // the CommissioningParameters at the beginning of commissioning flow.
    supportsConcurrentConnection = (CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION) != 0;

    return aEncoder.Encode(supportsConcurrentConnection);
}

void GeneralCommissioningGlobalInstance::InvokeCommand(HandlerContext & handlerContext)
{
    ChipLogDetail(Zcl, "GeneralCommissioningGlobalInstance: InvokeCommand");
    switch (handlerContext.mRequestPath.mCommandId)
    {
    case Commands::ArmFailSafe::Id:
        ChipLogDetail(Zcl, "GeneralCommissioningGlobalInstance: Entering ArmFailSafe");

        CommandHandlerInterface::HandleCommand<Commands::ArmFailSafe::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleArmFailSafe(ctx, commandData); });
        break;

    case Commands::CommissioningComplete::Id:
        ChipLogDetail(Zcl, "GeneralCommissioningGlobalInstance: Entering CommissioningComplete");

        CommandHandlerInterface::HandleCommand<Commands::CommissioningComplete::DecodableType>(
            handlerContext,
            [this](HandlerContext & ctx, const auto & commandData) { HandleCommissioningComplete(ctx, commandData); });
        break;

    case Commands::SetRegulatoryConfig::Id:
        ChipLogDetail(Zcl, "GeneralCommissioningGlobalInstance: Entering SetRegulatoryConfig");

        CommandHandlerInterface::HandleCommand<Commands::SetRegulatoryConfig::DecodableType>(
            handlerContext,
            [this](HandlerContext & ctx, const auto & commandData) { HandleSetRegulatoryConfig(ctx, commandData); });
        break;
    }
}

void GeneralCommissioningGlobalInstance::HandleArmFailSafe(HandlerContext & ctx,
                                                           const Commands::ArmFailSafe::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("ArmFailSafe", "GeneralCommissioning");
    auto & failSafeContext = Server::GetInstance().GetFailSafeContext();
    Commands::ArmFailSafeResponse::Type response;

    ChipLogProgress(FailSafe, "GeneralCommissioning: Received ArmFailSafe (%us)",
                    static_cast<unsigned>(commandData.expiryLengthSeconds));

    /*
     * If the fail-safe timer is not fully disarmed, don't allow arming a new fail-safe.
     * If the fail-safe timer was not currently armed, then the fail-safe timer SHALL be armed.
     * If the fail-safe timer was currently armed, and current accessing fabric matches the fail-safe
     * context’s Fabric Index, then the fail-safe timer SHALL be re-armed.
     */
    FabricIndex accessingFabricIndex = ctx.mCommandHandler.GetAccessingFabricIndex();

    // We do not allow CASE connections to arm the failsafe for the first time while the commissioning window is open in order
    // to allow commissioners the opportunity to obtain this failsafe for the purpose of commissioning
    if (!failSafeContext.IsFailSafeBusy() &&
        (!failSafeContext.IsFailSafeArmed() || failSafeContext.MatchesFabricIndex(accessingFabricIndex)))
    {
        // We do not allow CASE connections to arm the failsafe for the first time while the commissioning window is open in order
        // to allow commissioners the opportunity to obtain this failsafe for the purpose of commissioning
        if (!failSafeContext.IsFailSafeArmed() &&
            Server::GetInstance().GetCommissioningWindowManager().IsCommissioningWindowOpen() &&
            ctx.mCommandHandler.GetSubjectDescriptor().authMode == Access::AuthMode::kCase)
        {
            ChipLogError(FailSafe, "FailSafe arming denied: commissioning window is open.");
            response.errorCode = CommissioningErrorEnum::kBusyWithOtherAdmin;
        }
        else if (commandData.expiryLengthSeconds == 0)
        {
            // Force the timer to expire immediately.
            failSafeContext.ForceFailSafeTimerExpiry();
            // Don't set the breadcrumb, since expiring the failsafe should
            // reset it anyway.
            response.errorCode = CommissioningErrorEnum::kOk;
        }
        else
        {
            CheckSuccess(
                failSafeContext.ArmFailSafe(accessingFabricIndex, System::Clock::Seconds16(commandData.expiryLengthSeconds)),
                Failure);
            Breadcrumb::Set(ctx.mRequestPath.mEndpointId, commandData.breadcrumb);
            response.errorCode = CommissioningErrorEnum::kOk;
        }
    }
    else
    {
        ChipLogError(FailSafe, "FailSafe arming denied: busy with another admin or already armed.");
        response.errorCode = CommissioningErrorEnum::kBusyWithOtherAdmin;
    }

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

void GeneralCommissioningGlobalInstance::HandleCommissioningComplete(
    HandlerContext & ctx, const Commands::CommissioningComplete::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("CommissioningComplete", "GeneralCommissioning");

    DeviceControlServer * devCtrl = &DeviceLayer::DeviceControlServer::DeviceControlSvr();
    auto & failSafe               = Server::GetInstance().GetFailSafeContext();
    auto & fabricTable            = Server::GetInstance().GetFabricTable();

    ChipLogProgress(FailSafe, "GeneralCommissioning: Received CommissioningComplete");

    Commands::CommissioningCompleteResponse::Type response;
    if (!failSafe.IsFailSafeArmed())
    {
        response.errorCode = CommissioningErrorEnum::kNoFailSafe;
    }
    else
    {
        SessionHandle handle = ctx.mCommandHandler.GetExchangeContext()->GetSessionHandle();
        // If not a CASE session, or the fabric does not match the fail-safe,
        // error out.
        if (handle->GetSessionType() != Session::SessionType::kSecure ||
            handle->AsSecureSession()->GetSecureSessionType() != SecureSession::Type::kCASE ||
            !failSafe.MatchesFabricIndex(ctx.mCommandHandler.GetAccessingFabricIndex()))
        {
            response.errorCode = CommissioningErrorEnum::kInvalidAuthentication;
            ChipLogError(FailSafe, "GeneralCommissioning: Got commissioning complete in invalid security context");
        }
        else
        {
            if (failSafe.NocCommandHasBeenInvoked())
            {
                CHIP_ERROR err = fabricTable.CommitPendingFabricData();
                if (err != CHIP_NO_ERROR)
                {
                    // No need to revert on error: CommitPendingFabricData always reverts if not fully successful.
                    ChipLogError(FailSafe, "GeneralCommissioning: Failed to commit pending fabric data: %" CHIP_ERROR_FORMAT,
                                 err.Format());
                }
                else
                {
                    ChipLogProgress(FailSafe, "GeneralCommissioning: Successfully commited pending fabric data");
                }
                CheckSuccess(err, Failure);
            }

            /*
             * Pass fabric of commissioner to DeviceControlSvr.
             * This allows device to send messages back to commissioner.
             * Once bindings are implemented, this may no longer be needed.
             */
            failSafe.DisarmFailSafe();
            CheckSuccess(
                devCtrl->PostCommissioningCompleteEvent(handle->AsSecureSession()->GetPeerNodeId(), handle->GetFabricIndex()),
                Failure);

            Breadcrumb::Set(ctx.mRequestPath.mEndpointId, 0);
            response.errorCode = CommissioningErrorEnum::kOk;
        }
    }

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

void GeneralCommissioningGlobalInstance::HandleSetRegulatoryConfig(HandlerContext & ctx,
                                                                   const Commands::SetRegulatoryConfig::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("SetRegulatoryConfig", "GeneralCommissioning");
    DeviceControlServer * server = &DeviceLayer::DeviceControlServer::DeviceControlSvr();
    Commands::SetRegulatoryConfigResponse::Type response;

    auto & countryCode = commandData.countryCode;
    bool isValidLength = countryCode.size() == DeviceLayer::ConfigurationManager::kMaxLocationLength;
    if (!isValidLength)
    {
        ChipLogError(Zcl, "Invalid country code: '%.*s'", static_cast<int>(countryCode.size()), countryCode.data());
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::ConstraintError);
        return;
    }

    if (commandData.newRegulatoryConfig > RegulatoryLocationTypeEnum::kIndoorOutdoor)
    {
        response.errorCode = CommissioningErrorEnum::kValueOutsideRange;
    }
    else
    {
        uint8_t locationCapability;
        uint8_t location = to_underlying(commandData.newRegulatoryConfig);

        CheckSuccess(ConfigurationMgr().GetLocationCapability(locationCapability), Failure);

        // If the LocationCapability attribute is not Indoor/Outdoor and the NewRegulatoryConfig value received does not match
        // either the Indoor or Outdoor fixed value in LocationCapability.
        if ((locationCapability != to_underlying(RegulatoryLocationTypeEnum::kIndoorOutdoor)) && (location != locationCapability))
        {
            response.errorCode = CommissioningErrorEnum::kValueOutsideRange;
        }
        else
        {
            CheckSuccess(server->SetRegulatoryConfig(location, countryCode), Failure);
            Breadcrumb::Set(ctx.mRequestPath.mEndpointId, commandData.breadcrumb);
            response.errorCode = CommissioningErrorEnum::kOk;
        }
    }

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

} // anonymous namespace

namespace {
void OnPlatformEventHandler(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    if (event->Type == DeviceLayer::DeviceEventType::kFailSafeTimerExpired)
    {
        // Spec says to reset Breadcrumb attribute to 0.
        Breadcrumb::Set(0, 0);
    }
}

} // anonymous namespace

void MatterGeneralCommissioningPluginServerInitCallback()
{
    Breadcrumb::Set(0, 0);
    AttributeAccessInterfaceRegistry::Instance().Register(&gGeneralCommissioningInstance);
    ReturnOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(&gGeneralCommissioningInstance));
    DeviceLayer::PlatformMgrImpl().AddEventHandler(OnPlatformEventHandler);
}

namespace chip {
namespace app {
namespace Clusters {
namespace GeneralCommissioning {
void SetBreadcrumb(Attributes::Breadcrumb::TypeInfo::Type breadcrumb)
{
    Breadcrumb::Set(0, breadcrumb);
}
} // namespace GeneralCommissioning
} // namespace Clusters
} // namespace app
} // namespace chip
