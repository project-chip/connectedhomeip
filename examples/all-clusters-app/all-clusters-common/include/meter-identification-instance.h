#pragma once

#include <app/clusters/meter-identification-server/meter-identification-server.h>

namespace chip {
namespace app {
namespace Clusters {
namespace MeterIdentification {

Instance * GetInstance();

void Shutdown();

} // namespace MeterIdentification
} // namespace Clusters
} // namespace app
} // namespace chip
