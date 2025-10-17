/**
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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/icd-management-server/ICDManagementCluster.h>
#include <app/server/Server.h>
#include <app/static-cluster-config/IcdManagement.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <zap-generated/gen_config.h>

namespace {
using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::IcdManagement;

static_assert((IcdManagement::StaticApplicationConfig::kFixedClusterConfig.size() == 1 &&
               IcdManagement::StaticApplicationConfig::kFixedClusterConfig[0].endpointNumber == kRootEndpointId) ||
              IcdManagement::StaticApplicationConfig::kFixedClusterConfig.size() == 0);

#if CHIP_CONFIG_ENABLE_ICD_CIP
LazyRegisteredServerCluster<ICDManagementClusterWithCIP> gServer;
#else
LazyRegisteredServerCluster<ICDManagementCluster> gServer;
#endif

constexpr chip::BitMask<OptionalCommands> kEnabledCommands()
{
    chip::BitMask<OptionalCommands> result;
#if defined(ICD_MANAGEMENT_STAY_ACTIVE_REQUEST_COMMAND) || defined(ICD_MANAGEMENT_STAY_ACTIVE_RESPONSE_COMMAND)
    result.Set(kStayActive);
#endif
    return result;
}

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        ICDManagementCluster::OptionalAttributeSet optionalAttributeSet(optionalAttributeBits);
        const BitMask<OptionalCommands> enabledCommands = kEnabledCommands();

        // Get UserActiveModeTriggerHint
        BitMask<IcdManagement::UserActiveModeTriggerBitmap> userActiveModeTriggerHint(0);
        if (Clusters::IcdManagement::Attributes::UserActiveModeTriggerHint::Get(endpointId, &userActiveModeTriggerHint) !=
            Protocols::InteractionModel::Status::Success)
        {
            ChipLogError(Zcl, "Failed to get UserActiveModeTriggerHint, using default (0)");
        }

        // Get UserActiveModeTriggerInstruction
        char instructionBuffer[kUserActiveModeTriggerInstructionMaxLength];
        MutableCharSpan instructionSpan(instructionBuffer, sizeof(instructionBuffer));
        CharSpan userActiveModeTriggerInstruction;

        if (Clusters::IcdManagement::Attributes::UserActiveModeTriggerInstruction::Get(endpointId, instructionSpan) !=
            Protocols::InteractionModel::Status::Success)
        {
            ChipLogError(Zcl, "Failed to get UserActiveModeTriggerInstruction, using default (empty string)");
            userActiveModeTriggerInstruction = CharSpan();
        }
        else
        {
            userActiveModeTriggerInstruction = CharSpan(instructionSpan.data(), instructionSpan.size());
        }

        gServer.Create(endpointId, Server::GetInstance().GetPersistentStorage(), *Server::GetInstance().GetSessionKeystore(),
                       Server::GetInstance().GetFabricTable(), ICDConfigurationData::GetInstance().GetInstance(),
                       optionalAttributeSet, enabledCommands, userActiveModeTriggerHint, userActiveModeTriggerInstruction);
        return gServer.Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override { return &gServer.Cluster(); }
    void ReleaseRegistration(unsigned clusterInstanceIndex) override { gServer.Destroy(); }
};
} // namespace

void MatterIcdManagementClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;
    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = IcdManagement::Id,
            .fixedClusterInstanceCount = static_cast<uint16_t>(IcdManagement::StaticApplicationConfig::kFixedClusterConfig.size()),
            .maxClusterInstanceCount   = 1, // only root-node functionality supported by this implementation
            .fetchFeatureMap           = false,
            .fetchOptionalAttributes   = true,
        },
        integrationDelegate);
}

void MatterIcdManagementClusterShutdownCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;
    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = IcdManagement::Id,
            .fixedClusterInstanceCount = static_cast<uint16_t>(IcdManagement::StaticApplicationConfig::kFixedClusterConfig.size()),
            .maxClusterInstanceCount   = 1, // only root-node functionality supported by this implementation
        },
        integrationDelegate);
}
