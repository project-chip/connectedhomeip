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

#include <app/clusters/content-launch-server/content-launch-delegate.h>
#include <app/clusters/content-launch-server/content-launch-server.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/data-model/Encode.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <platform/CHIPDeviceConfig.h>

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
#include <app/app-platform/ContentAppPlatform.h>
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

#include <list>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ContentLauncher;
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
using namespace chip::AppPlatform;
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
using chip::Protocols::InteractionModel::Status;

static constexpr size_t kContentLaunchDelegateTableSize =
    MATTER_DM_CONTENT_LAUNCHER_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;
static_assert(kContentLaunchDelegateTableSize < kEmberInvalidEndpointIndex, "ContentLaunch Delegate table size error");

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

    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, ContentLauncher::Id,
                                                       MATTER_DM_CONTENT_LAUNCHER_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kContentLaunchDelegateTableSize ? nullptr : gDelegateTable[ep]);
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
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, ContentLauncher::Id,
                                                       MATTER_DM_CONTENT_LAUNCHER_CLUSTER_SERVER_ENDPOINT_COUNT);
    // if endpoint is found
    if (ep < kContentLaunchDelegateTableSize)
    {
        gDelegateTable[ep] = delegate;
    }
    else
    {
    }
}

bool Delegate::HasFeature(chip::EndpointId endpoint, Feature feature)
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
    CHIP_ERROR ReadRevisionAttribute(EndpointId endpoint, app::AttributeValueEncoder & aEncoder, Delegate * delegate);
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
    case app::Clusters::ContentLauncher::Attributes::ClusterRevision::Id: {
        return ReadRevisionAttribute(endpoint, aEncoder, delegate);
    }
    default:
        break;
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

CHIP_ERROR ContentLauncherAttrAccess::ReadRevisionAttribute(EndpointId endpoint, app::AttributeValueEncoder & aEncoder,
                                                            Delegate * delegate)
{
    uint16_t clusterRevision = delegate->GetClusterRevision(endpoint);
    return aEncoder.Encode(clusterRevision);
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
    auto & playbackPreferences    = commandData.playbackPreferences;
    auto & useCurrentContext      = commandData.useCurrentContext;

    app::CommandResponseHelper<Commands::LauncherResponse::Type> responder(commandObj, commandPath);

    Delegate * delegate = GetDelegate(endpoint);

    VerifyOrExit(isDelegateNull(delegate, endpoint) != true && delegate->HasFeature(endpoint, Feature::kContentSearch),
                 err = CHIP_ERROR_INCORRECT_STATE);

    // note assume if client does not send useCurrentContext then it's set to false
    delegate->HandleLaunchContent(responder, decodableParameterList, autoplay, data.HasValue() ? data.Value() : CharSpan(),
                                  playbackPreferences, useCurrentContext.HasValue() ? useCurrentContext.Value() : false);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfContentLauncherClusterLaunchContentCallback error: %s", err.AsString());
    }

    // If isDelegateNull, no one will call responder, so HasSentResponse will be false
    if (!responder.HasSentResponse())
    {
        commandObj->AddStatus(commandPath, Status::Failure);
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

    app::CommandResponseHelper<Commands::LauncherResponse::Type> responder(commandObj, commandPath);

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true && delegate->HasFeature(endpoint, Feature::kURLPlayback),
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
        commandObj->AddStatus(commandPath, Status::Failure);
    }

    return true;
}

// -----------------------------------------------------------------------------
// Plugin initialization

void MatterContentLauncherPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gContentLauncherAttrAccess);
}
