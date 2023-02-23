#pragma once

#include <platform_stdlib.h>

#define AMEBA_LOGLEVEL_ERROR 1
#define AMEBA_LOGLEVEL_PROGRESS 2
#define AMEBA_LOGLEVEL_DETAIL 3

namespace chip {
namespace Logging {
namespace Platform {

void LogSetLevel(uint8_t level);
uint8_t LogGetLevel(void);

} // namespace Platform
} // namespace Logging
} // namespace chip
