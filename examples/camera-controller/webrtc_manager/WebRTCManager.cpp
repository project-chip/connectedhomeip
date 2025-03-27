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

#include "WebRTCManager.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/ConcreteAttributePath.h>
#include <app/EventLogging.h>
#include <app/reporting/reporting.h>
#include <app/server/Server.h>
#include <app/util/af-types.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <app/util/util.h>
#include <commands/interactive/InteractiveCommands.h>
#include <crypto/RandUtils.h>
#include <lib/support/StringBuilder.h>
#include <lib/support/ZclString.h>

#include <cstdio>
#include <string>

using namespace chip;
using namespace chip::app;
using namespace std::chrono_literals;

namespace {

constexpr EndpointId kWebRTCRequesterEndpointId = 1;

} // namespace

WebRTCManager::WebRTCManager() {}

WebRTCManager::~WebRTCManager()
{
    // Close the data channel and peer connection if they exist
    if (mDataChannel)
    {
        mDataChannel->close();
        mDataChannel.reset();
    }

    if (mPeerConnection)
    {
        mPeerConnection->close();
        mPeerConnection.reset();
    }
}

void WebRTCManager::Init()
{
    // TODO:: mWebRTCRequestorServer.Init();
}

CHIP_ERROR WebRTCManager::SetRemoteDescription(uint16_t webRTCSessionID, const std::string & sdp)
{
    if (!mPeerConnection)
    {
        ChipLogError(NotSpecified, "Cannot set remote description: mPeerConnection is null");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    ChipLogProgress(NotSpecified, "WebRTCManager::SetRemoteDescription");
    mPeerConnection->setRemoteDescription(sdp);

    // Schedule the ProvideICECandidates() call to run asynchronously.
    DeviceLayer::SystemLayer().ScheduleLambda([this, webRTCSessionID]() { ProvideICECandidates(webRTCSessionID); });

    return CHIP_NO_ERROR;
}

CHIP_ERROR WebRTCManager::Connnect(Controller::DeviceCommissioner & commissioner, NodeId nodeId, EndpointId endpointId)
{
    ChipLogProgress(NotSpecified, "Attempting to establish WebRTC connection to node 0x" ChipLogFormatX64 " on endpoint 0x%x",
                    ChipLogValueX64(nodeId), endpointId);

    FabricIndex fabricIndex       = commissioner.GetFabricIndex();
    const FabricInfo * fabricInfo = commissioner.GetFabricTable()->FindFabricWithIndex(fabricIndex);
    VerifyOrReturnError(fabricInfo != nullptr, CHIP_ERROR_INCORRECT_STATE);

    uint64_t fabricId = fabricInfo->GetFabricId();
    ChipLogProgress(NotSpecified, "Commissioner is on Fabric ID 0x" ChipLogFormatX64, ChipLogValueX64(fabricId));

    chip::ScopedNodeId peerId(nodeId, fabricIndex);

    mWebRTCProviderClient.Init(peerId, endpointId);

    rtc::InitLogger(rtc::LogLevel::Warning);

    // Create the peer connection
    rtc::Configuration config;
    mPeerConnection = std::make_shared<rtc::PeerConnection>(config);

    // Use std::cout to print out to console
    mPeerConnection->onLocalDescription([this](rtc::Description description) {
        mLocalDescription = std::string(description);
        std::cout << "Local Description:" << std::endl;
        std::cout << mLocalDescription << std::endl;
    });

    mPeerConnection->onLocalCandidate([this](rtc::Candidate candidate) {
        std::string candidateStr = std::string(candidate);
        mLocalCandidates.push_back(candidateStr);
        std::cout << "Local Candidate:" << std::endl;
        std::cout << candidateStr << std::endl << std::endl;
    });

    mPeerConnection->onStateChange(
        [](rtc::PeerConnection::State state) { std::cout << "[PeerConnection State: " << state << "]" << std::endl; });

    mPeerConnection->onGatheringStateChange(
        [](rtc::PeerConnection::GatheringState state) { std::cout << "[Gathering State: " << state << "]" << std::endl; });

    // Create a data channel for this offerer
    mDataChannel = mPeerConnection->createDataChannel("test");

    if (mDataChannel)
    {
        mDataChannel->onOpen([&]() { std::cout << "[DataChannel open: " << mDataChannel->label() << "]" << std::endl; });

        mDataChannel->onClosed([&]() { std::cout << "[DataChannel closed: " << mDataChannel->label() << "]" << std::endl; });

        mDataChannel->onMessage([](auto data) {
            if (std::holds_alternative<std::string>(data))
            {
                std::cout << "[Received: " << std::get<std::string>(data) << "]" << std::endl;
            }
        });
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR WebRTCManager::ProvideOffer(DataModel::Nullable<uint16_t> webRTCSessionID,
                                       Clusters::WebRTCTransportProvider::StreamUsageEnum streamUsage)
{
    ChipLogProgress(NotSpecified, "Sending ProvideOffer command to the peer device");

    CHIP_ERROR err = mWebRTCProviderClient.ProvideOffer(webRTCSessionID, mLocalDescription, streamUsage, kWebRTCRequesterEndpointId,
                                                        MakeOptional(DataModel::NullNullable), // "Null" for video
                                                        MakeOptional(DataModel::NullNullable), // "Null" for audio
                                                        NullOptional, // Omit ICEServers (Optional not present)
                                                        NullOptional  // Omit ICETransportPolicy (Optional not present)
    );

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to send ProvideOffer: %" CHIP_ERROR_FORMAT, err.Format());
    }

    return err;
}

CHIP_ERROR WebRTCManager::ProvideICECandidates(uint16_t webRTCSessionID)
{
    ChipLogProgress(NotSpecified, "Sending ProvideICECandidates command to the peer device");

    if (mLocalCandidates.empty())
    {
        ChipLogError(NotSpecified, "No local ICE candidates to send");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Convert mLocalCandidates (std::vector<std::string>) into a list of CharSpans.
    std::vector<chip::CharSpan> candidateSpans;
    candidateSpans.reserve(mLocalCandidates.size());
    for (const auto & candidate : mLocalCandidates)
    {
        candidateSpans.push_back(chip::CharSpan(candidate.c_str(), static_cast<uint16_t>(candidate.size())));
    }

    auto ICECandidates = chip::app::DataModel::List<const chip::CharSpan>(candidateSpans.data(), candidateSpans.size());

    CHIP_ERROR err = mWebRTCProviderClient.ProvideICECandidates(webRTCSessionID, ICECandidates);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to send ProvideICECandidates: %" CHIP_ERROR_FORMAT, err.Format());
    }

    return err;
}

void WebRTCManager::HandleProvideOfferResponse(TLV::TLVReader & data)
{
    ChipLogProgress(NotSpecified, "WebRTCManager::HandleProvideOfferResponse.");
}

void WebRTCManager::HandleCommandResponse(const ConcreteCommandPath & path, TLV::TLVReader & data)
{
    ChipLogProgress(NotSpecified, "Command Response received.");

    if (path.mClusterId == Clusters::WebRTCTransportProvider::Id &&
        path.mCommandId == Clusters::WebRTCTransportProvider::Commands::ProvideOfferResponse::Id)
    {
        VerifyOrDie(path.mEndpointId == kWebRTCRequesterEndpointId);
        HandleProvideOfferResponse(data);
    }
}
