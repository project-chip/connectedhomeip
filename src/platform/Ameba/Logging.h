#pragma once

#include <platform_stdlib.h>

namespace chip {
namespace Logging {
namespace Ameba {

enum class AmebaLogLevel
{
    kError,
    kProgress,
    kDetail,
};

void LogSetLevel(uint8_t level);
uint8_t LogGetLevel();

} // namespace Ameba
} // namespace Logging
} // namespace chip
