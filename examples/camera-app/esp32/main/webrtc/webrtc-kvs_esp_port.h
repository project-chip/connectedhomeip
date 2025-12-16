#pragma once
#include "webrtc-abstract.h"
#include "webrtc-provider-manager.h"
#include <lib/support/logging/CHIPLogging.h>

#include <signaling_serializer.h>
#include <sstream>
#include <string.h>
#include <time.h>

void webrtc_bridge_message_received_cb(void *data, int len);

class EspWebRTCPeerConnection {
private:
  std::string peerConnectionId;

public:
  EspWebRTCPeerConnection(std::string peerConnectionId) {
    this->peerConnectionId = peerConnectionId;
  }
  std::string GetPeerConnectionId() { return peerConnectionId; }
};

class KVSWebRTCPeerConnection : public WebRTCPeerConnection {
public:
  KVSWebRTCPeerConnection();

  void SetCallbacks(OnLocalDescriptionCallback onLocalDescription,
                    OnICECandidateCallback onICECandidate,
                    OnConnectionStateCallback onConnectionState,
                    OnTrackCallback onTrack) override;
  void Close() override;
  void CreateOffer(uint16_t sessionId) override;
  void CreateAnswer() override;
  void SetRemoteDescription(const std::string &sdp, SDPType type) override;
  void AddRemoteCandidate(const std::string &candidate,
                          const std::string &mid) override;
  std::shared_ptr<WebRTCTrack> AddTrack(MediaType mediaType) override;
  std::string GetPeerConnectionId();

private:
  std::shared_ptr<EspWebRTCPeerConnection> mPeerConnection;
};
