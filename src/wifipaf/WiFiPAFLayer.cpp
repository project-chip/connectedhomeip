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
#include <cassert>
namespace chip {
namespace WiFiPAF {

void WiFiPAFLayer::Shutdown(OnCancelDeviceHandle OnCancelDevice)
{
    ChipLogProgress(Inet, "WiFiPAF: Closing all WiFiPAF sessions (%lu)", PafInfoVect.size());
    for (WiFiPAFSession & PafInfoElm : PafInfoVect)
    {
        if (PafInfoElm.id == UINT32_MAX)
        {
            // Unused session
            continue;
        }
        ChipLogProgress(Inet, "WiFiPAF: Canceling id: %u", PafInfoElm.id);
        OnCancelDevice(PafInfoElm.id, PafInfoElm.role);
    }
    PafInfoVect.clear();
    return;
}

void WiFiPAFLayer::OnWiFiPAFMessageReceived(WiFiPAFSession & RxInfo, System::PacketBufferHandle && msg)
{
    if (mWiFiPAFTransport != nullptr)
    {
        mWiFiPAFTransport->OnWiFiPAFMessageReceived(RxInfo, std::move(msg));
    }
}

void WiFiPAFLayer::SetWiFiPAFState(State state)
{
    mAppState = state;
}

static WiFiPAFLayer sInstance;
WiFiPAFLayer * WiFiPAFLayer::GetWiFiPAFLayer()
{
    return &sInstance;
}

void WiFiPAFLayer::AddPafSession(const NodeId nodeId, const uint16_t discriminator)
{
    for (auto PafInfoElm : PafInfoVect)
    {
        if (PafInfoElm.nodeId == nodeId)
        {
            assert(PafInfoElm.discriminator == discriminator);
            // Already exist
            return;
        }
    }
    WiFiPAFSession PafInfo{ .id = UINT32_MAX, .nodeId = nodeId, .discriminator = discriminator };
    PafInfoVect.push_back(PafInfo);
    ChipLogProgress(Inet, "WiFiPAF: Add session with nodeId: %lu, disc: %x, total %lu sessions", nodeId, discriminator,
                    PafInfoVect.size());
}

void WiFiPAFLayer::AddPafSession(uint32_t id)
{
    for (auto PafInfoElm : PafInfoVect)
    {
        if (PafInfoElm.id == id)
        {
            // Already exist
            return;
        }
    }
    WiFiPAFSession PafInfo{ .id = id };
    PafInfoVect.push_back(PafInfo);
    ChipLogProgress(Inet, "WiFiPAF: Add session with id: %u, total %lu sessions", id, PafInfoVect.size());
}
/*
void WiFiPAFLayer::AddPafSession()
{
    WiFiPAFSession PafInfo {
        .id = UINT32_MAX
    };
    PafInfoVect.push_back(PafInfo);
}
*/

void WiFiPAFLayer::RmPafSession(uint32_t id)
{
    for (std::vector<WiFiPAFSession>::iterator it = PafInfoVect.begin(); it != PafInfoVect.end(); it++)
    {
        if (it->id == id)
        {
            PafInfoVect.erase(it);
            return;
        }
    }
}

WiFiPAFSession * WiFiPAFLayer::GetPAFInfo(NodeId nodeId)
{
    for (WiFiPAFSession & PafInfoElm : PafInfoVect)
    {
        if (PafInfoElm.nodeId == nodeId)
        {
            return &PafInfoElm;
        }
    }
    return nullptr;
}

WiFiPAFSession * WiFiPAFLayer::GetPAFInfo(uint32_t id)
{
    for (WiFiPAFSession & PafInfoElm : PafInfoVect)
    {
        if (PafInfoElm.id == id)
        {
            return &PafInfoElm;
        }
    }
    return nullptr;
}

WiFiPAFSession * WiFiPAFLayer::GetPAFInfo(uint16_t discriminator)
{
    for (WiFiPAFSession & PafInfoElm : PafInfoVect)
    {
        if (PafInfoElm.discriminator == discriminator)
        {
            // Available session
            return &PafInfoElm;
        }
    }
    return nullptr;
}

} /* namespace WiFiPAF */
} /* namespace chip */
