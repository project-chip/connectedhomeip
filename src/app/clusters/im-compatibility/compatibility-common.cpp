#include "compatibility-common.h"

#include <lib/core/CHIPCore.h>
#include <util/util.h>

namespace chip {
namespace app {
namespace Compatibility {
EmberAfClusterCommand imCompatibilityEmberAfCluster;
EmberApsFrame imCompatibilityEmberApsFrame;
EmberAfInterpanHeader imCompatibilityInterpanHeader;

void SetupEmberAfObjects(ClusterId clusterId, CommandId commandId, EndpointId endpointId)
{
    imCompatibilityEmberApsFrame.clusterId           = clusterId;
    imCompatibilityEmberApsFrame.destinationEndpoint = endpointId;
    imCompatibilityEmberApsFrame.sourceEndpoint      = endpointId;
    imCompatibilityEmberAfCluster.commandId          = commandId;
    imCompatibilityEmberAfCluster.apsFrame           = &imCompatibilityEmberApsFrame;
    imCompatibilityEmberAfCluster.interPanHeader     = &imCompatibilityInterpanHeader;
    emAfCurrentCommand                               = &imCompatibilityEmberAfCluster;
}

void ResetEmberAfObjects()
{
    emAfCurrentCommand = nullptr;
}

} // namespace Compatibility
} // namespace app
} // namespace chip
