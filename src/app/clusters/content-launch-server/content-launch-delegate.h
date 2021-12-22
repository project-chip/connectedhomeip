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

#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/enums.h>

#include <app/util/af.h>
#include <lib/core/Optional.h>
#include <list>

struct LaunchResponse
{
    CHIP_ERROR err;
    chip::CharSpan data;
    chip::app::Clusters::ContentLauncher::StatusEnum status;
};

namespace chip {
namespace app {
namespace Clusters {
namespace ContentLauncher {

/** @brief
 *    Defines methods for implementing application-specific logic for the Content Launcher Cluster.
 */
class Delegate
{
public:
    virtual LaunchResponse HandleLaunchContent(chip::EndpointId endpointId, const std::list<Parameter> & parameterList,
                                               bool autoplay, const chip::CharSpan & data) = 0;

    virtual LaunchResponse HandleLaunchUrl(const chip::CharSpan & contentUrl, const chip::CharSpan & displayString,
                                           const std::list<BrandingInformation> & brandingInformation) = 0;

    virtual std::list<std::string> HandleGetAcceptHeaderList() = 0;

    virtual uint32_t HandleGetSupportedStreamingProtocols() = 0;

    virtual ~Delegate() = default;
};

} // namespace ContentLauncher
} // namespace Clusters
} // namespace app
} // namespace chip
