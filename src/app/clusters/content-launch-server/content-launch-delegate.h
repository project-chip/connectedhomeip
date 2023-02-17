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

#include <app-common/zap-generated/cluster-objects.h>

#include <app/AttributeAccessInterface.h>
#include <app/CommandResponseHelper.h>
#include <app/util/af.h>
#include <list>

namespace chip {
namespace app {
namespace Clusters {
namespace ContentLauncher {

using BrandingInformation = chip::app::Clusters::ContentLauncher::Structs::BrandingInformationStruct::Type;
using Parameter           = chip::app::Clusters::ContentLauncher::Structs::ParameterStruct::DecodableType;

/** @brief
 *    Defines methods for implementing application-specific logic for the Content Launcher Cluster.
 */
class Delegate
{
public:
    virtual void HandleLaunchContent(CommandResponseHelper<Commands::LauncherResponse::Type> & helper,
                                     const DataModel::DecodableList<Parameter> & parameterList, bool autoplay,
                                     const CharSpan & data) = 0;

    virtual void HandleLaunchUrl(CommandResponseHelper<Commands::LauncherResponse::Type> & helper, const CharSpan & contentUrl,
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
