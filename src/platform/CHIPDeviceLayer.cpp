/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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

#include <platform/CHIPDeviceLayer.h>

#include <inet/TCPEndPointImpl.h>
#include <inet/UDPEndPointImpl.h>
#include <system/SystemLayerImpl.h>

namespace chip {
namespace DeviceLayer {

namespace Internal {

const char * const TAG = "CHIP[DL]";

namespace {
System::LayerImpl gsSystemLayer;
Inet::UDPEndPointManagerImpl gsUDPEndPointManager;
}; // namespace

System::Clock::ClockBase * gClock                              = &System::SystemClock();
System::Layer * gSystemLayer                                   = &gsSystemLayer;
Inet::EndPointManager<Inet::UDPEndPoint> * gUDPEndPointManager = &gsUDPEndPointManager;
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
Inet::EndPointManager<Inet::TCPEndPoint> * gTCPEndPointManager = nullptr;
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

} // namespace Internal

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
CHIP_ERROR InitTCPEndPointManager()
{
    // Defining the TCPEndPointManagerImpl storage here means that it can be eliminated at link time along with the function
    // for applications that do not use TCP.
    static Inet::TCPEndPointManagerImpl sTCPEndPointManager;
    Internal::gTCPEndPointManager = &sTCPEndPointManager;
    return Internal::gTCPEndPointManager->Init(SystemLayer());
}
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

} // namespace DeviceLayer
} // namespace chip
