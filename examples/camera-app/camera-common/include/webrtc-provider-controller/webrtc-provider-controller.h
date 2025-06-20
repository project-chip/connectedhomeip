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

#include <app/clusters/webrtc-transport-provider-server/webrtc-transport-provider-server.h>

namespace chip {
namespace app {
namespace Clusters {
namespace WebRTCTransportProvider {

using ICEServerDecodableStruct = chip::app::Clusters::Globals::Structs::ICEServerStruct::DecodableType;
using WebRTCSessionStruct      = chip::app::Clusters::Globals::Structs::WebRTCSessionStruct::Type;
using ICECandidateStruct       = chip::app::Clusters::Globals::Structs::ICECandidateStruct::Type;
using StreamUsageEnum          = chip::app::Clusters::Globals::StreamUsageEnum;
using WebRTCEndReasonEnum      = chip::app::Clusters::Globals::WebRTCEndReasonEnum;

/**
 * The application interface to define the options & implement commands.
 */
class WebRTCProviderController : public Delegate
{
public:
    virtual ~WebRTCProviderController() = default;

    virtual CHIP_ERROR HandleSolicitOffer(const OfferRequestArgs & args, WebRTCSessionStruct & outSession,
                                          bool & outDeferredOffer) override = 0;

    virtual CHIP_ERROR HandleProvideOffer(const ProvideOfferRequestArgs & args, WebRTCSessionStruct & outSession) override = 0;

    virtual CHIP_ERROR HandleProvideAnswer(uint16_t sessionId, const std::string & sdpAnswer) override = 0;

    virtual CHIP_ERROR HandleProvideICECandidates(uint16_t sessionId,
                                                  const std::vector<ICECandidateStruct> & candidates) override = 0;

    virtual CHIP_ERROR HandleEndSession(uint16_t sessionId, WebRTCEndReasonEnum reasonCode,
                                        chip::app::DataModel::Nullable<uint16_t> videoStreamID,
                                        chip::app::DataModel::Nullable<uint16_t> audioStreamID) override = 0;

    virtual CHIP_ERROR
    ValidateStreamUsage(StreamUsageEnum streamUsage, const chip::Optional<chip::app::DataModel::Nullable<uint16_t>> & videoStreamId,
                        const chip::Optional<chip::app::DataModel::Nullable<uint16_t>> & audioStreamId) override = 0;

    virtual CHIP_ERROR ValidateVideoStreamID(uint16_t videoStreamId) override = 0;

    virtual CHIP_ERROR ValidateAudioStreamID(uint16_t audioStreamId) override = 0;

    virtual CHIP_ERROR IsPrivacyModeActive(bool & isActive) override = 0;

    virtual bool HasAllocatedVideoStreams() override = 0;

    virtual bool HasAllocatedAudioStreams() override = 0;
};

} // namespace WebRTCTransportProvider
} // namespace Clusters
} // namespace app
} // namespace chip
