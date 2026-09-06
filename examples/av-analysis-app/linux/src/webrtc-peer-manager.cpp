/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "webrtc-peer-manager.h"

#include <platform/CHIPDeviceLayer.h>
#include <system/SystemLayer.h>

namespace chip {
namespace app {

namespace {

constexpr int kVideoH264PayloadType = 96;

// libdatachannel invokes its callbacks on its own threads; Matter state may only be touched on the
// Matter thread. The peer connection is held weakly so a callback racing a teardown lapses.
void ScheduleOnMatterThread(const std::weak_ptr<rtc::PeerConnection> & aWeakPeerConnection,
                            std::function<void(const std::shared_ptr<rtc::PeerConnection> &)> && aCallback)
{
    struct CallbackState
    {
        std::weak_ptr<rtc::PeerConnection> weakPeerConnection;
        std::function<void(const std::shared_ptr<rtc::PeerConnection> &)> callback;
    };

    auto * callbackState = new CallbackState{ aWeakPeerConnection, std::move(aCallback) };
    CHIP_ERROR err       = DeviceLayer::SystemLayer().ScheduleLambda([callbackState]() {
        std::unique_ptr<CallbackState> guard(callbackState);
        auto peerConnection = guard->weakPeerConnection.lock();
        if (peerConnection)
        {
            guard->callback(peerConnection);
        }
    });
    if (err != CHIP_NO_ERROR)
    {
        delete callbackState;
        ChipLogError(AppServer, "AvAnalysisNode: WebRTC callback not scheduled: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

} // namespace

WebRTCPeerManager::WebRTCPeerManager()
{
    rtc::InitLogger(rtc::LogLevel::Warning);
}

WebRTCPeerManager::~WebRTCPeerManager()
{
    ReleasePendingSession();
    for (auto & entry : mSessions)
    {
        if (entry.second.peerConnection)
        {
            entry.second.peerConnection->close();
        }
    }
}

CHIP_ERROR WebRTCPeerManager::CreateOffer(OfferCallback & aCallback)
{
    if (HasPendingSession())
    {
        ChipLogError(AppServer, "AvAnalysisNode: an offer was still pending when the next was asked for");
        ReleasePendingSession();
    }

    rtc::Configuration config;
    config.disableAutoNegotiation = true;
    auto peerConnection           = std::make_shared<rtc::PeerConnection>(config);
    // Non-owning handle for the callbacks below
    std::weak_ptr<rtc::PeerConnection> weakPeerConnection = peerConnection;

    OfferCallback * callback = &aCallback;
    peerConnection->onLocalDescription([this, weakPeerConnection, callback](rtc::Description aDescription) {
        std::string sdp(aDescription);
        ScheduleOnMatterThread(weakPeerConnection, [this, callback, sdp](const std::shared_ptr<rtc::PeerConnection> & aConnection) {
            // Only the pending connection's offer is awaited; a stale description is dropped
            VerifyOrReturn(mPendingSession.peerConnection == aConnection);
            callback->OnOfferReady(CHIP_NO_ERROR, CharSpan(sdp.data(), sdp.size()));
        });
    });

    peerConnection->onLocalCandidate([this, weakPeerConnection](rtc::Candidate aCandidate) {
        std::string candidate(aCandidate);
        ScheduleOnMatterThread(weakPeerConnection, [this, candidate](const std::shared_ptr<rtc::PeerConnection> & aConnection) {
            PeerSession * session = FindSession(aConnection);
            VerifyOrReturn(session != nullptr);
            // TODO: trickle to the camera once the client supports sending ICE candidates
            session->localCandidates.push_back(candidate);
        });
    });

    peerConnection->onStateChange([weakPeerConnection](rtc::PeerConnection::State aState) {
        ScheduleOnMatterThread(weakPeerConnection, [aState](const std::shared_ptr<rtc::PeerConnection> &) {
            // TODO: report Failed/Closed to the WebRTC client once it routes media-layer failures
            ChipLogProgress(AppServer, "AvAnalysisNode: peer connection state %d", static_cast<int>(aState));
        });
    });

    // The offer asks to receive the camera's H.264 video
    rtc::Description::Video media("video", rtc::Description::Direction::RecvOnly);
    media.addH264Codec(kVideoH264PayloadType);
    auto videoTrack = peerConnection->addTrack(media);
    videoTrack->setMediaHandler(std::make_shared<rtc::RtcpReceivingSession>());

    auto firstPacketSeen = std::make_shared<bool>(false);
    videoTrack->onMessage(
        [firstPacketSeen](rtc::binary aMessage) {
            if (!*firstPacketSeen)
            {
                *firstPacketSeen = true;
                ChipLogProgress(AppServer, "AvAnalysisNode: media flowing (first RTP packet, %u bytes)",
                                static_cast<unsigned>(aMessage.size()));
            }
        },
        nullptr);

    mPendingSession.peerConnection = peerConnection;
    mPendingSession.videoTrack     = videoTrack;

    // Generates the offer; it arrives through onLocalDescription
    peerConnection->setLocalDescription();
    return CHIP_NO_ERROR;
}

void WebRTCPeerManager::OnSessionAssigned(uint16_t aWebRTCSessionId)
{
    VerifyOrReturn(HasPendingSession(),
                   ChipLogError(AppServer, "AvAnalysisNode: session %u assigned with no offer pending", aWebRTCSessionId));

    ChipLogProgress(AppServer, "AvAnalysisNode: peer connection bound to WebRTC session %u", aWebRTCSessionId);
    mSessions[aWebRTCSessionId] = std::move(mPendingSession);
    mPendingSession             = PeerSession{}; // the move already nulled the connection; this flushes the rest
}

void WebRTCPeerManager::OnOfferAbandoned()
{
    // Nothing pending after a CreateOffer that failed before creating the connection
    VerifyOrReturn(HasPendingSession());

    ChipLogProgress(AppServer, "AvAnalysisNode: releasing the peer connection of an abandoned offer");
    ReleasePendingSession();
}

void WebRTCPeerManager::ReleasePendingSession()
{
    if (HasPendingSession())
    {
        mPendingSession.peerConnection->close();
    }
    mPendingSession = PeerSession{};
}

void WebRTCPeerManager::OnSessionClosed(uint16_t aWebRTCSessionId)
{
    auto it = mSessions.find(aWebRTCSessionId);
    VerifyOrReturn(it != mSessions.end());

    ChipLogProgress(AppServer, "AvAnalysisNode: releasing the peer connection of WebRTC session %u", aWebRTCSessionId);
    if (it->second.peerConnection)
    {
        it->second.peerConnection->close();
    }
    mSessions.erase(it);
}

CHIP_ERROR WebRTCPeerManager::ApplyAnswer(uint16_t aWebRTCSessionId, const std::string & aSdp)
{
    auto it = mSessions.find(aWebRTCSessionId);
    VerifyOrReturnError(it != mSessions.end(), CHIP_ERROR_NOT_FOUND);

    it->second.peerConnection->setRemoteDescription(rtc::Description(aSdp, rtc::Description::Type::Answer));
    return CHIP_NO_ERROR;
}

CHIP_ERROR WebRTCPeerManager::AddRemoteCandidate(uint16_t aWebRTCSessionId, const std::string & aCandidate)
{
    auto it = mSessions.find(aWebRTCSessionId);
    VerifyOrReturnError(it != mSessions.end(), CHIP_ERROR_NOT_FOUND);

    it->second.peerConnection->addRemoteCandidate(rtc::Candidate(aCandidate));
    return CHIP_NO_ERROR;
}

WebRTCPeerManager::PeerSession * WebRTCPeerManager::FindSession(const std::shared_ptr<rtc::PeerConnection> & aPeerConnection)
{
    if (mPendingSession.peerConnection == aPeerConnection)
    {
        return &mPendingSession;
    }
    for (auto & entry : mSessions)
    {
        if (entry.second.peerConnection == aPeerConnection)
        {
            return &entry.second;
        }
    }
    return nullptr;
}

} // namespace app
} // namespace chip
