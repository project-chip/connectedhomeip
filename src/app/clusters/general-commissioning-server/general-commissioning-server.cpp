/**
 *
 *    Copyright (c) 2021-2024 Project CHIP Authors
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

#include "general-commissioning-server.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AppConfig.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandler.h>
#include <app/CommandHandlerInterface.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteCommandPath.h>
#include <app/reporting/reporting.h>
#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/ConfigurationManager.h>
#include <platform/DeviceControlServer.h>
#include <tracing/macros.h>
#include <transport/SecureSession.h>

#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
#include <app/server/TermsAndConditionsManager.h>  //nogncheck
#include <app/server/TermsAndConditionsProvider.h> //nogncheck
#endif

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::GeneralCommissioning;
using namespace chip::app::Clusters::GeneralCommissioning::Attributes;
using namespace chip::DeviceLayer;
using chip::app::Clusters::GeneralCommissioning::CommissioningErrorEnum;
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
#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    void HandleSetTCAcknowledgements(HandlerContext & ctx, const Commands::SetTCAcknowledgements::DecodableType & commandData);
#endif
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
#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    case TCAcceptedVersion::Id: {
        TermsAndConditionsProvider * tcProvider = TermsAndConditionsManager::GetInstance();
        Optional<TermsAndConditions> outTermsAndConditions;

        VerifyOrReturnError(nullptr != tcProvider, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
        ReturnErrorOnFailure(tcProvider->GetAcceptance(outTermsAndConditions));

        return aEncoder.Encode(outTermsAndConditions.ValueOr(TermsAndConditions(0, 0)).GetVersion());
    }
    case TCMinRequiredVersion::Id: {
        TermsAndConditionsProvider * tcProvider = TermsAndConditionsManager::GetInstance();
        Optional<TermsAndConditions> outTermsAndConditions;

        VerifyOrReturnError(nullptr != tcProvider, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
        ReturnErrorOnFailure(tcProvider->GetRequirements(outTermsAndConditions));

        return aEncoder.Encode(outTermsAndConditions.ValueOr(TermsAndConditions(0, 0)).GetVersion());
    }
    case TCAcknowledgements::Id: {
        TermsAndConditionsProvider * tcProvider = TermsAndConditionsManager::GetInstance();
        Optional<TermsAndConditions> outTermsAndConditions;

        VerifyOrReturnError(nullptr != tcProvider, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
        ReturnErrorOnFailure(tcProvider->GetAcceptance(outTermsAndConditions));

        return aEncoder.Encode(outTermsAndConditions.ValueOr(TermsAndConditions(0, 0)).GetValue());
    }
    case TCAcknowledgementsRequired::Id: {
        TermsAndConditionsProvider * tcProvider = TermsAndConditionsManager::GetInstance();
        bool acknowledgementsRequired;

        VerifyOrReturnError(nullptr != tcProvider, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
        ReturnErrorOnFailure(tcProvider->GetAcknowledgementsRequired(acknowledgementsRequired));

        return aEncoder.Encode(acknowledgementsRequired);
    }
    case TCUpdateDeadline::Id: {
        TermsAndConditionsProvider * tcProvider = TermsAndConditionsManager::GetInstance();
        Optional<uint32_t> outUpdateAcceptanceDeadline;

        VerifyOrReturnError(nullptr != tcProvider, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
        ReturnErrorOnFailure(tcProvider->GetUpdateAcceptanceDeadline(outUpdateAcceptanceDeadline));

        if (!outUpdateAcceptanceDeadline.HasValue())
        {
            return aEncoder.EncodeNull();
        }

        return aEncoder.Encode(outUpdateAcceptanceDeadline.Value());
    }
#endif // CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    default: {
        break;
    }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR GeneralCommissioningGlobalInstance::ReadIfSupported(CHIP_ERROR (ConfigurationManager::*getter)(uint8_t &),
                                                               AttributeValueEncoder & aEncoder)
{
    uint8_t data   = 0;
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
    BasicCommissioningInfo::TypeInfo::Type info;

    // TODO: The commissioner might use the critical parameters in BasicCommissioningInfo to initialize
    // the CommissioningParameters at the beginning of commissioning flow.
    info.failSafeExpiryLengthSeconds  = CHIP_DEVICE_CONFIG_FAILSAFE_EXPIRY_LENGTH_SEC;
    info.maxCumulativeFailsafeSeconds = CHIP_DEVICE_CONFIG_MAX_CUMULATIVE_FAILSAFE_SEC;
    static_assert(CHIP_DEVICE_CONFIG_MAX_CUMULATIVE_FAILSAFE_SEC >= CHIP_DEVICE_CONFIG_FAILSAFE_EXPIRY_LENGTH_SEC,
                  "Max cumulative failsafe seconds must be larger than failsafe expiry length seconds");

    return aEncoder.Encode(info);
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
    switch (handlerContext.mRequestPath.mCommandId)
    {
    case Commands::ArmFailSafe::Id:
        CommandHandlerInterface::HandleCommand<Commands::ArmFailSafe::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleArmFailSafe(ctx, commandData); });
        break;

    case Commands::CommissioningComplete::Id:
        CommandHandlerInterface::HandleCommand<Commands::CommissioningComplete::DecodableType>(
            handlerContext,
            [this](HandlerContext & ctx, const auto & commandData) { HandleCommissioningComplete(ctx, commandData); });
        break;

    case Commands::SetRegulatoryConfig::Id:
        CommandHandlerInterface::HandleCommand<Commands::SetRegulatoryConfig::DecodableType>(
            handlerContext,
            [this](HandlerContext & ctx, const auto & commandData) { HandleSetRegulatoryConfig(ctx, commandData); });
        break;

#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    case Commands::SetTCAcknowledgements::Id:
        CommandHandlerInterface::HandleCommand<Commands::SetTCAcknowledgements::DecodableType>(
            handlerContext,
            [this](HandlerContext & ctx, const auto & commandData) { HandleSetTCAcknowledgements(ctx, commandData); });
        break;
#endif
    }
}

#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
typedef struct sTermsAndConditionsState
{
    Optional<TermsAndConditions> acceptance;
    bool acknowledgementsRequired;
    Optional<TermsAndConditions> requirements;
    Optional<uint32_t> updateAcceptanceDeadline;
} TermsAndConditionsState;

CHIP_ERROR GetTermsAndConditionsAttributeState(TermsAndConditionsProvider * tcProvider,
                                               TermsAndConditionsState & outTermsAndConditionsState)
{
    TermsAndConditionsState termsAndConditionsState;

    ReturnErrorOnFailure(tcProvider->GetAcceptance(termsAndConditionsState.acceptance));
    ReturnErrorOnFailure(tcProvider->GetAcknowledgementsRequired(termsAndConditionsState.acknowledgementsRequired));
    ReturnErrorOnFailure(tcProvider->GetRequirements(termsAndConditionsState.requirements));
    ReturnErrorOnFailure(tcProvider->GetUpdateAcceptanceDeadline(termsAndConditionsState.updateAcceptanceDeadline));

    outTermsAndConditionsState = termsAndConditionsState;
    return CHIP_NO_ERROR;
}

void NotifyTermsAndConditionsAttributeChangeIfRequired(const TermsAndConditionsState & initialState,
                                                       const TermsAndConditionsState & updatedState)
{
    // Notify on TCAcknowledgementsRequired change
    if (initialState.acknowledgementsRequired != updatedState.acknowledgementsRequired)
    {
        MatterReportingAttributeChangeCallback(kRootEndpointId, GeneralCommissioning::Id, TCAcknowledgementsRequired::Id);
    }

    // Notify on TCAcceptedVersion change
    if ((initialState.acceptance.HasValue() != updatedState.acceptance.HasValue()) ||
        (initialState.acceptance.HasValue() &&
         (initialState.acceptance.Value().GetVersion() != updatedState.acceptance.Value().GetVersion())))
    {
        MatterReportingAttributeChangeCallback(kRootEndpointId, GeneralCommissioning::Id, TCAcceptedVersion::Id);
    }

    // Notify on TCAcknowledgements change
    if ((initialState.acceptance.HasValue() != updatedState.acceptance.HasValue()) ||
        (initialState.acceptance.HasValue() &&
         (initialState.acceptance.Value().GetValue() != updatedState.acceptance.Value().GetValue())))
    {
        MatterReportingAttributeChangeCallback(kRootEndpointId, GeneralCommissioning::Id, TCAcknowledgements::Id);
    }

    // Notify on TCRequirements change
    if ((initialState.requirements.HasValue() != updatedState.requirements.HasValue()) ||
        (initialState.requirements.HasValue() &&
         (initialState.requirements.Value().GetVersion() != updatedState.requirements.Value().GetVersion() ||
          initialState.requirements.Value().GetValue() != updatedState.requirements.Value().GetValue())))
    {
        MatterReportingAttributeChangeCallback(kRootEndpointId, GeneralCommissioning::Id, TCMinRequiredVersion::Id);
    }

    // Notify on TCUpdateDeadline change
    if ((initialState.updateAcceptanceDeadline.HasValue() != updatedState.updateAcceptanceDeadline.HasValue()) ||
        (initialState.updateAcceptanceDeadline.HasValue() &&
         (initialState.updateAcceptanceDeadline.Value() != updatedState.updateAcceptanceDeadline.Value())))
    {
        MatterReportingAttributeChangeCallback(kRootEndpointId, GeneralCommissioning::Id, TCUpdateDeadline::Id);
    }
}
#endif // CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED

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
    CHIP_ERROR err;

    // Fail-safe must be armed
    if (!failSafe.IsFailSafeArmed())
    {
        response.errorCode = CommissioningErrorEnum::kNoFailSafe;
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        return;
    }

#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    TermsAndConditionsProvider * tcProvider = TermsAndConditionsManager::GetInstance();

    // Ensure required terms and conditions have been accepted, then attempt to commit
    if (nullptr != tcProvider)
    {
        Optional<TermsAndConditions> requiredTermsAndConditionsMaybe;
        Optional<TermsAndConditions> acceptedTermsAndConditionsMaybe;

        CheckSuccess(tcProvider->GetRequirements(requiredTermsAndConditionsMaybe), Failure);
        CheckSuccess(tcProvider->GetAcceptance(acceptedTermsAndConditionsMaybe), Failure);

        if (requiredTermsAndConditionsMaybe.HasValue() && !acceptedTermsAndConditionsMaybe.HasValue())
        {
            response.errorCode = CommissioningErrorEnum::kTCAcknowledgementsNotReceived;
            ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
            return;
        }

        if (requiredTermsAndConditionsMaybe.HasValue() && acceptedTermsAndConditionsMaybe.HasValue())
        {
            TermsAndConditions requiredTermsAndConditions = requiredTermsAndConditionsMaybe.Value();
            TermsAndConditions acceptedTermsAndConditions = acceptedTermsAndConditionsMaybe.Value();

            if (!requiredTermsAndConditions.ValidateVersion(acceptedTermsAndConditions))
            {
                response.errorCode = CommissioningErrorEnum::kTCMinVersionNotMet;
                ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
                return;
            }

            if (!requiredTermsAndConditions.ValidateValue(acceptedTermsAndConditions))
            {
                response.errorCode = CommissioningErrorEnum::kRequiredTCNotAccepted;
                ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
                return;
            }
        }

        if (failSafe.UpdateTermsAndConditionsHasBeenInvoked())
        {
            // Commit terms and conditions acceptance on commissioning complete
            err = tcProvider->CommitAcceptance();
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(FailSafe, "GeneralCommissioning: Failed to commit terms and conditions: %" CHIP_ERROR_FORMAT,
                             err.Format());
            }
            else
            {
                ChipLogProgress(FailSafe, "GeneralCommissioning: Successfully committed terms and conditions");
            }
            CheckSuccess(err, Failure);
        }
    }
#endif // CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED

    SessionHandle handle = ctx.mCommandHandler.GetExchangeContext()->GetSessionHandle();

    // Ensure it's a valid CASE session
    if (handle->GetSessionType() != Session::SessionType::kSecure ||
        handle->AsSecureSession()->GetSecureSessionType() != SecureSession::Type::kCASE ||
        !failSafe.MatchesFabricIndex(ctx.mCommandHandler.GetAccessingFabricIndex()))
    {
        response.errorCode = CommissioningErrorEnum::kInvalidAuthentication;
        ChipLogError(FailSafe, "GeneralCommissioning: Got commissioning complete in invalid security context");
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        return;
    }

    // Handle NOC commands
    if (failSafe.NocCommandHasBeenInvoked())
    {
        err = fabricTable.CommitPendingFabricData();
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(FailSafe, "GeneralCommissioning: Failed to commit pending fabric data: %" CHIP_ERROR_FORMAT, err.Format());
            // CommitPendingFabricData reverts on error, no need to revert explicitly
        }
        else
        {
            ChipLogProgress(FailSafe, "GeneralCommissioning: Successfully committed pending fabric data");
        }
        CheckSuccess(err, Failure);
    }

    // Disarm the fail-safe and notify the DeviceControlServer
    failSafe.DisarmFailSafe();
    err = devCtrl->PostCommissioningCompleteEvent(handle->AsSecureSession()->GetPeerNodeId(), handle->GetFabricIndex());
    CheckSuccess(err, Failure);

    Breadcrumb::Set(ctx.mRequestPath.mEndpointId, 0);
    response.errorCode = CommissioningErrorEnum::kOk;
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

void GeneralCommissioningGlobalInstance::HandleSetRegulatoryConfig(HandlerContext & ctx,
                                                                   const Commands::SetRegulatoryConfig::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("SetRegulatoryConfig", "GeneralCommissioning");
    DeviceControlServer * server = &DeviceLayer::DeviceControlServer::DeviceControlSvr();
    Commands::SetRegulatoryConfigResponse::Type response;
    auto & countryCode = commandData.countryCode;

    if (countryCode.size() != ConfigurationManager::kMaxLocationLength)
    {
        ChipLogError(Zcl, "Invalid country code: '%.*s'", static_cast<int>(countryCode.size()), countryCode.data());
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::ConstraintError);
        return;
    }

    if (commandData.newRegulatoryConfig > RegulatoryLocationTypeEnum::kIndoorOutdoor)
    {
        response.errorCode = CommissioningErrorEnum::kValueOutsideRange;
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        return;
    }

    uint8_t locationCapability;
    if (ConfigurationMgr().GetLocationCapability(locationCapability) != CHIP_NO_ERROR)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Failure);
        return;
    }

    uint8_t location = to_underlying(commandData.newRegulatoryConfig);

    // If the LocationCapability attribute is not Indoor/Outdoor and the NewRegulatoryConfig value received does not match
    // either the Indoor or Outdoor fixed value in LocationCapability.
    if ((locationCapability != to_underlying(RegulatoryLocationTypeEnum::kIndoorOutdoor)) && (location != locationCapability))
    {
        response.errorCode = CommissioningErrorEnum::kValueOutsideRange;
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        return;
    }

    CheckSuccess(server->SetRegulatoryConfig(location, countryCode), Failure);
    Breadcrumb::Set(ctx.mRequestPath.mEndpointId, commandData.breadcrumb);
    response.errorCode = CommissioningErrorEnum::kOk;
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
void GeneralCommissioningGlobalInstance::HandleSetTCAcknowledgements(
    HandlerContext & ctx, const Commands::SetTCAcknowledgements::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("SetTCAcknowledgements", "GeneralCommissioning");

    auto & failSafeContext                  = Server::GetInstance().GetFailSafeContext();
    TermsAndConditionsProvider * tcProvider = TermsAndConditionsManager::GetInstance();

    if (nullptr == tcProvider)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Failure);
        return;
    }

    Optional<TermsAndConditions> requiredTermsAndConditionsMaybe;
    Optional<TermsAndConditions> previousAcceptedTermsAndConditionsMaybe;
    CheckSuccess(tcProvider->GetRequirements(requiredTermsAndConditionsMaybe), Failure);
    CheckSuccess(tcProvider->GetAcceptance(previousAcceptedTermsAndConditionsMaybe), Failure);
    TermsAndConditions acceptedTermsAndConditions = TermsAndConditions(commandData.TCUserResponse, commandData.TCVersion);
    Optional<TermsAndConditions> acceptedTermsAndConditionsPresent = Optional<TermsAndConditions>(acceptedTermsAndConditions);

    Commands::SetTCAcknowledgementsResponse::Type response;

    if (requiredTermsAndConditionsMaybe.HasValue())
    {
        TermsAndConditions requiredTermsAndConditions = requiredTermsAndConditionsMaybe.Value();

        if (!requiredTermsAndConditions.ValidateVersion(acceptedTermsAndConditions))
        {
            response.errorCode = CommissioningErrorEnum::kTCMinVersionNotMet;
            ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
            return;
        }

        if (!requiredTermsAndConditions.ValidateValue(acceptedTermsAndConditions))
        {
            response.errorCode = CommissioningErrorEnum::kRequiredTCNotAccepted;
            ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
            return;
        }
    }

    if (previousAcceptedTermsAndConditionsMaybe != acceptedTermsAndConditionsPresent)
    {
        TermsAndConditionsState initialState, updatedState;
        CheckSuccess(GetTermsAndConditionsAttributeState(tcProvider, initialState), Failure);
        CheckSuccess(tcProvider->SetAcceptance(acceptedTermsAndConditionsPresent), Failure);
        CheckSuccess(GetTermsAndConditionsAttributeState(tcProvider, updatedState), Failure);
        NotifyTermsAndConditionsAttributeChangeIfRequired(initialState, updatedState);

        // Commit or defer based on fail-safe state
        if (!failSafeContext.IsFailSafeArmed())
        {
            CheckSuccess(tcProvider->CommitAcceptance(), Failure);
        }
        else
        {
            failSafeContext.SetUpdateTermsAndConditionsHasBeenInvoked();
        }
    }

    response.errorCode = CommissioningErrorEnum::kOk;
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}
#endif // CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED

void OnPlatformEventHandler(const DeviceLayer::ChipDeviceEvent * event, intptr_t)
{
    if (event->Type == DeviceLayer::DeviceEventType::kFailSafeTimerExpired)
    {
        // Spec says to reset Breadcrumb attribute to 0.
        Breadcrumb::Set(0, 0);

#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
        if (event->FailSafeTimerExpired.updateTermsAndConditionsHasBeenInvoked)
        {
            // Clear terms and conditions acceptance on failsafe timer expiration
            TermsAndConditionsProvider * tcProvider = TermsAndConditionsManager::GetInstance();
            TermsAndConditionsState initialState, updatedState;
            VerifyOrReturn(nullptr != tcProvider);
            VerifyOrReturn(CHIP_NO_ERROR == GetTermsAndConditionsAttributeState(tcProvider, initialState));
            VerifyOrReturn(CHIP_NO_ERROR == tcProvider->RevertAcceptance());
            VerifyOrReturn(CHIP_NO_ERROR == GetTermsAndConditionsAttributeState(tcProvider, updatedState));
            NotifyTermsAndConditionsAttributeChangeIfRequired(initialState, updatedState);
        }
#endif // CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    }
}

} // anonymous namespace

class GeneralCommissioningFabricTableDelegate : public chip::FabricTable::Delegate
{
public:
    // Gets called when a fabric is deleted
    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override
    {
#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
        // If the FabricIndex matches the last remaining entry in the Fabrics list, then the device SHALL delete all Matter
        // related data on the node which was created since it was commissioned.
        if (Server::GetInstance().GetFabricTable().FabricCount() == 0)
        {
            ChipLogProgress(Zcl, "general-commissioning-server: Last Fabric index 0x%x was removed",
                            static_cast<unsigned>(fabricIndex));

            TermsAndConditionsProvider * tcProvider = TermsAndConditionsManager::GetInstance();
            TermsAndConditionsState initialState, updatedState;
            VerifyOrReturn(nullptr != tcProvider);
            VerifyOrReturn(CHIP_NO_ERROR == GetTermsAndConditionsAttributeState(tcProvider, initialState));
            VerifyOrReturn(CHIP_NO_ERROR == tcProvider->ResetAcceptance());
            VerifyOrReturn(CHIP_NO_ERROR == GetTermsAndConditionsAttributeState(tcProvider, updatedState));
            NotifyTermsAndConditionsAttributeChangeIfRequired(initialState, updatedState);
        }
#endif // CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    }
};

void MatterGeneralCommissioningPluginServerInitCallback()
{
    Breadcrumb::Set(0, 0);
    AttributeAccessInterfaceRegistry::Instance().Register(&gGeneralCommissioningInstance);
    ReturnOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(&gGeneralCommissioningInstance));
    DeviceLayer::PlatformMgrImpl().AddEventHandler(OnPlatformEventHandler);

    static GeneralCommissioningFabricTableDelegate fabricDelegate;
    Server::GetInstance().GetFabricTable().AddFabricDelegate(&fabricDelegate);
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
