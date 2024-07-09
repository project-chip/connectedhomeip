/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

using namespace chip::DeviceLayer;

class SensorManager
{
public:
    void Init(AsyncWorkFunct interruptHandler);

private:
    static void InterruptHandler(void * data, uint8_t port_pin);

    friend SensorManager & SensorMgr();
    static SensorManager sSensorManager;
    static AsyncWorkFunct sAppInterruptHandler;
};

inline SensorManager & SensorMgr()
{
    return SensorManager::sSensorManager;
}
