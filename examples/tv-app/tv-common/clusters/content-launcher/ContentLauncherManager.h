/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/clusters/content-launch-server/content-launch-server.h>
#include <vector>

using chip::CharSpan;
using chip::EndpointId;
using chip::app::AttributeValueEncoder;
using chip::app::CommandResponseHelper;
using ContentLauncherDelegate = chip::app::Clusters::ContentLauncher::Delegate;
using LaunchResponseType      = chip::app::Clusters::ContentLauncher::Commands::LauncherResponse::Type;
using ParameterType           = chip::app::Clusters::ContentLauncher::Structs::ParameterStruct::DecodableType;
using BrandingInformationType = chip::app::Clusters::ContentLauncher::Structs::BrandingInformationStruct::Type;

class ContentEntry
{
public:
    std::string mName;
    std::vector<ParameterType> mSearchFields;
};

class ContentLauncherManager : public ContentLauncherDelegate
{
public:
    ContentLauncherManager() : ContentLauncherManager({ "example", "example" }, 0){};
    ContentLauncherManager(std::list<std::string> acceptHeaderList, uint32_t supportedStreamingProtocols);

    void HandleLaunchContent(CommandResponseHelper<LaunchResponseType> & helper,
                             const chip::app::DataModel::DecodableList<ParameterType> & parameterList, bool autoplay,
                             const CharSpan & data) override;
    void HandleLaunchUrl(CommandResponseHelper<LaunchResponseType> & helper, const CharSpan & contentUrl,
                         const CharSpan & displayString, const BrandingInformationType & brandingInformation) override;
    CHIP_ERROR HandleGetAcceptHeaderList(AttributeValueEncoder & aEncoder) override;
    uint32_t HandleGetSupportedStreamingProtocols() override;

    uint32_t GetFeatureMap(chip::EndpointId endpoint) override;

protected:
    std::list<std::string> mAcceptHeaderList;
    uint32_t mSupportedStreamingProtocols;
    std::vector<ContentEntry> mContentList;

private:
    // TODO: set this based upon meta data from app
    uint32_t mDynamicEndpointFeatureMap = 3;
};
