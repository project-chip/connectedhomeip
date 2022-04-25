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
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/ConfigurationManager.h>
#include <platform/DeviceControlServer.h>
#include <trace/trace.h>

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
            CHIP_ERROR statusErr = commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::code);                  \
            if (statusErr != CHIP_NO_ERROR)                                                                                        \
            {                                                                                                                      \
                ChipLogError(Zcl, "%s: %" CHIP_ERROR_FORMAT, #expr, statusErr.Format());                                           \
            }                                                                                                                      \
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
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

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
    case Breadcrumb::Id: {
        return aEncoder.Encode(DeviceLayer::DeviceControlServer::DeviceControlSvr().GetBreadcrumb());
    }
    default: {
        break;
    }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR GeneralCommissioningAttrAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    // TODO: There was discussion about moving the breadcrumb to the attribute store, which would make this function obsolete

    if (aPath.mClusterId != GeneralCommissioning::Id)
    {
        // We shouldn't have been called at all.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    switch (aPath.mAttributeId)
    {
    case Attributes::Breadcrumb::Id:
        Attributes::Breadcrumb::TypeInfo::DecodableType value;
        ReturnErrorOnFailure(aDecoder.Decode(value));
        return DeviceLayer::DeviceControlServer::DeviceControlSvr().SetBreadcrumb(value);
    default:
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
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
    basicCommissioningInfo.failSafeExpiryLengthSeconds = CHIP_DEVICE_CONFIG_FAILSAFE_EXPIRY_LENGTH_SEC;

    return aEncoder.Encode(basicCommissioningInfo);
}

CHIP_ERROR GeneralCommissioningAttrAccess::ReadSupportsConcurrentConnection(AttributeValueEncoder & aEncoder)
{
    SupportsConcurrentConnection::TypeInfo::Type supportsConcurrentConnection;

    // TODO: The commissioner might use the critical parameters in BasicCommissioningInfo to initialize
    // the CommissioningParameters at the beginning of commissioning flow.
    supportsConcurrentConnection = (CHIP_DEVICE_CONFIG_FAILSAFE_EXPIRY_LENGTH_SEC) != 0;

    return aEncoder.Encode(supportsConcurrentConnection);
}

} // anonymous namespace

bool emberAfGeneralCommissioningClusterArmFailSafeCallback(app::CommandHandler * commandObj,
                                                           const app::ConcreteCommandPath & commandPath,
                                                           const Commands::ArmFailSafe::DecodableType & commandData)
{
    MATTER_TRACE_EVENT_SCOPE("ArmFailSafe", "GeneralCommissioning");
    FailSafeContext & failSafeContext = DeviceLayer::DeviceControlServer::DeviceControlSvr().GetFailSafeContext();
    Commands::ArmFailSafeResponse::Type response;

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
            Server::GetInstance().GetCommissioningWindowManager().CommissioningWindowStatus() !=
                AdministratorCommissioning::CommissioningWindowStatus::kWindowNotOpen &&
            commandObj->GetSubjectDescriptor().authMode == Access::AuthMode::kCase)
        {
            response.errorCode = CommissioningError::kBusyWithOtherAdmin;
            commandObj->AddResponse(commandPath, response);
        }
        else if (commandData.expiryLengthSeconds == 0)
        {
            // Force the timer to expire immediately.
            failSafeContext.ForceFailSafeTimerExpiry();
            response.errorCode = CommissioningError::kOk;
            commandObj->AddResponse(commandPath, response);
        }
        else
        {
            CheckSuccess(
                failSafeContext.ArmFailSafe(accessingFabricIndex, System::Clock::Seconds16(commandData.expiryLengthSeconds)),
                Failure);
            response.errorCode = CommissioningError::kOk;
            commandObj->AddResponse(commandPath, response);
        }
        DeviceLayer::DeviceControlServer::DeviceControlSvr().SetBreadcrumb(commandData.breadcrumb);
    }
    else
    {
        response.errorCode = CommissioningError::kBusyWithOtherAdmin;
        commandObj->AddResponse(commandPath, response);
    }

    return true;
}

bool emberAfGeneralCommissioningClusterCommissioningCompleteCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::CommissioningComplete::DecodableType & commandData)
{
    MATTER_TRACE_EVENT_SCOPE("CommissioningComplete", "GeneralCommissioning");

    DeviceControlServer * server = &DeviceLayer::DeviceControlServer::DeviceControlSvr();
    const auto & failSafe        = server->GetFailSafeContext();

    Commands::CommissioningCompleteResponse::Type response;
    if (!failSafe.IsFailSafeArmed())
    {
        response.errorCode = CommissioningError::kNoFailSafe;
    }
    else
    {
        SessionHandle handle = commandObj->GetExchangeContext()->GetSessionHandle();
        // If not a CASE session, or the fabric does not match the fail-safe,
        // error out.
        if (handle->GetSessionType() != Session::SessionType::kSecure ||
            handle->AsSecureSession()->GetSecureSessionType() != SecureSession::Type::kCASE ||
            !failSafe.MatchesFabricIndex(commandObj->GetAccessingFabricIndex()))
        {
            response.errorCode = CommissioningError::kInvalidAuthentication;
        }
        else
        {
            /*
             * Pass fabric of commissioner to DeviceControlSvr.
             * This allows device to send messages back to commissioner.
             * Once bindings are implemented, this may no longer be needed.
             */
            CheckSuccess(server->CommissioningComplete(handle->AsSecureSession()->GetPeerNodeId(), handle->GetFabricIndex()),
                         Failure);

            response.errorCode = CommissioningError::kOk;
        }
    }

    DeviceLayer::DeviceControlServer::DeviceControlSvr().SetBreadcrumb(0);
    commandObj->AddResponse(commandPath, response);

    return true;
}

bool emberAfGeneralCommissioningClusterSetRegulatoryConfigCallback(app::CommandHandler * commandObj,
                                                                   const app::ConcreteCommandPath & commandPath,
                                                                   const Commands::SetRegulatoryConfig::DecodableType & commandData)
{
    MATTER_TRACE_EVENT_SCOPE("SetRegulatoryConfig", "GeneralCommissioning");
    DeviceControlServer * server = &DeviceLayer::DeviceControlServer::DeviceControlSvr();

    CheckSuccess(server->SetRegulatoryConfig(to_underlying(commandData.newRegulatoryConfig), commandData.countryCode,
                                             commandData.breadcrumb),
                 Failure);

    Commands::SetRegulatoryConfigResponse::Type response;
    response.errorCode = CommissioningError::kOk;
    commandObj->AddResponse(commandPath, response);

    return true;
}

void MatterGeneralCommissioningPluginServerInitCallback()
{
    DeviceLayer::DeviceControlServer::DeviceControlSvr().SetBreadcrumb(0);
    registerAttributeAccessOverride(&gAttrAccess);
}
