/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app/clusters/channel-server/channel-server.h>
#include <vector>

using chip::CharSpan;
using chip::app::AttributeValueEncoder;
using chip::app::CommandResponseHelper;
using RecordingFlagBitmap       = chip::app::Clusters::Channel::RecordingFlagBitmap;
using ChannelDelegate           = chip::app::Clusters::Channel::Delegate;
using ChangeChannelResponseType = chip::app::Clusters::Channel::Commands::ChangeChannelResponse::Type;
using ProgramGuideResponseType  = chip::app::Clusters::Channel::Commands::ProgramGuideResponse::Type;
using ChannelInfoType           = chip::app::Clusters::Channel::Structs::ChannelInfoStruct::Type;
using AdditionalInfoType        = chip::app::Clusters::Channel::Structs::AdditionalInfoStruct::Type;
using LineupInfoType            = chip::app::Clusters::Channel::Structs::LineupInfoStruct::Type;
using PageTokenType             = chip::app::Clusters::Channel::Structs::PageTokenStruct::Type;
using ProgramType               = chip::app::Clusters::Channel::Structs::ProgramStruct::Type;
using ChannelPagingType         = chip::app::Clusters::Channel::Structs::ChannelPagingStruct::Type;

class ChannelManager : public ChannelDelegate
{
public:
    ChannelManager();

    CHIP_ERROR HandleGetChannelList(AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR HandleGetLineup(AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR HandleGetCurrentChannel(AttributeValueEncoder & aEncoder) override;

    void HandleChangeChannel(CommandResponseHelper<ChangeChannelResponseType> & helper, const CharSpan & match) override;
    bool HandleChangeChannelByNumber(const uint16_t & majorNumber, const uint16_t & minorNumber) override;
    bool HandleSkipChannel(const int16_t & count) override;
    void HandleGetProgramGuide(CommandResponseHelper<ProgramGuideResponseType> & helper, const chip::Optional<uint32_t> & startTime,
                               const chip::Optional<uint32_t> & endTime,
                               const chip::Optional<chip::app::DataModel::DecodableList<ChannelInfoType>> & channelList,
                               const chip::Optional<PageTokenType> & pageToken,
                               const chip::Optional<chip::BitMask<RecordingFlagBitmap>> & recordingFlag,
                               const chip::Optional<chip::app::DataModel::DecodableList<AdditionalInfoType>> & externalIdList,
                               const chip::Optional<chip::ByteSpan> & data) override;

    bool HandleRecordProgram(const chip::CharSpan & programIdentifier, bool shouldRecordSeries,
                             const chip::app::DataModel::DecodableList<AdditionalInfoType> & externalIdList,
                             const chip::ByteSpan & data) override;

    bool HandleCancelRecordProgram(const chip::CharSpan & programIdentifier, bool shouldRecordSeries,
                                   const chip::app::DataModel::DecodableList<AdditionalInfoType> & externalIdList,
                                   const chip::ByteSpan & data) override;

    uint32_t GetFeatureMap(chip::EndpointId endpoint) override;
    uint16_t GetClusterRevision(chip::EndpointId endpoint) override;

protected:
    uint16_t mCurrentChannelIndex;
    ChannelInfoType mCurrentChannel;
    std::vector<ChannelInfoType> mChannels;
    std::vector<ProgramType> mPrograms;

private:
    // TODO: set this based upon meta data from app
    static constexpr uint32_t kEndpointFeatureMap = 3;
    static constexpr uint16_t kClusterRevision    = 2;
};
