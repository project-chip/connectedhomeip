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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>
#include <app/server/TermsAndConditionsProvider.h>
#include <app/util/attribute-storage.h>
#include <credentials/FabricTable.h>
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
            commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::code, #expr);                                  \
            return true;                                                                                                           \
        }                                                                                                                          \
    } while (false)

namespace {

class GeneralCommissioningAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the GeneralCommissioning cluster on all endpoints.
    GeneralCommissioningAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), GeneralCommissioning::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadIfSupported(CHIP_ERROR (ConfigurationManager::*getter)(uint8_t &), AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadBasicCommissioningInfo(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadSupportsConcurrentConnection(AttributeValueEncoder & aEncoder);
};

GeneralCommissioningAttrAccess gAttrAccess;

CHIP_ERROR GeneralCommissioningAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
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
#if CHIP_CONFIG_TC_REQUIRED
    case TCAcceptedVersion::Id: {
        TermsAndConditionsProvider * const termsAndConditionsProvider = Server::GetInstance().GetTermsAndConditionsProvider();
        Optional<TermsAndConditions> outTermsAndConditions;

        if (nullptr == termsAndConditionsProvider)
        {
            return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
        }

        CHIP_ERROR err = termsAndConditionsProvider->GetAcceptance(outTermsAndConditions);
        if (CHIP_NO_ERROR != err)
        {
            return CHIP_ERROR_INTERNAL;
        }

        return aEncoder.Encode(outTermsAndConditions.ValueOr((TermsAndConditions){ .value = 0, .version = 0 }).version);
    }
    case TCMinRequiredVersion::Id: {
        TermsAndConditionsProvider * const termsAndConditionsProvider = Server::GetInstance().GetTermsAndConditionsProvider();
        Optional<TermsAndConditions> outTermsAndConditions;

        if (nullptr == termsAndConditionsProvider)
        {
            return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
        }

        CHIP_ERROR err = termsAndConditionsProvider->GetRequirements(outTermsAndConditions);
        if (CHIP_NO_ERROR != err)
        {
            return CHIP_ERROR_INTERNAL;
        }

        return aEncoder.Encode(outTermsAndConditions.ValueOr((TermsAndConditions){ .value = 0, .version = 0 }).version);
    }
    case TCAcknowledgements::Id: {
        TermsAndConditionsProvider * const termsAndConditionsProvider = Server::GetInstance().GetTermsAndConditionsProvider();
        Optional<TermsAndConditions> outTermsAndConditions;

        if (nullptr == termsAndConditionsProvider)
        {
            return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
        }

        CHIP_ERROR err = termsAndConditionsProvider->GetAcceptance(outTermsAndConditions);
        if (CHIP_NO_ERROR != err)
        {
            return CHIP_ERROR_INTERNAL;
        }

        return aEncoder.Encode(outTermsAndConditions.ValueOr((TermsAndConditions){ .value = 0, .version = 0 }).value);
    }
    case TCAcknowledgementsRequired::Id: {
        TermsAndConditionsProvider * const termsAndConditionsProvider = Server::GetInstance().GetTermsAndConditionsProvider();
        Optional<TermsAndConditions> outTermsAndConditions;
        TermsAndConditionsState termsAndConditionsState;

        if (nullptr == termsAndConditionsProvider)
        {
            return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
        }

        CHIP_ERROR err = termsAndConditionsProvider->GetAcceptance(outTermsAndConditions);
        if (CHIP_NO_ERROR != err)
        {
            return CHIP_ERROR_INTERNAL;
        }

        err = termsAndConditionsProvider->CheckAcceptance(outTermsAndConditions, termsAndConditionsState);
        if (CHIP_NO_ERROR != err)
        {
            return CHIP_ERROR_INTERNAL;
        }

        bool setTermsAndConditionsCallRequiredBeforeCommissioningCompleteSuccess =
            termsAndConditionsState != TermsAndConditionsState::OK;
        return aEncoder.Encode(setTermsAndConditionsCallRequiredBeforeCommissioningCompleteSuccess);
    }
    case TCUpdateDeadline::Id: {
        TermsAndConditionsProvider * const termsAndConditionsProvider = Server::GetInstance().GetTermsAndConditionsProvider();

        if (nullptr == termsAndConditionsProvider)
        {
            return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
        }

        return aEncoder.EncodeNull();
    }
#endif // CHIP_CONFIG_TC_REQUIRED
    default: {
        break;
    }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR GeneralCommissioningAttrAccess::ReadIfSupported(CHIP_ERROR (ConfigurationManager::*getter)(uint8_t &),
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

CHIP_ERROR GeneralCommissioningAttrAccess::ReadBasicCommissioningInfo(AttributeValueEncoder & aEncoder)
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

CHIP_ERROR GeneralCommissioningAttrAccess::ReadSupportsConcurrentConnection(AttributeValueEncoder & aEncoder)
{
    SupportsConcurrentConnection::TypeInfo::Type supportsConcurrentConnection;

    // TODO: The commissioner might use the critical parameters in BasicCommissioningInfo to initialize
    // the CommissioningParameters at the beginning of commissioning flow.
    supportsConcurrentConnection = (CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION) != 0;

    return aEncoder.Encode(supportsConcurrentConnection);
}

#if CHIP_CONFIG_TC_REQUIRED
CommissioningErrorEnum CheckTermsAndConditionsAcknowledgementsState(TermsAndConditionsProvider * const termsAndConditionsProvider,
                                                                    const Optional<TermsAndConditions> & acceptedTermsAndConditions)
{
    TermsAndConditionsState termsAndConditionsState;

    CHIP_ERROR err = termsAndConditionsProvider->CheckAcceptance(acceptedTermsAndConditions, termsAndConditionsState);
    if (CHIP_NO_ERROR != err)
    {
        return chip::app::Clusters::GeneralCommissioning::CommissioningErrorEnum::kUnknownEnumValue;
    }

    switch (termsAndConditionsState)
    {
    case TermsAndConditionsState::OK:
        return chip::app::Clusters::GeneralCommissioning::CommissioningErrorEnum::kOk;
    case TermsAndConditionsState::TC_ACKNOWLEDGEMENTS_NOT_RECEIVED:
        return chip::app::Clusters::GeneralCommissioning::CommissioningErrorEnum::kTCAcknowledgementsNotReceived;
    case TermsAndConditionsState::TC_MIN_VERSION_NOT_MET:
        return chip::app::Clusters::GeneralCommissioning::CommissioningErrorEnum::kTCMinVersionNotMet;
    case TermsAndConditionsState::REQUIRED_TC_NOT_ACCEPTED:
        return chip::app::Clusters::GeneralCommissioning::CommissioningErrorEnum::kRequiredTCNotAccepted;
    }

    return chip::app::Clusters::GeneralCommissioning::CommissioningErrorEnum::kOk;
}

CommissioningErrorEnum CheckTermsAndConditionsAcknowledgements(TermsAndConditionsProvider * const termsAndConditionsProvider)
{
    Optional<TermsAndConditions> acceptedTermsAndConditions;
    TermsAndConditionsState termsAndConditionsState;

    CHIP_ERROR err = termsAndConditionsProvider->GetAcceptance(acceptedTermsAndConditions);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(FailSafe, "GeneralCommissioning: Failed to get terms and conditions acceptance: %" CHIP_ERROR_FORMAT,
                     err.Format());
        return chip::app::Clusters::GeneralCommissioning::CommissioningErrorEnum::kTCAcknowledgementsNotReceived;
    }

    err = termsAndConditionsProvider->CheckAcceptance(acceptedTermsAndConditions, termsAndConditionsState);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(FailSafe, "GeneralCommissioning: Failed to verify terms and conditions acceptance: %" CHIP_ERROR_FORMAT,
                     err.Format());
        return chip::app::Clusters::GeneralCommissioning::CommissioningErrorEnum::kRequiredTCNotAccepted;
    }

    switch (termsAndConditionsState)
    {
    case TermsAndConditionsState::OK:
        return chip::app::Clusters::GeneralCommissioning::CommissioningErrorEnum::kOk;
    case TermsAndConditionsState::TC_ACKNOWLEDGEMENTS_NOT_RECEIVED:
        return chip::app::Clusters::GeneralCommissioning::CommissioningErrorEnum::kTCAcknowledgementsNotReceived;
    case TermsAndConditionsState::TC_MIN_VERSION_NOT_MET:
        return chip::app::Clusters::GeneralCommissioning::CommissioningErrorEnum::kTCMinVersionNotMet;
    case TermsAndConditionsState::REQUIRED_TC_NOT_ACCEPTED:
        return chip::app::Clusters::GeneralCommissioning::CommissioningErrorEnum::kRequiredTCNotAccepted;
    }

    return chip::app::Clusters::GeneralCommissioning::CommissioningErrorEnum::kOk;
}
#endif // CHIP_CONFIG_TC_REQUIRED

} // anonymous namespace

bool emberAfGeneralCommissioningClusterArmFailSafeCallback(app::CommandHandler * commandObj,
                                                           const app::ConcreteCommandPath & commandPath,
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

    FabricIndex accessingFabricIndex = commandObj->GetAccessingFabricIndex();

    // We do not allow CASE connections to arm the failsafe for the first time while the commissioning window is open in order
    // to allow commissioners the opportunity to obtain this failsafe for the purpose of commissioning
    if (!failSafeContext.IsFailSafeBusy() &&
        (!failSafeContext.IsFailSafeArmed() || failSafeContext.MatchesFabricIndex(accessingFabricIndex)))
    {
        // We do not allow CASE connections to arm the failsafe for the first time while the commissioning window is open in order
        // to allow commissioners the opportunity to obtain this failsafe for the purpose of commissioning
        if (!failSafeContext.IsFailSafeArmed() &&
            Server::GetInstance().GetCommissioningWindowManager().IsCommissioningWindowOpen() &&
            commandObj->GetSubjectDescriptor().authMode == Access::AuthMode::kCase)
        {
            response.errorCode = CommissioningErrorEnum::kBusyWithOtherAdmin;
            commandObj->AddResponse(commandPath, response);
        }
        else if (commandData.expiryLengthSeconds == 0)
        {
            // Force the timer to expire immediately.
            failSafeContext.ForceFailSafeTimerExpiry();
            // Don't set the breadcrumb, since expiring the failsafe should
            // reset it anyway.
            response.errorCode = CommissioningErrorEnum::kOk;
            commandObj->AddResponse(commandPath, response);
        }
        else
        {
            CheckSuccess(
                failSafeContext.ArmFailSafe(accessingFabricIndex, System::Clock::Seconds16(commandData.expiryLengthSeconds)),
                Failure);
            Breadcrumb::Set(commandPath.mEndpointId, commandData.breadcrumb);
            response.errorCode = CommissioningErrorEnum::kOk;
            commandObj->AddResponse(commandPath, response);
        }
    }
    else
    {
        response.errorCode = CommissioningErrorEnum::kBusyWithOtherAdmin;
        commandObj->AddResponse(commandPath, response);
    }

    return true;
}

bool emberAfGeneralCommissioningClusterCommissioningCompleteCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::CommissioningComplete::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("CommissioningComplete", "GeneralCommissioning");

    DeviceControlServer * devCtrl = &DeviceLayer::DeviceControlServer::DeviceControlSvr();
    auto & failSafe               = Server::GetInstance().GetFailSafeContext();
    auto & fabricTable            = Server::GetInstance().GetFabricTable();

    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogProgress(FailSafe, "GeneralCommissioning: Received CommissioningComplete");

    Commands::CommissioningCompleteResponse::Type response;

    // Fail-safe must be armed
    if (!failSafe.IsFailSafeArmed())
    {
        response.errorCode = CommissioningErrorEnum::kNoFailSafe;
        commandObj->AddResponse(commandPath, response);
        return true;
    }

    SessionHandle handle = commandObj->GetExchangeContext()->GetSessionHandle();

    // Ensure it's a valid CASE session
    if ((handle->GetSessionType() != Session::SessionType::kSecure) ||
        (handle->AsSecureSession()->GetSecureSessionType() != SecureSession::Type::kCASE) ||
        (!failSafe.MatchesFabricIndex(commandObj->GetAccessingFabricIndex())))
    {
        response.errorCode = CommissioningErrorEnum::kInvalidAuthentication;
        ChipLogError(FailSafe, "GeneralCommissioning: Got commissioning complete in invalid security context");
        commandObj->AddResponse(commandPath, response);
        return true;
    }

#if CHIP_CONFIG_TC_REQUIRED
    TermsAndConditionsProvider * const termsAndConditionsProvider = Server::GetInstance().GetTermsAndConditionsProvider();

    // Ensure required terms and conditions have been accepted, then attempt to commit
    if (nullptr != termsAndConditionsProvider)
    {
        response.errorCode = CheckTermsAndConditionsAcknowledgements(termsAndConditionsProvider);
        if (CommissioningErrorEnum::kOk != response.errorCode)
        {
            ChipLogError(FailSafe, "GeneralCommissioning: Terms and conditions not accepted");
            commandObj->AddResponse(commandPath, response);
            return true;
        }

        if (failSafe.UpdateTermsAndConditionsHasBeenInvoked())
        {
            // Commit terms and conditions acceptance on commissioning complete
            err = termsAndConditionsProvider->CommitAcceptance();
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
#endif // CHIP_CONFIG_TC_REQUIRED

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

    Breadcrumb::Set(commandPath.mEndpointId, 0);
    response.errorCode = CommissioningErrorEnum::kOk;

    commandObj->AddResponse(commandPath, response);
    return true;
}

bool emberAfGeneralCommissioningClusterSetRegulatoryConfigCallback(app::CommandHandler * commandObj,
                                                                   const app::ConcreteCommandPath & commandPath,
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
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::ConstraintError);
        return true;
    }

    if (commandData.newRegulatoryConfig > RegulatoryLocationTypeEnum::kIndoorOutdoor)
    {
        response.errorCode = CommissioningErrorEnum::kValueOutsideRange;
        // TODO: How does using the country code in debug text make sense, if
        // the real issue is the newRegulatoryConfig value?
        response.debugText = countryCode;
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
            // TODO: How does using the country code in debug text make sense, if
            // the real issue is the newRegulatoryConfig value?
            response.debugText = countryCode;
        }
        else
        {
            CheckSuccess(server->SetRegulatoryConfig(location, countryCode), Failure);
            Breadcrumb::Set(commandPath.mEndpointId, commandData.breadcrumb);
            response.errorCode = CommissioningErrorEnum::kOk;
        }
    }

    commandObj->AddResponse(commandPath, response);

    return true;
}

bool emberAfGeneralCommissioningClusterSetTCAcknowledgementsCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::GeneralCommissioning::Commands::SetTCAcknowledgements::DecodableType & commandData)
{
#if CHIP_CONFIG_TC_REQUIRED
    MATTER_TRACE_SCOPE("SetTCAcknowledgements", "GeneralCommissioning");

    auto & failSafeContext                                        = Server::GetInstance().GetFailSafeContext();
    TermsAndConditionsProvider * const termsAndConditionsProvider = Server::GetInstance().GetTermsAndConditionsProvider();

    Optional<TermsAndConditions> acceptedTermsAndConditions = Optional<TermsAndConditions>({
        .value   = commandData.TCUserResponse,
        .version = commandData.TCVersion,
    });

    Commands::SetTCAcknowledgementsResponse::Type response;
    response.errorCode = CheckTermsAndConditionsAcknowledgementsState(termsAndConditionsProvider, acceptedTermsAndConditions);

    if (CommissioningErrorEnum::kOk == response.errorCode)
    {
        CheckSuccess(termsAndConditionsProvider->SetAcceptance(acceptedTermsAndConditions), Failure);

        if (failSafeContext.IsFailSafeArmed())
        {
            failSafeContext.SetUpdateTermsAndConditionsHasBeenInvoked();
        }
        else
        {
            CheckSuccess(termsAndConditionsProvider->CommitAcceptance(), Failure);
        }
    }

    commandObj->AddResponse(commandPath, response);
#endif // CHIP_CONFIG_TC_REQUIRED
    return true;
}

namespace {
void OnPlatformEventHandler(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    if (event->Type == DeviceLayer::DeviceEventType::kFailSafeTimerExpired)
    {
        // Spec says to reset Breadcrumb attribute to 0.
        Breadcrumb::Set(0, 0);

        if (event->FailSafeTimerExpired.updateTermsAndConditionsHasBeenInvoked)
        {
#if CHIP_CONFIG_TC_REQUIRED
            TermsAndConditionsProvider * const termsAndConditionsProvider = Server::GetInstance().GetTermsAndConditionsProvider();

            if (nullptr == termsAndConditionsProvider)
            {
                return;
            }

            // Clear terms and conditions acceptance on failsafe timer expiration
            termsAndConditionsProvider->RevertAcceptance();
#endif // CHIP_CONFIG_TC_REQUIRED
        }
    }
}

} // anonymous namespace

class GeneralCommissioningFabricTableDelegate : public chip::FabricTable::Delegate
{
public:
    // Gets called when a fabric is deleted
    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override
    {
        // If the FabricIndex matches the last remaining entry in the Fabrics list, then the device SHALL delete all Matter
        // related data on the node which was created since it was commissioned.
        if (Server::GetInstance().GetFabricTable().FabricCount() == 0)
        {
            ChipLogProgress(Zcl, "general-commissioning-server: Last Fabric index 0x%x was removed",
                            static_cast<unsigned>(fabricIndex));

#if CHIP_CONFIG_TC_REQUIRED
            TermsAndConditionsProvider * const termsAndConditionsProvider = Server::GetInstance().GetTermsAndConditionsProvider();

            if (nullptr != termsAndConditionsProvider)
            {
                termsAndConditionsProvider->ResetAcceptance();
            }
#endif // CHIP_CONFIG_TC_REQUIRED
        }
    }
};

void MatterGeneralCommissioningPluginServerInitCallback()
{
    Breadcrumb::Set(0, 0);
    AttributeAccessInterfaceRegistry::Instance().Register(&gAttrAccess);
    DeviceLayer::PlatformMgrImpl().AddEventHandler(OnPlatformEventHandler);

    static GeneralCommissioningFabricTableDelegate generalCommissioningFabricTableDelegate;
    Server::GetInstance().GetFabricTable().AddFabricDelegate(&generalCommissioningFabricTableDelegate);
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
