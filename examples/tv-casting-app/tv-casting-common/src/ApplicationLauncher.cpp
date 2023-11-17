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

#include "ApplicationLauncher.h"

using namespace chip;
using namespace chip::app::Clusters;

CHIP_ERROR LaunchAppCommand::Invoke(chip::app::Clusters::ApplicationLauncher::Structs::ApplicationStruct::Type application,
                                    chip::Optional<chip::ByteSpan> data, std::function<void(CHIP_ERROR)> responseCallback)
{
    ApplicationLauncher::Commands::LaunchApp::Type request;
    request.application.SetValue(application);
    request.data = data;
    return MediaCommandBase::Invoke(request, responseCallback);
}

CHIP_ERROR StopAppCommand::Invoke(chip::app::Clusters::ApplicationLauncher::Structs::ApplicationStruct::Type application,
                                  std::function<void(CHIP_ERROR)> responseCallback)
{
    ApplicationLauncher::Commands::StopApp::Type request;
    request.application.SetValue(application);
    return MediaCommandBase::Invoke(request, responseCallback);
}

CHIP_ERROR HideAppCommand::Invoke(chip::app::Clusters::ApplicationLauncher::Structs::ApplicationStruct::Type application,
                                  std::function<void(CHIP_ERROR)> responseCallback)
{
    ApplicationLauncher::Commands::HideApp::Type request;
    request.application.SetValue(application);
    return MediaCommandBase::Invoke(request, responseCallback);
}
