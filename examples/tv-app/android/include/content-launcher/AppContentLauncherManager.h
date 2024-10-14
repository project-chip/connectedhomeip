/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "../../java/ContentAppAttributeDelegate.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/content-launch-server/content-launch-server.h>

#include <list>
#include <string>

using chip::CharSpan;
using chip::EndpointId;
using chip::app::AttributeValueEncoder;
using chip::app::CommandResponseHelper;
using ContentLauncherDelegate     = chip::app::Clusters::ContentLauncher::Delegate;
using LaunchResponseType          = chip::app::Clusters::ContentLauncher::Commands::LauncherResponse::Type;
using ParameterType               = chip::app::Clusters::ContentLauncher::Structs::ParameterStruct::DecodableType;
using PlaybackPreferencesType     = chip::app::Clusters::ContentLauncher::Structs::PlaybackPreferencesStruct::DecodableType;
using BrandingInformationType     = chip::app::Clusters::ContentLauncher::Structs::BrandingInformationStruct::Type;
using ContentAppAttributeDelegate = chip::AppPlatform::ContentAppAttributeDelegate;

class AppContentLauncherManager : public ContentLauncherDelegate
{
public:
    AppContentLauncherManager(ContentAppAttributeDelegate * attributeDelegate, std::list<std::string> acceptHeaderList,
                              uint32_t supportedStreamingProtocols);

    void HandleLaunchContent(CommandResponseHelper<LaunchResponseType> & helper,
                             const chip::app::DataModel::DecodableList<ParameterType> & parameterList, bool autoplay,
                             const CharSpan & data, const chip::Optional<PlaybackPreferencesType> playbackPreferences,
                             bool useCurrentContext) override;

    void HandleLaunchUrl(CommandResponseHelper<LaunchResponseType> & helper, const CharSpan & contentUrl,
                         const CharSpan & displayString, const BrandingInformationType & brandingInformation) override;
    CHIP_ERROR HandleGetAcceptHeaderList(AttributeValueEncoder & aEncoder) override;
    uint32_t HandleGetSupportedStreamingProtocols() override;

    void SetEndpointId(EndpointId epId) { mEndpointId = epId; };

    uint32_t GetFeatureMap(chip::EndpointId endpoint) override;
    uint16_t GetClusterRevision(chip::EndpointId endpoint) override;

protected:
    std::list<std::string> mAcceptHeaderList;
    uint32_t mSupportedStreamingProtocols;

private:
    EndpointId mEndpointId;

    // TODO: set this based upon meta data from app
    static constexpr uint32_t kEndpointFeatureMap = 3;
    static constexpr uint16_t kClusterRevision    = 2;
    ContentAppAttributeDelegate * mAttributeDelegate;
};
