#pragma once

#include <app/clusters/air-quality-server/air-quality-server.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AirQuality {

Instance * GetInstance();

void Shutdown();

} // namespace AirQuality
} // namespace Clusters
} // namespace app
} // namespace chip
