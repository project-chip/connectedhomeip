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
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/ConfigurationManager.h>
#include <platform/DeviceControlServer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::GeneralCommissioning;
using namespace chip::app::Clusters::GeneralCommissioning::Attributes;
using namespace chip::DeviceLayer;

#define CheckSuccess(expr, code)                                                                                                   \
    do                                                                                                                             \
    {                                                                                                                              \
        if (!::chip::ChipError::IsSuccess(expr))                                                                                   \
        {                                                                                                                          \
            LogErrorOnFailure(commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::code));                      \
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
    basicCommissioningInfo.failSafeExpiryLengthSeconds = CHIP_DEVICE_CONFIG_FAILSAFE_EXPIRY_LENGTH_SEC;

    return aEncoder.Encode(basicCommissioningInfo);
}

} // anonymous namespace

bool emberAfGeneralCommissioningClusterArmFailSafeCallback(app::CommandHandler * commandObj,
                                                           const app::ConcreteCommandPath & commandPath,
                                                           const Commands::ArmFailSafe::DecodableType & commandData)
{
    DeviceControlServer * server = &DeviceLayer::DeviceControlServer::DeviceControlSvr();
    CheckSuccess(server->ArmFailSafe(System::Clock::Seconds16(commandData.expiryLengthSeconds)), Failure);

    Commands::ArmFailSafeResponse::Type response;
    response.errorCode = CommissioningError::kOk;
    response.debugText = CharSpan("", 0);
    CheckSuccess(commandObj->AddResponseData(commandPath, response), Failure);

    return true;
}

bool emberAfGeneralCommissioningClusterCommissioningCompleteCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::CommissioningComplete::DecodableType & commandData)
{
    DeviceControlServer * server = &DeviceLayer::DeviceControlServer::DeviceControlSvr();

    /*
     * Pass fabric and nodeId of commissioner to DeviceControlSvr.
     * This allows device to send messages back to commissioner.
     * Once bindings are implemented, this may no longer be needed.
     */
    SessionHandle handle = commandObj->GetExchangeContext()->GetSessionHandle();
    server->SetFabricIndex(handle->GetFabricIndex());
    server->SetPeerNodeId(handle->AsSecureSession()->GetPeerNodeId());

    CheckSuccess(server->CommissioningComplete(), Failure);

    Commands::CommissioningCompleteResponse::Type response;
    response.errorCode = CommissioningError::kOk;
    response.debugText = CharSpan("", 0);
    CheckSuccess(commandObj->AddResponseData(commandPath, response), Failure);

    return true;
}

bool emberAfGeneralCommissioningClusterSetRegulatoryConfigCallback(app::CommandHandler * commandObj,
                                                                   const app::ConcreteCommandPath & commandPath,
                                                                   const Commands::SetRegulatoryConfig::DecodableType & commandData)
{
    DeviceControlServer * server = &DeviceLayer::DeviceControlServer::DeviceControlSvr();

    CheckSuccess(server->SetRegulatoryConfig(to_underlying(commandData.location), commandData.countryCode, commandData.breadcrumb),
                 Failure);

    Commands::SetRegulatoryConfigResponse::Type response;
    response.errorCode = CommissioningError::kOk;
    response.debugText = CharSpan("", 0);
    CheckSuccess(commandObj->AddResponseData(commandPath, response), Failure);

    return true;
}

void MatterGeneralCommissioningPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gAttrAccess);
}
