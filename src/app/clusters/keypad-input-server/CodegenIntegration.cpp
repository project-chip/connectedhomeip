/**
 *
 *    Copyright (c) 2021-2026 Project CHIP Authors
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

#include <app/clusters/keypad-input-server/CodegenIntegration.h>

#include <app/static-cluster-config/KeypadInput.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceConfig.h>

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
#include <app/app-platform/ContentAppPlatform.h> // nogncheck
#endif                                           // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::KeypadInput;
using chip::Protocols::InteractionModel::Status;

namespace {

constexpr size_t kKeypadInputFixedClusterCount = KeypadInput::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kKeypadInputMaxClusterCount   = kKeypadInputFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

static_assert(kKeypadInputFixedClusterCount == MATTER_DM_KEYPAD_INPUT_CLUSTER_SERVER_ENDPOINT_COUNT,
              "KeypadInput static cluster config must match ZAP server endpoint count");
static_assert(kKeypadInputMaxClusterCount < kEmberInvalidEndpointIndex, "KeypadInput cluster table size error");

/// Delegate given to each KeypadInputCluster instance.
///
/// Applications set their delegate through SetDefaultDelegate after the cluster is created,
/// and ContentApp endpoints are registered with ContentAppPlatform only after the cluster is created.
/// The application delegate is therefore resolved on every call:
///
/// | Order | Delegate                             | Applies to           |
/// |-------|--------------------------------------|----------------------|
/// | 1     | ContentApp::GetKeypadInputDelegate() | ContentApp endpoints |
/// | 2     | value given to SetDefaultDelegate    | other endpoints      |
///
/// Without a delegate, FeatureMap reports the ZAP default value and SendKey returns a Failure status.
class IntegrationDelegateWrapper final : public KeypadInput::Delegate
{
public:
    void SetWrapped(KeypadInput::Delegate * wrapped) { mWrapped = wrapped; }

    void SetCluster(EndpointId endpointId, uint32_t defaultFeatureMap)
    {
        // A dynamic endpoint slot may be reused by a different endpoint, which must not get the previous delegate.
        if (endpointId != mEndpointId)
        {
            mWrapped = nullptr;
        }
        mEndpointId        = endpointId;
        mDefaultFeatureMap = defaultFeatureMap;
    }

    void HandleSendKey(CommandResponseHelper<Commands::SendKeyResponse::Type> & helper, const CECKeyCodeEnum & keyCode) override
    {
        KeypadInput::Delegate * delegate = ResolveDelegate(mEndpointId);
        if (delegate == nullptr)
        {
            LogErrorOnFailure(helper.Failure(Status::Failure));
            return;
        }
        delegate->HandleSendKey(helper, keyCode);
    }

    uint32_t GetFeatureMap(EndpointId endpoint) override
    {
        KeypadInput::Delegate * delegate = ResolveDelegate(endpoint);
        VerifyOrReturnValue(delegate != nullptr, mDefaultFeatureMap);
        return delegate->GetFeatureMap(endpoint);
    }

private:
    KeypadInput::Delegate * ResolveDelegate(EndpointId endpoint) const
    {
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
        AppPlatform::ContentApp * app = AppPlatform::ContentAppPlatform::GetInstance().GetContentApp(endpoint);
        if (app != nullptr)
        {
            return LogIfNull(app->GetKeypadInputDelegate(), endpoint);
        }
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
        return LogIfNull(mWrapped, endpoint);
    }

    static KeypadInput::Delegate * LogIfNull(KeypadInput::Delegate * delegate, EndpointId endpoint)
    {
        if (delegate == nullptr)
        {
            ChipLogProgress(Zcl, "Keypad Input has no delegate set for endpoint:%u", static_cast<unsigned>(endpoint));
        }
        return delegate;
    }

    KeypadInput::Delegate * mWrapped = nullptr;
    EndpointId mEndpointId           = kInvalidEndpointId;
    uint32_t mDefaultFeatureMap      = 0;
};

struct ClusterWithDelegate
{
    IntegrationDelegateWrapper integrationDelegateWrapper;
    LazyRegisteredServerCluster<KeypadInputCluster> server;
};

ClusterWithDelegate gClusters[kKeypadInputMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    explicit IntegrationDelegate(MatterClusterShutdownType shutdownType = MatterClusterShutdownType::kClusterShutdown) :
        mShutdownType(shutdownType)
    {}

    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        ClusterWithDelegate & entry = gClusters[clusterInstanceIndex];
        entry.integrationDelegateWrapper.SetCluster(endpointId, featureMap);
        entry.server.Create(endpointId, entry.integrationDelegateWrapper);
        return entry.server.Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(gClusters[clusterInstanceIndex].server.IsConstructed(), nullptr);
        return &gClusters[clusterInstanceIndex].server.Cluster();
    }

    void ReleaseRegistration(unsigned clusterInstanceIndex) override
    {
        gClusters[clusterInstanceIndex].server.Destroy();
        // A permanently removed dynamic endpoint frees its slot for reuse by a different endpoint.
        if (mShutdownType == MatterClusterShutdownType::kPermanentRemove)
        {
            gClusters[clusterInstanceIndex].integrationDelegateWrapper.SetWrapped(nullptr);
        }
    }

private:
    const MatterClusterShutdownType mShutdownType;
};

} // namespace

void MatterKeypadInputClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = KeypadInput::Id,
            .fixedClusterInstanceCount = kKeypadInputFixedClusterCount,
            .maxClusterInstanceCount   = kKeypadInputMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

void MatterKeypadInputClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate(shutdownType);

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = KeypadInput::Id,
            .fixedClusterInstanceCount = kKeypadInputFixedClusterCount,
            .maxClusterInstanceCount   = kKeypadInputMaxClusterCount,
        },
        integrationDelegate, shutdownType);
}

void MatterKeypadInputPluginServerInitCallback() {}

void MatterKeypadInputPluginServerShutdownCallback() {}

namespace chip {
namespace app {
namespace Clusters {
namespace KeypadInput {

// The delegate is stored next to the cluster slot instead of in the cluster, so the slot is resolved
// through ember metadata. The delegate survives the cluster being destroyed and created again.
void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t index =
        emberAfGetClusterServerEndpointIndex(endpoint, KeypadInput::Id, MATTER_DM_KEYPAD_INPUT_CLUSTER_SERVER_ENDPOINT_COUNT);
    if (index >= kKeypadInputMaxClusterCount)
    {
        ChipLogError(Zcl, "KeypadInput cluster on endpoint %u not found", static_cast<unsigned>(endpoint));
        return;
    }

    gClusters[index].integrationDelegateWrapper.SetWrapped(delegate);
}

KeypadInputCluster * FindClusterOnEndpoint(EndpointId endpoint)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * serverCluster = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpoint,
            .clusterId                 = KeypadInput::Id,
            .fixedClusterInstanceCount = kKeypadInputFixedClusterCount,
            .maxClusterInstanceCount   = kKeypadInputMaxClusterCount,
        },
        integrationDelegate);

    return static_cast<KeypadInputCluster *>(serverCluster);
}

} // namespace KeypadInput
} // namespace Clusters
} // namespace app
} // namespace chip
