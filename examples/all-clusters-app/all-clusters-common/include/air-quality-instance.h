#pragma once

#include <app/clusters/air-quality-server/CodegenIntegration.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AirQuality {

AirQualityCluster * GetInstance();

void Shutdown();

} // namespace AirQuality
} // namespace Clusters
} // namespace app
} // namespace chip
