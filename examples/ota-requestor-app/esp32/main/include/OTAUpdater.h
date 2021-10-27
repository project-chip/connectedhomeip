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
#include <esp_err.h>

#pragma once

class OTAUpdater
{
public:
    static OTAUpdater & GetInstance(void)
    {
        static OTAUpdater instance;
        return instance;
    }

    bool IsInProgress(void);

    esp_err_t Begin(void);

    esp_err_t End(void);

    esp_err_t Write(const void *data, size_t length);

    esp_err_t Abort(void);

    esp_err_t Apply(uint32_t delayedActionTime);
private:
    OTAUpdater(void) {}
    ~OTAUpdater() {}
};
