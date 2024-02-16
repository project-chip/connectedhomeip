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
#include <lib/support/logging/CHIPLogging.h>

#define REF_TIME 1546300800 /* 01-Jan-2019 00:00:00 */
/* Timer interval once every day (24 Hours) */
#define TIME_PERIOD_SEC 86400000ULL
namespace chip {

CHIP_ERROR Esp32TimeSync::GetLocalTimeString(char * buf, size_t buf_len)
{
    struct tm timeinfo;
    char strftime_buf[64];
    time_t now;
    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c %z[%Z]", &timeinfo);
    size_t print_size = snprintf(buf, buf_len, "%s, DST: %s", strftime_buf, timeinfo.tm_isdst ? "Yes" : "No");
    if (print_size >= buf_len)
    {
        ChipLogError(DeviceLayer, "Buffer size %d insufficient for localtime string. Required size: %d", buf_len, print_size);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_NO_ERROR;
}

bool Esp32TimeSync::ValidateTime()
{
    time_t now;
    time(&now);
    return (now > REF_TIME);
}

CHIP_ERROR Esp32TimeSync::PrintCurrentTime()
{
    char local_time[64];
    if (GetLocalTimeString(local_time, sizeof(local_time)) == CHIP_NO_ERROR)
    {
        if (!ValidateTime())
        {
            ChipLogProgress(DeviceLayer, "Time not synchronised yet.");
        }
        ChipLogProgress(DeviceLayer, "The current time is: %s.", local_time);
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_INTERNAL;
}

void Esp32TimeSync::TimeSyncCallback(struct timeval * tv)
{
    ChipLogProgress(DeviceLayer, "SNTP Synchronised.");
    PrintCurrentTime();
}

CHIP_ERROR Esp32TimeSync::Init()
{
    if (esp_sntp_enabled())
    {
        ChipLogProgress(DeviceLayer, "SNTP already initialized.");
        return CHIP_NO_ERROR;
    }
    ChipLogProgress(DeviceLayer, "Initializing SNTP. Using the SNTP server: %s", mSntpServerName);
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, mSntpServerName);
    esp_sntp_set_sync_interval(TIME_PERIOD_SEC * mSyncSntpIntervalDay);
    esp_sntp_init();
    sntp_set_time_sync_notification_cb(TimeSyncCallback);
    return CHIP_NO_ERROR;
}
} // namespace chip
