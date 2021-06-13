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
 *      This file defines an object for a CHIP Ping client which using Echo
 *      Protocol to measure packet loss across network paths for a paired device.
 */

#pragma once

#include <messaging/ExchangeMgr.h>
#include <protocols/echo/Echo.h>

namespace chip {
namespace Controller {

class PingClient : public Protocols::Echo::EchoDelegate
{
public:
    void Reset()
    {
        mEchoIntervalMillis = 1000;
        mLastEchoTimeMillis = 0;
        mEchoReqCount       = 0;
        mEchoRespCount      = 0;
        mEchoReqSize        = 32;
        mWaitingForEchoResp = false;
        mUsingMRP           = false;
    }

    void SetEchoIntervalMillis(uint32_t value) { mEchoIntervalMillis = value; }

    void SetEchoReqSize(uint16_t value) { mEchoReqSize = value; }

    void SetUsingMRP(bool value) { mUsingMRP = value; }

    void OnMessageReceived(Messaging::ExchangeContext * ec, System::PacketBufferHandle && payload) override;

    CHIP_ERROR PingDevice(NodeId remoteDeviceId, uint16_t maxCount, Messaging::ExchangeManager * exchangeMgr);

private:
    // The last time a echo request was attempted to be sent.
    uint64_t mLastEchoTimeMillis = 0;

    // The CHIP Echo interval time in milliseconds.
    uint32_t mEchoIntervalMillis = 1000;

    // Count of the number of echo requests sent.
    uint16_t mEchoReqCount = 0;

    // Count of the number of echo responses received.
    uint16_t mEchoRespCount = 0;

    // The CHIP Echo request payload size in bytes.
    uint16_t mEchoReqSize = 32;

    // True, if the echo client is waiting for an echo response
    // after sending an echo request, false otherwise.
    bool mWaitingForEchoResp = false;

    bool mUsingMRP = false;

    Protocols::Echo::EchoClient mEchoClient;

    bool EchoIntervalExpired(void);

    CHIP_ERROR SendEchoRequest(NodeId remoteDeviceId);
};

} // namespace Controller
} // namespace chip
