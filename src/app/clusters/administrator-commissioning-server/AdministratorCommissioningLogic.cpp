/*
 *    Copyright (c) 2025 Project CHIP Authors
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
#include "AdministratorCommissioningLogic.h"

#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/DataModelTypes.h>
#include <platform/CommissionableDataProvider.h>
#include <protocols/interaction_model/StatusCode.h>
#include <tracing/macros.h>

using namespace chip::Crypto;
using namespace chip::app::Clusters::AdministratorCommissioning;
using chip::Protocols::InteractionModel::ClusterStatusCode;
using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {

DataModel::ActionReturnStatus AdministratorCommissioningLogic::OpenCommissioningWindow(
    FabricIndex fabricIndex, const AdministratorCommissioning::Commands::OpenCommissioningWindow::DecodableType & commandData)
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
        return ClusterStatusCode::ClusterSpecificFailure(status.Value());
    }

    if (globalStatus != Status::Success)
    {
        ChipLogError(Zcl, "Failed to open commissioning window. Global status " ChipLogFormatIMStatus,
                     ChipLogValueIMStatus(globalStatus));
    }
    return globalStatus;
}

DataModel::ActionReturnStatus AdministratorCommissioningLogic::OpenBasicCommissioningWindow(
    FabricIndex fabricIndex, const AdministratorCommissioning::Commands::OpenBasicCommissioningWindow::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("OpenBasicCommissioningWindow", "AdministratorCommissioning");
    auto commissioningTimeout = System::Clock::Seconds16(commandData.commissioningTimeout);

    Optional<StatusCode> status = Optional<StatusCode>::Missing();
    Status globalStatus         = Status::Success;
    ChipLogProgress(Zcl, "Received command to open basic commissioning window");

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
        return ClusterStatusCode::ClusterSpecificFailure(status.Value());
    }

    if (globalStatus != Status::Success)
    {
        ChipLogError(Zcl, "Failed to open commissioning window. Global status " ChipLogFormatIMStatus,
                     ChipLogValueIMStatus(globalStatus));
    }
    return globalStatus;
}

DataModel::ActionReturnStatus AdministratorCommissioningLogic::RevokeCommissioning(
    const AdministratorCommissioning::Commands::RevokeCommissioning::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("RevokeCommissioning", "AdministratorCommissioning");
    ChipLogProgress(Zcl, "Received command to close commissioning window");

    Server::GetInstance().GetFailSafeContext().ForceFailSafeTimerExpiry();

    if (!Server::GetInstance().GetCommissioningWindowManager().IsCommissioningWindowOpen())
    {
        ChipLogError(Zcl, "Commissioning window is currently not open");
        return ClusterStatusCode::ClusterSpecificFailure(StatusCode::kWindowNotOpen);
    }

    Server::GetInstance().GetCommissioningWindowManager().CloseCommissioningWindow();
    ChipLogProgress(Zcl, "Commissioning window is now closed");
    return Status::Success;
}

} // namespace Clusters
} // namespace app
} // namespace chip
