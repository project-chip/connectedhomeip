#ifndef SRC_APP_CLUSTERS_SAMPLE_MEI_CLUSTER_SERVER_SERVER_H_
#define SRC_APP_CLUSTERS_SAMPLE_MEI_CLUSTER_SERVER_SERVER_H_

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af-types.h>
#include <app/util/basic-types.h>
#include <app/util/config.h>
#include <lib/support/Span.h>
#include <platform/CHIPDeviceConfig.h>

#ifdef ZCL_USING_SAMPLE_MEI_CLUSTER_SERVER
#define SAMPLE_MEI_NUM_SUPPORTED_ENDPOINTS                                                                                         \
    (MATTER_DM_SAMPLE_MEI_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
#else
#define SAMPLE_MEI_NUM_SUPPORTED_ENDPOINTS CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT
#endif /* ZCL_USING_SAMPLE_MEI_CLUSTER_SERVER */
static constexpr size_t kNumSupportedEndpoints = SAMPLE_MEI_NUM_SUPPORTED_ENDPOINTS;

namespace chip {
namespace app {
namespace Clusters {
namespace SampleMei {

// *****************************************************************************
// SampleMeiContent has the stateful attributes of the cluster: its endpoint
// and attributes

class SampleMeiContent
{
public:
    EndpointId endpoint;
    uint32_t pingCount;

    // Attribute List
    bool flipflop; /* Attributes::FlipFlop::Id */

    SampleMeiContent(EndpointId endpoint);
    SampleMeiContent();
};

// *****************************************************************************
// SampleMeiServer implements both Attributes and Commands

class SampleMeiServer : public AttributeAccessInterface, public CommandHandlerInterface
{
public:
    // Register on all endpoints.
    SampleMeiServer() :
        AttributeAccessInterface(Optional<EndpointId>::Missing(), SampleMei::Id),
        CommandHandlerInterface(Optional<EndpointId>(), Id)
    {}
    static SampleMeiServer & Instance();

    // Currently not used, but should be called from a whole-cluster shutdown
    // callback once cluster lifecycle is clearer
    void Shutdown();

    // Attributes
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

    // Commands
    void InvokeCommand(HandlerContext & ctx) override;

    // Attribute storage
#if SAMPLE_MEI_NUM_SUPPORTED_ENDPOINTS > 0
    SampleMeiContent content[kNumSupportedEndpoints];
#else
    SampleMeiContent * content = nullptr;
#endif

    size_t GetNumSupportedEndpoints() const;
    CHIP_ERROR RegisterEndpoint(EndpointId endpointId);
    CHIP_ERROR UnregisterEndpoint(EndpointId endpointId);

private:
    // both return std::numeric_limits<size_t>::max() for not found
    size_t EndpointIndex(EndpointId endpointId) const;
    size_t NextEmptyIndex() const;
};

} // namespace SampleMei
} // namespace Clusters
} // namespace app
} // namespace chip

#endif // SRC_APP_CLUSTERS_SAMPLE_MEI_CLUSTER_SERVER_SERVER_H_
