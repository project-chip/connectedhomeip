#ifndef SRC_APP_CLUSTERS_SAMPLE_MANUFACTURER_SPECIFIC_CLUSTER_SERVER_SERVER_H_
#define SRC_APP_CLUSTERS_SAMPLE_MANUFACTURER_SPECIFIC_CLUSTER_SERVER_SERVER_H_

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af-types.h>
#include <app/util/basic-types.h>
#include <app/util/config.h>
#include <lib/support/Span.h>
#include <platform/CHIPDeviceConfig.h>

#ifdef ZCL_USING_SAMPLE_MANUFACTURER_SPECIFIC_CLUSTER_SERVER
#define SAMPLE_MANUFACTURER_SPECIFIC_NUM_SUPPORTED_ENDPOINTS                                                                                         \
    (MATTER_DM_SAMPLE_MANUFACTURER_SPECIFIC_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
#else
#define SAMPLE_MANUFACTURER_SPECIFIC_NUM_SUPPORTED_ENDPOINTS CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT
#endif /* ZCL_USING_SAMPLE_MANUFACTURER_SPECIFIC_CLUSTER_SERVER */
static constexpr size_t kNumSupportedEndpoints = SAMPLE_MANUFACTURER_SPECIFIC_NUM_SUPPORTED_ENDPOINTS;

namespace chip {
namespace app {
namespace Clusters {
namespace SampleManufacturerSpecific {

// *****************************************************************************
// SampleManufacturerSpecificContent has the stateful attributes of the cluster: its endpoint
// and attributes

class SampleManufacturerSpecificContent
{
public:
    EndpointId endpoint;
    uint32_t pingCount;

    // Attribute List
    bool flipflop; /* Attributes::FlipFlop::Id */

    SampleManufacturerSpecificContent(EndpointId endpoint);
    SampleManufacturerSpecificContent();
};

// *****************************************************************************
// SampleManufacturerSpecificServer implements both Attributes and Commands

class SampleManufacturerSpecificServer : public AttributeAccessInterface, public CommandHandlerInterface
{
public:
    // Register on all endpoints.
    SampleManufacturerSpecificServer() :
        AttributeAccessInterface(Optional<EndpointId>::Missing(), SampleManufacturerSpecific::Id),
        CommandHandlerInterface(Optional<EndpointId>(), Id)
    {}
    static SampleManufacturerSpecificServer & Instance();

    // Currently not used, but should be called from a whole-cluster shutdown
    // callback once cluster lifecycle is clearer
    void Shutdown();

    // Attributes
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

    // Commands
    void InvokeCommand(HandlerContext & ctx) override;

    // Attribute storage
#if SAMPLE_MANUFACTURER_SPECIFIC_NUM_SUPPORTED_ENDPOINTS > 0
    SampleManufacturerSpecificContent content[kNumSupportedEndpoints];
#else
    SampleManufacturerSpecificContent * content = nullptr;
#endif

    size_t GetNumSupportedEndpoints() const;
    CHIP_ERROR RegisterEndpoint(EndpointId endpointId);
    CHIP_ERROR UnregisterEndpoint(EndpointId endpointId);

private:
    // both return std::numeric_limits<size_t>::max() for not found
    size_t EndpointIndex(EndpointId endpointId) const;
    size_t NextEmptyIndex() const;
};

} // namespace SampleManufacturerSpecific
} // namespace Clusters
} // namespace app
} // namespace chip

#endif // SRC_APP_CLUSTERS_SAMPLE_MANUFACTURER_SPECIFIC_CLUSTER_SERVER_SERVER_H_
