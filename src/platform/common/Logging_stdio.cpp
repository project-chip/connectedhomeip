/* See Project chip LICENSE file for licensing information. */

#include <core/CHIPCore.h>
#include <support/CodeUtils.h>
#include <support/DLLUtil.h>
#include <support/logging/CHIPLogging.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

namespace chip {
namespace Logging {

DLL_EXPORT void LogV(uint8_t module, uint8_t category, const char * msg, va_list v)
{
    if (!IsCategoryEnabled(category))
    {
        return;
    }

    char moduleName[chip::Logging::kMaxModuleNameLen + 1];
    GetModuleName(moduleName, sizeof(moduleName), module);

    printf("CHIP:%s: ", moduleName);
    vprintf(msg, v);
    printf("\n");
}

} // namespace Logging
} // namespace chip
