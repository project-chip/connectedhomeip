/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <transport/TransportMgr.h>
#include <transport/raw/UDP.h>
#include <transport/raw/tests/NetworkTestHelpers.h>

namespace chip {
namespace Test {

class UDPTransportManager
{
public:
    /// Initialize the underlying layers.
    CHIP_ERROR Init()
    {
        ReturnErrorOnFailure(mIOContext.Init());
        ReturnErrorOnFailure(DeviceLayer::PlatformMgr().InitChipStack());
        ReturnErrorOnFailure(mTransportManager.Init(Transport::UdpListenParameters(DeviceLayer::UDPEndPointManager())
                                                        .SetAddressType(Inet::IPAddressType::kIPv6)
                                                        .SetListenPort(CHIP_PORT + 1)));

        return CHIP_NO_ERROR;
    }

    // Shutdown all layers, finalize operations
    void Shutdown()
    {
        mTransportManager.Close();
        DeviceLayer::PlatformMgr().Shutdown();
        mIOContext.Shutdown();
    }

    System::Layer & GetSystemLayer() { return mIOContext.GetSystemLayer(); }
    chip::Transport::UDP & GetUDP() { return mTransportManager.GetTransport().template GetImplAtIndex<0>(); }
    TransportMgrBase & GetTransportMgr() { return mTransportManager; }
    IOContext & GetIOContext() { return mIOContext; }

private:
    Test::IOContext mIOContext;
    TransportMgr<chip::Transport::UDP> mTransportManager;
};

} // namespace Test
} // namespace chip
