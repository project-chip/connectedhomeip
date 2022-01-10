/* See Project CHIP LICENSE file for licensing information. */

#include <lib/support/EnforceFormat.h>
#include <lib/support/logging/Constants.h>
#include <platform/logging/LogV.h>

#include <stdio.h>

namespace chip {
namespace Logging {
namespace Platform {

void ENFORCE_FORMAT(3, 0) LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
    printf("CHIP:%s: ", module);
    vprintf(msg, v);
    printf("\n");
}

} // namespace Platform
} // namespace Logging
} // namespace chip
