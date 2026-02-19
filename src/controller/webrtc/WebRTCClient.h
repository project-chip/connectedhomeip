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
#include <functional>
#include <lib/core/CHIPError.h>
#include <rtc/rtc.hpp>
#include <string>

namespace chip {
namespace webrtc {

constexpr int kVideoH264PayloadType     = 96; // 96 is just the first value in the dynamic RTP payload‑type range (96‑127).
constexpr int kVideoBitRate             = 3000;
const std::string kVideoMid             = "video";
constexpr const char * kStreamDestIp    = "127.0.0.1";
constexpr uint16_t kVideoStreamDestPort = 5000;

// Constants for Audio
constexpr int kAudioBitRate             = 64000;
constexpr int kOpusPayloadType          = 111;
constexpr uint16_t kAudioStreamDestPort = 5001;
const std::string kAudioMid             = "audio";

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
    void OnStateChange(std::function<void(const char *)> callback);

    /* Call to fetch the local session description string. This is used by the
     * Python binding layer to get the local SDP string with ice candidates
     * Should be called after setting local/remote SDP and GatheringComplete.
     */
    const char * GetLocalSessionDescriptionInternal();

    const char * GetPeerConnectionState();
    void Disconnect();

private:
    rtc::PeerConnection * mPeerConnection;
    std::function<void(const std::string &, const std::string &)> mLocalDescriptionCallback;
    std::function<void(const std::string &, const std::string &)> mIceCandidateCallback;
    std::function<void()> mGatheringCompleteCallback;
    std::function<void(const char *)> mStateChangeCallback;

    std::string mLocalDescription;

    // Local vector to store the ICE Candidate strings coming from the WebRTC stack
    std::vector<std::string> mLocalCandidates;

    std::shared_ptr<rtc::Track> mVideoTrack;
    std::shared_ptr<rtc::Track> mAudioTrack;

    // UDP socket for stream forwarding
    int mVideoRTPSocket = -1;
    int mAudioRTPSocket = -1;

    void addVideoTrack(std::string mid = kVideoMid, int payloadType = kVideoH264PayloadType);
    void addAudioTrack(std::string mid = kAudioMid, int payloadType = kOpusPayloadType);

    // Close and reset the UDP socket
    void CloseRTPSocket();
};

} // namespace webrtc
} // namespace chip
