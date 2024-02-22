/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "TimeSync.h"
#include <esp_sntp.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/logging/CHIPLogging.h>

#define REF_TIME 1546300800 /* 01-Jan-2019 00:00:00 */
/* Timer interval once every day (24 Hours) */
#define TIME_PERIOD_SEC 86400000ULL
namespace {
const uint8_t kMaxNtpServerStringSize = 128;
char mSntpServerName[kMaxNtpServerStringSize];

static CHIP_ERROR GetLocalTimeString(char * buf, size_t buf_len)
{
    struct tm timeinfo;
    char strftime_buf[64];
    time_t now;
    time(&now);
    localtime_r(&now, &timeinfo);
    if (strftime(strftime_buf, sizeof(strftime_buf), "%Y-%m-%dT%H:%M:%S%z", &timeinfo) == 0)
    {
        ChipLogError(DeviceLayer, "Buffer too small");
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    size_t print_size = snprintf(buf, buf_len, "%s, DST: %s", strftime_buf, timeinfo.tm_isdst ? "Yes" : "No");
    if (print_size >= buf_len)
    {
        ChipLogError(DeviceLayer, "Buffer size %d insufficient for localtime string. Required size: %d", static_cast<int>(buf_len),
                     static_cast<int>(print_size));
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    return CHIP_NO_ERROR;
}

static bool ValidateTime()
{
    time_t now;
    time(&now);
    return (now > REF_TIME);
}

static CHIP_ERROR PrintCurrentTime()
{
    char local_time[64] = { 0 };
    ReturnErrorCodeIf(GetLocalTimeString(local_time, sizeof(local_time)) != CHIP_NO_ERROR, CHIP_ERROR_BUFFER_TOO_SMALL);
    if (!ValidateTime())
    {
        ChipLogProgress(DeviceLayer, "Time not synchronised yet.");
        return CHIP_ERROR_INCORRECT_STATE;
    }
    ChipLogProgress(DeviceLayer, "The current time is: %s.", local_time);
    return CHIP_NO_ERROR;
}

static void TimeSyncCallback(struct timeval * tv)
{
    ChipLogProgress(DeviceLayer, "SNTP Synchronised.");
    if (PrintCurrentTime() != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "SNTP time is not synchronised.");
    }
}

} // anonymous namespace

namespace chip {

void Esp32TimeSync::Init(const char * aSntpServerName, const uint16_t aSyncSntpIntervalDay)
{
    chip::Platform::CopyString(mSntpServerName, aSntpServerName);
    if (esp_sntp_enabled())
    {
        ChipLogProgress(DeviceLayer, "SNTP already initialized.");
    }
    ChipLogProgress(DeviceLayer, "Initializing SNTP. Using the SNTP server: %s", mSntpServerName);
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, mSntpServerName);
    esp_sntp_set_sync_interval(TIME_PERIOD_SEC * aSyncSntpIntervalDay);
    esp_sntp_init();
    sntp_set_time_sync_notification_cb(TimeSyncCallback);
}
} // namespace chip
