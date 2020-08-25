/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *      This file implements a platform-specific CHIP log interface.
 *
 */

#include <stdarg.h>
#include <stdio.h>

#import <Foundation/Foundation.h>
#import <os/log.h>

#include <support/logging/CHIPLogging.h>

#if CHIP_ERROR_LOGGING || CHIP_PROGRESS_LOGGING || CHIP_DETAIL_LOGGING

namespace chip {

namespace Logging {

    /*
     *  void LogV()
     *
     *  Description:
     *    This routine writes to the Foundation log stream, the
     *    specified CHIP-related variable argument message for the
     *    specified CHIP module and category.
     *
     *  Input(s):
     *    aModule   - An enumeration indicating the CHIP module or
     *                subsystem the message is associated with.
     *    aCategory - An enumeration indicating the CHIP category
     *                the message is associated with.
     *    aMsg      - A NULL-terminated C string containing the message,
     *                with C Standard I/O-style format specifiers, to log.
     *    aV        - A variable argument list, corresponding to format
     *                specifiers in the message.
     *
     *  Output(s):
     *    N/A
     *
     *  Returns:
     *    N/A
     *
     * @note There is a function called Log(), which takes a varargs intead of a
     * va_list and will call this function as needed.  It's declared and
     * implemented in the core CHIP library.
     */
    void LogV(uint8_t aModule, uint8_t aCategory, const char * aMsg, va_list aV)
    {
        if (IsCategoryEnabled(aCategory)) {
            char formattedMsg[512];
            size_t prefixLen;

            char moduleName[ChipLoggingModuleNameLen + 1];
            GetModuleName(moduleName, aModule);

            prefixLen = snprintf(
                formattedMsg, sizeof(formattedMsg), "CHIP:%s: %s", moduleName, (aCategory == kLogCategory_Error) ? "ERROR: " : "");
            if (prefixLen >= sizeof(formattedMsg))
                prefixLen = sizeof(formattedMsg) - 1;

            vsnprintf(formattedMsg + prefixLen, sizeof(formattedMsg) - prefixLen, aMsg, aV);

            os_log(OS_LOG_DEFAULT, "%s", formattedMsg);
        }
    }

} // namespace Logging

} // namespace chip

#endif // CHIP_ERROR_LOGGING || CHIP_PROGRESS_LOGGING || CHIP_DETAIL_LOGGING
