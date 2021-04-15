/*
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <platform/CHIPDeviceLayer.h>

#include <nfc_t2t_lib.h>

class NFCWidget
{
public:
    int Init(chip::DeviceLayer::ConnectivityManager & mgr);
    int StartTagEmulation(const char * tagPayload, uint8_t tagPayloadLength);
    int StopTagEmulation();
    bool IsTagEmulationStarted() const;

private:
    static void FieldDetectionHandler(void * context, enum nfc_t2t_event event, const uint8_t * data, size_t data_length);

    constexpr static uint8_t mNdefBufferSize = 128;

    uint8_t mNdefBuffer[mNdefBufferSize];

    bool mIsTagStarted;
};
