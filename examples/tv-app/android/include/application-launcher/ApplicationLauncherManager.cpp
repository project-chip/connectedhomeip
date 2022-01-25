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

#include "ApplicationLauncherManager.h"

using namespace std;
using namespace chip::app::Clusters::ApplicationLauncher;

Structs::ApplicationEP::Type ApplicationLauncherManager::HandleGetCurrentApp()
{
    Structs::ApplicationEP::Type currentApp;
    currentApp.application.catalogVendorId = 123;
    currentApp.application.applicationId   = chip::CharSpan::fromCharString("applicationId");
    currentApp.endpoint                    = chip::CharSpan::fromCharString("endpointId");
    return currentApp;
}

std::list<uint16_t> ApplicationLauncherManager::HandleGetCatalogList()
{
    return { 123, 456 };
}

Commands::LauncherResponse::Type ApplicationLauncherManager::HandleLaunchApp(
    const chip::CharSpan & data,
    const chip::app::Clusters::ApplicationLauncher::Structs::ApplicationLauncherApplication::Type & application)
{
    // TODO: Insert code here
    Commands::LauncherResponse::Type response;
    response.data   = chip::CharSpan::fromCharString("data");
    response.status = StatusEnum::kSuccess;
    return response;
}

Commands::LauncherResponse::Type ApplicationLauncherManager::HandleStopApp(
    const chip::app::Clusters::ApplicationLauncher::Structs::ApplicationLauncherApplication::Type & application)
{
    // TODO: Insert code here
    Commands::LauncherResponse::Type response;
    response.data   = chip::CharSpan::fromCharString("data");
    response.status = StatusEnum::kSuccess;
    return response;
}

Commands::LauncherResponse::Type ApplicationLauncherManager::HandleHideApp(
    const chip::app::Clusters::ApplicationLauncher::Structs::ApplicationLauncherApplication::Type & application)
{
    // TODO: Insert code here
    Commands::LauncherResponse::Type response;
    response.data   = chip::CharSpan::fromCharString("data");
    response.status = StatusEnum::kSuccess;
    return response;
}
