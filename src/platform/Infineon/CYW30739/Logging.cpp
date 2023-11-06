/* See Project CHIP LICENSE file for licensing information. */

#include <assert.h>
#include <lib/support/logging/Constants.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/logging/LogV.h>
#include <stdio.h>
#include <wiced_rtos.h>

namespace chip {
namespace Logging {
namespace Platform {

void LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
    const char * category_str;
    switch (category)
    {
    case LogCategory::kLogCategory_None:
        return;
    case LogCategory::kLogCategory_Error:
        category_str = "Err";
        break;
    case LogCategory::kLogCategory_Progress:
        category_str = "Prg";
        break;
    case LogCategory::kLogCategory_Detail:
        category_str = "Dtl";
        break;
    case LogCategory::kLogCategory_Automation:
        category_str = "Atm";
        break;
    }

    char task_symbol;
    if (DeviceLayer::PlatformMgrImpl().IsCurrentTask())
    {
        task_symbol = 'M';
    }
    else if (DeviceLayer::ThreadStackMgrImpl().IsCurrentTask())
    {
        task_symbol = 'T';
    }
    else
    {
        task_symbol = 'A';
    }

    static char buffer[256];
    vsnprintf(buffer, sizeof(buffer), msg, v);
    printf("%s%c CHIP:%s: %s\n", category_str, task_symbol, module, buffer);

    assert(!wiced_rtos_check_for_stack_overflow());
}

} // namespace Platform
} // namespace Logging
} // namespace chip
