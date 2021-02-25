#pragma once

#include <lib/core/CHIPCore.h>
#include <util/af-types.h>

namespace chip {
namespace app {
namespace Compatibility {
extern EmberAfClusterCommand imCompatibilityEmberAfCluster;
extern EmberApsFrame imCompatibilityEmberApsFrame;
extern EmberAfInterpanHeader imCompatibilityInterpanHeader;

void SetupEmberAfObjects(ClusterId clusterId, CommandId commandId, EndpointId endpointId);
void ResetEmberAfObjects();

} // namespace Compatibility
} // namespace app
} // namespace chip
