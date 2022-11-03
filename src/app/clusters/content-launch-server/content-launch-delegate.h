/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/cluster-objects.h>

#include <app/AttributeAccessInterface.h>
#include <app/CommandResponseHelper.h>
#include <app/util/af.h>
#include <list>

namespace chip {
namespace app {
namespace Clusters {
namespace ContentLauncher {

using BrandingInformation = chip::app::Clusters::ContentLauncher::Structs::BrandingInformation::Type;
using Parameter           = chip::app::Clusters::ContentLauncher::Structs::Parameter::DecodableType;

/** @brief
 *    Defines methods for implementing application-specific logic for the Content Launcher Cluster.
 */
class Delegate
{
public:
    virtual void HandleLaunchContent(CommandResponseHelper<Commands::LaunchResponse::Type> & helper,
                                     const DataModel::DecodableList<Parameter> & parameterList, bool autoplay,
                                     const CharSpan & data) = 0;

    virtual void HandleLaunchUrl(CommandResponseHelper<Commands::LaunchResponse::Type> & helper, const CharSpan & contentUrl,
                                 const CharSpan & displayString, const BrandingInformation & brandingInformation) = 0;

    virtual CHIP_ERROR HandleGetAcceptHeaderList(app::AttributeValueEncoder & aEncoder) = 0;

    virtual uint32_t HandleGetSupportedStreamingProtocols() = 0;

    bool HasFeature(chip::EndpointId endpoint, ContentLauncherFeature feature);
    virtual uint32_t GetFeatureMap(chip::EndpointId endpoint) = 0;

    virtual ~Delegate() = default;
};

} // namespace ContentLauncher
} // namespace Clusters
} // namespace app
} // namespace chip
