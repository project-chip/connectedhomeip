#ifndef SRC_APP_CLUSTERS_CHEF_TEST_CLUSTER_SERVER_CHEF_TEST_CLUSTER_SERVER_H_
#define SRC_APP_CLUSTERS_CHEF_TEST_CLUSTER_SERVER_CHEF_TEST_CLUSTER_SERVER_H_

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
namespace ChefTestCluster {

class ChefTestClusterServer : public CommandHandlerInterface, public AttributeAccessInterface
{
public:
    ChefTestClusterServer(EndpointId aEndpointId) :
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
    bool attribute1;
};
} // namespace ChefTestCluster
} // namespace Clusters
} // namespace app
} // namespace chip

#endif // SRC_APP_CLUSTERS_CHEF_TEST_CLUSTER_SERVER_CHEF_TEST_CLUSTER_SERVER_H_
