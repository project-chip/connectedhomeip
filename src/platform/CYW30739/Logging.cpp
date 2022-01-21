/* See Project CHIP LICENSE file for licensing information. */

#include <assert.h>
#include <lib/support/logging/Constants.h>
#include <platform/logging/LogV.h>
#include <stdio.h>
#include <wiced_rtos.h>

namespace chip {
namespace Logging {
namespace Platform {

void LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
    switch (category)
    {
    case chip::Logging::LogCategory::kLogCategory_Error:
        printf("Error");
        break;
    case chip::Logging::LogCategory::kLogCategory_Progress:
        printf("InfoP");
        break;
    case chip::Logging::LogCategory::kLogCategory_Detail:
        printf("InfoD");
        break;
    }

    static char buffer[256];
    vsnprintf(buffer, sizeof(buffer), msg, v);
    printf(" CHIP:%s: %s\n", module, buffer);

    assert(!wiced_rtos_check_for_stack_overflow());
}

} // namespace Platform
} // namespace Logging
} // namespace chip
