/*
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
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

#include "CodegenIntegration.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/mode-select-server/ModeSelectCluster.h>
#include <app/clusters/mode-select-server/SupportedModesManager.h>

// ManufacturerExtension (0xFFF10001) is a MEI test extension defined in
// mode-select-extensions.xml. Apps using zcl-with-test-extensions.json (e.g. Realtek
// all-clusters-app) include it, causing the generated static-cluster-config/ModeSelect.h
// to reference Attributes::ManufacturerExtension::Id which is not in standard AttributeIds.h.
namespace chip::app::Clusters::ModeSelect::Attributes::ManufacturerExtension {
inline constexpr AttributeId Id = 0xFFF10001u;
} // namespace chip::app::Clusters::ModeSelect::Attributes::ManufacturerExtension

#include <app/static-cluster-config/ModeSelect.h>
#include <app/util/attribute-storage.h>
#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
#include <app/clusters/scenes-server/CodegenIntegration.h>
#endif
#include <app/util/endpoint-config-api.h>
#include <app/util/util.h>
#include <clusters/ModeSelect/Attributes.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <platform/DiagnosticDataProvider.h>

#ifdef MATTER_DM_PLUGIN_ON_OFF_SERVER
#include <app/clusters/on-off-server/on-off-server.h>
#endif

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ModeSelect;
using namespace chip::app::Clusters::ModeSelect::Attributes;
using Status = Protocols::InteractionModel::Status;

namespace {

// Global backward-compat pointer for app code that calls setSupportedModesManager()
ModeSelect::SupportedModesManager * sSupportedModesManager = nullptr;

// Adapts SupportedModesManager to ModeSelectCluster::Delegate for a single endpoint
class SupportedModesManagerDelegate : public ModeSelectCluster::Delegate
{
public:
    SupportedModesManagerDelegate() = default;
    explicit SupportedModesManagerDelegate(EndpointId endpointId) : mEndpointId(endpointId) {}

    Span<const ModeSelect::Structs::ModeOptionStruct::Type> GetSupportedModes() const override
    {
        const SupportedModesManager * mgr = sSupportedModesManager;
        if (mgr == nullptr)
        {
            return Span<const ModeSelect::Structs::ModeOptionStruct::Type>();
        }
        auto provider = mgr->getModeOptionsProvider(mEndpointId);
        if (provider.begin() == nullptr)
        {
            return Span<const ModeSelect::Structs::ModeOptionStruct::Type>();
        }
        return Span<const ModeSelect::Structs::ModeOptionStruct::Type>(provider.begin(),
                                                                       static_cast<size_t>(provider.end() - provider.begin()));
    }

private:
    EndpointId mEndpointId = kInvalidEndpointId;
};

struct ModeSelectEntry
{
    char descriptionBuffer[64]; // persistent backing store for Description attribute
    SupportedModesManagerDelegate delegate;
    LazyRegisteredServerCluster<ModeSelectCluster> server;
};

constexpr size_t kFixedClusterCount = ModeSelect::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kMaxClusterCount   = kFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

ModeSelectEntry gEntries[kMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        VerifyOrDie(clusterInstanceIndex < kMaxClusterCount);
        ModeSelectEntry & entry = gEntries[clusterInstanceIndex];
        entry.delegate          = SupportedModesManagerDelegate(endpointId);

        ModeSelectCluster::OptionalAttributeSet optionalAttributeSet(optionalAttributeBits);

        // Determine if OnMode overrides CurrentMode at startup (OnOff feature startup behavior)
        bool onOffValueForStartUp = false;
#ifdef MATTER_DM_PLUGIN_ON_OFF_SERVER
        if (BitMask<Feature>(featureMap).Has(Feature::kOnOff) && emberAfContainsServer(endpointId, OnOff::Id) &&
            emberAfContainsAttribute(endpointId, OnOff::Id, OnOff::Attributes::StartUpOnOff::Id) &&
            !emberAfIsKnownVolatileAttribute(endpointId, OnOff::Id, OnOff::Attributes::StartUpOnOff::Id))
        {
            bool startUpValue = false;
            if (OnOffServer::Instance().getOnOffValueForStartUp(endpointId, startUpValue) == Status::Success)
            {
                onOffValueForStartUp = startUpValue;
            }
        }
#endif

        // Read Description from Ember RAM storage (ZAP default = "Coffee" in all-clusters-app).
        // descriptionBuffer is in gEntries (global) so the CharSpan remains valid for the cluster lifetime.
        chip::MutableCharSpan descSpan(entry.descriptionBuffer, sizeof(entry.descriptionBuffer));
        if (Attributes::Description::GetDefault(endpointId, descSpan) != Status::Success)
        {
            descSpan = chip::MutableCharSpan(entry.descriptionBuffer, 0);
        }

        // Read StandardNamespace from Ember RAM storage (ZAP default = null).
        DataModel::Nullable<uint16_t> standardNamespace;
        if (Attributes::StandardNamespace::GetDefault(endpointId, standardNamespace) != Status::Success)
        {
            standardNamespace.SetNull();
        }

        // StartUpMode and OnMode are EXTERNAL_STORAGE (NVM) in code-driven clusters;
        // Ember raw reads do not work for them. Provide ZAP defaults directly:
        //   StartUpMode default = 0 (first mode, per all-clusters ZAP config)
        //   OnMode default = null (ZAP default 255 = null sentinel, meaning no override)
        DataModel::Nullable<uint8_t> initialStartUpMode = optionalAttributeSet.IsSet(StartUpMode::Id)
            ? DataModel::MakeNullable(static_cast<uint8_t>(0))
            : DataModel::NullNullable;
        DataModel::Nullable<uint8_t> initialOnMode      = DataModel::NullNullable;

        ModeSelectCluster::Config config{
            .featureMap             = BitMask<Feature>(featureMap),
            .optionalAttributeSet   = optionalAttributeSet,
            .description            = chip::CharSpan(entry.descriptionBuffer, descSpan.size()),
            .standardNamespace      = standardNamespace,
            .onOffValueForStartUp   = onOffValueForStartUp,
            .initialStartUpMode     = initialStartUpMode,
            .initialOnMode          = initialOnMode,
            .diagnosticDataProvider = DeviceLayer::GetDiagnosticDataProvider(),
        };

        entry.server.Create(endpointId, entry.delegate, config);
#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
        ScenesManagement::ScenesServer::Instance().RegisterSceneHandler(endpointId, &entry.server.Cluster());
#endif
        return entry.server.Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(clusterInstanceIndex < kMaxClusterCount, nullptr);
        VerifyOrReturnValue(gEntries[clusterInstanceIndex].server.IsConstructed(), nullptr);
        return &gEntries[clusterInstanceIndex].server.Cluster();
    }

    void ReleaseRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturn(clusterInstanceIndex < kMaxClusterCount);
        if (gEntries[clusterInstanceIndex].server.IsConstructed())
        {
#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
            ModeSelectCluster & cluster = gEntries[clusterInstanceIndex].server.Cluster();
            EndpointId endpointId       = cluster.GetPaths()[0].mEndpointId;
            ScenesManagement::ScenesServer::Instance().UnregisterSceneHandler(endpointId, &cluster);
#endif
            gEntries[clusterInstanceIndex].server.Destroy();
        }
    }
};

} // namespace

// ---- Code-driven init/shutdown callbacks (generated by ZAP CodeDrivenCallback.h) ----

void MatterModeSelectClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = ModeSelect::Id,
            .fixedClusterInstanceCount = kFixedClusterCount,
            .maxClusterInstanceCount   = kMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = true,
        },
        integrationDelegate);
}

void MatterModeSelectClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = ModeSelect::Id,
            .fixedClusterInstanceCount = kFixedClusterCount,
            .maxClusterInstanceCount   = kMaxClusterCount,
        },
        integrationDelegate, shutdownType);
}

// ---- Legacy ember callbacks (stubs) ----

void MatterModeSelectPluginServerInitCallback() {}
void MatterModeSelectPluginServerShutdownCallback() {}

void emberAfModeSelectClusterServerInitCallback(EndpointId endpointId) {}

// ---- Backward-compat global API for app code ----

namespace chip::app::Clusters::ModeSelect {

const SupportedModesManager * getSupportedModesManager()
{
    return sSupportedModesManager;
}

void setSupportedModesManager(SupportedModesManager * mgr)
{
    sSupportedModesManager = mgr;
    // Apply startup mode logic to all clusters that started before the manager was set.
    // (Example apps call this in ApplicationInit, after Server::Init.)
    for (auto & entry : gEntries)
    {
        if (entry.server.IsConstructed())
        {
            entry.server.Cluster().ApplyStartupModeLogic();
        }
    }
}

ModeSelectCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * cluster = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = ModeSelect::Id,
            .fixedClusterInstanceCount = kFixedClusterCount,
            .maxClusterInstanceCount   = kMaxClusterCount,
        },
        integrationDelegate);

    return static_cast<ModeSelectCluster *>(cluster);
}

} // namespace chip::app::Clusters::ModeSelect
