#pragma once

#include <app/clusters/soil-measurement-server/soil-measurement-server.h>

namespace chip {
namespace app {
namespace Clusters {
namespace SoilMeasurement {

Instance * GetInstance();

void Shutdown();

} // namespace SoilMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
