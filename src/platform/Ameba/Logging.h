#pragma once

#include <platform_stdlib.h>

namespace chip {
namespace Logging {
namespace Ameba {

enum class LogLevel
{
    kError,
    kProgress,
    kDetail,
};

void LogSetLevel(LogLevel level);
uint8_t LogGetLevel();

} // namespace Ameba
} // namespace Logging
} // namespace chip
