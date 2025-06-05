#pragma once

#include <app/clusters/meter-identification-server/meter-identification-server.h>

CHIP_ERROR MeterIdentificationInit(chip::EndpointId endpointId);
CHIP_ERROR MeterIdentificationShutdown();

namespace chip {
namespace app {
namespace Clusters {
namespace MeterIdentification {

Instance * GetInstance();

} // namespace MeterIdentification
} // namespace Clusters
} // namespace app
} // namespace chip
