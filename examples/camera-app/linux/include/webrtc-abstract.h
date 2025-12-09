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

#pragma once

#include <functional>
#include <lib/support/Span.h>
#include <memory>
#include <string>
#include <vector>

// Forward declarations
class WebRTCPeerConnection;
class WebRTCTrack;

enum class SDPType : uint8_t
{
    Offer,
    Answer,
    Pranswer,
    Rollback
};

enum class MediaType : uint8_t
{
    Audio,
    Video,
};

struct ICECandidateInfo
{
    std::string candidate;
    std::string mid;
    int mlineIndex;
};

using OnLocalDescriptionCallback = std::function<void(const std::string & sdp, SDPType type)>;
using OnICECandidateCallback     = std::function<void(const ICECandidateInfo & candidateInfo)>;
using OnConnectionStateCallback  = std::function<void(bool connected)>;
using OnTrackCallback            = std::function<void(std::shared_ptr<WebRTCTrack> track)>;

// Abstract track interface
class WebRTCTrack
{
public:
    virtual ~WebRTCTrack() = default;

    virtual void SendData(const chip::ByteSpan & data)                     = 0;
    virtual void SendFrame(const chip::ByteSpan & data, int64_t timestamp) = 0;
    virtual bool IsReady()                                                 = 0;
    virtual std::string GetType()                                          = 0; // "video" or "audio"
};

// Abstract peer connection interface
class WebRTCPeerConnection
{
public:
    virtual ~WebRTCPeerConnection() = default;

    virtual void SetCallbacks(OnLocalDescriptionCallback onLocalDescription, OnICECandidateCallback onICECandidate,
                              OnConnectionStateCallback onConnectionState, OnTrackCallback onTrack)              = 0;
    virtual void Close()                                                                                         = 0;
    virtual void CreateOffer()                                                                                   = 0;
    virtual void CreateAnswer()                                                                                  = 0;
    virtual void SetRemoteDescription(const std::string & sdp, SDPType type)                                     = 0;
    virtual void AddRemoteCandidate(const std::string & candidate, const std::string & mid)                      = 0;
    virtual std::shared_ptr<WebRTCTrack> AddTrack(MediaType mediaType, const std::string & mid, int payloadType) = 0;
    virtual int GetPayloadType(const std::string & sdp, SDPType type, const std::string & codec) { return -1; };
};

std::shared_ptr<WebRTCPeerConnection> CreateWebRTCPeerConnection();
