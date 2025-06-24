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

WebRTCClient::WebRTCClient() {}
WebRTCClient::~WebRTCClient() {}

void WebRTCClient::createPeerConnection(const std::string & stunUrl)
{
    rtc::Configuration config;
    if (!stunUrl.empty())
    {
        config.iceServers.emplace_back(stunUrl);
    }
    else
    {
        ChipLogError(NotSpecified, "No STUN server URL provided");
    }
    pc_ = std::make_shared<rtc::PeerConnection>(config);

    pc_->onLocalDescription([this](rtc::Description desc) {
        if (localDescriptionCallback_)
        {
            localDescriptionCallback_(rtc::Description::typeToString(desc.type()), desc.typeString());
        }
    });

    pc_->onLocalCandidate([this](rtc::Candidate cand) {
        if (iceCandidateCallback_)
        {
            iceCandidateCallback_(cand.candidate(), cand.mid());
        }
    });
}

void WebRTCClient::createOffer()
{
    pc_->setLocalDescription();
}

void WebRTCClient::createAnswer()
{
    pc_->setLocalDescription();
}

void WebRTCClient::setRemoteDescription(const std::string & sdp, const std::string & type)
{
    pc_->setRemoteDescription(rtc::Description(sdp, type));
}

void WebRTCClient::addIceCandidate(const std::string & candidate, const std::string & mid)
{
    pc_->addRemoteCandidate(rtc::Candidate(candidate, mid));
}

void WebRTCClient::onLocalDescription(std::function<void(const std::string &, const std::string &)> callback)
{
    localDescriptionCallback_ = callback;
}

void WebRTCClient::onIceCandidate(std::function<void(const std::string &, const std::string &)> callback)
{
    iceCandidateCallback_ = callback;
}

} // namespace webrtc
} // namespace chip
