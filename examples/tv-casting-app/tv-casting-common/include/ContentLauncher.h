/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "MediaCommandBase.h"

#include <functional>
#include <zap-generated/CHIPClusters.h>

class LaunchURLCommand : public MediaCommandBase<chip::app::Clusters::ContentLauncher::Commands::LaunchURL::Type,
                                                 chip::app::Clusters::ContentLauncher::Commands::LaunchResponse::DecodableType>
{
public:
    LaunchURLCommand() : MediaCommandBase(chip::app::Clusters::ContentLauncher::Id) {}

    CHIP_ERROR Invoke(const char * contentUrl, const char * contentDisplayStr,
                      chip::Optional<chip::app::Clusters::ContentLauncher::Structs::BrandingInformation::Type> brandingInformation,
                      std::function<void(CHIP_ERROR)> responseCallback);
};

class LaunchContentCommand : public MediaCommandBase<chip::app::Clusters::ContentLauncher::Commands::LaunchContent::Type,
                                                     chip::app::Clusters::ContentLauncher::Commands::LaunchResponse::DecodableType>
{
public:
    LaunchContentCommand() : MediaCommandBase(chip::app::Clusters::ContentLauncher::Id) {}

    CHIP_ERROR Invoke(chip::app::Clusters::ContentLauncher::Structs::ContentSearch::Type search, bool autoPlay,
                      chip::Optional<chip::CharSpan> data, std::function<void(CHIP_ERROR)> responseCallback);
};
