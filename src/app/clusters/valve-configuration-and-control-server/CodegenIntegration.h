#pragma once

#include <app/clusters/valve-configuration-and-control-server/valve-configuration-and-control-cluster.h>

namespace chip::app::Clusters::ValveConfigurationAndControl {

void SetDelegate(EndpointId endpointId,  Delegate * delegate);

ValveConfigurationAndControlCluster * FindClusterOnEndpoint(EndpointId endpointId);

} // namespace chip::app::Clusters::ValveConfigurationAndControl
