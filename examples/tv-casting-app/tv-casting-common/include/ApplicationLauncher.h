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

class LaunchAppCommand
    : public MediaCommandBase<chip::app::Clusters::ApplicationLauncher::Commands::LaunchApp::Type,
                              chip::app::Clusters::ApplicationLauncher::Commands::LauncherResponse::DecodableType>
{
public:
    LaunchAppCommand() : MediaCommandBase(chip::app::Clusters::ApplicationLauncher::Id) {}

    CHIP_ERROR Invoke(chip::app::Clusters::ApplicationLauncher::Structs::Application::Type application,
                      chip::Optional<chip::ByteSpan> data, std::function<void(CHIP_ERROR)> responseCallback);
};

class StopAppCommand : public MediaCommandBase<chip::app::Clusters::ApplicationLauncher::Commands::StopApp::Type,
                                               chip::app::Clusters::ApplicationLauncher::Commands::LauncherResponse::DecodableType>
{
public:
    StopAppCommand() : MediaCommandBase(chip::app::Clusters::ApplicationLauncher::Id) {}

    CHIP_ERROR Invoke(chip::app::Clusters::ApplicationLauncher::Structs::Application::Type application,
                      std::function<void(CHIP_ERROR)> responseCallback);
};

class HideAppCommand : public MediaCommandBase<chip::app::Clusters::ApplicationLauncher::Commands::HideApp::Type,
                                               chip::app::Clusters::ApplicationLauncher::Commands::LauncherResponse::DecodableType>
{
public:
    HideAppCommand() : MediaCommandBase(chip::app::Clusters::ApplicationLauncher::Id) {}

    CHIP_ERROR Invoke(chip::app::Clusters::ApplicationLauncher::Structs::Application::Type application,
                      std::function<void(CHIP_ERROR)> responseCallback);
};
