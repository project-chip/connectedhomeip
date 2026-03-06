#include <app/clusters/zone-management-server/ZoneManagementCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <app/server/Server.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <lib/support/CodeUtils.h>

namespace {
using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ZoneManagement;

// Use LazyRegisteredServerCluster for automatic lifecycle management
LazyRegisteredServerCluster<ZoneManagementCluster> gServer;

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {

        Delegate & appDelegate = GetDelegateForEndpoint(endpointId);

        gServer.Create(ZoneManagementCluster::Context{ .delegate   = appDelegate,
                                                       .endpointId = endpointId,
                                                       .features   = aFeatures,
                                                       .config     = { .maxUserDefinedZones = aMaxUserDefinedZones,
                                                                       .maxZones            = aMaxZones,
                                                                       .sensitivityMax      = aSensitivityMax,
                                                                       .twoDCartesianMax    = aTwoDCartesianMax } });

        return gServer.Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        return gServer.IsConstructed() ? &gServer.Cluster() : nullptr;
    }

    void ReleaseRegistration(unsigned clusterInstanceIndex) override { gServer.Destroy(); }

    void Deinit()
    {
        Shutdown(kClusterShutdownType_Immediate);

        mIsRegistered = false;
    }

    void MatterZoneManagementPluginServerShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
    {
        // 1. Identify the instance being shut down
        // Since we use LazyRegisteredServerCluster, we can access the cluster instance directly.
        if (gServer.IsConstructed())
        {
            // 2. Call the explicit Deinit() to break references before destruction
            gServer.Cluster().Deinit();

            // 3. Destroy the cluster instance via the framework
            gServer.Destroy();
        }

        // 4. Perform final unregistration from the Codegen integration layer
        // (We reuse the IntegrationDelegate to identify the cluster type)
        static IntegrationDelegate integrationDelegate;
        CodegenClusterIntegration::UnregisterServer(
            {
                .endpointId                = endpointId,
                .clusterId                 = ZoneManagement::Id,
                .fixedClusterInstanceCount = 0,
                .maxClusterInstanceCount   = 1,
            },
            integrationDelegate, shutdownType);
    }
};

} // namespace

// The required callbacks the Matter SDK will call at startup/shutdown
void MatterZoneManagementPluginServerInitCallback(EndpointId endpointId)
{
    static IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = ZoneManagement::Id,
            .fixedClusterInstanceCount = 0, // Set based on your needs
            .maxClusterInstanceCount   = 1,
            .fetchFeatureMap           = false,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}
