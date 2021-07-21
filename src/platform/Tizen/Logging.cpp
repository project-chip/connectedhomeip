/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <platform/logging/LogV.h>

#include <core/CHIPConfig.h>
#include <support/logging/Constants.h>

#include <stdio.h>
#include <dlog.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "CHIP"

#ifndef LOGD
#define LOGD(fmt, args...) dlog_print(DLOG_DEBUG, LOG_TAG, fmt, ##args)
#endif
#ifndef LOGE
#define LOGE(fmt, args...) dlog_print(DLOG_ERROR, LOG_TAG, fmt, ##args)
#endif
#ifndef LOGI
#define LOGI(fmt, args...) dlog_print(DLOG_INFO, LOG_TAG, fmt, ##args)
#endif

namespace chip {
namespace Logging {
namespace Platform {

/**
 * CHIP log output functions.
 */
void LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
    char msgBuf[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE] = { 0, };
    vsnprintf(msgBuf, sizeof(msgBuf), msg, v);

    switch (category)
    {
    case kLogCategory_Error:
        LOGE("%s: %s", module, msgBuf);
        break;
    case kLogCategory_Detail:
        LOGD("%s: %s", module, msgBuf);
        break;
    case kLogCategory_Progress:
    default:
        LOGI("%s: %s", module, msgBuf);
        break;
    }
}

} // namespace Platform
} // namespace Logging
} // namespace chip
