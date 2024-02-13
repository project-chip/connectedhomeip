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

#include <esp_err.h>
#include <esp_log.h>

#if CONFIG_ENABLE_SNTP_TIME_SYNC
#include <esp_sntp.h>

static const char * TAG = "ESP_TIME_SYNC";

#define REF_TIME 1546300800 /* 01-Jan-2019 00:00:00 */
/* Timer interval once every day (24 Hours) */
#define TIME_PERIOD (CONFIG_SNTP_SYNC_INTERVAL_DAY * 86400000ULL)

namespace Esp32Time {

esp_err_t GetLocalTimeString(char * buf, size_t buf_len)
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
        ESP_LOGE(TAG, "Buffer size %d insufficient for localtime string. Required size: %d", buf_len, print_size);
        return ESP_ERR_INVALID_ARG;
    }
    return ESP_OK;
}

bool ValidateTime(void)
{
    time_t now;
    time(&now);
    if (now > REF_TIME)
    {
        return true;
    }
    return false;
}
static esp_err_t PrintCurrentTime(void)
{
    char local_time[64];
    if (GetLocalTimeString(local_time, sizeof(local_time)) == ESP_OK)
    {
        if (ValidateTime() == false)
        {
            ESP_LOGI(TAG, "Time not synchronised yet.");
        }
        ESP_LOGI(TAG, "The current time is: %s.", local_time);
        return ESP_OK;
    }
    return ESP_FAIL;
}

static void TimeSyncCallback(struct timeval * tv)
{
    ESP_LOGI(TAG, "SNTP Synchronised.");
    PrintCurrentTime();
}

esp_err_t TimeSycnInit(void)
{
    if (esp_sntp_enabled())
    {
        ESP_LOGI(TAG, "SNTP already initialized.");
        return ESP_OK;
    }
    char * sntp_server_name = CONFIG_SNTP_SERVER_NAME;
    ESP_LOGI(TAG, "Initializing SNTP. Using the SNTP server: %s", sntp_server_name);
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, sntp_server_name);
    esp_sntp_set_sync_interval(TIME_PERIOD);
    esp_sntp_init();
    sntp_set_time_sync_notification_cb(TimeSyncCallback);
    return ESP_OK;
}

} // namespace Esp32Time
#endif
