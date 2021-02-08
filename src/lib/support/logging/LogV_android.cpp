/* See Project chip LICENSE file for licensing information. */

#include "CHIPLogging.h"

#include <core/CHIPCore.h>
#include <support/CodeUtils.h>
#include <support/DLLUtil.h>

#include <android/log.h>

namespace chip {
namespace Logging {

DLL_EXPORT void LogV(uint8_t module, uint8_t category, const char * msg, va_list v)
{
    if (IsCategoryEnabled(category))
    {

        char moduleName[chip::Logging::kMaxModuleNameLen + 1];
        GetModuleName(moduleName, sizeof(moduleName), module);

        int priority = (category == kLogCategory_Error) ? ANDROID_LOG_ERROR : ANDROID_LOG_DEBUG;
        __android_log_vprint(priority, moduleName, msg, v);
    }
}

} // namespace Logging
} // namespace chip
