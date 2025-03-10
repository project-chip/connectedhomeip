#pragma once

#include <app/clusters/meter-identification-server/meter-identification-server.h>
#include <json/value.h>

namespace chip {
namespace app {
namespace Clusters {
namespace MeterIdentification {

Instance * GetInstance();

CHIP_ERROR LoadJson(const Json::Value & root);

void Shutdown();

} // namespace MeterIdentification
} // namespace Clusters
} // namespace app
} // namespace chip
