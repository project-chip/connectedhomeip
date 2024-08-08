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
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/DeviceControlServer.h>
#include <protocols/interaction_model/Constants.h>
#include <setup_payload/SetupPayload.h>
#include <system/SystemClock.h>
#include <tracing/macros.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AdministratorCommissioning;
using namespace chip::Protocols;
using namespace chip::Crypto;
using chip::Protocols::InteractionModel::Status;

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
    MATTER_TRACE_SCOPE("OpenCommissioningWindow", "AdministratorCommissioning");
    auto commissioningTimeout = System::Clock::Seconds16(commandData.commissioningTimeout);
    auto & pakeVerifier       = commandData.PAKEPasscodeVerifier;
    auto & discriminator      = commandData.discriminator;
    auto & iterations         = commandData.iterations;
    auto & salt               = commandData.salt;

    Optional<StatusCode> status = Optional<StatusCode>::Missing();
    Status globalStatus         = Status::Success;
    Spake2pVerifier verifier;

    ChipLogProgress(Zcl, "Received command to open commissioning window");

    FabricIndex fabricIndex       = commandObj->GetAccessingFabricIndex();
    const FabricInfo * fabricInfo = Server::GetInstance().GetFabricTable().FindFabricWithIndex(fabricIndex);
    auto & failSafeContext        = Server::GetInstance().GetFailSafeContext();
    auto & commissionMgr          = Server::GetInstance().GetCommissioningWindowManager();

    VerifyOrExit(fabricInfo != nullptr, status.Emplace(StatusCode::kPAKEParameterError));
    VerifyOrExit(failSafeContext.IsFailSafeFullyDisarmed(), status.Emplace(StatusCode::kBusy));

    VerifyOrExit(!commissionMgr.IsCommissioningWindowOpen(), status.Emplace(StatusCode::kBusy));
    VerifyOrExit(iterations >= kSpake2p_Min_PBKDF_Iterations, status.Emplace(StatusCode::kPAKEParameterError));
    VerifyOrExit(iterations <= kSpake2p_Max_PBKDF_Iterations, status.Emplace(StatusCode::kPAKEParameterError));
    VerifyOrExit(salt.size() >= kSpake2p_Min_PBKDF_Salt_Length, status.Emplace(StatusCode::kPAKEParameterError));
    VerifyOrExit(salt.size() <= kSpake2p_Max_PBKDF_Salt_Length, status.Emplace(StatusCode::kPAKEParameterError));
    VerifyOrExit(commissioningTimeout <= commissionMgr.MaxCommissioningTimeout(), globalStatus = Status::InvalidCommand);
    VerifyOrExit(commissioningTimeout >= commissionMgr.MinCommissioningTimeout(), globalStatus = Status::InvalidCommand);
    VerifyOrExit(discriminator <= kMaxDiscriminatorValue, globalStatus = Status::InvalidCommand);

    VerifyOrExit(verifier.Deserialize(pakeVerifier) == CHIP_NO_ERROR, status.Emplace(StatusCode::kPAKEParameterError));
    VerifyOrExit(commissionMgr.OpenEnhancedCommissioningWindow(commissioningTimeout, discriminator, verifier, iterations, salt,
                                                               fabricIndex, fabricInfo->GetVendorId()) == CHIP_NO_ERROR,
                 status.Emplace(StatusCode::kPAKEParameterError));
    ChipLogProgress(Zcl, "Commissioning window is now open");

exit:
    if (status.HasValue())
    {
        ChipLogError(Zcl, "Failed to open commissioning window. Cluster status 0x%02x", to_underlying(status.Value()));
        commandObj->AddClusterSpecificFailure(commandPath, to_underlying(status.Value()));
    }
    else
    {
        if (globalStatus != Status::Success)
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
    MATTER_TRACE_SCOPE("OpenBasicCommissioningWindow", "AdministratorCommissioning");
    auto commissioningTimeout = System::Clock::Seconds16(commandData.commissioningTimeout);

    Optional<StatusCode> status = Optional<StatusCode>::Missing();
    Status globalStatus         = Status::Success;
    ChipLogProgress(Zcl, "Received command to open basic commissioning window");

    FabricIndex fabricIndex       = commandObj->GetAccessingFabricIndex();
    const FabricInfo * fabricInfo = Server::GetInstance().GetFabricTable().FindFabricWithIndex(fabricIndex);
    auto & failSafeContext        = Server::GetInstance().GetFailSafeContext();
    auto & commissionMgr          = Server::GetInstance().GetCommissioningWindowManager();

    VerifyOrExit(fabricInfo != nullptr, status.Emplace(StatusCode::kPAKEParameterError));

    VerifyOrExit(!commissionMgr.IsCommissioningWindowOpen(), status.Emplace(StatusCode::kBusy));
    VerifyOrExit(failSafeContext.IsFailSafeFullyDisarmed(), status.Emplace(StatusCode::kBusy));
    VerifyOrExit(commissioningTimeout <= commissionMgr.MaxCommissioningTimeout(), globalStatus = Status::InvalidCommand);
    VerifyOrExit(commissioningTimeout >= commissionMgr.MinCommissioningTimeout(), globalStatus = Status::InvalidCommand);
    VerifyOrExit(commissionMgr.OpenBasicCommissioningWindowForAdministratorCommissioningCluster(
                     commissioningTimeout, fabricIndex, fabricInfo->GetVendorId()) == CHIP_NO_ERROR,
                 status.Emplace(StatusCode::kPAKEParameterError));
    ChipLogProgress(Zcl, "Commissioning window is now open");

exit:
    if (status.HasValue())
    {
        ChipLogError(Zcl, "Failed to open commissioning window. Cluster status 0x%02x", to_underlying(status.Value()));
        commandObj->AddClusterSpecificFailure(commandPath, to_underlying(status.Value()));
    }
    else
    {
        if (globalStatus != Status::Success)
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
    MATTER_TRACE_SCOPE("RevokeCommissioning", "AdministratorCommissioning");
    ChipLogProgress(Zcl, "Received command to close commissioning window");

    Server::GetInstance().GetFailSafeContext().ForceFailSafeTimerExpiry();

    if (!Server::GetInstance().GetCommissioningWindowManager().IsCommissioningWindowOpen())
    {
        ChipLogError(Zcl, "Commissioning window is currently not open");
        commandObj->AddClusterSpecificFailure(commandPath, to_underlying(StatusCode::kWindowNotOpen));
    }
    else
    {
        Server::GetInstance().GetCommissioningWindowManager().CloseCommissioningWindow();
        ChipLogProgress(Zcl, "Commissioning window is now closed");
        commandObj->AddStatus(commandPath, Status::Success);
    }
    return true;
}

void MatterAdministratorCommissioningPluginServerInitCallback()
{
    ChipLogProgress(Zcl, "Initiating Admin Commissioning cluster.");
    AttributeAccessInterfaceRegistry::Instance().Register(&gAdminCommissioningAttrAccess);
}
