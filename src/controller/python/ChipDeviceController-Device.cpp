/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      Implementation of the native methods expected by the Python
 *      version of Chip Device Manager.
 *
 */

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include <system/SystemError.h>
#include <system/SystemLayer.h>

#include <controller/CHIPDevice.h>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/DLLUtil.h>
#include <support/ReturnMacros.h>
#include <support/logging/CHIPLogging.h>

using namespace chip;

using OnStatusChangeFunction = void (*)();

class PythonDeviceStatusDelegate : public Controller::DeviceStatusDelegate
{
public:
    PythonDeviceStatusDelegate(OnStatusChangeFunction callback) { mCallback = callback; }

    void OnStatusChange() override { mCallback(); }

    void OnMessage(System::PacketBufferHandle msg) override {}

private:
    OnStatusChangeFunction mCallback;
};

extern "C" {
void nl_Chip_DeviceStatusDelegate_Create(PythonDeviceStatusDelegate ** pDelegate, OnStatusChangeFunction callback);
void nl_Chip_DeviceStatusDelegate_Destory(PythonDeviceStatusDelegate * delegate);

void nl_Chip_Device_SetDelegate(Controller::Device * device, PythonDeviceStatusDelegate * delegate);
CHIP_ERROR nl_Chip_Device_EstablishPaseSession(Controller::Device * device, const char * peerAddrStr, uint32_t setupPINCode);
}

void nl_Chip_DeviceStatusDelegate_Create(PythonDeviceStatusDelegate ** pDelegate, OnStatusChangeFunction callback)
{
    auto delegate = new PythonDeviceStatusDelegate(callback);
    *pDelegate    = delegate;
}

void nl_Chip_DeviceStatusDelegate_Destory(PythonDeviceStatusDelegate * delegate)
{
    delete delegate;
}

void nl_Chip_Device_SetDelegate(Controller::Device * device, PythonDeviceStatusDelegate * delegate)
{
    device->SetDelegate(delegate);
}

CHIP_ERROR nl_Chip_Device_EstablishPaseSession(Controller::Device * device, const char * peerAddrStr, uint32_t setupPINCode)
{
    chip::Inet::IPAddress peerAddr;
    VerifyOrReturnError(chip::Inet::IPAddress::FromString(peerAddrStr, peerAddr), CHIP_ERROR_INVALID_ARGUMENT);
    return device->EstablishPaseSession(peerAddr, setupPINCode);
}
