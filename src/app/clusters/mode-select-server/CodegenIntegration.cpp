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
#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/SafeAttributePersistenceProvider.h>
#include <app/clusters/mode-select-server/ModeSelectCluster.h>
#include <app/clusters/mode-select-server/SupportedModesManager.h>
#include <app/static-cluster-config/ModeSelect.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <app/util/util.h>
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
        return Span<const ModeSelect::Structs::ModeOptionStruct::Type>(
            provider.begin(), static_cast<size_t>(provider.end() - provider.begin()));
    }

private:
    EndpointId mEndpointId = kInvalidEndpointId;
};

// Storage for cluster instance + its delegate + description buffer
struct ModeSelectEntry
{
    // Description is a fixed attribute stored in Ember; copy it here so the CharSpan stays valid
    char descriptionBuf[ModeSelect::Attributes::Description::TypeInfo::MaxLength() + 1];
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
        ModeSelectEntry & entry = gEntries[clusterInstanceIndex];
        entry.delegate          = SupportedModesManagerDelegate(endpointId);

        // Read Description from Ember attribute store. Get() requires span size == MaxLength().
        MutableCharSpan mutableDesc(entry.descriptionBuf, Description::TypeInfo::MaxLength());
        if (Description::Get(endpointId, mutableDesc) != Status::Success)
        {
            mutableDesc = MutableCharSpan(entry.descriptionBuf, 0);
        }
        CharSpan description(entry.descriptionBuf, mutableDesc.size());

        // Read StandardNamespace from Ember attribute store
        DataModel::Nullable<uint16_t> standardNamespace;
        if (StandardNamespace::Get(endpointId, standardNamespace) != Status::Success)
        {
            standardNamespace.SetNull();
        }

        ModeSelectCluster::OptionalAttributeSet optionalAttributeSet(optionalAttributeBits);

        // Determine if OnMode overrides CurrentMode at startup (OnOff feature startup behavior)
        bool onOffValueForStartUp = false;
#ifdef MATTER_DM_PLUGIN_ON_OFF_SERVER
        if (BitMask<Feature>(featureMap).Has(Feature::kOnOff) &&
            emberAfContainsServer(endpointId, OnOff::Id) &&
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

        SafeAttributePersistenceProvider * persistenceProvider = GetSafeAttributePersistenceProvider();
        VerifyOrDie(persistenceProvider != nullptr);

        ModeSelectCluster::Config config{
            .description            = description,
            .standardNamespace      = standardNamespace,
            .featureMap             = BitMask<Feature>(featureMap),
            .optionalAttributeSet   = optionalAttributeSet,
            .onOffValueForStartUp   = onOffValueForStartUp,
            .persistenceProvider    = *persistenceProvider,
            .diagnosticDataProvider = DeviceLayer::GetDiagnosticDataProvider(),
        };

        entry.server.Create(endpointId, entry.delegate, config);
        return entry.server.Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(gEntries[clusterInstanceIndex].server.IsConstructed(), nullptr);
        return &gEntries[clusterInstanceIndex].server.Cluster();
    }

    void ReleaseRegistration(unsigned clusterInstanceIndex) override { gEntries[clusterInstanceIndex].server.Destroy(); }
};

} // namespace

// ---- Legacy ember callbacks (stubs — cluster logic handled by ModeSelectCluster directly) ----

void MatterModeSelectPluginServerInitCallback() {}
void MatterModeSelectPluginServerShutdownCallback() {}

void emberAfModeSelectClusterServerInitCallback(EndpointId endpointId)
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

Status MatterModeSelectClusterServerPreAttributeChangedCallback(const ConcreteAttributePath &, EmberAfAttributeType, uint16_t,
                                                                uint8_t *)
{
    // Attribute validation is handled by ModeSelectCluster::WriteAttribute
    return Status::Success;
}

bool emberAfModeSelectClusterChangeToModeCallback(CommandHandler *, const ConcreteCommandPath &,
                                                  const Commands::ChangeToMode::DecodableType &)
{
    // Command is handled by ModeSelectCluster::InvokeCommand in the code-driven path
    return false;
}

// ---- Backward-compat global API for app code ----

namespace chip::app::Clusters::ModeSelect {

const SupportedModesManager * getSupportedModesManager()
{
    return sSupportedModesManager;
}

void setSupportedModesManager(SupportedModesManager * mgr)
{
    sSupportedModesManager = mgr;
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
