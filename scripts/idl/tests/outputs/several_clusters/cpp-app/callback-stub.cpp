#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-id.h>
#include <lib/support/Span.h>
#include <protocols/interaction_model/Constants.h>

using namespace chip;

// Cluster Init Functions
void emberAfClusterInitCallback(EndpointId endpoint, ClusterId clusterId)
{
    switch (clusterId)
    {
    case ZCL_FIRST_CLUSTER_ID:
        emberAfFirstClusterInitCallback(endpoint);
        break;
    case ZCL_SECOND_CLUSTER_ID:
        emberAfSecondClusterInitCallback(endpoint);
        break;
    case ZCL_THIRD_CLUSTER_ID:
        emberAfThirdClusterInitCallback(endpoint);
        break;
    default:
        // Unrecognized cluster ID
        break;
    }
}
void __attribute__((weak)) emberAfFirstClusterInitCallback(EndpointId endpoint)
{
    // To prevent warning
    (void) endpoint;
}
void __attribute__((weak)) emberAfSecondClusterInitCallback(EndpointId endpoint)
{
    // To prevent warning
    (void) endpoint;
}
void __attribute__((weak)) emberAfThirdClusterInitCallback(EndpointId endpoint)
{
    // To prevent warning
    (void) endpoint;
}
