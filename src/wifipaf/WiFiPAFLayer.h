/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "WiFiPAFLayerDelegate.h"
#include <lib/core/CHIPError.h>
#include <lib/support/DLLUtil.h>
#include <system/SystemPacketBuffer.h>
#include <vector>

namespace chip {
namespace WiFiPAF {
/**
 *  The State of the Wi-Fi-PAF connection
 *
 */
enum class State
{
    kNotReady    = 0, /**< State before initialization. */
    kInitialized = 1, /**< State after class is connected and ready. */
    kConnected   = 2, /**< Endpoint connected. */
};

class DLL_EXPORT WiFiPAFLayer
{
public:
    State mAppState                          = State::kNotReady;
    WiFiPAFLayerDelegate * mWiFiPAFTransport = nullptr;

    WiFiPAFLayer() = default;
    static WiFiPAFLayer * GetWiFiPAFLayer();
    typedef void (*OnCancelDeviceHandle)(uint32_t id, WiFiPAFSession::PAFRole role);
    void Shutdown(OnCancelDeviceHandle OnCancelDevice);
    void OnWiFiPAFMessageReceived(WiFiPAFSession & RxInfo, System::PacketBufferHandle && msg);
    State GetWiFiPAFState() { return mAppState; };
    void SetWiFiPAFState(State state);

    void AddPafSession(const NodeId nodeId, const uint16_t discriminator);
    void AddPafSession(uint32_t id);
    void RmPafSession(uint32_t id);
    WiFiPAFSession * GetPAFInfo(NodeId nodeId);
    WiFiPAFSession * GetPAFInfo(uint32_t id);
    WiFiPAFSession * GetPAFInfo(uint16_t discriminator);

private:
    std::vector<WiFiPAFSession> PafInfoVect;
};

} /* namespace WiFiPAF */
} /* namespace chip */
