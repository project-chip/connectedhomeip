#pragma once

#include <platform_stdlib.h>

namespace chip {
namespace Logging {
namespace Platform {

enum class LogLevel
{
    kError,
    kProgress,
    kDetail,
};

void LogSetLevel(LogLevel level);
LogLevel LogGetLevel();

} // namespace Platform
} // namespace Logging
} // namespace chip
