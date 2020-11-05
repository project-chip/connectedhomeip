/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <cstdio>

#include <inet/UDPEndPoint.h>
#include <mdns/minimal/DnsHeader.h>
#include <platform/CHIPDeviceLayer.h>
#include <support/CHIPMem.h>
#include <system/SystemTimer.h>

using namespace chip;

namespace {

constexpr uint32_t kRunTimeMs = 3'000;

}

int main(int argc, char ** args)
{
    if (Platform::MemoryInit() != CHIP_NO_ERROR)
    {
        printf("FAILED to initialize memory");
        return 1;
    }

    if (DeviceLayer::PlatformMgr().InitChipStack() != CHIP_NO_ERROR)
    {
        printf("FAILED to initialize chip stack");
        return 1;
    }

    printf("Running...\n");

    Inet::UDPEndPoint * udp = nullptr;

    if (DeviceLayer::InetLayer.NewUDPEndPoint(&udp) != CHIP_NO_ERROR)
    {
        printf("FAILED to create new udp endpoint");
        return 1;
    }

    System::Timer * timer = nullptr;

    if (DeviceLayer::SystemLayer.NewTimer(timer) == CHIP_NO_ERROR)
    {
        timer->Start(
            kRunTimeMs, [](System::Layer *, void *, System::Error err) { DeviceLayer::PlatformMgr().Shutdown(); }, nullptr);
    }

    // TODO:
    //   - MDNS:
    //     - build a query (TODO header + query)
    //     - Send it out (where? IPV4 works but what about V6?)
    //     - Listen
    //     - timeout

    DeviceLayer::PlatformMgr().RunEventLoop();

    udp->Free();
    printf("Done...\n");
    return 0;
}