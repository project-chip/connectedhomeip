/*
 *
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

/****************************************************************************
 * @file
 * @brief Implementation for the Joint Fabric Administrator Cluster
 ***************************************************************************/

#include "joint-fabric-administrator-server.h"

#include <access/AccessControl.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/ConcreteCommandPath.h>
#include <app/reporting/reporting.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <credentials/CHIPCert.h>
#include <credentials/CertificationDeclaration.h>
#include <credentials/DeviceAttestationConstructor.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/FabricTable.h>
#include <credentials/GroupDataProvider.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/core/PeerId.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/TestGroupData.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/CommissionableDataProvider.h>
#include <string.h>
#include <tracing/macros.h>

using namespace chip;
using namespace chip::Transport;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::JointFabricAdministrator;
using namespace chip::Credentials;
using namespace chip::Crypto;
using namespace chip::Protocols::InteractionModel;

namespace JointFabricAdministratorCluster = chip::app::Clusters::JointFabricAdministrator;

class JointFabricAdministratorAttrAccess : public AttributeAccessInterface
{
public:
    JointFabricAdministratorAttrAccess() :
        AttributeAccessInterface(Optional<EndpointId>::Missing(), JointFabricAdministratorCluster::Id)
    {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadAdministratorFabricIndex(AttributeValueEncoder & aEncoder);
};

JointFabricAdministratorAttrAccess gJointFabricAdministratorAttrAccess;

CHIP_ERROR JointFabricAdministratorAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == JointFabricAdministratorCluster::Id);

    switch (aPath.mAttributeId)
    {
    case JointFabricAdministrator::Attributes::AdministratorFabricIndex::Id: {
        return ReadAdministratorFabricIndex(aEncoder);
    }
    default:
        break;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR JointFabricAdministratorAttrAccess::ReadAdministratorFabricIndex(AttributeValueEncoder & aEncoder)
{
    return CHIP_NO_ERROR;
}

void MatterJointFabricAdministratorPluginServerInitCallback()
{
    ChipLogProgress(DataManagement, "JointFabricAdministrator: initializing");
    AttributeAccessInterfaceRegistry::Instance().Register(&gJointFabricAdministratorAttrAccess);
}

// TODO
bool emberAfJointFabricAdministratorClusterICACCSRRequestCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::JointFabricAdministrator::Commands::ICACCSRRequest::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("ICACCSRRequest", "JointFabricAdministrator");

    ChipLogProgress(Zcl, "JointFabricAdministrator: Received a ICACCSRRequest command");

    return true;
}

// TODO
bool emberAfJointFabricAdministratorClusterAddICACCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::JointFabricAdministrator::Commands::AddICAC::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("AddICAC", "JointFabricAdministrator");

    ChipLogProgress(Zcl, "JointFabricAdministrator: Received a AddICAC command");

    return true;
}

// TODO
bool emberAfJointFabricAdministratorClusterOpenJointCommissioningWindowCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::JointFabricAdministrator::Commands::OpenJointCommissioningWindow::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("OpenJointCommissioningWindow", "JointFabricAdministrator");

    ChipLogProgress(Zcl, "Received command to open joint commissioning window");

    return true;
}

// TODO
bool emberAfJointFabricAdministratorClusterTransferAnchorRequestCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::JointFabricAdministrator::Commands::TransferAnchorRequest::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("TransferAnchorRequest", "JointFabricAdministrator");

    return true;
}

// TODO
bool emberAfJointFabricAdministratorClusterTransferAnchorCompleteCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::JointFabricAdministrator::Commands::TransferAnchorComplete::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("TransferAnchorComplete", "JointFabricAdministrator");

    return true;
}

// TODO
bool emberAfJointFabricAdministratorClusterAnnounceJointFabricAdministratorCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::JointFabricAdministrator::Commands::AnnounceJointFabricAdministrator::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("AnnounceJointFabricAdministrator", "JointFabricAdministrator");

    return true;
}

JointFabricAdministratorServer JointFabricAdministratorServer::sJointFabricAdministratorServerInstance;

JointFabricAdministratorServer & JointFabricAdministratorServer::GetInstance()
{
    return sJointFabricAdministratorServerInstance;
}
