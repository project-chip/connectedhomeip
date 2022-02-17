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
#include <app/server/Server.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <setup_payload/SetupPayload.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AdministratorCommissioning;

// Specifications section 5.4.2.3. Announcement Duration
constexpr uint32_t kMaxCommissionioningTimeoutSeconds = 15 * 60;

class AdministratorCommissioningAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the OperationalCredentials cluster on all endpoints.
    AdministratorCommissioningAttrAccess() :
        AttributeAccessInterface(Optional<EndpointId>::Missing(), Clusters::AdministratorCommissioning::Id)
    {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

    // Vendor ID and Fabric Index of the admin that has opened the commissioning window
    uint16_t mVendorId;
    FabricIndex mFabricIndex;
};

AdministratorCommissioningAttrAccess gAdminCommissioningAttrAccess;

CHIP_ERROR AdministratorCommissioningAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == Clusters::AdministratorCommissioning::Id);

    switch (aPath.mAttributeId)
    {
    case Attributes::WindowStatus::Id: {
        return aEncoder.Encode(Server::GetInstance().GetCommissioningWindowManager().CommissioningWindowStatus());
    }
    case Attributes::AdminFabricIndex::Id: {
        FabricIndex fabricIndex = (Server::GetInstance().GetCommissioningWindowManager().CommissioningWindowStatus() ==
                                   CommissioningWindowStatus::kWindowNotOpen)
            ? 0
            : mFabricIndex;
        return aEncoder.Encode(fabricIndex);
    }
    case Attributes::AdminVendorId::Id: {
        uint16_t vendorId = (Server::GetInstance().GetCommissioningWindowManager().CommissioningWindowStatus() ==
                             CommissioningWindowStatus::kWindowNotOpen)
            ? 0
            : mVendorId;
        return aEncoder.Encode(vendorId);
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
    auto & commissioningTimeout = commandData.commissioningTimeout;
    auto & pakeVerifier         = commandData.PAKEVerifier;
    auto & discriminator        = commandData.discriminator;
    auto & iterations           = commandData.iterations;
    auto & salt                 = commandData.salt;
    auto & passcodeID           = commandData.passcodeID;

    Optional<StatusCode> status = Optional<StatusCode>::Missing();
    PASEVerifier verifier;

    ChipLogProgress(Zcl, "Received command to open commissioning window");

    FabricIndex fabricIndex = commandObj->GetAccessingFabricIndex();
    FabricInfo * fabricInfo = Server::GetInstance().GetFabricTable().FindFabricWithIndex(fabricIndex);
    VerifyOrExit(fabricInfo != nullptr, status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_PAKE_PARAMETER_ERROR));

    VerifyOrExit(Server::GetInstance().GetCommissioningWindowManager().CommissioningWindowStatus() ==
                     CommissioningWindowStatus::kWindowNotOpen,
                 status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_BUSY));
    VerifyOrExit(iterations >= Crypto::kSpake2pPBKDFMinimumIterations,
                 status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_PAKE_PARAMETER_ERROR));
    VerifyOrExit(iterations <= Crypto::kSpake2pPBKDFMaximumIterations,
                 status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_PAKE_PARAMETER_ERROR));
    VerifyOrExit(salt.size() >= Crypto::kSpake2pPBKDFMinimumSaltLen,
                 status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_PAKE_PARAMETER_ERROR));
    VerifyOrExit(salt.size() <= Crypto::kSpake2pPBKDFMaximumSaltLen,
                 status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_PAKE_PARAMETER_ERROR));
    VerifyOrExit(commissioningTimeout <= kMaxCommissionioningTimeoutSeconds,
                 status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_PAKE_PARAMETER_ERROR));
    VerifyOrExit(discriminator <= kMaxDiscriminatorValue, status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_PAKE_PARAMETER_ERROR));

    VerifyOrExit(verifier.Deserialize(pakeVerifier) == CHIP_NO_ERROR,
                 status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_PAKE_PARAMETER_ERROR));
    VerifyOrExit(Server::GetInstance().GetCommissioningWindowManager().OpenEnhancedCommissioningWindow(
                     commissioningTimeout, discriminator, verifier, iterations, salt, passcodeID) == CHIP_NO_ERROR,
                 status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_PAKE_PARAMETER_ERROR));
    ChipLogProgress(Zcl, "Commissioning window is now open");

    gAdminCommissioningAttrAccess.mFabricIndex = fabricIndex;
    gAdminCommissioningAttrAccess.mVendorId    = fabricInfo->GetVendorId();

exit:
    if (status.HasValue())
    {
        ChipLogError(Zcl, "Failed to open commissioning window. Status %d", status.Value());
        commandObj->AddClusterSpecificFailure(commandPath, status.Value());
    }
    else
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    }
    return true;
}

bool emberAfAdministratorCommissioningClusterOpenBasicCommissioningWindowCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::OpenBasicCommissioningWindow::DecodableType & commandData)
{
    auto & commissioningTimeout = commandData.commissioningTimeout;

    Optional<StatusCode> status = Optional<StatusCode>::Missing();
    ChipLogProgress(Zcl, "Received command to open basic commissioning window");

    FabricIndex fabricIndex = commandObj->GetAccessingFabricIndex();
    FabricInfo * fabricInfo = Server::GetInstance().GetFabricTable().FindFabricWithIndex(fabricIndex);
    VerifyOrExit(fabricInfo != nullptr, status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_PAKE_PARAMETER_ERROR));

    VerifyOrExit(Server::GetInstance().GetCommissioningWindowManager().CommissioningWindowStatus() ==
                     CommissioningWindowStatus::kWindowNotOpen,
                 status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_BUSY));
    VerifyOrExit(commissioningTimeout <= kMaxCommissionioningTimeoutSeconds,
                 status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_PAKE_PARAMETER_ERROR));
    VerifyOrExit(Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow(
                     commissioningTimeout, CommissioningWindowAdvertisement::kDnssdOnly) == CHIP_NO_ERROR,
                 status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_PAKE_PARAMETER_ERROR));
    ChipLogProgress(Zcl, "Commissioning window is now open");

    gAdminCommissioningAttrAccess.mFabricIndex = fabricIndex;
    gAdminCommissioningAttrAccess.mVendorId    = fabricInfo->GetVendorId();

exit:
    if (status.HasValue())
    {
        ChipLogError(Zcl, "Failed to open commissioning window. Status %d", status.Value());
        commandObj->AddClusterSpecificFailure(commandPath, status.Value());
    }
    else
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    }
    return true;
}

bool emberAfAdministratorCommissioningClusterRevokeCommissioningCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::RevokeCommissioning::DecodableType & commandData)
{
    ChipLogProgress(Zcl, "Received command to close commissioning window");

    if (Server::GetInstance().GetCommissioningWindowManager().CommissioningWindowStatus() ==
        CommissioningWindowStatus::kWindowNotOpen)
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
