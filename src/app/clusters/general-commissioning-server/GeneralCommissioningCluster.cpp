/*
 *    Copyright (c) 2021-2025 Project CHIP Authors
 *    All rights reserved.
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

#include "GeneralCommissioningCluster.h"

#include <app/AppConfig.h>
#include <app/reporting/reporting.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server/Server.h>
#include <clusters/GeneralCommissioning/AttributeIds.h>
#include <clusters/GeneralCommissioning/CommandIds.h>
#include <clusters/GeneralCommissioning/Metadata.h>
#include <cstdint>
#include <optional>
#include <platform/ConfigurationManager.h>
#include <platform/DeviceControlServer.h>
#include <platform/PlatformManager.h>
#include <tracing/macros.h>
#include <transport/SecureSession.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters::GeneralCommissioning;
using namespace chip::app::Clusters::GeneralCommissioning::Attributes;
using chip::Transport::SecureSession;
using chip::Transport::Session;

namespace {

#define CheckSuccess(expr, code)                                                                                                   \
    do                                                                                                                             \
    {                                                                                                                              \
        if (!::chip::ChipError::IsSuccess(expr))                                                                                   \
        {                                                                                                                          \
            handler->AddStatus(request.path, Protocols::InteractionModel::Status::code, #expr);                                    \
            return std::nullopt;                                                                                                   \
        }                                                                                                                          \
    } while (false)

CHIP_ERROR ReadIfSupported(ConfigurationManager & mgr, CHIP_ERROR (ConfigurationManager::*getter)(uint8_t &),
                           AttributeValueEncoder & aEncoder)
{
    uint8_t data   = 0;
    CHIP_ERROR err = (mgr.*getter)(data);
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

#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
typedef struct sTermsAndConditionsState
{
    Optional<TermsAndConditions> acceptance;
    bool acknowledgementsRequired;
    Optional<TermsAndConditions> requirements;
    Optional<uint32_t> updateAcceptanceDeadline;
} TermsAndConditionsState;

CHIP_ERROR GetTermsAndConditionsAttributeState(TermsAndConditionsProvider & tcProvider,
                                               TermsAndConditionsState & outTermsAndConditionsState)
{
    TermsAndConditionsState termsAndConditionsState;

    ReturnErrorOnFailure(tcProvider.GetAcceptance(termsAndConditionsState.acceptance));
    ReturnErrorOnFailure(tcProvider.GetAcknowledgementsRequired(termsAndConditionsState.acknowledgementsRequired));
    ReturnErrorOnFailure(tcProvider.GetRequirements(termsAndConditionsState.requirements));
    ReturnErrorOnFailure(tcProvider.GetUpdateAcceptanceDeadline(termsAndConditionsState.updateAcceptanceDeadline));

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

void OnPlatformEventHandler(const DeviceLayer::ChipDeviceEvent * event, intptr_t self_ptr_arg)
{
    if (event->Type == DeviceLayer::DeviceEventType::kFailSafeTimerExpired)
    {
        auto self = reinterpret_cast<GeneralCommissioningCluster *>(self_ptr_arg);

        // Spec says to reset Breadcrumb attribute to 0.
        self->SetBreadCrumb(0);

#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
        if (event->FailSafeTimerExpired.updateTermsAndConditionsHasBeenInvoked)
        {
            // Clear terms and conditions acceptance on failsafe timer expiration
            TermsAndConditionsProvider & tcProvider = self->ClusterContext().termsAndConditionsProvider;
            TermsAndConditionsState initialState, updatedState;
            VerifyOrReturn(CHIP_NO_ERROR == GetTermsAndConditionsAttributeState(tcProvider, initialState));
            VerifyOrReturn(CHIP_NO_ERROR == tcProvider.RevertAcceptance());
            VerifyOrReturn(CHIP_NO_ERROR == GetTermsAndConditionsAttributeState(tcProvider, updatedState));
            NotifyTermsAndConditionsAttributeChangeIfRequired(initialState, updatedState);
        }
#endif // CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    }
}

} // anonymous namespace

namespace chip::app::Clusters {

DataModel::ActionReturnStatus GeneralCommissioningCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                         AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case FeatureMap::Id:
        return encoder.Encode(kFeatures);
    case ClusterRevision::Id:
        return encoder.Encode(GeneralCommissioning::kRevision);
    case Breadcrumb::Id:
        return encoder.Encode(mBreadCrumb);

    case BasicCommissioningInfo::Id: {
        BasicCommissioningInfo::TypeInfo::Type info;

        // TODO: The commissioner might use the critical parameters in BasicCommissioningInfo to initialize
        // the CommissioningParameters at the beginning of commissioning flow.
        info.failSafeExpiryLengthSeconds  = CHIP_DEVICE_CONFIG_FAILSAFE_EXPIRY_LENGTH_SEC;
        info.maxCumulativeFailsafeSeconds = CHIP_DEVICE_CONFIG_MAX_CUMULATIVE_FAILSAFE_SEC;
        static_assert(CHIP_DEVICE_CONFIG_MAX_CUMULATIVE_FAILSAFE_SEC >= CHIP_DEVICE_CONFIG_FAILSAFE_EXPIRY_LENGTH_SEC,
                      "Max cumulative failsafe seconds must be larger than failsafe expiry length seconds");

        return encoder.Encode(info);
    }
    case RegulatoryConfig::Id:
        return ReadIfSupported(mClusterContext.configurationManager, &ConfigurationManager::GetRegulatoryLocation, encoder);
    case LocationCapability::Id:
        return ReadIfSupported(mClusterContext.configurationManager, &ConfigurationManager::GetLocationCapability, encoder);
    case SupportsConcurrentConnection::Id:
        return encoder.Encode(CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION != 0);

#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    case TCAcceptedVersion::Id: {
        Optional<TermsAndConditions> outTermsAndConditions;
        ReturnErrorOnFailure(mClusterContext.termsAndConditionsProvider.GetAcceptance(outTermsAndConditions));
        return encoder.Encode(outTermsAndConditions.ValueOr(TermsAndConditions(0, 0)).GetVersion());
    }
    case TCMinRequiredVersion::Id: {
        Optional<TermsAndConditions> outTermsAndConditions;

        ReturnErrorOnFailure(mClusterContext.termsAndConditionsProvider.GetRequirements(outTermsAndConditions));
        return encoder.Encode(outTermsAndConditions.ValueOr(TermsAndConditions(0, 0)).GetVersion());
    }
    case TCAcknowledgements::Id: {
        Optional<TermsAndConditions> outTermsAndConditions;

        ReturnErrorOnFailure(mClusterContext.termsAndConditionsProvider.GetAcceptance(outTermsAndConditions));
        return encoder.Encode(outTermsAndConditions.ValueOr(TermsAndConditions(0, 0)).GetValue());
    }
    case TCAcknowledgementsRequired::Id: {
        bool acknowledgementsRequired;
        ReturnErrorOnFailure(mClusterContext.termsAndConditionsProvider.GetAcknowledgementsRequired(acknowledgementsRequired));
        return encoder.Encode(acknowledgementsRequired);
    }
    case TCUpdateDeadline::Id: {
        Optional<uint32_t> outUpdateAcceptanceDeadline;
        ReturnErrorOnFailure(mClusterContext.termsAndConditionsProvider.GetUpdateAcceptanceDeadline(outUpdateAcceptanceDeadline));

        // NOTE: encoding an optional as a Nullable (they are not fully compatible)
        if (!outUpdateAcceptanceDeadline.HasValue())
        {
            return encoder.EncodeNull();
        }
        return encoder.Encode(outUpdateAcceptanceDeadline.Value());
    }
#endif

    case RecoveryIdentifier::Id:
    case NetworkRecoveryReason::Id:
    case IsCommissioningWithoutPower::Id:
        // TODO: implement the above - they are currently PROVISIONAL and not supported by AAI
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus GeneralCommissioningCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                          AttributeValueDecoder & decoder)
{
    using namespace GeneralCommissioning::Attributes;
    switch (request.path.mAttributeId)
    {
    case Breadcrumb::Id: {
        uint64_t value;
        ReturnErrorOnFailure(decoder.Decode(value));
        // SetBreadCrumb handles notification internally via NotifyAttributeChanged(),
        // so we don't need to call NotifyAttributeChangedIfSuccess here.
        SetBreadCrumb(value);
        return CHIP_NO_ERROR;
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedWrite;
    }
}

std::optional<DataModel::ActionReturnStatus> GeneralCommissioningCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                        TLV::TLVReader & input_arguments,
                                                                                        CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case Commands::ArmFailSafe::Id: {
        Commands::ArmFailSafe::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments));
        return HandleArmFailSafe(request, handler, request_data);
    }
    case Commands::CommissioningComplete::Id: {
        Commands::CommissioningComplete::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments, request.GetAccessingFabricIndex()));
        return HandleCommissioningComplete(request, handler, request_data);
    }
    case Commands::SetRegulatoryConfig::Id: {
        Commands::SetRegulatoryConfig::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments));
        return HandleSetRegulatoryConfig(request, handler, request_data);
    }
#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    case Commands::SetTCAcknowledgements::Id: {
        Commands::SetTCAcknowledgements::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments));
        return HandleSetTCAcknowledgements(request, handler, request_data);
    }
#endif
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

CHIP_ERROR GeneralCommissioningCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                         ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    if constexpr (kFeatures.Has(GeneralCommissioning::Feature::kTermsAndConditions))
    {
        ReturnErrorOnFailure(builder.AppendElements({ Commands::SetTCAcknowledgements::kMetadataEntry }));
    }

    return builder.AppendElements({
        Commands::ArmFailSafe::kMetadataEntry,
        Commands::SetRegulatoryConfig::kMetadataEntry,
        Commands::CommissioningComplete::kMetadataEntry,
    });
}

CHIP_ERROR GeneralCommissioningCluster::GeneratedCommands(const ConcreteClusterPath & path,
                                                          ReadOnlyBufferBuilder<CommandId> & builder)
{

    if constexpr (kFeatures.Has(GeneralCommissioning::Feature::kTermsAndConditions))
    {
        ReturnErrorOnFailure(builder.AppendElements({ Commands::SetTCAcknowledgementsResponse::Id }));
    }

    return builder.AppendElements({
        Commands::ArmFailSafeResponse::Id,
        Commands::SetRegulatoryConfigResponse::Id,
        Commands::CommissioningCompleteResponse::Id,
    });
}

CHIP_ERROR GeneralCommissioningCluster::Attributes(const ConcreteClusterPath & path,
                                                   ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{

    if constexpr (kFeatures.Has(GeneralCommissioning::Feature::kTermsAndConditions))
    {
        ReturnErrorOnFailure(builder.AppendElements({
            TCAcceptedVersion::kMetadataEntry,
            TCMinRequiredVersion::kMetadataEntry,
            TCAcknowledgements::kMetadataEntry,
            TCAcknowledgementsRequired::kMetadataEntry,
            TCUpdateDeadline::kMetadataEntry,
        }));
    }

    if constexpr (kFeatures.Has(GeneralCommissioning::Feature::kNetworkRecovery))
    {
        ReturnErrorOnFailure(builder.AppendElements({
            RecoveryIdentifier::kMetadataEntry,
            NetworkRecoveryReason::kMetadataEntry,
        }));
    }

    static constexpr DataModel::AttributeEntry optionalEntries[] = {
        IsCommissioningWithoutPower::kMetadataEntry,
    };

    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(Attributes::kMandatoryMetadata), Span(optionalEntries), mOptionalAttributes);
}

void GeneralCommissioningCluster::OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex)
{
#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    // If the FabricIndex matches the last remaining entry in the Fabrics list, then the device SHALL delete all Matter
    // related data on the node which was created since it was commissioned.
    if (fabricTable.FabricCount() == 0)
    {
        ChipLogProgress(Zcl, "general-commissioning-server: Last Fabric index 0x%x was removed",
                        static_cast<unsigned>(fabricIndex));

        TermsAndConditionsProvider & tcProvider = mClusterContext.termsAndConditionsProvider;
        TermsAndConditionsState initialState, updatedState;
        VerifyOrReturn(CHIP_NO_ERROR == GetTermsAndConditionsAttributeState(tcProvider, initialState));
        VerifyOrReturn(CHIP_NO_ERROR == tcProvider.ResetAcceptance());
        VerifyOrReturn(CHIP_NO_ERROR == GetTermsAndConditionsAttributeState(tcProvider, updatedState));
        NotifyTermsAndConditionsAttributeChangeIfRequired(initialState, updatedState);
    }
#endif // CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
}

void GeneralCommissioningCluster::SetBreadCrumb(uint64_t value)
{
    VerifyOrReturn(mBreadCrumb != value);
    mBreadCrumb = value;
    NotifyAttributeChanged(Breadcrumb::Id);
}

CHIP_ERROR GeneralCommissioningCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));
    ReturnErrorOnFailure(mClusterContext.platformManager.AddEventHandler(OnPlatformEventHandler, reinterpret_cast<intptr_t>(this)));
    return mClusterContext.fabricTable.AddFabricDelegate(this);
}

void GeneralCommissioningCluster::Shutdown()
{
    mClusterContext.platformManager.RemoveEventHandler(OnPlatformEventHandler, reinterpret_cast<intptr_t>(this));
    mClusterContext.fabricTable.RemoveFabricDelegate(this);
    DefaultServerCluster::Shutdown();
}

std::optional<DataModel::ActionReturnStatus>
GeneralCommissioningCluster::HandleArmFailSafe(const DataModel::InvokeRequest & request, CommandHandler * handler,
                                               const GeneralCommissioning::Commands::ArmFailSafe::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("ArmFailSafe", "GeneralCommissioning");
    auto & failSafeContext = mClusterContext.failsafeContext;
    Commands::ArmFailSafeResponse::Type response;

    ChipLogProgress(FailSafe, "GeneralCommissioning: Received ArmFailSafe (%us)",
                    static_cast<unsigned>(commandData.expiryLengthSeconds));

    /*
     * If the fail-safe timer is not fully disarmed, don't allow arming a new fail-safe.
     * If the fail-safe timer was not currently armed, then the fail-safe timer SHALL be armed.
     * If the fail-safe timer was currently armed, and current accessing fabric matches the fail-safe
     * context’s Fabric Index, then the fail-safe timer SHALL be re-armed.
     */
    FabricIndex accessingFabricIndex = request.GetAccessingFabricIndex();

    // We do not allow CASE connections to arm the failsafe for the first time while the commissioning window is open in order
    // to allow commissioners the opportunity to obtain this failsafe for the purpose of commissioning
    if (!failSafeContext.IsFailSafeBusy() &&
        (!failSafeContext.IsFailSafeArmed() || failSafeContext.MatchesFabricIndex(accessingFabricIndex)))
    {
        // We do not allow CASE connections to arm the failsafe for the first time while the commissioning window is open in order
        // to allow commissioners the opportunity to obtain this failsafe for the purpose of commissioning
        if (!failSafeContext.IsFailSafeArmed() && mClusterContext.commissioningWindowManager.IsCommissioningWindowOpen() &&
            request.subjectDescriptor->authMode == Access::AuthMode::kCase)
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
            SetBreadCrumb(commandData.breadcrumb);
            response.errorCode = CommissioningErrorEnum::kOk;
        }
    }
    else
    {
        response.errorCode = CommissioningErrorEnum::kBusyWithOtherAdmin;
    }
    handler->AddResponse(request.path, response);
    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus>
GeneralCommissioningCluster::HandleCommissioningComplete(const DataModel::InvokeRequest & request, CommandHandler * handler,
                                                         const Commands::CommissioningComplete::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("CommissioningComplete", "GeneralCommissioning");

    auto & failSafe = mClusterContext.failsafeContext;

    ChipLogProgress(FailSafe, "GeneralCommissioning: Received CommissioningComplete");

    Commands::CommissioningCompleteResponse::Type response;
    CHIP_ERROR err;

    // Fail-safe must be armed
    if (!failSafe.IsFailSafeArmed())
    {
        response.errorCode = CommissioningErrorEnum::kNoFailSafe;
        handler->AddResponse(request.path, response);
        return std::nullopt;
    }

#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    TermsAndConditionsProvider & tcProvider = mClusterContext.termsAndConditionsProvider;

    // Ensure required terms and conditions have been accepted, then attempt to commit
    Optional<TermsAndConditions> requiredTermsAndConditionsMaybe;
    Optional<TermsAndConditions> acceptedTermsAndConditionsMaybe;

    CheckSuccess(tcProvider.GetRequirements(requiredTermsAndConditionsMaybe), Failure);
    CheckSuccess(tcProvider.GetAcceptance(acceptedTermsAndConditionsMaybe), Failure);

    if (requiredTermsAndConditionsMaybe.HasValue() && !acceptedTermsAndConditionsMaybe.HasValue())
    {
        response.errorCode = CommissioningErrorEnum::kTCAcknowledgementsNotReceived;
        handler->AddResponse(request.path, response);
        return std::nullopt;
    }

    if (requiredTermsAndConditionsMaybe.HasValue() && acceptedTermsAndConditionsMaybe.HasValue())
    {
        TermsAndConditions requiredTermsAndConditions = requiredTermsAndConditionsMaybe.Value();
        TermsAndConditions acceptedTermsAndConditions = acceptedTermsAndConditionsMaybe.Value();

        if (!requiredTermsAndConditions.ValidateVersion(acceptedTermsAndConditions))
        {
            response.errorCode = CommissioningErrorEnum::kTCMinVersionNotMet;
            handler->AddResponse(request.path, response);
            return std::nullopt;
        }

        if (!requiredTermsAndConditions.ValidateValue(acceptedTermsAndConditions))
        {
            response.errorCode = CommissioningErrorEnum::kRequiredTCNotAccepted;
            handler->AddResponse(request.path, response);
            return std::nullopt;
        }
    }

    if (failSafe.UpdateTermsAndConditionsHasBeenInvoked())
    {
        // Commit terms and conditions acceptance on commissioning complete
        err = tcProvider.CommitAcceptance();
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
#endif // CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED

    SessionHandle handle = mContext->interactionContext.actionContext.CurrentExchange()->GetSessionHandle();

    // Ensure it's a valid CASE session
    if (handle->GetSessionType() != Session::SessionType::kSecure ||
        handle->AsSecureSession()->GetSecureSessionType() != SecureSession::Type::kCASE ||
        !failSafe.MatchesFabricIndex(request.GetAccessingFabricIndex()))
    {
        response.errorCode = CommissioningErrorEnum::kInvalidAuthentication;
        ChipLogError(FailSafe, "GeneralCommissioning: Got commissioning complete in invalid security context");
        handler->AddResponse(request.path, response);
        return std::nullopt;
    }

    // Handle NOC commands
    if (failSafe.NocCommandHasBeenInvoked())
    {
        err = mClusterContext.fabricTable.CommitPendingFabricData();
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
    err = mClusterContext.deviceControlServer.PostCommissioningCompleteEvent(handle->AsSecureSession()->GetPeerNodeId(),
                                                                             handle->GetFabricIndex());
    CheckSuccess(err, Failure);

    SetBreadCrumb(0);
    response.errorCode = CommissioningErrorEnum::kOk;
    handler->AddResponse(request.path, response);
    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus>
GeneralCommissioningCluster::HandleSetRegulatoryConfig(const DataModel::InvokeRequest & request, CommandHandler * handler,
                                                       const Commands::SetRegulatoryConfig::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("SetRegulatoryConfig", "GeneralCommissioning");
    Commands::SetRegulatoryConfigResponse::Type response;
    auto & countryCode = commandData.countryCode;

    if (countryCode.size() != ConfigurationManager::kMaxLocationLength)
    {
        ChipLogError(Zcl, "Invalid country code: '%s'", NullTerminated(countryCode).c_str());
        return Protocols::InteractionModel::Status::ConstraintError;
    }

    if (commandData.newRegulatoryConfig > RegulatoryLocationTypeEnum::kIndoorOutdoor)
    {
        response.errorCode = CommissioningErrorEnum::kValueOutsideRange;
        handler->AddResponse(request.path, response);
        return std::nullopt;
    }

    uint8_t locationCapability;
    if (mClusterContext.configurationManager.GetLocationCapability(locationCapability) != CHIP_NO_ERROR)
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    uint8_t location = to_underlying(commandData.newRegulatoryConfig);

    // If the LocationCapability attribute is not Indoor/Outdoor and the NewRegulatoryConfig value received does not match
    // either the Indoor or Outdoor fixed value in LocationCapability.
    if ((locationCapability != to_underlying(RegulatoryLocationTypeEnum::kIndoorOutdoor)) && (location != locationCapability))
    {
        response.errorCode = CommissioningErrorEnum::kValueOutsideRange;
        handler->AddResponse(request.path, response);
        return std::nullopt;
    }

    CheckSuccess(mClusterContext.deviceControlServer.SetRegulatoryConfig(location, countryCode), Failure);
    SetBreadCrumb(commandData.breadcrumb);
    response.errorCode = CommissioningErrorEnum::kOk;
    handler->AddResponse(request.path, response);
    return std::nullopt;
}

#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
std::optional<DataModel::ActionReturnStatus>
GeneralCommissioningCluster::HandleSetTCAcknowledgements(const DataModel::InvokeRequest & request, CommandHandler * handler,
                                                         const Commands::SetTCAcknowledgements::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("SetTCAcknowledgements", "GeneralCommissioning");

    auto & failSafeContext                  = mClusterContext.failsafeContext;
    TermsAndConditionsProvider & tcProvider = mClusterContext.termsAndConditionsProvider;

    Optional<TermsAndConditions> requiredTermsAndConditionsMaybe;
    Optional<TermsAndConditions> previousAcceptedTermsAndConditionsMaybe;
    CheckSuccess(tcProvider.GetRequirements(requiredTermsAndConditionsMaybe), Failure);
    CheckSuccess(tcProvider.GetAcceptance(previousAcceptedTermsAndConditionsMaybe), Failure);
    TermsAndConditions acceptedTermsAndConditions = TermsAndConditions(commandData.TCUserResponse, commandData.TCVersion);
    Optional<TermsAndConditions> acceptedTermsAndConditionsPresent = Optional<TermsAndConditions>(acceptedTermsAndConditions);

    Commands::SetTCAcknowledgementsResponse::Type response;

    if (requiredTermsAndConditionsMaybe.HasValue())
    {
        TermsAndConditions requiredTermsAndConditions = requiredTermsAndConditionsMaybe.Value();

        if (!requiredTermsAndConditions.ValidateVersion(acceptedTermsAndConditions))
        {
            response.errorCode = CommissioningErrorEnum::kTCMinVersionNotMet;
            handler->AddResponse(request.path, response);
            return std::nullopt;
        }

        if (!requiredTermsAndConditions.ValidateValue(acceptedTermsAndConditions))
        {
            response.errorCode = CommissioningErrorEnum::kRequiredTCNotAccepted;
            handler->AddResponse(request.path, response);
            return std::nullopt;
        }
    }

    if (previousAcceptedTermsAndConditionsMaybe != acceptedTermsAndConditionsPresent)
    {
        TermsAndConditionsState initialState, updatedState;
        CheckSuccess(GetTermsAndConditionsAttributeState(tcProvider, initialState), Failure);
        CheckSuccess(tcProvider.SetAcceptance(acceptedTermsAndConditionsPresent), Failure);
        CheckSuccess(GetTermsAndConditionsAttributeState(tcProvider, updatedState), Failure);
        NotifyTermsAndConditionsAttributeChangeIfRequired(initialState, updatedState);

        // Commit or defer based on fail-safe state
        if (!failSafeContext.IsFailSafeArmed())
        {
            CheckSuccess(tcProvider.CommitAcceptance(), Failure);
        }
        else
        {
            failSafeContext.SetUpdateTermsAndConditionsHasBeenInvoked();
        }
    }

    response.errorCode = CommissioningErrorEnum::kOk;
    handler->AddResponse(request.path, response);
    return std::nullopt;
}
#endif // CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED

} // namespace chip::app::Clusters
