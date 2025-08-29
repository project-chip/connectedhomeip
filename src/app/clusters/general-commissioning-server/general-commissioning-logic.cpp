#include "general-commissioning-logic.h"

#include <app/server/CommissioningWindowManager.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <platform/DeviceControlServer.h>

#include <app/data-model/Decode.h>
#include <app/data-model/Encode.h>

#include <app/ConcreteCommandPath.h>
#include <lib/core/Optional.h>
#include <transport/SecureSession.h>

#include <lib/core/DataModelTypes.h>
#include <protocols/interaction_model/StatusCode.h>
#include <tracing/macros.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::GeneralCommissioning;
using namespace chip::app::Clusters::GeneralCommissioning::Attributes;
using chip::app::Clusters::GeneralCommissioning::CommissioningErrorEnum;
using chip::Protocols::InteractionModel::ClusterStatusCode;
using chip::Protocols::InteractionModel::Status;
using chip::Transport::SecureSession;
using chip::Transport::Session;

namespace {

#define CheckSuccess(expr, code)                                                                                                   \
    do                                                                                                                             \
    {                                                                                                                              \
        if (!::chip::ChipError::IsSuccess(expr))                                                                                   \
        {                                                                                                                          \
            commandObj->AddStatus(path, Protocols::InteractionModel::Status::code, #expr);                                         \
            return std::nullopt;                                                                                                   \
        }                                                                                                                          \
    } while (false)
} // namespace

namespace chip {
namespace app {
namespace Clusters {

CHIP_ERROR GeneralCommissioningLogic::ReadBasicCommissioningInfo(AttributeValueEncoder & aEncoder)
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

CHIP_ERROR GeneralCommissioningLogic::ReadSupportsConcurrentConnection(AttributeValueEncoder & aEncoder)
{
    SupportsConcurrentConnection::TypeInfo::Type supportsConcurrentConnection;

    // TODO: The commissioner might use the critical parameters in BasicCommissioningInfo to initialize
    // the CommissioningParameters at the beginning of commissioning flow.
    supportsConcurrentConnection = (CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION) != 0;

    return aEncoder.Encode(supportsConcurrentConnection);
}

#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
CHIP_ERROR GeneralCommissioningLogic::ReadTCAcceptedVersion(AttributeValueEncoder & aEncoder)
{
    TermsAndConditionsProvider * tcProvider = TermsAndConditionsManager::GetInstance();
    Optional<TermsAndConditions> outTermsAndConditions;

    VerifyOrReturnError(nullptr != tcProvider, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    ReturnErrorOnFailure(tcProvider->GetAcceptance(outTermsAndConditions));

    return aEncoder.Encode(outTermsAndConditions.ValueOr(TermsAndConditions(0, 0)).GetVersion());
}

CHIP_ERROR GeneralCommissioningLogic::ReadTCMinRequiredVersion(AttributeValueEncoder & aEncoder)
{
    TermsAndConditionsProvider * tcProvider = TermsAndConditionsManager::GetInstance();
    Optional<TermsAndConditions> outTermsAndConditions;

    VerifyOrReturnError(nullptr != tcProvider, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    ReturnErrorOnFailure(tcProvider->GetRequirements(outTermsAndConditions));

    return aEncoder.Encode(outTermsAndConditions.ValueOr(TermsAndConditions(0, 0)).GetVersion());
}

CHIP_ERROR GeneralCommissioningLogic::ReadTCAcknowledgements(AttributeValueEncoder & aEncoder)
{
    TermsAndConditionsProvider * tcProvider = TermsAndConditionsManager::GetInstance();
    Optional<TermsAndConditions> outTermsAndConditions;

    VerifyOrReturnError(nullptr != tcProvider, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    ReturnErrorOnFailure(tcProvider->GetAcceptance(outTermsAndConditions));

    return aEncoder.Encode(outTermsAndConditions.ValueOr(TermsAndConditions(0, 0)).GetValue());
}

CHIP_ERROR GeneralCommissioningLogic::ReadTCAcknowledgementsRequired(AttributeValueEncoder & aEncoder)
{
    TermsAndConditionsProvider * tcProvider = TermsAndConditionsManager::GetInstance();
    bool acknowledgementsRequired;

    VerifyOrReturnError(nullptr != tcProvider, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    ReturnErrorOnFailure(tcProvider->GetAcknowledgementsRequired(acknowledgementsRequired));

    return aEncoder.Encode(acknowledgementsRequired);
}

CHIP_ERROR GeneralCommissioningLogic::ReadTCUpdateDeadLine(AttributeValueEncoder & aEncoder)
{
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

std::optional<DataModel::ActionReturnStatus>
GeneralCommissioningLogic::HandleArmFailSafe(CommandHandler * commandObj, const ConcreteCommandPath & path,
                                             const GeneralCommissioning::Commands::ArmFailSafe::DecodableType & commandData)
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

    if (!failSafeContext.IsFailSafeBusy() &&
        (!failSafeContext.IsFailSafeArmed() || failSafeContext.MatchesFabricIndex(commandObj->GetAccessingFabricIndex())))
    {
        // We do not allow CASE connections to arm the failsafe for the first time while the commissioning window is open in order
        // to allow commissioners the opportunity to obtain this failsafe for the purpose of commissioning
        if (!failSafeContext.IsFailSafeArmed() &&
            Server::GetInstance().GetCommissioningWindowManager().IsCommissioningWindowOpen() &&
            commandObj->GetSubjectDescriptor().authMode == Access::AuthMode::kCase)
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
            CheckSuccess(failSafeContext.ArmFailSafe(commandObj->GetAccessingFabricIndex(),
                                                     System::Clock::Seconds16(commandData.expiryLengthSeconds)),
                         Failure);
            Breadcrumb::Set(path.mEndpointId, commandData.breadcrumb);
            response.errorCode = CommissioningErrorEnum::kOk;
        }
    }
    else
    {
        response.errorCode = CommissioningErrorEnum::kBusyWithOtherAdmin;
    }

    commandObj->AddResponse(path, response);
    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus> GeneralCommissioningLogic::HandleCommissioningComplete(
    CommandHandler * commandObj, const ConcreteCommandPath & path, FabricIndex accessingFabricIndex,
    const GeneralCommissioning::Commands::CommissioningComplete::DecodableType & commandData)
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
        commandObj->AddResponse(path, response);
        return std::nullopt;
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
            commandObj->AddResponse(path, response);
            return std::nullopt;
        }

        if (requiredTermsAndConditionsMaybe.HasValue() && acceptedTermsAndConditionsMaybe.HasValue())
        {
            TermsAndConditions requiredTermsAndConditions = requiredTermsAndConditionsMaybe.Value();
            TermsAndConditions acceptedTermsAndConditions = acceptedTermsAndConditionsMaybe.Value();

            if (!requiredTermsAndConditions.ValidateVersion(acceptedTermsAndConditions))
            {
                response.errorCode = CommissioningErrorEnum::kTCMinVersionNotMet;
                commandObj->AddResponse(path, response);
                return std::nullopt;
            }

            if (!requiredTermsAndConditions.ValidateValue(acceptedTermsAndConditions))
            {
                response.errorCode = CommissioningErrorEnum::kRequiredTCNotAccepted;
                commandObj->AddResponse(path, response);
                return std::nullopt;
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

    SessionHandle handle = commandObj->GetExchangeContext()->GetSessionHandle();

    // Ensure it's a valid CASE session
    if (handle->GetSessionType() != Session::SessionType::kSecure ||
        handle->AsSecureSession()->GetSecureSessionType() != SecureSession::Type::kCASE ||
        !failSafe.MatchesFabricIndex(accessingFabricIndex))
    {
        response.errorCode = CommissioningErrorEnum::kInvalidAuthentication;
        ChipLogError(FailSafe, "GeneralCommissioning: Got commissioning complete in invalid security context");
        commandObj->AddResponse(path, response);
        return std::nullopt;
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

    Breadcrumb::Set(path.mEndpointId, 0);
    response.errorCode = CommissioningErrorEnum::kOk;
    commandObj->AddResponse(path, response);
    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus> GeneralCommissioningLogic::HandleSetRegulatoryConfig(
    CommandHandler * commandObj, const ConcreteCommandPath & path,
    const GeneralCommissioning::Commands::SetRegulatoryConfig::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("SetRegulatoryConfig", "GeneralCommissioning");
    DeviceControlServer * server = &DeviceLayer::DeviceControlServer::DeviceControlSvr();
    Commands::SetRegulatoryConfigResponse::Type response;
    auto & countryCode = commandData.countryCode;

    if (countryCode.size() != ConfigurationManager::kMaxLocationLength)
    {
        ChipLogError(Zcl, "Invalid country code: '%s'", NullTerminated(countryCode).c_str());
        commandObj->AddStatus(path, Protocols::InteractionModel::Status::ConstraintError);
        return std::nullopt;
    }

    if (commandData.newRegulatoryConfig > RegulatoryLocationTypeEnum::kIndoorOutdoor)
    {
        response.errorCode = CommissioningErrorEnum::kValueOutsideRange;
        commandObj->AddResponse(path, response);
        return std::nullopt;
    }

    uint8_t locationCapability;
    if (ConfigurationMgr().GetLocationCapability(locationCapability) != CHIP_NO_ERROR)
    {
        commandObj->AddStatus(path, Protocols::InteractionModel::Status::Failure);
        return std::nullopt;
    }

    uint8_t location = to_underlying(commandData.newRegulatoryConfig);

    // If the LocationCapability attribute is not Indoor/Outdoor and the NewRegulatoryConfig value received does not match
    // either the Indoor or Outdoor fixed value in LocationCapability.
    if ((locationCapability != to_underlying(RegulatoryLocationTypeEnum::kIndoorOutdoor)) && (location != locationCapability))
    {
        response.errorCode = CommissioningErrorEnum::kValueOutsideRange;
        commandObj->AddResponse(path, response);
        return std::nullopt;
    }

    CheckSuccess(server->SetRegulatoryConfig(location, countryCode), Failure);
    Breadcrumb::Set(path.mEndpointId, commandData.breadcrumb);
    response.errorCode = CommissioningErrorEnum::kOk;
    commandObj->AddResponse(path, response);
    return std::nullopt;
}

#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
std::optional<DataModel::ActionReturnStatus>
HandleSetTCAcknowledgements(CommandHandler * commandObj, const ConcreteCommandPath & path,
                            const GeneralCommissioning::Commands::SetTCAcknowledgements::DecodableType & commandData);
{
    MATTER_TRACE_SCOPE("SetTCAcknowledgements", "GeneralCommissioning");

    auto & failSafeContext                  = Server::GetInstance().GetFailSafeContext();
    TermsAndConditionsProvider * tcProvider = TermsAndConditionsManager::GetInstance();

    if (nullptr == tcProvider)
    {
        commandObj->AddStatus(path, Protocols::InteractionModel::Status::Failure);
        return std::nullopt;
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
            commandObj->AddResponse(path, response);
            return std::nullopt;
        }

        if (!requiredTermsAndConditions.ValidateValue(acceptedTermsAndConditions))
        {
            response.errorCode = CommissioningErrorEnum::kRequiredTCNotAccepted;
            commandObj->AddResponse(path, response);
            return std::nullopt;
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
    commandObj->AddResponse(path, response);
}
#endif

void GeneralCommissioningLogic::OnPlatformEventHandler(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
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

namespace GeneralCommissioning {
void SetBreadcrumb(Attributes::Breadcrumb::TypeInfo::Type breadcrumb)
{
    Breadcrumb::Set(0, breadcrumb);
}
} // namespace GeneralCommissioning

} // namespace Clusters
} // namespace app
} // namespace chip
