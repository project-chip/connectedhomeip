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

#include <app/clusters/content-launch-server/content-launch-delegate.h>
#include <app/clusters/content-launch-server/content-launch-server.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
#include <app/app-platform/ContentAppPlatform.h>
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
#include <app/data-model/Encode.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <platform/CHIPDeviceConfig.h>

#include <list>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ContentLauncher;
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
using namespace chip::AppPlatform;
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

static constexpr size_t kContentLaunchDelegateTableSize =
    EMBER_AF_CONTENT_LAUNCH_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

// -----------------------------------------------------------------------------
// Delegate Implementation

using chip::app::Clusters::ContentLauncher::Delegate;

namespace {

Delegate * gDelegateTable[kContentLaunchDelegateTableSize] = { nullptr };

Delegate * GetDelegate(EndpointId endpoint)
{
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ContentApp * app = ContentAppPlatform::GetInstance().GetContentApp(endpoint);
    if (app != nullptr)
    {
        ChipLogProgress(Zcl, "Content Launcher returning ContentApp delegate for endpoint:%u", endpoint);
        return app->GetContentLauncherDelegate();
    }
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ChipLogProgress(Zcl, "Content Launcher NOT returning ContentApp delegate for endpoint:%u", endpoint);

    uint16_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ContentLauncher::Id);
    return ((ep == 0xFFFF || ep >= EMBER_AF_CONTENT_LAUNCH_CLUSTER_SERVER_ENDPOINT_COUNT) ? nullptr : gDelegateTable[ep]);
}

bool isDelegateNull(Delegate * delegate, EndpointId endpoint)
{
    if (delegate == nullptr)
    {
        ChipLogProgress(Zcl, "Content Launcher has no delegate set for endpoint:%u", endpoint);
        return true;
    }
    return false;
}
} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace ContentLauncher {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ContentLauncher::Id);
    // if endpoint is found and is not a dynamic endpoint
    if (ep != 0xFFFF && ep < EMBER_AF_CONTENT_LAUNCH_CLUSTER_SERVER_ENDPOINT_COUNT)
    {
        gDelegateTable[ep] = delegate;
    }
    else
    {
    }
}

bool Delegate::HasFeature(chip::EndpointId endpoint, ContentLauncherFeature feature)
{
    uint32_t featureMap = GetFeatureMap(endpoint);
    return (featureMap & chip::to_underlying(feature));
}

} // namespace ContentLauncher
} // namespace Clusters
} // namespace app
} // namespace chip

// -----------------------------------------------------------------------------
// Attribute Accessor Implementation

namespace {

class ContentLauncherAttrAccess : public app::AttributeAccessInterface
{
public:
    ContentLauncherAttrAccess() : app::AttributeAccessInterface(Optional<EndpointId>::Missing(), ContentLauncher::Id) {}

    CHIP_ERROR Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadAcceptHeaderAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadSupportedStreamingProtocolsAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadFeatureFlagAttribute(EndpointId endpoint, app::AttributeValueEncoder & aEncoder, Delegate * delegate);
};

ContentLauncherAttrAccess gContentLauncherAttrAccess;

CHIP_ERROR ContentLauncherAttrAccess::Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder)
{
    EndpointId endpoint = aPath.mEndpointId;
    Delegate * delegate = GetDelegate(endpoint);

    switch (aPath.mAttributeId)
    {
    case app::Clusters::ContentLauncher::Attributes::AcceptHeader::Id: {
        if (isDelegateNull(delegate, endpoint))
        {
            return aEncoder.EncodeEmptyList();
        }

        return ReadAcceptHeaderAttribute(aEncoder, delegate);
    }
    case app::Clusters::ContentLauncher::Attributes::SupportedStreamingProtocols::Id: {
        if (isDelegateNull(delegate, endpoint))
        {
            return CHIP_NO_ERROR;
        }

        return ReadSupportedStreamingProtocolsAttribute(aEncoder, delegate);
    }
    case app::Clusters::ContentLauncher::Attributes::FeatureMap::Id: {
        if (isDelegateNull(delegate, endpoint))
        {
            return CHIP_NO_ERROR;
        }

        return ReadFeatureFlagAttribute(endpoint, aEncoder, delegate);
    }
    default: {
        break;
    }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ContentLauncherAttrAccess::ReadFeatureFlagAttribute(EndpointId endpoint, app::AttributeValueEncoder & aEncoder,
                                                               Delegate * delegate)
{
    uint32_t featureFlag = delegate->GetFeatureMap(endpoint);
    return aEncoder.Encode(featureFlag);
}

CHIP_ERROR ContentLauncherAttrAccess::ReadAcceptHeaderAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    return delegate->HandleGetAcceptHeaderList(aEncoder);
}

CHIP_ERROR ContentLauncherAttrAccess::ReadSupportedStreamingProtocolsAttribute(app::AttributeValueEncoder & aEncoder,
                                                                               Delegate * delegate)
{
    uint32_t streamingProtocols = delegate->HandleGetSupportedStreamingProtocols();
    return aEncoder.Encode(streamingProtocols);
}

} // anonymous namespace

// -----------------------------------------------------------------------------
// Matter Framework Callbacks Implementation

bool emberAfContentLauncherClusterLaunchContentCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                        const ContentLauncher::Commands::LaunchContent::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;

    auto & autoplay               = commandData.autoPlay;
    auto & data                   = commandData.data;
    auto & decodableParameterList = commandData.search.parameterList;

    app::CommandResponseHelper<Commands::LaunchResponse::Type> responder(commandObj, commandPath);

    Delegate * delegate = GetDelegate(endpoint);

    VerifyOrExit(isDelegateNull(delegate, endpoint) != true &&
                     delegate->HasFeature(endpoint, ContentLauncherFeature::kContentSearch),
                 err = CHIP_ERROR_INCORRECT_STATE);

    delegate->HandleLaunchContent(responder, decodableParameterList, autoplay, data.HasValue() ? data.Value() : CharSpan());

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfContentLauncherClusterLaunchContentCallback error: %s", err.AsString());
    }

    // If isDelegateNull, no one will call responder, so HasSentResponse will be false
    if (!responder.HasSentResponse())
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    return true;
}

bool emberAfContentLauncherClusterLaunchURLCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                    const ContentLauncher::Commands::LaunchURL::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;

    auto & contentUrl          = commandData.contentURL;
    auto & displayString       = commandData.displayString;
    auto & brandingInformation = commandData.brandingInformation;

    app::CommandResponseHelper<Commands::LaunchResponse::Type> responder(commandObj, commandPath);

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true && delegate->HasFeature(endpoint, ContentLauncherFeature::kURLPlayback),
                 err = CHIP_ERROR_INCORRECT_STATE);
    {
        delegate->HandleLaunchUrl(responder, contentUrl, displayString.HasValue() ? displayString.Value() : CharSpan(),
                                  brandingInformation.HasValue() ? brandingInformation.Value()
                                                                 : chip::app::Clusters::ContentLauncher::BrandingInformation());
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfContentLauncherClusterLaunchURLCallback error: %s", err.AsString());
    }

    // If isDelegateNull, no one will call responder, so HasSentResponse will be false
    if (!responder.HasSentResponse())
    {
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
