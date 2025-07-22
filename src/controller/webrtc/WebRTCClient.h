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

#pragma once
#include <controller/webrtc/WebRTCTransportProviderClient.h>
#include <functional>
#include <lib/core/CHIPError.h>
#include <rtc/rtc.hpp>
#include <string>

namespace chip {
namespace webrtc {

class WebRTCClient
{
public:
    WebRTCClient();
    ~WebRTCClient();

    CHIP_ERROR CreatePeerConnection(const std::string & stunUrl);
    void CreateOffer();
    void CreateAnswer();
    void SetRemoteDescription(const std::string & sdp, const std::string & type);
    void AddIceCandidate(const std::string & candidate, const std::string & mid);

    void OnLocalDescription(std::function<void(const std::string &, const std::string &)> callback);
    void OnIceCandidate(std::function<void(const std::string &, const std::string &)> callback);
    void OnGatheringComplete(std::function<void()> callback);
    void OnStateChange(std::function<void(int)> callback);

    const char * GetLocalDescription();
    int GetPeerConnectionState();
    void Disconnect();

    void WebRTCProviderClientInit(uint32_t nodeId, uint8_t fabricIndex, uint16_t endpoint);
    PyChipError SendCommand(void * appContext, uint16_t endpointId, uint32_t clusterId, uint32_t commandId, const uint8_t * payload,
                            size_t length);
    void WebRTCProviderClientInitCallbacks(OnCommandSenderResponseCallback onCommandSenderResponseCallback,
                                           OnCommandSenderErrorCallback onCommandSenderErrorCallback,
                                           OnCommandSenderDoneCallback onCommandSenderDoneCallback);

private:
    rtc::PeerConnection * mPeerConnection;
    std::function<void(const std::string &, const std::string &)> mLocalDescriptionCallback;
    std::function<void(const std::string &, const std::string &)> mIceCandidateCallback;
    std::function<void()> mGatheringCompleteCallback;
    std::function<void(int)> mStateChangeCallback;

    std::string mLocalDescription;

    // Local vector to store the ICE Candidate strings coming from the WebRTC stack
    std::vector<std::string> mLocalCandidates;

    std::shared_ptr<rtc::Track> mTrack;

    // UDP socket for stream forwarding
    int mRTPSocket = -1;

    // Close and reset the UDP socket
    void CloseRTPSocket();

    std::unique_ptr<WebRTCTransportProviderClient> mTransportProviderClient;
};

} // namespace webrtc
} // namespace chip
