#include "webrtc-abstract.h"
#include <lib/support/logging/CHIPLogging.h>
#include <webrtc-transport.h>

WebrtcTransport::WebrtcTransport() {
  ChipLogProgress(Camera, "WebrtcTransport created");
  mRequestArgs = {0, 0, 0, 0, 0, 0}; // Initialize request arguments to zero
}

WebrtcTransport::~WebrtcTransport() {
  ClosePeerConnection();
  ChipLogProgress(Camera, "WebrtcTransport destroyed for sessionID: [%u]",
                  mRequestArgs.sessionId);
}

void WebrtcTransport::SetCallbacks(
    OnTransportLocalDescriptionCallback onLocalDescription,
    OnTransportConnectionStateCallback onConnectionState) {
  mOnLocalDescription = onLocalDescription;
  mOnConnectionState = onConnectionState;
}

void WebrtcTransport::SetRequestArgs(const RequestArgs &args) {
  mRequestArgs = args;
}

WebrtcTransport::RequestArgs &WebrtcTransport::GetRequestArgs() {
  return mRequestArgs;
}

const char *WebrtcTransport::GetStateStr() const {
  switch (mState) {
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

void WebrtcTransport::MoveToState(const State targetState) {
  mState = targetState;
  ChipLogProgress(Camera, "WebrtcTransport moving to [ %s ]", GetStateStr());
}

void WebrtcTransport::SetCommandType(const CommandType commandtype) {
  mCommandType = commandtype;
}

void WebrtcTransport::Start() {
  if (mPeerConnection.get()) {
    ChipLogProgress(Camera, "Start, mPeerConnection is already created");
    return;
  }

  mPeerConnection = CreateWebRTCPeerConnection();

  mPeerConnection->SetCallbacks(
      [this](const std::string &sdp, SDPType type) {
        this->OnLocalDescription(sdp, type);
      },
      [this](const std::string &candidate) { this->OnICECandidate(candidate); },
      [this](bool connected) { this->OnConnectionStateChanged(connected); },
      [this](std::shared_ptr<WebRTCTrack> track) { this->OnTrack(track); });
}

void WebrtcTransport::Stop() {
  mVideoTrack = nullptr;
  mAudioTrack = nullptr;
  if (mPeerConnection != nullptr) {
    // KVSWebRTC close is handled by the KVSWebRTCManager.
  }
}

void WebrtcTransport::AddTracks() {
  if (mPeerConnection != nullptr) {
    mVideoTrack = mPeerConnection->AddTrack(MediaType::Video);
    mAudioTrack = mPeerConnection->AddTrack(MediaType::Audio);
  }
}

// Implementation of SetVideoTrack method
void WebrtcTransport::SetVideoTrack(std::shared_ptr<WebRTCTrack> videoTrack) {
  ChipLogProgress(Camera, "Setting video track for sessionID: %u",
                  mRequestArgs.sessionId);
  mVideoTrack = videoTrack;
}

// Implementation of SetAudioTrack method
void WebrtcTransport::SetAudioTrack(std::shared_ptr<WebRTCTrack> audioTrack) {
  ChipLogProgress(Camera, "Setting audio track for sessionID: %u",
                  mRequestArgs.sessionId);
  mAudioTrack = audioTrack;
}

void WebrtcTransport::AddRemoteCandidate(const std::string &candidate,
                                         const std::string &mid) {
  ChipLogProgress(Camera, "Adding remote candidate for sessionID: %u",
                  mRequestArgs.sessionId);
  mPeerConnection->AddRemoteCandidate(candidate, mid);
}

// WebRTC Callbacks
void WebrtcTransport::OnLocalDescription(const std::string &sdp, SDPType type) {
  ChipLogProgress(Camera, "Local description received for sessionID: %u",
                  mRequestArgs.sessionId);
  mLocalSdp = sdp;
  mLocalSdpType = type;
  if (mOnLocalDescription)
    mOnLocalDescription(sdp, type, mRequestArgs.sessionId);
}

bool WebrtcTransport::ClosePeerConnection() {
  if (mPeerConnection == nullptr) {
    return false;
  }
  // KVSWebRTC close is handled by the KVSWebRTCManager.
  return true;
}

void WebrtcTransport::OnICECandidate(const std::string &candidate) {
  ChipLogProgress(Camera, "ICE Candidate received for sessionID: %u",
                  mRequestArgs.sessionId);
  mLocalCandidates.push_back(candidate);
  ChipLogProgress(Camera, "Local Candidate:");
  ChipLogProgress(Camera, "%s", candidate.c_str());
}

void WebrtcTransport::OnConnectionStateChanged(bool connected) {
  ChipLogProgress(Camera, "Connection state changed for sessionID: %u",
                  mRequestArgs.sessionId);
  if (mOnConnectionState)
    mOnConnectionState(connected, mRequestArgs.sessionId);
}

void WebrtcTransport::OnTrack(std::shared_ptr<WebRTCTrack> track) {
  ChipLogProgress(Camera, "Track received for sessionID: %u, type: %s",
                  mRequestArgs.sessionId, track->GetType().c_str());
  if (track->GetType() == "video") {
    ChipLogProgress(Camera, "Video track updated from remote peer");
    SetVideoTrack(track);
  } else if (track->GetType() == "audio") {
    ChipLogProgress(Camera, "audio track updated from remote peer");
    SetAudioTrack(track);
  }
}
