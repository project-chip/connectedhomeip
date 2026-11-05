/*
 *   Copyright (c) 2025 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#pragma once

#include <commands/common/CHIPCommand.h>
#include <device-manager/DeviceManager.h>
#include <webrtc-manager/WebRTCProviderClient.h>

namespace webrtc {

class ConnectCommand : public CHIPCommand
{
public:
    ConnectCommand(CredentialIssuerCommands * credIssuerCommands) : CHIPCommand("connect", credIssuerCommands)
    {
        AddArgument("node-id", 0, UINT64_MAX, &mPeerNodeId);
        AddArgument("endpointid", 0, UINT16_MAX, &mPeerEndpointId);
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;

    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(1); }

private:
    chip::NodeId mPeerNodeId         = chip::kUndefinedNodeId;
    chip::EndpointId mPeerEndpointId = chip::kInvalidEndpointId;
};

// The optional SolicitOffer / ProvideOffer arguments shared by both commands.
class OfferOptionArguments
{
public:
    OfferOptionArguments() :
        mComplex_VideoStreams(&mVideoStreams), mComplex_AudioStreams(&mAudioStreams), mComplex_SFrameConfig(&mSFrameConfig),
        mComplex_ICEServers(&mICEServers)
    {}

    void AddTo(Command & command)
    {
        command.AddArgument("video-streams", &mComplex_VideoStreams, "JSON list of video stream IDs, e.g. [1,2]",
                            Argument::kOptional);
        command.AddArgument("audio-streams", &mComplex_AudioStreams, "JSON list of audio stream IDs, e.g. [3]",
                            Argument::kOptional);
        command.AddArgument("metadata-enabled", 0, 1, &mMetadataEnabled);
        command.AddArgument("sframe-config", &mComplex_SFrameConfig,
                            "JSON SFrameStruct, e.g. {\"cipherSuite\":2,\"baseKey\":\"hex:<32 bytes>\",\"kid\":\"hex:0102\"}",
                            Argument::kOptional);
        command.AddArgument("ice-servers", &mComplex_ICEServers,
                            "JSON list of ICEServerStruct, e.g. [{\"URLs\":[\"stun:stun.example.com\"]}]", Argument::kOptional);
        command.AddArgument("ice-transport-policy", &mICETransportPolicy);
    }

    // Returns a copy of the arguments that owns its data.
    WebRTCProviderClient::OfferOptions ToOfferOptions() const;

private:
    chip::Optional<chip::app::DataModel::List<const uint16_t>> mVideoStreams;
    TypedComplexArgument<chip::Optional<chip::app::DataModel::List<const uint16_t>>> mComplex_VideoStreams;
    chip::Optional<chip::app::DataModel::List<const uint16_t>> mAudioStreams;
    TypedComplexArgument<chip::Optional<chip::app::DataModel::List<const uint16_t>>> mComplex_AudioStreams;
    chip::Optional<chip::app::Clusters::WebRTCTransportProvider::Structs::SFrameStruct::Type> mSFrameConfig;
    TypedComplexArgument<chip::Optional<chip::app::Clusters::WebRTCTransportProvider::Structs::SFrameStruct::Type>>
        mComplex_SFrameConfig;
    chip::Optional<chip::app::DataModel::List<const WebRTCProviderClient::ICEServerStruct>> mICEServers;
    TypedComplexArgument<chip::Optional<chip::app::DataModel::List<const WebRTCProviderClient::ICEServerStruct>>>
        mComplex_ICEServers;
    chip::Optional<bool> mMetadataEnabled;
    chip::Optional<char *> mICETransportPolicy;
};

class ProvideOfferCommand : public CHIPCommand
{
public:
    ProvideOfferCommand(CredentialIssuerCommands * credIssuerCommands) : CHIPCommand("provide-offer", credIssuerCommands)
    {
        AddArgument("stream-usage", 0, UINT8_MAX, &mStreamUsage);
        AddArgument("webrtc-session-id", 0, UINT16_MAX, &mWebRTCSessionId);
        AddArgument("video-stream-id", 0, UINT16_MAX, &mVideoStreamId);
        AddArgument("audio-stream-id", 0, UINT16_MAX, &mAudioStreamId);
        mOfferOptionArguments.AddTo(*this);
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;

    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(1); }

private:
    uint8_t mStreamUsage = 0;
    chip::Optional<uint16_t> mWebRTCSessionId;
    chip::Optional<uint16_t> mVideoStreamId;
    chip::Optional<uint16_t> mAudioStreamId;
    OfferOptionArguments mOfferOptionArguments;
};

class SolicitOfferCommand : public CHIPCommand
{
public:
    SolicitOfferCommand(CredentialIssuerCommands * credIssuerCommands) : CHIPCommand("solicit-offer", credIssuerCommands)
    {
        AddArgument("stream-usage", 0, UINT8_MAX, &mStreamUsage);
        AddArgument("video-stream-id", 0, UINT16_MAX, &mVideoStreamId);
        AddArgument("audio-stream-id", 0, UINT16_MAX, &mAudioStreamId);
        mOfferOptionArguments.AddTo(*this);
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;

    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(1); }

private:
    uint8_t mStreamUsage = 0;
    chip::Optional<uint16_t> mVideoStreamId;
    chip::Optional<uint16_t> mAudioStreamId;
    OfferOptionArguments mOfferOptionArguments;
};

class EstablishSessionCommand : public CHIPCommand
{
public:
    EstablishSessionCommand(CredentialIssuerCommands * credIssuerCommands) : CHIPCommand("establish-session", credIssuerCommands)
    {
        AddArgument("node-id", 0, UINT64_MAX, &mPeerNodeId);
        AddArgument("offer-type", 0, 1, &mOfferType, "WebRTC offer type: 0=ProvideOffer (default), 1=SolicitOffer");
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;

    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(5); }

private:
    chip::NodeId mPeerNodeId = chip::kUndefinedNodeId;
    chip::Optional<uint8_t> mOfferType;
};

} // namespace webrtc
