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

/**
 *
 *    Copyright (c) 2021 Silicon Labs
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
/****************************************************************************
 * @file
 * @brief Routines for the Content Launch plugin, the
 *server implementation of the Content Launch cluster.
 *******************************************************************************
 ******************************************************************************/

#include "content-launch-server.h"

#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/enums.h>

#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/data-model/Encode.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <list>

using namespace chip;
using namespace chip::app;

ContentLaunchResponse contentLauncherClusterLaunchContent(chip::EndpointId endpointId,
                                                          std::list<ContentLaunchParamater> parameterList, bool autoplay,
                                                          const chip::CharSpan & data);

ContentLaunchResponse contentLauncherClusterLaunchUrl(const chip::CharSpan & contentUrl, const chip::CharSpan & displayString,
                                                      ContentLaunchBrandingInformation & brandingInformation);


std::list<std::string> contentLauncherClusterGetAcceptsHeaderList();

uint32_t contentLauncherClusterGetSupportedStreamingProtocols();

namespace {

class ContentLauncherAttrAccess : public app::AttributeAccessInterface
{
public:
    ContentLauncherAttrAccess() : app::AttributeAccessInterface(Optional<EndpointId>::Missing(), app::Clusters::ContentLauncher::Id)
    {}

    CHIP_ERROR Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadAcceptsHeaderAttribute(app::AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadSupportedStreamingProtocols(app::AttributeValueEncoder & aEncoder);
};

ContentLauncherAttrAccess gContentLauncherAttrAccess;

CHIP_ERROR ContentLauncherAttrAccess::Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case app::Clusters::ContentLauncher::Attributes::AcceptsHeaderList::Id: {
        return ReadAcceptsHeaderAttribute(aEncoder);
    }
    case app::Clusters::ContentLauncher::Attributes::SupportedStreamingProtocols::Id: {
        return ReadSupportedStreamingProtocols(aEncoder);
    }
    default: {
        break;
    }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ContentLauncherAttrAccess::ReadAcceptsHeaderAttribute(app::AttributeValueEncoder & aEncoder)
{
    std::list<std::string> acceptsHeaderList = contentLauncherClusterGetAcceptsHeaderList();
    return aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {

        std::list<std::string> acceptsHeaderList = contentLauncherClusterGetAcceptsHeaderList();

        for (std::string acceptedHeader : acceptsHeaderList) {
            CharSpan span(acceptedHeader.c_str(), strlen(acceptedHeader.c_str()));
            ReturnErrorOnFailure(encoder.Encode(span));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR ContentLauncherAttrAccess::ReadSupportedStreamingProtocols(app::AttributeValueEncoder & aEncoder)
{
    uint32_t streamingProtocols = contentLauncherClusterGetSupportedStreamingProtocols();
    return aEncoder.Encode(streamingProtocols);
}

} // anonymous namespace

bool emberAfContentLauncherClusterLaunchContentCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::ContentLauncher::Commands::LaunchContent::DecodableType & commandData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::app::Clusters::ContentLauncher::Commands::LaunchContentResponse::Type response;

    auto & autoplay = commandData.autoPlay;
    auto & data     = commandData.data;
    std::list<ContentLaunchParamater> parameterList;

    ContentLaunchResponse resp = contentLauncherClusterLaunchContent(emberAfCurrentEndpoint(), parameterList, autoplay, data);
    VerifyOrExit(resp.err == CHIP_NO_ERROR, err = resp.err);

    response.contentLaunchStatus = resp.status;
    response.data                = resp.data;

    err = commandObj->AddResponseData(commandPath, response);
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfContentLauncherClusterLaunchContentCallback error: %s", err.AsString());

        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    return true;
}

bool emberAfContentLauncherClusterLaunchURLCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::ContentLauncher::Commands::LaunchURL::DecodableType & commandData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::app::Clusters::ContentLauncher::Commands::LaunchURLResponse::Type response;

    auto & contentUrl    = commandData.contentURL;
    auto & displayString = commandData.displayString;
    ContentLaunchBrandingInformation brandingInformation;

    ContentLaunchResponse resp = contentLauncherClusterLaunchUrl(contentUrl, displayString, brandingInformation);
    VerifyOrExit(resp.err == CHIP_NO_ERROR, err = resp.err);

    response.contentLaunchStatus = resp.status;
    response.data                = resp.data;

    err = commandObj->AddResponseData(commandPath, response);
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfContentLauncherClusterLaunchURLCallback error: %s", err.AsString());

        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    return true;
}

// -----------------------------------------------------------------------------
// Plugin initialization

void MatterContentLauncherPluginServerInitCallback(void)
{
    registerAttributeAccessOverride(&gContentLauncherAttrAccess);
}
