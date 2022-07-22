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
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ApplicationLauncher;
using namespace chip::Uint8;

CHIP_ERROR ApplicationLauncherManager::HandleGetCatalogList(AttributeValueEncoder & aEncoder)
{
    std::list<uint16_t> catalogList = { 123, 456 };
    return aEncoder.EncodeList([catalogList](const auto & encoder) -> CHIP_ERROR {
        for (const auto & catalog : catalogList)
        {
            ReturnErrorOnFailure(encoder.Encode(catalog));
        }
        return CHIP_NO_ERROR;
    });
}

void ApplicationLauncherManager::HandleLaunchApp(CommandResponseHelper<LauncherResponseType> & helper, const ByteSpan & data,
                                                 const ApplicationType & application)
{
    ChipLogProgress(Zcl, "ApplicationLauncherManager::HandleLaunchApp");

    // TODO: Insert code here
    LauncherResponseType response;
    const char * buf = "data";
    response.data    = ByteSpan(from_const_char(buf), strlen(buf));
    response.status  = ApplicationLauncherStatusEnum::kSuccess;
    helper.Success(response);
}

void ApplicationLauncherManager::HandleStopApp(CommandResponseHelper<LauncherResponseType> & helper,
                                               const ApplicationType & application)
{
    ChipLogProgress(Zcl, "ApplicationLauncherManager::HandleStopApp");

    // TODO: Insert code here
    LauncherResponseType response;
    const char * buf = "data";
    response.data    = ByteSpan(from_const_char(buf), strlen(buf));
    response.status  = ApplicationLauncherStatusEnum::kSuccess;
    helper.Success(response);
}

void ApplicationLauncherManager::HandleHideApp(CommandResponseHelper<LauncherResponseType> & helper,
                                               const ApplicationType & application)
{
    ChipLogProgress(Zcl, "ApplicationLauncherManager::HandleHideApp");

    // TODO: Insert code here
    LauncherResponseType response;
    const char * buf = "data";
    response.data    = ByteSpan(from_const_char(buf), strlen(buf));
    response.status  = ApplicationLauncherStatusEnum::kSuccess;
    helper.Success(response);
}
