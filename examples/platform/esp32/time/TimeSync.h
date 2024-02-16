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

#pragma once
#include <esp_sntp.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

namespace chip {
class Esp32TimeSync
{
public:
    Esp32TimeSync(char * aSntpServerName, uint16_t aSyncSntpIntervalDay)
    {
        mSntpServerName = new char[strlen(aSntpServerName) + 1];
        strcpy(mSntpServerName, aSntpServerName);
        mSyncSntpIntervalDay = aSyncSntpIntervalDay;
    }

    CHIP_ERROR Init();

private:
    static CHIP_ERROR GetLocalTimeString(char * buf, size_t buf_len);
    static bool ValidateTime();
    static CHIP_ERROR PrintCurrentTime();
    static void TimeSyncCallback(struct timeval * tv);

    char * mSntpServerName;
    uint16_t mSyncSntpIntervalDay;
};
} // namespace chip
