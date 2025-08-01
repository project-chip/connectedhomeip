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

#include <app/clusters/general-diagnostics-server/CodegenIntegration.h>
#include <app/clusters/general-diagnostics-server/general-diagnostics-cluster.h>
#include <app/static-cluster-config/GeneralDiagnostics.h>
#include <app/util/config.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::GeneralDiagnostics;

// for fixed endpoint, this file is ever only included IF general diagnostics is enabled and that MUST happen only on endpoint 0
// the static assert is skipped in case of dynamic endpoints.
static_assert((GeneralDiagnostics::StaticApplicationConfig::kFixedClusterConfig.size() == 1 &&
               GeneralDiagnostics::StaticApplicationConfig::kFixedClusterConfig[0].endpointNumber == kRootEndpointId) ||
              GeneralDiagnostics::StaticApplicationConfig::kFixedClusterConfig.size() == 0);

namespace {

// Determine if the configurable version of the general diagnostics cluster with additonal command options is needed
#if defined(ZCL_USING_TIME_SYNCHRONIZATION_CLUSTER_SERVER) || defined(GENERAL_DIAGNOSTICS_ENABLE_PAYLOAD_TEST_REQUEST_CMD)
LazyRegisteredServerCluster<GeneralDiagnosticsClusterFullConfigurable> gServer;
#else
LazyRegisteredServerCluster<GeneralDiagnosticsCluster> gServer;
#endif

// compile-time evaluated method if "is <EP>::GeneralDiagnostics::<ATTR>" enabled
constexpr bool IsAttributeEnabled(EndpointId endpointId, AttributeId attributeId)
{
    for (auto & config : GeneralDiagnostics::StaticApplicationConfig::kFixedClusterConfig)
    {
        if (config.endpointNumber != endpointId)
        {
            continue;
        }
        for (auto & attr : config.enabledAttributes)
        {
            if (attr == attributeId)
            {
                return true;
            }
        }
    }
    return false;
}
} // namespace

void emberAfGeneralDiagnosticsClusterInitCallback(EndpointId endpointId)
{
    VerifyOrDie(endpointId == kRootEndpointId);
    const GeneralDiagnosticsEnabledAttributes enabledAttributes{
        .enableTotalOperationalHours = IsAttributeEnabled(endpointId, Attributes::TotalOperationalHours::Id),
        .enableBootReason            = IsAttributeEnabled(endpointId, Attributes::BootReason::Id),
        .enableActiveHardwareFaults  = IsAttributeEnabled(endpointId, Attributes::ActiveHardwareFaults::Id),
        .enableActiveRadioFaults     = IsAttributeEnabled(endpointId, Attributes::ActiveRadioFaults::Id),
        .enableActiveNetworkFaults   = IsAttributeEnabled(endpointId, Attributes::ActiveNetworkFaults::Id),
    };

#if defined(ZCL_USING_TIME_SYNCHRONIZATION_CLUSTER_SERVER) || defined(GENERAL_DIAGNOSTICS_ENABLE_PAYLOAD_TEST_REQUEST_CMD)
    const GeneralDiagnosticsFunctionsConfig functionsConfig
    {
        /*
        Only consider real time if time sync cluster is actually enabled. If it's not
        enabled, this avoids likelihood of frequently reporting unusable unsynched time.
        */
        .enablePosixTime =
#if defined(ZCL_USING_TIME_SYNCHRONIZATION_CLUSTER_SERVER)
            true,
#else
            false,
#endif
        .enablePayloadSnaphot =
#if defined(GENERAL_DIAGNOSTICS_ENABLE_PAYLOAD_TEST_REQUEST_CMD)
            true,
#else
            false,
#endif
    };
    gServer.Create(enabledAttributes, functionsConfig);
#else
    gServer.Create(enabledAttributes);
#endif

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(gServer.Registration());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to register GeneralDiagnostics on endpoint %u: %" CHIP_ERROR_FORMAT, endpointId,
                     err.Format());
    }
}

void emberAfGeneralDiagnosticsClusterShutdownCallback(EndpointId endpointId)
{
    VerifyOrReturn(endpointId == kRootEndpointId);
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&gServer.Cluster());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to unregister GeneralDiagnostics on endpoint %u: %" CHIP_ERROR_FORMAT, endpointId,
                     err.Format());
    }
    gServer.Destroy();
}

void MatterGeneralDiagnosticsPluginServerInitCallback() {}

void MatterGeneralDiagnosticsPluginServerShutdownCallback() {}

namespace chip::app::Clusters::GeneralDiagnostics {
void GlobalNotifyDeviceReboot(GeneralDiagnostics::BootReasonEnum bootReason)
{
    if (gServer.IsConstructed())
    {
        gServer.Cluster().OnDeviceReboot(bootReason);
    }
}

void GlobalNotifyHardwareFaultsDetect(const DeviceLayer::GeneralFaults<DeviceLayer::kMaxHardwareFaults> & previous,
                                      const DeviceLayer::GeneralFaults<DeviceLayer::kMaxHardwareFaults> & current)
{
    if (gServer.IsConstructed())
    {
        gServer.Cluster().OnHardwareFaultsDetect(previous, current);
    }
}

void GlobalNotifyRadioFaultsDetect(const DeviceLayer::GeneralFaults<DeviceLayer::kMaxRadioFaults> & previous,
                                   const DeviceLayer::GeneralFaults<DeviceLayer::kMaxRadioFaults> & current)
{
    if (gServer.IsConstructed())
    {
        gServer.Cluster().OnRadioFaultsDetect(previous, current);
    }
}

void GlobalNotifyNetworkFaultsDetect(const DeviceLayer::GeneralFaults<DeviceLayer::kMaxNetworkFaults> & previous,
                                     const DeviceLayer::GeneralFaults<DeviceLayer::kMaxNetworkFaults> & current)
{
    if (gServer.IsConstructed())
    {
        gServer.Cluster().OnNetworkFaultsDetect(previous, current);
    }
}
} // namespace chip::app::Clusters::GeneralDiagnostics
