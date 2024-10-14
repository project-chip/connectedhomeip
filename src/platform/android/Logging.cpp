/* See Project chip LICENSE file for licensing information. */

#include <platform/logging/LogV.h>

#include <lib/support/logging/Constants.h>

#include <android/log.h>

namespace chip {
namespace Logging {
namespace Platform {

void LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
    int priority = ANDROID_LOG_DEBUG;
    switch (category)
    {
    case kLogCategory_Error:
        priority = ANDROID_LOG_ERROR;
        break;
    case kLogCategory_Progress:
        priority = ANDROID_LOG_INFO;
        break;
    case kLogCategory_Detail:
        priority = ANDROID_LOG_DEBUG;
        break;
    default:
        break;
    }
    __android_log_vprint(priority, module, msg, v);
}

} // namespace Platform
} // namespace Logging
} // namespace chip
