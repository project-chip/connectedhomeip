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

#include <app/clusters/descriptor/DescriptorCluster.h>

#if !CHIP_CONFIG_SKIP_APP_SPECIFIC_GENERATED_HEADER_INCLUDES
#include <app/static-cluster-config/Descriptor.h>
#endif // CHIP_CONFIG_SKIP_APP_SPECIFIC_GENERATED_HEADER_INCLUDES

#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <lib/core/CHIPConfig.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Descriptor;

namespace {

/**
 * This is a DescriptorCluster class made specifically to fetch the tag list once through ember before one of either the
 * Attributes() or ReadAttribute() functions are called. This can NOT be called before endpoint init and passed to the constructor
 * of the regular DescriptorCluster class. This is because for fixed endpoints, we define endpoints in emberAfEndpointConfigure()
 * and init them in emberAfInit() with back to back calls in InitDataModelHandler(). For dynamic endpoints, we init endpoints in
 * emberAfSetDynamicEndpointWithEpUniqueId() by calling emberAfEndpointEnableDisable(), which calls initializeEndpoint(). The tag
 * list is a fixed attribute, but to maintain backwards compatiblility we get that information within the functions here.
 */
class EmberDescriptorCluster : public DescriptorCluster
{
public:
    EmberDescriptorCluster(EndpointId endpointId, DescriptorCluster::OptionalAttributesSet optionalAttributeSet,
                           Span<const SemanticTag> semanticTags) :
        DescriptorCluster(endpointId, optionalAttributeSet, semanticTags)
    {}

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override
    {
        if (!mFetchedSemanticTags)
        {
            GetSemanticTagsForEndpoint(path.mEndpointId, mSemanticTags);
            mFetchedSemanticTags = true;
        }
        return DescriptorCluster::Attributes(path, builder);
    }

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override
    {
        if (!mFetchedSemanticTags)
        {
            GetSemanticTagsForEndpoint(request.path.mEndpointId, mSemanticTags);
            mFetchedSemanticTags = true;
        }
        return DescriptorCluster::ReadAttribute(request, encoder);
    }

private:
    bool mFetchedSemanticTags = false;
};

#if CHIP_CONFIG_SKIP_APP_SPECIFIC_GENERATED_HEADER_INCLUDES
static constexpr size_t kDescriptorFixedClusterCount = 0;
#else
static constexpr size_t kDescriptorFixedClusterCount = Descriptor::StaticApplicationConfig::kFixedClusterConfig.size();
#endif
static constexpr size_t kDescriptorMaxClusterCount = kDescriptorFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<EmberDescriptorCluster> gServers[kDescriptorMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        gServers[clusterInstanceIndex].Create(endpointId, DescriptorCluster::OptionalAttributesSet(optionalAttributeBits),
                                              Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>());
        return gServers[clusterInstanceIndex].Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(gServers[clusterInstanceIndex].IsConstructed(), nullptr);
        return &gServers[clusterInstanceIndex].Cluster();
    }
    void ReleaseRegistration(unsigned clusterInstanceIndex) override { gServers[clusterInstanceIndex].Destroy(); }
};
} // namespace

void MatterDescriptorClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = Descriptor::Id,
            .fixedClusterInstanceCount = kDescriptorFixedClusterCount,
            .maxClusterInstanceCount   = kDescriptorMaxClusterCount,
            .fetchFeatureMap           = false,
            .fetchOptionalAttributes   = true,
        },
        integrationDelegate);
}

void MatterDescriptorClusterShutdownCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = Descriptor::Id,
            .fixedClusterInstanceCount = kDescriptorFixedClusterCount,
            .maxClusterInstanceCount   = kDescriptorMaxClusterCount,
        },
        integrationDelegate);
}

void MatterDescriptorPluginServerInitCallback() {}

void MatterDescriptorPluginServerShutdownCallback() {}
