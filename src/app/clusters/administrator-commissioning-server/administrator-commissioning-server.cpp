/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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
 * @brief Implementation for the Administrator Commissioning Cluster
 ***************************************************************************/

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/DeviceControlServer.h>
#include <protocols/interaction_model/Constants.h>
#include <setup_payload/SetupPayload.h>
#include <system/SystemClock.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AdministratorCommissioning;
using namespace chip::Protocols;

class AdministratorCommissioningAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the OperationalCredentials cluster on all endpoints.
    AdministratorCommissioningAttrAccess() :
        AttributeAccessInterface(Optional<EndpointId>::Missing(), Clusters::AdministratorCommissioning::Id)
    {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
};

AdministratorCommissioningAttrAccess gAdminCommissioningAttrAccess;

CHIP_ERROR AdministratorCommissioningAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == Clusters::AdministratorCommissioning::Id);

    switch (aPath.mAttributeId)
    {
    case Attributes::WindowStatus::Id: {
        return aEncoder.Encode(Server::GetInstance().GetCommissioningWindowManager().CommissioningWindowStatusForCluster());
    }
    case Attributes::AdminFabricIndex::Id: {
        return aEncoder.Encode(Server::GetInstance().GetCommissioningWindowManager().GetOpenerFabricIndex());
    }
    case Attributes::AdminVendorId::Id: {
        return aEncoder.Encode(Server::GetInstance().GetCommissioningWindowManager().GetOpenerVendorId());
    }
    default:
        break;
    }

    return CHIP_NO_ERROR;
}

bool emberAfAdministratorCommissioningClusterOpenCommissioningWindowCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::OpenCommissioningWindow::DecodableType & commandData)
{
    auto commissioningTimeout = System::Clock::Seconds16(commandData.commissioningTimeout);
    auto & pakeVerifier       = commandData.PAKEVerifier;
    auto & discriminator      = commandData.discriminator;
    auto & iterations         = commandData.iterations;
    auto & salt               = commandData.salt;

    Optional<StatusCode> status           = Optional<StatusCode>::Missing();
    InteractionModel::Status globalStatus = InteractionModel::Status::Success;
    Spake2pVerifier verifier;

    ChipLogProgress(Zcl, "Received command to open commissioning window");

    FabricIndex fabricIndex       = commandObj->GetAccessingFabricIndex();
    const FabricInfo * fabricInfo = Server::GetInstance().GetFabricTable().FindFabricWithIndex(fabricIndex);
    auto & failSafeContext        = Server::GetInstance().GetFailSafeContext();
    auto & commissionMgr          = Server::GetInstance().GetCommissioningWindowManager();

    VerifyOrExit(fabricInfo != nullptr, status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_PAKE_PARAMETER_ERROR));
    VerifyOrExit(failSafeContext.IsFailSafeFullyDisarmed(), status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_BUSY));

    VerifyOrExit(!commissionMgr.IsCommissioningWindowOpen(), status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_BUSY));
    VerifyOrExit(iterations >= kSpake2p_Min_PBKDF_Iterations,
                 status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_PAKE_PARAMETER_ERROR));
    VerifyOrExit(iterations <= kSpake2p_Max_PBKDF_Iterations,
                 status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_PAKE_PARAMETER_ERROR));
    VerifyOrExit(salt.size() >= kSpake2p_Min_PBKDF_Salt_Length,
                 status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_PAKE_PARAMETER_ERROR));
    VerifyOrExit(salt.size() <= kSpake2p_Max_PBKDF_Salt_Length,
                 status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_PAKE_PARAMETER_ERROR));
    VerifyOrExit(commissioningTimeout <= commissionMgr.MaxCommissioningTimeout(),
                 globalStatus = InteractionModel::Status::InvalidCommand);
    VerifyOrExit(commissioningTimeout >= commissionMgr.MinCommissioningTimeout(),
                 globalStatus = InteractionModel::Status::InvalidCommand);
    VerifyOrExit(discriminator <= kMaxDiscriminatorValue, globalStatus = InteractionModel::Status::InvalidCommand);

    VerifyOrExit(verifier.Deserialize(pakeVerifier) == CHIP_NO_ERROR,
                 status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_PAKE_PARAMETER_ERROR));
    VerifyOrExit(commissionMgr.OpenEnhancedCommissioningWindow(commissioningTimeout, discriminator, verifier, iterations, salt,
                                                               fabricIndex, fabricInfo->GetVendorId()) == CHIP_NO_ERROR,
                 status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_PAKE_PARAMETER_ERROR));
    ChipLogProgress(Zcl, "Commissioning window is now open");

exit:
    if (status.HasValue())
    {
        ChipLogError(Zcl, "Failed to open commissioning window. Cluster status %d", status.Value());
        commandObj->AddClusterSpecificFailure(commandPath, status.Value());
    }
    else
    {
        if (globalStatus != InteractionModel::Status::Success)
        {
            ChipLogError(Zcl, "Failed to open commissioning window. Global status " ChipLogFormatIMStatus,
                         ChipLogValueIMStatus(globalStatus));
        }
        commandObj->AddStatus(commandPath, globalStatus);
    }
    return true;
}

bool emberAfAdministratorCommissioningClusterOpenBasicCommissioningWindowCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::OpenBasicCommissioningWindow::DecodableType & commandData)
{
    auto commissioningTimeout = System::Clock::Seconds16(commandData.commissioningTimeout);

    Optional<StatusCode> status           = Optional<StatusCode>::Missing();
    InteractionModel::Status globalStatus = InteractionModel::Status::Success;
    ChipLogProgress(Zcl, "Received command to open basic commissioning window");

    FabricIndex fabricIndex       = commandObj->GetAccessingFabricIndex();
    const FabricInfo * fabricInfo = Server::GetInstance().GetFabricTable().FindFabricWithIndex(fabricIndex);
    auto & failSafeContext        = Server::GetInstance().GetFailSafeContext();
    auto & commissionMgr          = Server::GetInstance().GetCommissioningWindowManager();

    VerifyOrExit(fabricInfo != nullptr, status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_PAKE_PARAMETER_ERROR));

    VerifyOrExit(!commissionMgr.IsCommissioningWindowOpen(), status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_BUSY));
    VerifyOrExit(failSafeContext.IsFailSafeFullyDisarmed(), status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_BUSY));
    VerifyOrExit(commissioningTimeout <= commissionMgr.MaxCommissioningTimeout(),
                 globalStatus = InteractionModel::Status::InvalidCommand);
    VerifyOrExit(commissioningTimeout >= commissionMgr.MinCommissioningTimeout(),
                 globalStatus = InteractionModel::Status::InvalidCommand);
    VerifyOrExit(commissionMgr.OpenBasicCommissioningWindowForAdministratorCommissioningCluster(
                     commissioningTimeout, fabricIndex, fabricInfo->GetVendorId()) == CHIP_NO_ERROR,
                 status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_PAKE_PARAMETER_ERROR));
    ChipLogProgress(Zcl, "Commissioning window is now open");

exit:
    if (status.HasValue())
    {
        ChipLogError(Zcl, "Failed to open commissioning window. Cluster status %d", status.Value());
        commandObj->AddClusterSpecificFailure(commandPath, status.Value());
    }
    else
    {
        if (globalStatus != InteractionModel::Status::Success)
        {
            ChipLogError(Zcl, "Failed to open commissioning window. Global status " ChipLogFormatIMStatus,
                         ChipLogValueIMStatus(globalStatus));
        }
        commandObj->AddStatus(commandPath, globalStatus);
    }
    return true;
}

bool emberAfAdministratorCommissioningClusterRevokeCommissioningCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::RevokeCommissioning::DecodableType & commandData)
{
    ChipLogProgress(Zcl, "Received command to close commissioning window");

    if (!Server::GetInstance().GetCommissioningWindowManager().IsCommissioningWindowOpen())
    {
        ChipLogError(Zcl, "Commissioning window is currently not open");
        commandObj->AddClusterSpecificFailure(commandPath, StatusCode::EMBER_ZCL_STATUS_CODE_WINDOW_NOT_OPEN);
    }
    else
    {
        Server::GetInstance().GetCommissioningWindowManager().CloseCommissioningWindow();
        ChipLogProgress(Zcl, "Commissioning window is now closed");
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    }
    return true;
}

void MatterAdministratorCommissioningPluginServerInitCallback()
{
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "Initiating Admin Commissioning cluster.");
    registerAttributeAccessOverride(&gAdminCommissioningAttrAccess);
}
