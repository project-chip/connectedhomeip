#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <lib/support/Span.h>
#include <protocols/interaction_model/Constants.h>

using namespace chip;

// Cluster Init Functions
void emberAfClusterInitCallback(EndpointId endpoint, ClusterId clusterId)
{
    switch (clusterId)
    {
    case app::Clusters::First::Id:
        emberAfFirstClusterInitCallback(endpoint);
        break;
    case app::Clusters::Second::Id:
        emberAfSecondClusterInitCallback(endpoint);
        break;
    case app::Clusters::Third::Id:
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
