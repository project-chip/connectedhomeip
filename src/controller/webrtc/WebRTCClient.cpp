/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "WebRTCClient.h"
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace webrtc {

WebRTCClient::WebRTCClient()
{
     mPeerConnection = nullptr;
}

WebRTCClient::~WebRTCClient()
{
    if (mPeerConnection == nullptr)
        return;

    delete mPeerConnection;
}

CHIP_ERROR WebRTCClient::CreatePeerConnection(const std::string & stunUrl)
{
    if (mPeerConnection != nullptr)
    {
        ChipLogError(NotSpecified, "PeerConnection exists already!");
        return CHIP_ERROR_ALREADY_INITIALIZED;
    }
    rtc::Configuration config;
    if (!stunUrl.empty())
    {
        config.iceServers.emplace_back(stunUrl);
    }
    else
    {
        ChipLogError(NotSpecified, "No STUN server URL provided");
    }
    mPeerConnection = new rtc::PeerConnection(config);
    if (mPeerConnection == nullptr)
    {
        ChipLogError(NotSpecified, "Failed to create PeerConnection");
        return CHIP_ERROR_NO_MEMORY;
    }

    mPeerConnection->onLocalDescription([this](rtc::Description desc) {
        if (mLocalDescriptionCallback)
        {
            mLocalDescriptionCallback(rtc::Description::typeToString(desc.type()), desc.typeString());
        }
    });

    mPeerConnection->onLocalCandidate([this](rtc::Candidate cand) {
        if (mIceCandidateCallback)
        {
            mIceCandidateCallback(cand.candidate(), cand.mid());
        }
    });

    return CHIP_NO_ERROR;
}

void WebRTCClient::CreateOffer()
{
    if (mPeerConnection == nullptr)
    {
        ChipLogError(NotSpecified, "Peerconnection is null");
        return;
    }

    mPeerConnection->setLocalDescription();
}

void WebRTCClient::CreateAnswer()
{
    if (mPeerConnection == nullptr)
    {
        ChipLogError(NotSpecified, "Peerconnection is null");
        return;
    }

    mPeerConnection->setLocalDescription();
}

void WebRTCClient::SetRemoteDescription(const std::string & sdp, const std::string & type)
{
    if (mPeerConnection == nullptr)
    {
        ChipLogError(NotSpecified, "Peerconnection is null");
        return;
    }

    mPeerConnection->setRemoteDescription(rtc::Description(sdp, type));
}

void WebRTCClient::AddIceCandidate(const std::string & candidate, const std::string & mid)
{
    if (mPeerConnection == nullptr)
    {
        ChipLogError(NotSpecified, "Peerconnection is null");
        return;
    }

    mPeerConnection->addRemoteCandidate(rtc::Candidate(candidate, mid));
}

void WebRTCClient::OnLocalDescription(std::function<void(const std::string &, const std::string &)> callback)
{
    mLocalDescriptionCallback = callback;
}

void WebRTCClient::OnIceCandidate(std::function<void(const std::string &, const std::string &)> callback)
{
    mIceCandidateCallback = callback;
}

} // namespace webrtc
} // namespace chip
