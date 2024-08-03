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

/**
 *    @file
 *      This file implements objects which provide an abstraction layer between
 *      a platform's WiFiPAF implementation and the CHIP
 *      stack.
 *
 */

#include "WiFiPAFLayer.h"
namespace chip {
namespace WiFiPAF {

void WiFiPAFLayer::OnWiFiPAFMessageReceived(System::PacketBufferHandle && msg)
{
    if (mWiFiPAFTransport != nullptr)
    {
        mWiFiPAFTransport->OnWiFiPAFMessageReceived(std::move(msg));
    }
}

void WiFiPAFLayer::SetWiFiPAFState(State state)
{
    mAppState = state;
    return;
}

static WiFiPAFLayer sInstance;
WiFiPAFLayer * WiFiPAFLayer::GetWiFiPAFLayer()
{
    return &sInstance;
}

} /* namespace WiFiPAF */
} /* namespace chip */
