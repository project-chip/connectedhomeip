#pragma once

#include <functional>
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

using OnLocalDescriptionCallback = std::function<void(const std::string & sdp, SDPType type)>;
using OnICECandidateCallback     = std::function<void(const std::string & candidate)>;
using OnConnectionStateCallback  = std::function<void(bool connected)>;
using OnTrackCallback            = std::function<void(std::shared_ptr<WebRTCTrack> track)>;

// Abstract track interface
class WebRTCTrack
{
public:
    virtual ~WebRTCTrack() = default;

    virtual void SendData(const char * data, size_t size) = 0;
    virtual bool IsReady()                                = 0;
    virtual std::string GetType()                         = 0; // "video" or "audio"
};

// Abstract peer connection interface
class WebRTCPeerConnection
{
public:
    virtual ~WebRTCPeerConnection() = default;

    virtual void SetCallbacks(OnLocalDescriptionCallback onLocalDescription, OnICECandidateCallback onICECandidate,
                              OnConnectionStateCallback onConnectionState, OnTrackCallback onTrack) = 0;
    virtual void Close()                                                                            = 0;
    virtual void CreateOffer(uint16_t sessionId)                                                    = 0;
    virtual void CreateAnswer()                                                                     = 0;
    virtual void SetRemoteDescription(const std::string & sdp, SDPType type)                        = 0;
    virtual void AddRemoteCandidate(const std::string & candidate, const std::string & mid)         = 0;
    virtual std::shared_ptr<WebRTCTrack> AddTrack(MediaType mediaType)                              = 0;
};

std::shared_ptr<WebRTCPeerConnection> CreateWebRTCPeerConnection();
