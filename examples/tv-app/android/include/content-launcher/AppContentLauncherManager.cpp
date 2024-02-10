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

#include "AppContentLauncherManager.h"
#include "../../java/ContentAppAttributeDelegate.h"
#include <app/util/config.h>
#include <json/json.h>

using namespace std;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters::ContentLauncher;
using ContentAppAttributeDelegate = chip::AppPlatform::ContentAppAttributeDelegate;

AppContentLauncherManager::AppContentLauncherManager(ContentAppAttributeDelegate * attributeDelegate,
                                                     list<std::string> acceptHeaderList, uint32_t supportedStreamingProtocols) :
    mAttributeDelegate(attributeDelegate)
{
    mAcceptHeaderList            = acceptHeaderList;
    mSupportedStreamingProtocols = supportedStreamingProtocols;
}

void AppContentLauncherManager::HandleLaunchContent(CommandResponseHelper<LaunchResponseType> & helper,
                                                    const DecodableList<ParameterType> & parameterList, bool autoplay,
                                                    const CharSpan & data,
                                                    const chip::Optional<PlaybackPreferencesType> playbackPreferences,
                                                    bool useCurrentContext)
{
    ChipLogProgress(Zcl, "AppContentLauncherManager::HandleLaunchContent for endpoint %d", mEndpointId);
    string dataString(data.data(), data.size());

    ChipLogProgress(Zcl, " AutoPlay=%s", (autoplay ? "true" : "false"));

    bool foundMatch = false;
    auto iter       = parameterList.begin();
    while (iter.Next())
    {
        auto & parameterType = iter.GetValue();
        ChipLogProgress(Zcl, " TEST CASE found match=Example TV Show type=%d", static_cast<uint16_t>(parameterType.type));
        foundMatch = true;
    }

    if (!foundMatch)
    {
        ChipLogProgress(Zcl, " TEST CASE did not find a match");
    }

    LaunchResponseType response;
    // TODO: Insert code here
    response.data   = chip::MakeOptional(CharSpan::fromCharString("exampleData"));
    response.status = ContentLauncher::StatusEnum::kSuccess;
    helper.Success(response);
}

void AppContentLauncherManager::HandleLaunchUrl(CommandResponseHelper<LaunchResponseType> & helper, const CharSpan & contentUrl,
                                                const CharSpan & displayString, const BrandingInformationType & brandingInformation)
{
    ChipLogProgress(Zcl, "AppContentLauncherManager::HandleLaunchUrl");

    string contentUrlString(contentUrl.data(), contentUrl.size());
    string displayStringString(displayString.data(), displayString.size());

    // TODO: Insert code here
    LaunchResponseType response;
    response.data   = chip::MakeOptional(CharSpan::fromCharString("Success"));
    response.status = ContentLauncher::StatusEnum::kSuccess;
    helper.Success(response);
}

CHIP_ERROR AppContentLauncherManager::HandleGetAcceptHeaderList(AttributeValueEncoder & aEncoder)
{
    ChipLogProgress(Zcl, "AppContentLauncherManager::HandleGetAcceptHeaderList");
    chip::app::ConcreteReadAttributePath aPath(mEndpointId, chip::app::Clusters::ContentLauncher::Id,
                                               chip::app::Clusters::ContentLauncher::Attributes::AcceptHeader::Id);
    std::string resStr = mAttributeDelegate->Read(aPath);
    ChipLogProgress(Zcl, "AppContentLauncherManager::HandleGetAcceptHeaderList response %s", resStr.c_str());

    if (resStr.length() != 0)
    {
        Json::Reader reader;
        Json::Value value;
        if (reader.parse(resStr, value))
        {
            std::string attrId = to_string(chip::app::Clusters::ContentLauncher::Attributes::AcceptHeader::Id);
            ChipLogProgress(Zcl, "AppContentLauncherManager::HandleGetAcceptHeaderList response parsing done. reading attr %s",
                            attrId.c_str());
            if (value[attrId].isArray())
            {
                mAcceptHeaderList.clear();
                for (Json::Value & entry : value[attrId])
                {
                    if (entry.isString())
                    {
                        mAcceptHeaderList.push_back(entry.asString());
                    }
                }
            }
        }
    }

    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (std::string & entry : mAcceptHeaderList)
        {
            CharSpan data = CharSpan::fromCharString(entry.c_str());
            ReturnErrorOnFailure(encoder.Encode(data));
        }
        return CHIP_NO_ERROR;
    });
}

uint32_t AppContentLauncherManager::HandleGetSupportedStreamingProtocols()
{
    ChipLogProgress(Zcl, "AppContentLauncherManager::HandleGetSupportedStreamingProtocols");
    chip::app::ConcreteReadAttributePath aPath(mEndpointId, chip::app::Clusters::ContentLauncher::Id,
                                               chip::app::Clusters::ContentLauncher::Attributes::SupportedStreamingProtocols::Id);
    std::string resStr = mAttributeDelegate->Read(aPath);
    ChipLogProgress(Zcl, "AppContentLauncherManager::HandleGetSupportedStreamingProtocols response %s", resStr.c_str());

    if (resStr.length() == 0)
    {
        return mSupportedStreamingProtocols;
    }

    Json::Reader reader;
    Json::Value value;
    if (!reader.parse(resStr, value))
    {
        return mSupportedStreamingProtocols;
    }
    std::string attrId = to_string(chip::app::Clusters::ContentLauncher::Attributes::SupportedStreamingProtocols::Id);
    ChipLogProgress(Zcl, "AppContentLauncherManager::HandleGetSupportedStreamingProtocols response parsing done. reading attr %s",
                    attrId.c_str());
    if (!value[attrId].empty() && value[attrId].isInt())
    {
        uint32_t supportedStreamingProtocols = static_cast<uint32_t>(value[attrId].asInt());
        mSupportedStreamingProtocols         = supportedStreamingProtocols;
    }
    return mSupportedStreamingProtocols;
}

uint32_t AppContentLauncherManager::GetFeatureMap(chip::EndpointId endpoint)
{
    if (endpoint >= MATTER_DM_CONTENT_LAUNCHER_CLUSTER_SERVER_ENDPOINT_COUNT)
    {
        return kEndpointFeatureMap;
    }

    uint32_t featureMap = 0;
    Attributes::FeatureMap::Get(endpoint, &featureMap);
    return featureMap;
}

uint16_t AppContentLauncherManager::GetClusterRevision(chip::EndpointId endpoint)
{
    if (endpoint >= MATTER_DM_CONTENT_LAUNCHER_CLUSTER_SERVER_ENDPOINT_COUNT)
    {
        return kClusterRevision;
    }

    uint16_t clusterRevision = 0;
    bool success =
        (Attributes::ClusterRevision::Get(endpoint, &clusterRevision) == chip::Protocols::InteractionModel::Status::Success);
    if (!success)
    {
        ChipLogError(Zcl, "AppContentLauncherManager::GetClusterRevision error reading cluster revision");
    }
    return clusterRevision;
}
