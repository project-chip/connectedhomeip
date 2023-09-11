/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Provides implementations for the CHIP logging functions
 *          on the Beken platform.
 */
/* this file behaves like a config.h, comes first */
#include <platform/logging/LogV.h>

#include <core/CHIPConfig.h>
#include <support/logging/Constants.h>

#include <stdio.h>
extern "C" void bk_printf(const char * fmt, ...);

#ifdef LOG_LOCAL_LEVEL
#undef LOG_LOCAL_LEVEL
#endif

namespace chip {
namespace Logging {
namespace Platform {

void LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
    char tag[11];

    snprintf(tag, sizeof(tag), "chip[%s]", module);
    tag[sizeof(tag) - 1] = 0;

    char formattedMsg[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
    vsnprintf(formattedMsg, sizeof(formattedMsg), msg, v);

    switch (category)
    {
    case kLogCategory_Error:
        bk_printf("%s %s\r\n", tag, formattedMsg);
        break;
    case kLogCategory_Progress:
    default:
        bk_printf("%s %s\r\n", tag, formattedMsg);
        break;
    case kLogCategory_Detail:
    case kLogCategory_Automation:
        bk_printf("%s %s\r\n", tag, formattedMsg);
        break;
    }
}

} // namespace Platform
} // namespace Logging
} // namespace chip
