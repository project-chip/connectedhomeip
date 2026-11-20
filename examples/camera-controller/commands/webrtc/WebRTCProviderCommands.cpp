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

#include "WebRTCProviderCommands.h"
#include <commands/common/RemoteDataModelLogger.h>
#include <commands/interactive/InteractiveCommands.h>
#include <device-manager/DeviceManager.h>
#include <thread>
#include <unistd.h>
#include <webrtc-manager/WebRTCManager.h>

using namespace ::chip;
using StreamUsageEnum = chip::app::Clusters::Globals::StreamUsageEnum;

namespace webrtc {

CHIP_ERROR ConnectCommand::RunCommand()
{
    ChipLogProgress(Camera, "Run ConnectCommand");
    return WebRTCManager::Instance().Connect(CurrentCommissioner(), mPeerNodeId, mPeerEndpointId);
}

WebRTCProviderClient::OfferOptions OfferOptionArguments::ToOfferOptions() const
{
    WebRTCProviderClient::OfferOptions options;

    if (mVideoStreams.HasValue())
    {
        options.videoStreams.emplace(mVideoStreams.Value().begin(), mVideoStreams.Value().end());
    }
    if (mAudioStreams.HasValue())
    {
        options.audioStreams.emplace(mAudioStreams.Value().begin(), mAudioStreams.Value().end());
    }
    if (mMetadataEnabled.HasValue())
    {
        options.metadataEnabled = mMetadataEnabled.Value();
    }
    if (mSFrameConfig.HasValue())
    {
        const auto & sFrameConfig = mSFrameConfig.Value();
        options.sFrameConfig.emplace();
        options.sFrameConfig->cipherSuite = sFrameConfig.cipherSuite;
        options.sFrameConfig->baseKey.assign(sFrameConfig.baseKey.begin(), sFrameConfig.baseKey.end());
        options.sFrameConfig->kid.assign(sFrameConfig.kid.begin(), sFrameConfig.kid.end());
    }
    if (mICEServers.HasValue())
    {
        options.iceServers.emplace();
        for (const auto & server : mICEServers.Value())
        {
            WebRTCProviderClient::OfferOptions::ICEServer iceServer;
            for (const auto & url : server.URLs)
            {
                iceServer.urls.emplace_back(url.data(), url.size());
            }
            if (server.username.HasValue())
            {
                iceServer.username.emplace(server.username.Value().data(), server.username.Value().size());
            }
            if (server.credential.HasValue())
            {
                iceServer.credential.emplace(server.credential.Value().data(), server.credential.Value().size());
            }
            if (server.caid.HasValue())
            {
                iceServer.caid = server.caid.Value();
            }
            options.iceServers->push_back(std::move(iceServer));
        }
    }
    if (mICETransportPolicy.HasValue())
    {
        options.iceTransportPolicy = mICETransportPolicy.Value();
    }

    return options;
}

CHIP_ERROR ProvideOfferCommand::RunCommand()
{
    ChipLogProgress(Camera, "Run ProvideOfferCommand");

    app::DataModel::Nullable<uint16_t> webrtcSessionId;
    if (mWebRTCSessionId.HasValue())
    {
        webrtcSessionId.SetNonNull(mWebRTCSessionId.Value());
    }
    else
    {
        webrtcSessionId.SetNull();
    }

    // Convert the stream usage into its enum type:
    auto streamUsage = static_cast<StreamUsageEnum>(mStreamUsage);

    chip::Optional<chip::app::DataModel::Nullable<uint16_t>> videoStreamIdOptional;
    if (mVideoStreamId.HasValue())
    {
        auto videoStreamIdNullable = app::DataModel::MakeNullable(mVideoStreamId.Value());
        videoStreamIdOptional      = MakeOptional(videoStreamIdNullable);
    }
    else
    {
        videoStreamIdOptional = NullOptional;
    }

    chip::Optional<chip::app::DataModel::Nullable<uint16_t>> audioStreamIdOptional;
    if (mAudioStreamId.HasValue())
    {
        auto audioStreamIdNullable = app::DataModel::MakeNullable(mAudioStreamId.Value());
        audioStreamIdOptional      = MakeOptional(audioStreamIdNullable);
    }
    else
    {
        audioStreamIdOptional = NullOptional;
    }

    return WebRTCManager::Instance().ProvideOffer(webrtcSessionId, streamUsage, videoStreamIdOptional, audioStreamIdOptional,
                                                  mOfferOptionArguments.ToOfferOptions());
}

CHIP_ERROR SolicitOfferCommand::RunCommand()
{
    ChipLogProgress(Camera, "Run SolicitOfferCommand");

    // Convert the stream usage into its enum type:
    auto streamUsage = static_cast<StreamUsageEnum>(mStreamUsage);

    chip::Optional<chip::app::DataModel::Nullable<uint16_t>> videoStreamIdOptional;
    if (mVideoStreamId.HasValue())
    {
        auto videoStreamIdNullable = app::DataModel::MakeNullable(mVideoStreamId.Value());
        videoStreamIdOptional      = MakeOptional(videoStreamIdNullable);
    }
    else
    {
        videoStreamIdOptional = NullOptional;
    }

    chip::Optional<chip::app::DataModel::Nullable<uint16_t>> audioStreamIdOptional;
    if (mAudioStreamId.HasValue())
    {
        auto audioStreamIdNullable = app::DataModel::MakeNullable(mAudioStreamId.Value());
        audioStreamIdOptional      = MakeOptional(audioStreamIdNullable);
    }
    else
    {
        audioStreamIdOptional = NullOptional;
    }

    return WebRTCManager::Instance().SolicitOffer(streamUsage, videoStreamIdOptional, audioStreamIdOptional,
                                                  mOfferOptionArguments.ToOfferOptions());
}

CHIP_ERROR EstablishSessionCommand::RunCommand()
{
    ChipLogProgress(Camera, "Run EstablishSessionCommand");

    if (mPeerNodeId == chip::kUndefinedNodeId)
    {
        ChipLogError(Camera, "Missing --node-id");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    uint8_t streamUsage = static_cast<uint8_t>(StreamUsageEnum::kRecording);

    // Use provided offer type or default to ProvideOffer
    camera::WebRTCOfferType offerType = camera::WebRTCOfferType::kProvideOffer;
    if (mOfferType.HasValue())
    {
        offerType = static_cast<camera::WebRTCOfferType>(mOfferType.Value());
    }

    return camera::DeviceManager::Instance().AllocateVideoStream(mPeerNodeId, streamUsage, offerType);
}

} // namespace webrtc
