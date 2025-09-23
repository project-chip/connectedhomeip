/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "webrtc-abstract.h"
#include <lib/support/logging/CHIPLogging.h>
#include <rtc/rtc.hpp>
#include <webrtc-transport.h>

SDPType RtcTypeToSDPType(rtc::Description::Type type)
{
    switch (type)
    {
    case rtc::Description::Type::Offer:
        return SDPType::Offer;
    case rtc::Description::Type::Answer:
        return SDPType::Answer;
    case rtc::Description::Type::Pranswer:
        return SDPType::Pranswer;
    case rtc::Description::Type::Rollback:
        return SDPType::Rollback;
    default:
        return SDPType::Offer;
    }
}

WebrtcTransport::WebrtcTransport()
{
    ChipLogProgress(Camera, "WebrtcTransport created");
    mRequestArgs = { 0, 0, 0, 0, 0, 0 }; // Initialize request arguments to zero
}

WebrtcTransport::~WebrtcTransport()
{
    ClosePeerConnection();
    ChipLogProgress(Camera, "WebrtcTransport destroyed for sessionID: [%u]", mRequestArgs.sessionId);
}

void WebrtcTransport::SetCallbacks(OnTransportLocalDescriptionCallback onLocalDescription,
                                   OnTransportConnectionStateCallback onConnectionState)
{
    mOnLocalDescription = onLocalDescription;
    mOnConnectionState  = onConnectionState;
}

void WebrtcTransport::SetRequestArgs(const RequestArgs & args)
{
    mRequestArgs = args;
}

WebrtcTransport::RequestArgs & WebrtcTransport::GetRequestArgs()
{
    return mRequestArgs;
}

void WebrtcTransport::SendVideo(const char * data, size_t size, int64_t timestamp, uint16_t videoStreamID)
{
    if (mLocalVideoTrack)
    {
        mLocalVideoTrack->SendFrame(data, size, timestamp);
    }
}

// Implementation of SendAudio method
void WebrtcTransport::SendAudio(const char * data, size_t size, int64_t timestamp, uint16_t audioStreamID)
{
    if (mLocalAudioTrack)
    {
        mLocalAudioTrack->SendFrame(data, size, timestamp);
    }
}

// Implementation of SendAudioVideo method
void WebrtcTransport::SendAudioVideo(const char * data, size_t size, uint16_t videoStreamID, uint16_t audioStreamID)
{
    // Placeholder for actual WebRTC implementation to send synchronized audio/video data
}

// Implementation of CanSendVideo method
bool WebrtcTransport::CanSendVideo()
{
    return mLocalVideoTrack != nullptr;
}

// Implementation of CanSendAudio method
bool WebrtcTransport::CanSendAudio()
{
    return mLocalAudioTrack != nullptr;
}

const char * WebrtcTransport::GetStateStr() const
{
    switch (mState)
    {
    case State::Idle:
        return "Idle";

    case State::SendingOffer:
        return "SendingOffer";

    case State::SendingAnswer:
        return "SendingAnswer";

    case State::SendingICECandidates:
        return "SendingICECandidates";

    case State::SendingEnd:
        return "SendingEnd";
    }
    return "N/A";
}

void WebrtcTransport::MoveToState(const State targetState)
{
    mState = targetState;
    ChipLogProgress(Camera, "WebrtcTransport moving to [ %s ]", GetStateStr());
}

void WebrtcTransport::SetCommandType(const CommandType commandtype)
{
    mCommandType = commandtype;
}

void WebrtcTransport::Start()
{
    if (mPeerConnection.get())
    {
        ChipLogProgress(Camera, "Start, mPeerConnection is already created");
        return;
    }

    mPeerConnection = CreateWebRTCPeerConnection();

    mPeerConnection->SetCallbacks([this](const std::string & sdp, SDPType type) { this->OnLocalDescription(sdp, type); },
                                  [this](const std::string & candidate) { this->OnICECandidate(candidate); },
                                  [this](bool connected) { this->OnConnectionStateChanged(connected); },
                                  [this](std::shared_ptr<WebRTCTrack> track) { this->OnTrack(track); });
}

void WebrtcTransport::Stop()
{
    if (mPeerConnection != nullptr)
    {
        mPeerConnection->Close();
    }

    mLocalVideoTrack = nullptr;
    mLocalAudioTrack = nullptr;
}

void WebrtcTransport::AddVideoTrack(const std::string & videoMid, int payloadType)
{
    if (mPeerConnection != nullptr)
    {
        // Adding local tracks to send video data to remote peer
        mLocalVideoTrack = mPeerConnection->AddTrack(MediaType::Video, videoMid, payloadType);
    }
}

void WebrtcTransport::AddAudioTrack(const std::string & audioMid, int payloadType)
{
    if (mPeerConnection != nullptr)
    {
        // Adding local tracks to send audio data to remote peer
        mLocalAudioTrack = mPeerConnection->AddTrack(MediaType::Audio, audioMid, payloadType);
    }
}

void WebrtcTransport::AddRemoteCandidate(const std::string & candidate, const std::string & mid)
{
    ChipLogProgress(Camera, "Adding remote candidate for sessionID: %u", mRequestArgs.sessionId);
    mPeerConnection->AddRemoteCandidate(candidate, mid);
}

// WebRTC Callbacks
void WebrtcTransport::OnLocalDescription(const std::string & sdp, SDPType type)
{
    ChipLogProgress(Camera, "Local description received for sessionID: %u", mRequestArgs.sessionId);
    mLocalSdp     = sdp;
    mLocalSdpType = type;
    if (mOnLocalDescription)
        mOnLocalDescription(sdp, type, mRequestArgs.sessionId);
}

bool WebrtcTransport::ClosePeerConnection()
{
    if (mPeerConnection == nullptr)
    {
        return false;
    }
    mPeerConnection->Close();
    mPeerConnection.reset();

    return true;
}

void WebrtcTransport::OnICECandidate(const std::string & candidate)
{
    ChipLogProgress(Camera, "ICE Candidate received for sessionID: %u", mRequestArgs.sessionId);
    mLocalCandidates.push_back(candidate);
    ChipLogProgress(Camera, "Local Candidate:");
    ChipLogProgress(Camera, "%s", candidate.c_str());
}

void WebrtcTransport::OnConnectionStateChanged(bool connected)
{
    ChipLogProgress(Camera, "Connection state changed for sessionID: %u", mRequestArgs.sessionId);
    if (mOnConnectionState)
        mOnConnectionState(connected, mRequestArgs.sessionId);
}

void WebrtcTransport::OnTrack(std::shared_ptr<WebRTCTrack> track)
{
    // Only logging the track addition here as it's not used in the current implementation. In future, we can add functionality to
    // handle
    ChipLogProgress(Camera, "Remote track added for the sessionID: %u, type: %s", mRequestArgs.sessionId, track->GetType().c_str());
}
