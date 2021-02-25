#pragma once

#include <app/Command.h>
#include <lib/core/CHIPCore.h>
#include <util/af-types.h>

namespace chip {
namespace app {
namespace Compatibility {

void SetupEmberAfObjects(Command * command, ClusterId clusterId, CommandId commandId, EndpointId endpointId);
bool IMEmberAfSendImmediateDefaultResponseHandle(EmberAfStatus status);
void ResetEmberAfObjects();

} // namespace Compatibility
} // namespace app
} // namespace chip
