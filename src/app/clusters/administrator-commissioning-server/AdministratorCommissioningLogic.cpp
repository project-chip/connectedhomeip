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

    Spake2pVerifier verifier;

    ChipLogProgress(Zcl, "Received command to open commissioning window");

    const FabricInfo * fabricInfo = Server::GetInstance().GetFabricTable().FindFabricWithIndex(fabricIndex);
    auto & failSafeContext        = Server::GetInstance().GetFailSafeContext();
    auto & commissionMgr          = Server::GetInstance().GetCommissioningWindowManager();

    VerifyOrReturnError(fabricInfo != nullptr, ClusterStatusCode::ClusterSpecificFailure(StatusCode::kPAKEParameterError));
    VerifyOrReturnError(failSafeContext.IsFailSafeFullyDisarmed(), ClusterStatusCode::ClusterSpecificFailure(StatusCode::kBusy));

    VerifyOrReturnError(!commissionMgr.IsCommissioningWindowOpen(), ClusterStatusCode::ClusterSpecificFailure(StatusCode::kBusy));
    VerifyOrReturnError(iterations >= kSpake2p_Min_PBKDF_Iterations,
                        ClusterStatusCode::ClusterSpecificFailure(StatusCode::kPAKEParameterError));
    VerifyOrReturnError(iterations <= kSpake2p_Max_PBKDF_Iterations,
                        ClusterStatusCode::ClusterSpecificFailure(StatusCode::kPAKEParameterError));
    VerifyOrReturnError(salt.size() >= kSpake2p_Min_PBKDF_Salt_Length,
                        ClusterStatusCode::ClusterSpecificFailure(StatusCode::kPAKEParameterError));
    VerifyOrReturnError(salt.size() <= kSpake2p_Max_PBKDF_Salt_Length,
                        ClusterStatusCode::ClusterSpecificFailure(StatusCode::kPAKEParameterError));

    VerifyOrReturnError(commissioningTimeout <= commissionMgr.MaxCommissioningTimeout(), Status::InvalidCommand);
    VerifyOrReturnError(commissioningTimeout >= commissionMgr.MinCommissioningTimeout(), Status::InvalidCommand);
    VerifyOrReturnError(discriminator <= kMaxDiscriminatorValue, Status::InvalidCommand);

    VerifyOrReturnError(verifier.Deserialize(pakeVerifier) == CHIP_NO_ERROR,
                        ClusterStatusCode::ClusterSpecificFailure(StatusCode::kPAKEParameterError));
    VerifyOrReturnError(commissionMgr.OpenEnhancedCommissioningWindow(commissioningTimeout, discriminator, verifier, iterations,
                                                                      salt, fabricIndex,
                                                                      fabricInfo->GetVendorId()) == CHIP_NO_ERROR,
                        ClusterStatusCode::ClusterSpecificFailure(StatusCode::kPAKEParameterError));
    ChipLogProgress(Zcl, "Commissioning window is now open");
    return Status::Success;
}

DataModel::ActionReturnStatus AdministratorCommissioningLogic::OpenBasicCommissioningWindow(
    FabricIndex fabricIndex, const AdministratorCommissioning::Commands::OpenBasicCommissioningWindow::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("OpenBasicCommissioningWindow", "AdministratorCommissioning");
    auto commissioningTimeout = System::Clock::Seconds16(commandData.commissioningTimeout);

    ChipLogProgress(Zcl, "Received command to open basic commissioning window");

    const FabricInfo * fabricInfo = Server::GetInstance().GetFabricTable().FindFabricWithIndex(fabricIndex);
    auto & failSafeContext        = Server::GetInstance().GetFailSafeContext();
    auto & commissionMgr          = Server::GetInstance().GetCommissioningWindowManager();

    VerifyOrReturnError(fabricInfo != nullptr, ClusterStatusCode::ClusterSpecificFailure(StatusCode::kPAKEParameterError));

    VerifyOrReturnError(!commissionMgr.IsCommissioningWindowOpen(), ClusterStatusCode::ClusterSpecificFailure(StatusCode::kBusy));
    VerifyOrReturnError(failSafeContext.IsFailSafeFullyDisarmed(), ClusterStatusCode::ClusterSpecificFailure(StatusCode::kBusy));
    VerifyOrReturnError(commissioningTimeout <= commissionMgr.MaxCommissioningTimeout(), Status::InvalidCommand);
    VerifyOrReturnError(commissioningTimeout >= commissionMgr.MinCommissioningTimeout(), Status::InvalidCommand);
    VerifyOrReturnError(commissionMgr.OpenBasicCommissioningWindowForAdministratorCommissioningCluster(
                            commissioningTimeout, fabricIndex, fabricInfo->GetVendorId()) == CHIP_NO_ERROR,
                        ClusterStatusCode::ClusterSpecificFailure(StatusCode::kPAKEParameterError));
    ChipLogProgress(Zcl, "Commissioning window is now open");

    return Status::Success;
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
