// platform/webos/wbs/WebosLockTracker.cpp
#include <lib/support/logging/CHIPLogging.h>
#include <platform/PlatformManager.h>
#include <platform/webos/wbs/WebosLockTracker.h>

namespace chip {
namespace Platform {
namespace Internal {

void AssertChipStackLockedByCurrentThread(const char * file, int line)
{
    // ChipLogDetail(Ble, "Bypassing assertChipStackLockedByCurrentThread at %s:%d", file, line);
}

} // namespace Internal
} // namespace Platform
} // namespace chip
