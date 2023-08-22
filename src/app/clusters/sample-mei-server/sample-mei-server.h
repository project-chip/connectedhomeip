#ifndef SRC_APP_CLUSTERS_SAMPLE_MEI_CLUSTER_SERVER_SERVER_H_
#define SRC_APP_CLUSTERS_SAMPLE_MEI_CLUSTER_SERVER_SERVER_H_

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af-types.h>
#include <app/util/basic-types.h>

#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace SampleMei {

class SampleMeiServer : public CommandHandlerInterface, public AttributeAccessInterface
{
public:
    SampleMeiServer(EndpointId aEndpointId) :
        CommandHandlerInterface(Optional<EndpointId>(aEndpointId), Id),
        AttributeAccessInterface(Optional<EndpointId>(aEndpointId), Id)
    {}

    CHIP_ERROR Init();

    // CommandHandlerInterface
    void InvokeCommand(HandlerContext & ctx) override;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

private:
    bool flipflop;
};
} // namespace SampleMei
} // namespace Clusters
} // namespace app
} // namespace chip

#endif // SRC_APP_CLUSTERS_SAMPLE_MEI_CLUSTER_SERVER_SERVER_H_
