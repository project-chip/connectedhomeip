#pragma once

#include <platform_stdlib.h>

namespace chip {
namespace Logging {
namespace Platform {

enum AmebaLogLevel
{
    ameba_loglevel_error = 0,
    ameba_loglevel_progress,
    ameba_loglevel_detail,
};

void LogSetLevel(uint8_t level);
uint8_t LogGetLevel();

} // namespace Platform
} // namespace Logging
} // namespace chip
