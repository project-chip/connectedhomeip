/* See Project chip LICENSE file for licensing information. */

#include <lib/support/logging/Constants.h>
#include <platform/logging/LogV.h>

#include <android/log.h>

namespace chip {
namespace Logging {
namespace Platform {

void LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
    int priority = (category == kLogCategory_Error) ? ANDROID_LOG_ERROR : ANDROID_LOG_DEBUG;
    __android_log_vprint(priority, module, msg, v);
}

} // namespace Platform
} // namespace Logging
} // namespace chip
