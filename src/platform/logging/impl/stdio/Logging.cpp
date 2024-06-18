/* See Project CHIP LICENSE file for licensing information. */

#include <platform/logging/LogV.h>

#include <lib/support/logging/Constants.h>

#include <stdio.h>

namespace chip {
namespace Logging {
namespace Platform {

void LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
    printf("CHIP:%s: ", module);
    vprintf(msg, v);
    printf("\n");
}

} // namespace Platform
} // namespace Logging
} // namespace chip
