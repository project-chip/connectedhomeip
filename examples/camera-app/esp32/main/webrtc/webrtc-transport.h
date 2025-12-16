#pragma once

#include "webrtc-abstract.h"
#include <lib/core/DataModelTypes.h>
#include <lib/core/ScopedNodeId.h>

using OnTransportLocalDescriptionCallback = std::function<void(
    const std::string &sdp, SDPType type, const int16_t sessionId)>;
using OnTransportConnectionStateCallback =
    std::function<void(bool connected, const int16_t sessionId)>;

class WebrtcTransport {
public:
  enum class CommandType : uint8_t {
    kUndefined = 0,
    kOffer = 1,
    kAnswer = 2,
    kICECandidates = 3,
    kEnd = 4,
  };

  enum class State : uint8_t {
    Idle,                 ///< Default state, no communication initiated yet
    SendingOffer,         ///< Sending Offer command from camera
    SendingAnswer,        ///< Sending Answer command from camera
    SendingICECandidates, ///< Sending ICECandidates command from camera
    SendingEnd,           ///< Sending End command from camera
  };

  struct RequestArgs {
    uint16_t sessionId;
    uint16_t videoStreamId;
    uint16_t audioStreamId;
    chip::NodeId peerNodeId;
    chip::FabricIndex fabricIndex;
    chip::EndpointId originatingEndpointId;
    chip::ScopedNodeId peerId;
  };

  WebrtcTransport();

  ~WebrtcTransport();

  void SetCallbacks(OnTransportLocalDescriptionCallback onLocalDescription,
                    OnTransportConnectionStateCallback onConnectionState);

  void MoveToState(const State targetState);
  const char *GetStateStr() const;

  State GetState() { return mState; }

  // Takes care of creation WebRTC peer connection and registering the necessary
  // callbacks
  void Start();

  // Stops WebRTC peer connection and cleanup
  void Stop();

  void AddTracks();

  // Set video track for the transport
  void SetVideoTrack(std::shared_ptr<WebRTCTrack> videoTrack);

  // Set audio track for the transport
  void SetAudioTrack(std::shared_ptr<WebRTCTrack> audioTrack);

  std::shared_ptr<WebRTCPeerConnection> GetPeerConnection() {
    return mPeerConnection;
  }

  std::string GetLocalDescription() { return mLocalSdp; }

  void SetSdpAnswer(std::string localSdp) { mLocalSdp = localSdp; }

  std::vector<std::string> GetCandidates() { return mLocalCandidates; }

  void SetCandidates(std::vector<std::string> candidates) {
    mLocalCandidates = candidates;
  }

  void AddRemoteCandidate(const std::string &candidate, const std::string &mid);

  bool ClosePeerConnection();

  void SetCommandType(const CommandType commandtype);

  CommandType GetCommandType() { return mCommandType; }

  // WebRTC Callbacks
  void OnLocalDescription(const std::string &sdp, SDPType type);
  void OnICECandidate(const std::string &candidate);
  void OnConnectionStateChanged(bool connected);
  void OnTrack(std::shared_ptr<WebRTCTrack> track);

  void SetRequestArgs(const RequestArgs &args);
  RequestArgs &GetRequestArgs();

private:
  CommandType mCommandType = CommandType::kUndefined;
  State mState = State::Idle;

  std::shared_ptr<WebRTCPeerConnection> mPeerConnection;
  std::shared_ptr<WebRTCTrack> mVideoTrack;
  std::shared_ptr<WebRTCTrack> mAudioTrack;
  std::string mLocalSdp;
  SDPType mLocalSdpType;
  std::vector<std::string> mLocalCandidates;

  RequestArgs mRequestArgs;
  OnTransportLocalDescriptionCallback mOnLocalDescription = nullptr;
  OnTransportConnectionStateCallback mOnConnectionState = nullptr;
};
