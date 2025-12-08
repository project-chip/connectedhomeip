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

#include "webrtc-provider-manager.h"

#include <Options.h>
#include <app/server/Server.h>
#include <controller/InvokeInteraction.h>
#include <lib/support/CHIPFaultInjection.h>
#include <lib/support/logging/CHIPLogging.h>
#include <webrtc-transport.h>

#include <iostream>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WebRTCTransportProvider;

namespace {

// Constants
constexpr uint16_t kMaxConcurrentWebRTCSessions = 5;

} // namespace

void WebRTCProviderManager::SetCameraDevice(CameraDeviceInterface * aCameraDevice)
{
    mCameraDevice = aCameraDevice;
}

void WebRTCProviderManager::Init()
{
    ChipLogProgress(Camera, "Initializing WebRTC PeerConnection");
}

void WebRTCProviderManager::CloseConnection()
{
    // Clean up all the Webrtc Transports
    mWebrtcTransportMap.clear();
    mSessionIdMap.clear();
}

void WebRTCProviderManager::SetMediaController(MediaController * mediaController)
{
    mMediaController = mediaController;
}

void WebRTCProviderManager::SetWebRTCTransportProvider(WebRTCTransportProviderCluster * webRTCTransportProvider)
{
    mWebRTCTransportProvider = webRTCTransportProvider;
}

CHIP_ERROR WebRTCProviderManager::HandleSolicitOffer(const OfferRequestArgs & args, WebRTCSessionStruct & outSession,
                                                     bool & outDeferredOffer)
{
    // Initialize a new WebRTC session from the SolicitOfferRequestArgs
    outSession.id             = args.sessionId;
    outSession.peerNodeID     = args.peerNodeId;
    outSession.peerEndpointID = args.originatingEndpointId;
    outSession.streamUsage    = args.streamUsage;
    outSession.fabricIndex    = args.fabricIndex;
    uint16_t videoStreamID    = 0;
    uint16_t audioStreamID    = 0;

    // Resolve or allocate a VIDEO stream
    if (args.videoStreamId.HasValue())
    {
        // Stream has been validated and potentially selected by ValidateStreamUsage()
        // in the cluster server before invoking this delegate method
        const auto & videoStreamIdNullable = args.videoStreamId.Value();
        outSession.videoStreamID           = videoStreamIdNullable;
        if (!videoStreamIdNullable.IsNull())
        {
            videoStreamID = videoStreamIdNullable.Value();
        }
    }
    else
    {
        outSession.videoStreamID.SetNull();
    }

    // Resolve or allocate an AUDIO stream
    if (args.audioStreamId.HasValue())
    {
        // Stream has been validated and potentially selected by ValidateStreamUsage()
        // in the cluster server before invoking this delegate method
        const auto & audioStreamIdNullable = args.audioStreamId.Value();
        outSession.audioStreamID           = audioStreamIdNullable;
        if (!audioStreamIdNullable.IsNull())
        {
            audioStreamID = audioStreamIdNullable.Value();
        }
    }
    else
    {
        outSession.audioStreamID.SetNull();
    }

    outDeferredOffer = LinuxDeviceOptions::GetInstance().cameraDeferredOffer;

    WebrtcTransport * transport = GetTransport(args.sessionId);
    WebrtcTransport::RequestArgs requestArgs;
    requestArgs.sessionId             = args.sessionId;
    requestArgs.fabricIndex           = args.fabricIndex;
    requestArgs.peerNodeId            = args.peerNodeId;
    requestArgs.originatingEndpointId = args.originatingEndpointId;
    requestArgs.videoStreamId         = videoStreamID;
    requestArgs.audioStreamId         = audioStreamID;
    requestArgs.peerId                = ScopedNodeId(args.peerNodeId, args.fabricIndex);

    if (transport == nullptr)
    {
        mWebrtcTransportMap[args.sessionId]                            = std::unique_ptr<WebrtcTransport>(new WebrtcTransport());
        mSessionIdMap[ScopedNodeId(args.peerNodeId, args.fabricIndex)] = args.sessionId;
        transport                                                      = mWebrtcTransportMap[args.sessionId].get();
        transport->SetCallbacks(
            [this](const std::string & sdp, SDPType type, const uint16_t sessionId) {
                this->OnLocalDescription(sdp, type, sessionId);
            },
            [this](bool connected, const uint16_t sessionId) { this->OnConnectionStateChanged(connected, sessionId); });
    }

    transport->SetRequestArgs(requestArgs);

    // Store SFrameConfig in Transport base class if provided for later use in frame encryption
    if (args.sFrameConfig.HasValue())
    {
        transport->sFrameConfig = args.sFrameConfig;
        ChipLogProgress(Camera, "SFrame encryption enabled for session %u", args.sessionId);
    }

    // Check resource availability before proceeding
    // If we cannot allocate resources, send End command with OutOfResources reason
    if (mWebrtcTransportMap.size() > kMaxConcurrentWebRTCSessions)
    {
        ChipLogProgress(Camera, "Resource exhaustion detected: maximum WebRTC sessions (%u)", kMaxConcurrentWebRTCSessions);

        transport->SetCommandType(WebrtcTransport::CommandType::kEnd);
        transport->MoveToState(WebrtcTransport::State::SendingEnd);

        // The resource exhaustion happens internally in the DUT, but it still creates a session
        // and then sends an End command with OutOfResources reason.
        ScheduleEndSend(args.sessionId);

        return CHIP_NO_ERROR;
    }

    transport->Start();
    transport->AddAudioTrack();
    transport->AddVideoTrack();

    // Acquire the Video and Audio Streams from the CameraAVStreamManagement
    // cluster and update the reference counts.
    TEMPORARY_RETURN_IGNORED AcquireAudioVideoStreams(args.sessionId);

    transport->MoveToState(WebrtcTransport::State::SendingOffer);

    ChipLogProgress(Camera, "Generate and set the SDP");
    if (transport->GetPeerConnection())
        transport->GetPeerConnection()->CreateOffer();

    return CHIP_NO_ERROR;
}

void WebRTCProviderManager::RegisterWebrtcTransport(uint16_t sessionId)
{

    WebrtcTransport * transport = GetTransport(sessionId);
    if (transport == nullptr)
    {
        ChipLogProgress(Camera, "WebRTC Transport is null for sessionId %u. Failed to Register WebRTC Transport", sessionId);
        return;
    }

    if (mMediaController == nullptr)
    {
        ChipLogProgress(Camera, "mMediaController is null. Failed to Register WebRTC Transport");
        return;
    }

    WebrtcTransport::RequestArgs args = transport->GetRequestArgs();
    mMediaController->RegisterTransport(transport, args.videoStreamId, args.audioStreamId);
}

void WebRTCProviderManager::UnregisterWebrtcTransport(uint16_t sessionId)
{
    ChipLogProgress(Camera, "UnregisterWebrtcTransport called for sessionId: %u", sessionId);

    WebrtcTransport * transport = GetTransport(sessionId);
    if (transport == nullptr)
    {
        ChipLogProgress(Camera, "WebRTC Transport is null for sessionId %u. Already unregistered or not found", sessionId);
        return;
    }

    if (mMediaController == nullptr)
    {
        ChipLogProgress(Camera, "mMediaController is null. Cannot unregister WebRTC Transport");
        return;
    }

    mMediaController->UnregisterTransport(transport);
    ChipLogProgress(Camera, "Successfully unregistered transport for sessionId: %u", sessionId);
}

std::string WebRTCProviderManager::ExtractMidFromSdp(const std::string & sdp, const std::string & mediaType)
{
    if (sdp.empty() || mediaType.empty())
    {
        ChipLogError(Camera, "ExtractMidFromSdp: empty SDP or media type");
        return "";
    }

    const std::string mediaPrefix = "m=" + mediaType;
    const std::string midPrefix   = "a=mid:";

    std::istringstream stream(sdp);
    std::string line;
    bool inTargetBlock = false;

    while (std::getline(stream, line))
    {
        // Trim possible Windows carriage return
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        if (inTargetBlock)
        {
            if (line.rfind(midPrefix, 0) == 0) // line starts with "a=mid:"
                return line.substr(midPrefix.length());

            if (line.rfind("m=", 0) == 0) // next media block – stop searching
                break;
        }
        else if (line.rfind(mediaPrefix, 0) == 0) // found the desired media block
        {
            inTargetBlock = true;
        }
    }

    // No MID found for the requested media type
    return "";
}

CHIP_ERROR WebRTCProviderManager::HandleProvideOffer(const ProvideOfferRequestArgs & args, WebRTCSessionStruct & outSession)
{
    ChipLogProgress(Camera, "HandleProvideOffer called");

    // Initialize a new WebRTC session from the ProvideOfferRequestArgs
    outSession.id             = args.sessionId;
    outSession.peerNodeID     = args.peerNodeId;
    outSession.peerEndpointID = args.originatingEndpointId;
    outSession.streamUsage    = args.streamUsage;
    outSession.fabricIndex    = args.fabricIndex;
    uint16_t videoStreamID    = 0;
    uint16_t audioStreamID    = 0;

    // Resolve or allocate a VIDEO stream
    if (args.videoStreamId.HasValue())
    {
        // Stream has been validated and potentially selected by ValidateStreamUsage()
        // in the cluster server before invoking this delegate method
        const auto & videoStreamIdNullable = args.videoStreamId.Value();
        outSession.videoStreamID           = videoStreamIdNullable;
        if (!videoStreamIdNullable.IsNull())
        {
            videoStreamID = videoStreamIdNullable.Value();
        }
    }
    else
    {
        outSession.videoStreamID.SetNull();
    }

    // Resolve or allocate an AUDIO stream
    if (args.audioStreamId.HasValue())
    {
        // Stream has been validated and potentially selected by ValidateStreamUsage()
        // in the cluster server before invoking this delegate method
        const auto & audioStreamIdNullable = args.audioStreamId.Value();
        outSession.audioStreamID           = audioStreamIdNullable;
        if (!audioStreamIdNullable.IsNull())
        {
            audioStreamID = audioStreamIdNullable.Value();
        }
    }
    else
    {
        outSession.audioStreamID.SetNull();
    }

    // Process the SDP Offer, begin the ICE Candidate gathering phase, create the SDP Answer, and invoke Answer.
    WebrtcTransport::RequestArgs requestArgs;
    requestArgs.sessionId             = args.sessionId;
    requestArgs.fabricIndex           = args.fabricIndex;
    requestArgs.peerNodeId            = args.peerNodeId;
    requestArgs.originatingEndpointId = args.originatingEndpointId;
    requestArgs.videoStreamId         = videoStreamID;
    requestArgs.audioStreamId         = audioStreamID;
    requestArgs.peerId                = ScopedNodeId(args.peerNodeId, args.fabricIndex);

    WebrtcTransport * transport = GetTransport(args.sessionId);
    if (transport == nullptr)
    {
        mWebrtcTransportMap[args.sessionId]                            = std::unique_ptr<WebrtcTransport>(new WebrtcTransport());
        mSessionIdMap[ScopedNodeId(args.peerNodeId, args.fabricIndex)] = args.sessionId;
        transport                                                      = mWebrtcTransportMap[args.sessionId].get();
        transport->SetCallbacks(
            [this](const std::string & sdp, SDPType type, const uint16_t sessionId) {
                this->OnLocalDescription(sdp, type, sessionId);
            },
            [this](bool connected, const uint16_t sessionId) { this->OnConnectionStateChanged(connected, sessionId); });
    }

    // Check resource availability before proceeding
    // If we cannot allocate resources, respond with a response status of RESOURCE_EXHAUSTED
    if (mWebrtcTransportMap.size() > kMaxConcurrentWebRTCSessions)
    {
        ChipLogProgress(Camera, "Resource exhaustion detected in ProvideOffer: maximum WebRTC sessions (%u)",
                        kMaxConcurrentWebRTCSessions);
        return CHIP_IM_GLOBAL_STATUS(ResourceExhausted);
    }

    transport->SetRequestArgs(requestArgs);

    // Store SFrameConfig in Transport base class if provided for later use in frame encryption
    if (args.sFrameConfig.HasValue())
    {
        transport->sFrameConfig = args.sFrameConfig;
        ChipLogProgress(Camera, "SFrame encryption enabled for session %u", args.sessionId);
    }

    transport->Start();
    auto peerConnection  = transport->GetPeerConnection();
    std::string audioMid = ExtractMidFromSdp(args.sdp, "audio");
    std::string videoMid = ExtractMidFromSdp(args.sdp, "video");
    int videoPt          = peerConnection->GetPayloadType(args.sdp, SDPType::Offer, /*codec*/ "H264");
    int audioPt          = peerConnection->GetPayloadType(args.sdp, SDPType::Offer, /*codec*/ "opus");

    ChipLogProgress(Camera, "Extracted audioMid: %s, payloadType: %d", audioMid.c_str(), audioPt);
    ChipLogProgress(Camera, "Extracted videoMid: %s, payloadType: %d", videoMid.c_str(), videoPt);

    transport->AddVideoTrack(videoMid, videoPt);
    transport->AddAudioTrack(audioMid, audioPt);

    // Acquire the Video and Audio Streams from the CameraAVStreamManagement
    // cluster and update the reference counts.
    TEMPORARY_RETURN_IGNORED AcquireAudioVideoStreams(args.sessionId);

    transport->MoveToState(WebrtcTransport::State::SendingAnswer);

    if (peerConnection != nullptr)
    {
        transport->GetPeerConnection()->SetRemoteDescription(args.sdp, SDPType::Offer);
        transport->GetPeerConnection()->CreateAnswer();
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR WebRTCProviderManager::HandleProvideAnswer(uint16_t sessionId, const std::string & sdpAnswer)
{
    ChipLogProgress(Camera, "HandleProvideAnswer called with sessionId: %u", sessionId);

    // Check if the provided sessionId matches your current sessions
    WebrtcTransport * transport = GetTransport(sessionId);
    if (transport == nullptr)
    {
        ChipLogError(Camera, "Session ID %u does not match the current sessions", sessionId);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (sdpAnswer.empty())
    {
        ChipLogError(Camera, "Provided SDP Answer is empty for session ID %u", sessionId);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (!transport->GetPeerConnection())
    {
        ChipLogError(Camera, "Cannot set remote description: mPeerConnection is null for session ID %u", sessionId);
        return CHIP_ERROR_INCORRECT_STATE;
    }

    // Check if we already received an SDP answer (duplicate answer scenario)
    // If we're already in SendingICECandidates or later state, we've already processed an answer
    if (transport->GetState() != WebrtcTransport::State::SendingOffer && transport->GetState() != WebrtcTransport::State::Idle)
    {
        ChipLogProgress(Camera, "Ignoring duplicate SDP answer for session ID %u (current state: %s)", sessionId,
                        transport->GetStateStr());
        return CHIP_NO_ERROR;
    }

    transport->GetPeerConnection()->SetRemoteDescription(sdpAnswer, SDPType::Answer);

    transport->MoveToState(WebrtcTransport::State::SendingICECandidates);
    ScheduleICECandidatesSend(sessionId);

    return CHIP_NO_ERROR;
}

CHIP_ERROR WebRTCProviderManager::HandleProvideICECandidates(uint16_t sessionId, const std::vector<ICECandidateStruct> & candidates)
{
    ChipLogProgress(Camera, "HandleProvideICECandidates called with session ID %u", sessionId);

    // Check if the provided sessionId matches your current sessions
    WebrtcTransport * transport = GetTransport(sessionId);
    if (transport == nullptr)
    {
        ChipLogError(Camera, "Session ID %u does not match the current sessions", sessionId);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (!transport->GetPeerConnection())
    {
        ChipLogError(Camera, "Cannot process ICE candidates: PeerConnection is null for session ID %u", sessionId);
        return CHIP_ERROR_INCORRECT_STATE;
    }

    if (candidates.empty())
    {
        ChipLogError(Camera, "Candidate list is empty. At least one candidate is expected.");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    for (const auto & candidate : candidates)
    {
        ChipLogProgress(Camera, "Applying candidate: %s",
                        std::string(candidate.candidate.begin(), candidate.candidate.end()).c_str());
        std::string mid =
            candidate.SDPMid.IsNull() ? "" : std::string(candidate.SDPMid.Value().begin(), candidate.SDPMid.Value().end());
        transport->AddRemoteCandidate(std::string(candidate.candidate.begin(), candidate.candidate.end()), mid);
    }

    // Schedule sending Ice Candidates when remote candidates are received. This keeps the exchange simple
    transport->MoveToState(WebrtcTransport::State::SendingICECandidates);
    ScheduleICECandidatesSend(sessionId);

    return CHIP_NO_ERROR;
}

CHIP_ERROR WebRTCProviderManager::HandleEndSession(uint16_t sessionId, WebRTCEndReasonEnum reasonCode,
                                                   DataModel::Nullable<uint16_t> videoStreamID,
                                                   DataModel::Nullable<uint16_t> audioStreamID)
{
    WebrtcTransport * transport = GetTransport(sessionId);
    if (transport == nullptr)
    {
        ChipLogError(Camera, "Session ID %u does not match the current sessions", sessionId);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (transport != nullptr)
    {
        ChipLogProgress(Camera, "Delete Webrtc Transport for the session: %u", sessionId);

        // Release the Video and Audio Streams from the CameraAVStreamManagement
        // cluster and update the reference counts.
        // TODO: Lookup the sessionID to get the Video/Audio StreamID
        TEMPORARY_RETURN_IGNORED ReleaseAudioVideoStreams(sessionId);

        UnregisterWebrtcTransport(sessionId);
        mWebrtcTransportMap.erase(sessionId);
        WebrtcTransport::RequestArgs args = transport->GetRequestArgs();
        mSessionIdMap.erase(ScopedNodeId(args.peerNodeId, args.fabricIndex));
    }

    if (transport->ClosePeerConnection())
    {
        ChipLogProgress(Camera, "Closing peer connection: %u", sessionId);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR
WebRTCProviderManager::ValidateStreamUsage(StreamUsageEnum streamUsage, Optional<DataModel::Nullable<uint16_t>> & videoStreamId,
                                           Optional<DataModel::Nullable<uint16_t>> & audioStreamId)
{
    if (mCameraDevice == nullptr)
    {
        ChipLogError(Camera, "CameraDeviceInterface not initialized");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    auto & avsmController = mCameraDevice->GetCameraAVStreamMgmtController();

    return avsmController.ValidateStreamUsage(streamUsage, videoStreamId, audioStreamId);
}

CHIP_ERROR WebRTCProviderManager::ValidateVideoStreamID(uint16_t videoStreamId)
{
    if (mCameraDevice == nullptr)
    {
        ChipLogError(Camera, "CameraDeviceInterface not initialized");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    auto & avsmController = mCameraDevice->GetCameraAVStreamMgmtController();

    return avsmController.ValidateVideoStreamID(videoStreamId);
}

CHIP_ERROR WebRTCProviderManager::ValidateAudioStreamID(uint16_t audioStreamId)
{
    if (mCameraDevice == nullptr)
    {
        ChipLogError(Camera, "CameraDeviceInterface not initialized");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    auto & avsmController = mCameraDevice->GetCameraAVStreamMgmtController();

    return avsmController.ValidateAudioStreamID(audioStreamId);
}

CHIP_ERROR WebRTCProviderManager::IsStreamUsageSupported(StreamUsageEnum streamUsage)
{
    if (mCameraDevice == nullptr)
    {
        ChipLogError(Camera, "CameraDeviceInterface not initialized");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    auto & hal                   = mCameraDevice->GetCameraHALInterface();
    auto & streamUsagePriorities = hal.GetStreamUsagePriorities();

    // Check if the streamUsage is in the StreamUsagePriorities list
    for (const auto & usage : streamUsagePriorities)
    {
        if (usage == streamUsage)
        {
            return CHIP_NO_ERROR;
        }
    }

    ChipLogError(Camera, "StreamUsage %u not found in StreamUsagePriorities", to_underlying(streamUsage));
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR WebRTCProviderManager::IsHardPrivacyModeActive(bool & isActive)
{
    if (mCameraDevice == nullptr)
    {
        ChipLogError(Camera, "CameraDeviceInterface not initialized");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    auto & avsmController = mCameraDevice->GetCameraAVStreamMgmtController();

    return avsmController.IsHardPrivacyModeActive(isActive);
}

CHIP_ERROR WebRTCProviderManager::IsSoftRecordingPrivacyModeActive(bool & isActive)
{
    if (mCameraDevice == nullptr)
    {
        ChipLogError(Camera, "CameraDeviceInterface not initialized");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    auto & avsmController = mCameraDevice->GetCameraAVStreamMgmtController();

    return avsmController.IsSoftRecordingPrivacyModeActive(isActive);
}

CHIP_ERROR WebRTCProviderManager::IsSoftLivestreamPrivacyModeActive(bool & isActive)
{
    if (mCameraDevice == nullptr)
    {
        ChipLogError(Camera, "CameraDeviceInterface not initialized");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    auto & avsmController = mCameraDevice->GetCameraAVStreamMgmtController();

    return avsmController.IsSoftLivestreamPrivacyModeActive(isActive);
}

bool WebRTCProviderManager::HasAllocatedVideoStreams()
{
    if (mCameraDevice == nullptr)
    {
        ChipLogError(Camera, "CameraDeviceInterface not initialized");
        return false;
    }

    auto & avsmController = mCameraDevice->GetCameraAVStreamMgmtController();

    return avsmController.HasAllocatedVideoStreams();
}

bool WebRTCProviderManager::HasAllocatedAudioStreams()
{
    if (mCameraDevice == nullptr)
    {
        ChipLogError(Camera, "CameraDeviceInterface not initialized");
        return false;
    }

    auto & avsmController = mCameraDevice->GetCameraAVStreamMgmtController();

    return avsmController.HasAllocatedAudioStreams();
}

CHIP_ERROR WebRTCProviderManager::ValidateSFrameConfig(uint16_t cipherSuite, size_t baseKeyLength)
{
    // Define supported cipher suites and their expected key lengths
    // Based on SFrame RFC: https://datatracker.ietf.org/doc/html/draft-ietf-sframe-enc
    // 0x0001: AES-128-GCM-SHA256 (16 byte key)
    // 0x0002: AES-256-GCM-SHA512 (32 byte key)
    constexpr uint16_t kCipherSuite_AES_128_GCM = 0x0001;
    constexpr uint16_t kCipherSuite_AES_256_GCM = 0x0002;
    constexpr size_t kAES_128_KeyLength         = 16;
    constexpr size_t kAES_256_KeyLength         = 32;

    size_t expectedKeyLength = 0;

    // Validate cipher suite and determine expected key length
    switch (cipherSuite)
    {
    case kCipherSuite_AES_128_GCM:
        expectedKeyLength = kAES_128_KeyLength;
        break;
    case kCipherSuite_AES_256_GCM:
        expectedKeyLength = kAES_256_KeyLength;
        break;
    default:
        ChipLogError(Camera, "Unsupported SFrame cipher suite 0x%04X", cipherSuite);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Validate base key length matches the expected length for the cipher suite
    if (baseKeyLength != expectedKeyLength)
    {
        ChipLogError(Camera, "SFrame base key length mismatch - expected %u bytes for cipher suite 0x%04X, got %u bytes",
                     static_cast<unsigned int>(expectedKeyLength), cipherSuite, static_cast<unsigned int>(baseKeyLength));
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR WebRTCProviderManager::IsUTCTimeNull(bool & isNull)
{
    // TODO: The implementation SHALL:
    //  - Read the UTCTime attribute from the Time Synchronization cluster (0x0038)
    //  - Return whether the attribute is null or has a valid value

    return CHIP_NO_ERROR;
}

void WebRTCProviderManager::ScheduleOfferSend(uint16_t sessionId)
{
    ChipLogProgress(Camera, "ScheduleOfferSend called.");

    TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda([this, sessionId]() {
        WebrtcTransport * transport = GetTransport(sessionId);
        if (transport == nullptr)
        {
            return;
        }

        WebrtcTransport::RequestArgs args = transport->GetRequestArgs();
        ChipLogProgress(Camera, "Sending Offer command to node " ChipLogFormatX64, ChipLogValueX64(args.peerNodeId));

        transport->SetCommandType(WebrtcTransport::CommandType::kOffer);

        // Attempt to find or establish a CASE session to the target PeerId.
        CASESessionManager * caseSessionMgr = Server::GetInstance().GetCASESessionManager();
        VerifyOrDie(caseSessionMgr != nullptr);

        // WebRTC Answer requires a large payload session establishment.
        caseSessionMgr->FindOrEstablishSession(args.peerId, &mOnConnectedCallback, &mOnConnectionFailureCallback,
                                               TransportPayloadCapability::kLargePayload);
    });
}

void WebRTCProviderManager::ScheduleAnswerSend(uint16_t sessionId)
{
    ChipLogProgress(Camera, "ScheduleAnswerSend called.");

    TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda([this, sessionId]() {
        WebrtcTransport * transport = GetTransport(sessionId);
        if (transport == nullptr)
        {
            return;
        }

        WebrtcTransport::RequestArgs requestArgs = transport->GetRequestArgs();
        chip::ScopedNodeId peerId                = requestArgs.peerId;
        ChipLogProgress(Camera, "Sending Answer command to node " ChipLogFormatX64, ChipLogValueX64(peerId.GetNodeId()));

        transport->SetCommandType(WebrtcTransport::CommandType::kAnswer);

        // Attempt to find or establish a CASE session to the target PeerId.
        CASESessionManager * caseSessionMgr = Server::GetInstance().GetCASESessionManager();
        VerifyOrDie(caseSessionMgr != nullptr);

        // WebRTC Answer requires a large payload session establishment.
        caseSessionMgr->FindOrEstablishSession(peerId, &mOnConnectedCallback, &mOnConnectionFailureCallback,
                                               TransportPayloadCapability::kLargePayload);
    });
}

void WebRTCProviderManager::ScheduleEndSend(uint16_t sessionId)
{
    ChipLogProgress(Camera, "ScheduleEndSend called.");

    TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda([this, sessionId]() {
        WebrtcTransport * transport = GetTransport(sessionId);
        if (transport == nullptr)
        {
            return;
        }

        WebrtcTransport::RequestArgs requestArgs = transport->GetRequestArgs();
        chip::ScopedNodeId peerId                = requestArgs.peerId;
        ChipLogProgress(Camera, "Sending End command to node " ChipLogFormatX64, ChipLogValueX64(peerId.GetNodeId()));

        transport->SetCommandType(WebrtcTransport::CommandType::kEnd);

        // Attempt to find or establish a CASE session to the target PeerId.
        CASESessionManager * caseSessionMgr = Server::GetInstance().GetCASESessionManager();
        VerifyOrDie(caseSessionMgr != nullptr);

        // WebRTC Answer requires a large payload session establishment.
        caseSessionMgr->FindOrEstablishSession(peerId, &mOnConnectedCallback, &mOnConnectionFailureCallback,
                                               TransportPayloadCapability::kLargePayload);
    });
}

void WebRTCProviderManager::ScheduleICECandidatesSend(uint16_t sessionId)
{
    ChipLogProgress(Camera, "ScheduleICECandidatesSend called.");

    TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda([this, sessionId]() {
        WebrtcTransport * transport = GetTransport(sessionId);
        if (transport == nullptr)
        {
            return;
        }

        WebrtcTransport::RequestArgs requestArgs = transport->GetRequestArgs();
        chip::ScopedNodeId peerId                = requestArgs.peerId;
        ChipLogProgress(Camera, "Sending ICECandidates command to node " ChipLogFormatX64, ChipLogValueX64(peerId.GetNodeId()));

        transport->SetCommandType(WebrtcTransport::CommandType::kICECandidates);

        // Attempt to find or establish a CASE session to the target PeerId.
        CASESessionManager * caseSessionMgr = Server::GetInstance().GetCASESessionManager();
        VerifyOrDie(caseSessionMgr != nullptr);

        // WebRTC Answer requires a large payload session establishment.
        caseSessionMgr->FindOrEstablishSession(peerId, &mOnConnectedCallback, &mOnConnectionFailureCallback,
                                               TransportPayloadCapability::kLargePayload);
    });
}

void WebRTCProviderManager::OnDeviceConnected(void * context, Messaging::ExchangeManager & exchangeMgr,
                                              const SessionHandle & sessionHandle)
{
    WebRTCProviderManager * self = reinterpret_cast<WebRTCProviderManager *>(context);
    VerifyOrReturn(self != nullptr, ChipLogError(Camera, "OnDeviceConnected:: context is null"));

    // Derive sessionId from sessionHandle by looking up the peer ScopedNodeId (NodeId + FabricIndex)
    ScopedNodeId peerScopedNodeId = sessionHandle->GetPeer();
    auto sessionIt                = self->mSessionIdMap.find(peerScopedNodeId);
    if (sessionIt == self->mSessionIdMap.end())
    {
        ChipLogError(Camera, "OnDeviceConnected:: no session found for peer ScopedNodeId: [%d:" ChipLogFormatX64 "]",
                     peerScopedNodeId.GetFabricIndex(), ChipLogValueX64(peerScopedNodeId.GetNodeId()));
        return;
    }

    uint16_t sessionId          = sessionIt->second;
    WebrtcTransport * transport = self->GetTransport(sessionId);
    if (transport == nullptr)
    {
        ChipLogError(Camera, "OnDeviceConnected:: transport not found for sessionId: %u", sessionId);
        return;
    }

    ChipLogProgress(Camera, "CASE session established, sending command with Command Type: %d, for sessionID: %u",
                    static_cast<int>(transport->GetCommandType()), sessionId);

    CHIP_ERROR err = CHIP_NO_ERROR;

    switch (transport->GetCommandType())
    {
    case WebrtcTransport::CommandType::kOffer:
        err = self->SendOfferCommand(exchangeMgr, sessionHandle, sessionId);
        transport->MoveToState(WebrtcTransport::State::Idle);
        break;

    case WebrtcTransport::CommandType::kAnswer:
        err = self->SendAnswerCommand(exchangeMgr, sessionHandle, sessionId);
        transport->MoveToState(WebrtcTransport::State::Idle);
        break;
    case WebrtcTransport::CommandType::kICECandidates:
        err = self->SendICECandidatesCommand(exchangeMgr, sessionHandle, sessionId);
        transport->MoveToState(WebrtcTransport::State::Idle);
        break;
    case WebrtcTransport::CommandType::kEnd: {
        // Determine the end reason - check if it's due to privacy mode or resource exhaustion
        WebRTCEndReasonEnum endReason = WebRTCEndReasonEnum::kOutOfResources;

        if (self->mSoftLiveStreamPrivacyEnabled)
        {
            endReason = WebRTCEndReasonEnum::kPrivacyMode;
        }

        err = self->SendEndCommand(exchangeMgr, sessionHandle, sessionId, endReason);
        // Release the Video and Audio Streams from the CameraAVStreamManagement
        // cluster and update the reference counts.
        TEMPORARY_RETURN_IGNORED self->ReleaseAudioVideoStreams(sessionId);
        self->UnregisterWebrtcTransport(sessionId);
        WebrtcTransport::RequestArgs args = transport->GetRequestArgs();
        self->mSessionIdMap.erase(ScopedNodeId(args.peerNodeId, args.fabricIndex));

        transport->MoveToState(WebrtcTransport::State::Idle);

        // remove from current sessions list
        self->mWebRTCTransportProvider->RemoveSession(sessionId);
        self->mWebrtcTransportMap.erase(sessionId);
        break;
    }
    default:
        err = CHIP_ERROR_INVALID_ARGUMENT;
        break;
    }

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "OnDeviceConnected::SendCommand failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void WebRTCProviderManager::OnDeviceConnectionFailure(void * context, const ScopedNodeId & peerId, CHIP_ERROR err)
{
    LogErrorOnFailure(err);
    WebRTCProviderManager * self = reinterpret_cast<WebRTCProviderManager *>(context);
    VerifyOrReturn(self != nullptr, ChipLogError(Camera, "OnDeviceConnectionFailure: context is null"));
}

WebrtcTransport * WebRTCProviderManager::GetTransport(uint16_t sessionId)
{
    WebrtcTransport * transport = nullptr;
    if (mWebrtcTransportMap.find(sessionId) != mWebrtcTransportMap.end())
    {
        transport = mWebrtcTransportMap[sessionId].get();
    }

    return transport;
}

void WebRTCProviderManager::LiveStreamPrivacyModeChanged(bool privacyModeEnabled)
{
    mSoftLiveStreamPrivacyEnabled = privacyModeEnabled;

    if (privacyModeEnabled)
    {
        WebrtcTransport * transport = nullptr;
        uint16_t sessionId          = 0;
        for (auto & mapEntry : mWebrtcTransportMap)
        {
            sessionId = mapEntry.first;

            transport = (WebrtcTransport *) mapEntry.second.get();

            if (transport == nullptr)
            {
                continue;
            }

            transport->MoveToState(WebrtcTransport::State::SendingEnd);

            ScheduleEndSend(sessionId);
        }
    }
    else
    {
        ChipLogProgress(Camera, "Privacy mode is disabled");
    }
}

CHIP_ERROR WebRTCProviderManager::SendOfferCommand(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle,
                                                   uint16_t sessionId)
{
    auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
        ChipLogProgress(Camera, "Offer command succeeds");
    };

    auto onFailure = [](CHIP_ERROR error) { ChipLogError(Camera, "Offer command failed: %" CHIP_ERROR_FORMAT, error.Format()); };

    WebrtcTransport * transport = GetTransport(sessionId);
    if (transport == nullptr)
    {
        ChipLogError(Camera, "Offer command failed, WebTransport not found for sessionId: %u", sessionId);
        return CHIP_ERROR_INTERNAL;
    }

    CHIP_FAULT_INJECT(chip::FaultInjection::kFault_ModifyWebRTCOfferSessionId, sessionId++);

    // Build the command
    WebRTCTransportRequestor::Commands::Offer::Type command;
    command.webRTCSessionID = sessionId;
    std::string localSdp    = transport->GetLocalDescription();
    command.sdp             = CharSpan::fromCharString(localSdp.c_str());

    WebrtcTransport::RequestArgs args = transport->GetRequestArgs();
    // Now invoke the command using the found session handle
    return Controller::InvokeCommandRequest(&exchangeMgr, sessionHandle, args.originatingEndpointId, command, onSuccess, onFailure,
                                            /* timedInvokeTimeoutMs = */ NullOptional, /* responseTimeout = */ NullOptional,
                                            /* outCancelFn = */ nullptr, /*allowLargePayload = */ true);
}

void WebRTCProviderManager::OnLocalDescription(const std::string & sdp, SDPType type, const uint16_t sessionId)
{
    WebrtcTransport * transport = GetTransport(sessionId);
    if (transport == nullptr)
    {
        ChipLogError(Camera, "OnLocalDescription: WebTransport not found for sessionId: %u", sessionId);
        return;
    }

    WebrtcTransport::State state = transport->GetState();
    if (state == WebrtcTransport::State::SendingAnswer && type != SDPType::Answer)
    {
        return;
    }

    const char * typeStr = (type == SDPType::Offer) ? "offer" : "answer";
    std::string localSdp = sdp;
    ChipLogProgress(Camera, "Local Description (%s):", typeStr);
    ChipLogProgress(Camera, "%s", localSdp.c_str());

    switch (state)
    {
    case WebrtcTransport::State::SendingOffer:
        ScheduleOfferSend(sessionId);
        break;
    case WebrtcTransport::State::SendingAnswer:
        ScheduleAnswerSend(sessionId);
        break;
    default:
        break;
    }
}

void WebRTCProviderManager::OnConnectionStateChanged(bool connected, const uint16_t sessionId)
{
    ChipLogProgress(Camera, "Connection state changed for session %u: %s", sessionId, connected ? "connected" : "disconnected");

    if (connected)
    {
        RegisterWebrtcTransport(sessionId);
    }
    else
    {
        // Schedule cleanup on Matter thread to ensure proper locking when calling RemoveSession.
        // Safe to capture 'this' by value: WebRTCProviderManager is a member of the global CameraDevice
        // object which has static storage duration and lives for the entire program lifetime.
        TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda([this, sessionId]() {
            WebrtcTransport * transport = GetTransport(sessionId);
            if (transport == nullptr)
            {
                ChipLogProgress(Camera,
                                "Transport not found for session %u during disconnect; session may have already been cleaned up",
                                sessionId);
                return;
            }

            // Connection was closed/disconnected by the peer - clean up the session
            ChipLogProgress(Camera, "Peer connection closed for session %u, cleaning up resources", sessionId);

            // Release the Video and Audio Streams from the CameraAVStreamManagement
            // cluster and update the reference counts.
            TEMPORARY_RETURN_IGNORED ReleaseAudioVideoStreams(sessionId);

            // Capture args before unregistering in case the transport is invalidated
            WebrtcTransport::RequestArgs args = transport->GetRequestArgs();

            // Unregister the transport from the media controller
            UnregisterWebrtcTransport(sessionId);

            // Remove from session maps
            mSessionIdMap.erase(ScopedNodeId(args.peerNodeId, args.fabricIndex));

            // Remove from current sessions list in the WebRTC Transport Provider
            // This MUST be called on the Matter thread with the stack lock held
            if (mWebRTCTransportProvider != nullptr)
            {
                mWebRTCTransportProvider->RemoveSession(sessionId);
            }

            // Finally, remove and destroy the transport
            mWebrtcTransportMap.erase(sessionId);

            ChipLogProgress(Camera, "Session %u cleanup completed", sessionId);
        });
    }
}

CHIP_ERROR WebRTCProviderManager::SendAnswerCommand(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle,
                                                    uint16_t sessionId)
{
    auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
        ChipLogProgress(Camera, "Answer command succeeds");
    };

    auto onFailure = [](CHIP_ERROR error) { ChipLogError(Camera, "Answer command failed: %" CHIP_ERROR_FORMAT, error.Format()); };

    WebrtcTransport * transport = GetTransport(sessionId);
    if (transport == nullptr)
    {
        ChipLogError(Camera, "Answer command failed, WebTransport not found for sessionId: %u", sessionId);
        return CHIP_ERROR_INTERNAL;
    }

    CHIP_FAULT_INJECT(chip::FaultInjection::kFault_ModifyWebRTCAnswerSessionId, sessionId++);

    // Build the command
    WebRTCTransportRequestor::Commands::Answer::Type command;
    command.webRTCSessionID = sessionId;
    std::string localSdp    = transport->GetLocalDescription();
    command.sdp             = CharSpan::fromCharString(localSdp.c_str());

    WebrtcTransport::RequestArgs requestArgs = transport->GetRequestArgs();
    // Now invoke the command using the found session handle
    return Controller::InvokeCommandRequest(&exchangeMgr, sessionHandle, requestArgs.originatingEndpointId, command, onSuccess,
                                            onFailure,
                                            /* timedInvokeTimeoutMs = */ NullOptional, /* responseTimeout = */ NullOptional,
                                            /* outCancelFn = */ nullptr, /*allowLargePayload = */ true);
}

CHIP_ERROR WebRTCProviderManager::SendICECandidatesCommand(Messaging::ExchangeManager & exchangeMgr,
                                                           const SessionHandle & sessionHandle, uint16_t sessionId)
{
    auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
        ChipLogProgress(Camera, "ICECandidates command succeeds");
    };

    auto onFailure = [](CHIP_ERROR error) {
        ChipLogError(Camera, "ICECandidates command failed: %" CHIP_ERROR_FORMAT, error.Format());
    };

    WebrtcTransport * transport = GetTransport(sessionId);
    if (transport == nullptr)
    {
        ChipLogError(Camera, "WebTransport not found for the sessionId: %u", sessionId);
        return CHIP_ERROR_INTERNAL;
    }

    const std::vector<ICECandidateInfo> & localCandidates = transport->GetCandidates();

    // Build the command
    WebRTCTransportRequestor::Commands::ICECandidates::Type command;

    if (localCandidates.empty())
    {
        ChipLogError(Camera, "No local ICE candidates to send");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    std::vector<ICECandidateStruct> iceCandidateStructList;
    for (const auto & candidateInfo : localCandidates)
    {
        ICECandidateStruct iceCandidate;
        iceCandidate.candidate = CharSpan(candidateInfo.candidate.data(), candidateInfo.candidate.size());

        // Set SDPMid if available
        if (!candidateInfo.mid.empty())
        {
            iceCandidate.SDPMid.SetNonNull(CharSpan(candidateInfo.mid.data(), candidateInfo.mid.size()));
        }
        else
        {
            iceCandidate.SDPMid.SetNull();
        }

        // Set SDPMLineIndex if valid
        if (candidateInfo.mlineIndex >= 0)
        {
            iceCandidate.SDPMLineIndex.SetNonNull(static_cast<uint16_t>(candidateInfo.mlineIndex));
        }
        else
        {
            iceCandidate.SDPMLineIndex.SetNull();
        }

        iceCandidateStructList.push_back(iceCandidate);
    }

    CHIP_FAULT_INJECT(chip::FaultInjection::kFault_ModifyWebRTCICECandidatesSessionId, sessionId++);
    CHIP_FAULT_INJECT(chip::FaultInjection::kFault_EmptyWebRTCICECandidatesList, iceCandidateStructList.clear());

    command.webRTCSessionID = sessionId;
    if (iceCandidateStructList.empty())
    {
        command.ICECandidates = DataModel::List<const ICECandidateStruct>();
    }
    else
    {
        command.ICECandidates =
            DataModel::List<const ICECandidateStruct>(iceCandidateStructList.data(), iceCandidateStructList.size());
    }

    WebrtcTransport::RequestArgs requestArgs = transport->GetRequestArgs();
    // Now invoke the command using the found session handle
    return Controller::InvokeCommandRequest(&exchangeMgr, sessionHandle, requestArgs.originatingEndpointId, command, onSuccess,
                                            onFailure,
                                            /* timedInvokeTimeoutMs = */ NullOptional, /* responseTimeout = */ NullOptional,
                                            /* outCancelFn = */ nullptr, /*allowLargePayload = */ true);
}

CHIP_ERROR WebRTCProviderManager::SendEndCommand(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle,
                                                 uint16_t sessionId, WebRTCEndReasonEnum endReason)
{
    auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
        ChipLogProgress(Camera, "End command succeeds");
    };

    auto onFailure = [](CHIP_ERROR error) { ChipLogError(Camera, "End command failed: %" CHIP_ERROR_FORMAT, error.Format()); };

    WebrtcTransport * transport = GetTransport(sessionId);
    if (transport == nullptr)
    {
        ChipLogError(Camera, "WebTransport not found for the sessionId: %u", sessionId);
        return CHIP_ERROR_INTERNAL;
    }

    // Build the command
    WebRTCTransportRequestor::Commands::End::Type command;

    command.webRTCSessionID = sessionId;
    command.reason          = endReason;

    WebrtcTransport::RequestArgs requestArgs = transport->GetRequestArgs();
    // Now invoke the command using the found session handle
    return Controller::InvokeCommandRequest(&exchangeMgr, sessionHandle, requestArgs.originatingEndpointId, command, onSuccess,
                                            onFailure,
                                            /* timedInvokeTimeoutMs = */ NullOptional, /* responseTimeout = */ NullOptional,
                                            /* outCancelFn = */ nullptr, /*allowLargePayload = */ true);
}

CHIP_ERROR WebRTCProviderManager::AcquireAudioVideoStreams(uint16_t sessionId)
{
    WebrtcTransport * transport = GetTransport(sessionId);
    if (transport == nullptr)
    {
        ChipLogError(Camera, "WebTransport not found for the sessionId: %u", sessionId);
        return CHIP_ERROR_INTERNAL;
    }

    WebrtcTransport::RequestArgs args = transport->GetRequestArgs();
    return mCameraDevice->GetCameraAVStreamMgmtDelegate().OnTransportAcquireAudioVideoStreams(args.audioStreamId,
                                                                                              args.videoStreamId);
}

CHIP_ERROR WebRTCProviderManager::ReleaseAudioVideoStreams(uint16_t sessionId)
{
    WebrtcTransport * transport = GetTransport(sessionId);
    if (transport == nullptr)
    {
        ChipLogError(Camera, "WebTransport not found for the sessionId: %u", sessionId);
        return CHIP_ERROR_INTERNAL;
    }

    WebrtcTransport::RequestArgs args = transport->GetRequestArgs();
    // TODO: Use passed in audio/video stream ids corresponding to a sessionId.
    return mCameraDevice->GetCameraAVStreamMgmtDelegate().OnTransportReleaseAudioVideoStreams(args.audioStreamId,
                                                                                              args.videoStreamId);
}
