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

#include <app/AttributeValueEncoder.h>
#include <app/CommandResponseHelper.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ApplicationLauncher {

using Application          = chip::app::Clusters::ApplicationLauncher::Structs::ApplicationStruct::Type;
using ApplicationEPType    = chip::app::Clusters::ApplicationLauncher::Structs::ApplicationEPStruct::Type;
using LauncherResponseType = chip::app::Clusters::ApplicationLauncher::Commands::LauncherResponse::Type;

/** @brief
 *    Defines methods for implementing application-specific logic for the Application Launcher Cluster.
 */
class Delegate
{
public:
    Delegate() : Delegate(false){};
    Delegate(bool featureMapContentPlatform) { mFeatureMapContentPlatform = featureMapContentPlatform; };

    inline bool HasFeature(Feature feature)
    {
        if (feature == Feature::kApplicationPlatform)
        {
            return mFeatureMapContentPlatform;
        }
        return false;
    }

    // this attribute should only be enabled for app platform instance (endpoint 1)
    CHIP_ERROR HandleGetCurrentApp(app::AttributeValueEncoder & aEncoder);

    virtual CHIP_ERROR HandleGetCatalogList(app::AttributeValueEncoder & aEncoder) = 0;

    virtual void HandleLaunchApp(CommandResponseHelper<LauncherResponseType> & helper, const ByteSpan & data,
                                 const Application & application)                                                     = 0;
    virtual void HandleStopApp(CommandResponseHelper<LauncherResponseType> & helper, const Application & application) = 0;
    virtual void HandleHideApp(CommandResponseHelper<LauncherResponseType> & helper, const Application & application) = 0;

    virtual ~Delegate() = default;

protected:
    bool mFeatureMapContentPlatform = false;
};

} // namespace ApplicationLauncher
} // namespace Clusters
} // namespace app
} // namespace chip
