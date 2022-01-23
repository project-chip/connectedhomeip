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

#include <app/clusters/application-launcher-server/application-launcher-server.h>
#include <list>

using namespace chip;
using namespace chip::app::Clusters;

class ApplicationLauncherManager : public ApplicationLauncher::Delegate
{
public:
    ApplicationLauncherManager() : ApplicationLauncher::Delegate(){};
    ApplicationLauncherManager(bool featureMapContentPlatform) : ApplicationLauncher::Delegate(featureMapContentPlatform){};

    std::list<uint16_t> HandleGetCatalogList() override;

    ApplicationLauncher::Commands::LauncherResponse::Type
    HandleLaunchApp(const CharSpan & data,
                    const ApplicationLauncher::Structs::ApplicationLauncherApplication::Type & application) override;
    ApplicationLauncher::Commands::LauncherResponse::Type
    HandleStopApp(const ApplicationLauncher::Structs::ApplicationLauncherApplication::Type & application) override;
    ApplicationLauncher::Commands::LauncherResponse::Type
    HandleHideApp(const ApplicationLauncher::Structs::ApplicationLauncherApplication::Type & application) override;
};
