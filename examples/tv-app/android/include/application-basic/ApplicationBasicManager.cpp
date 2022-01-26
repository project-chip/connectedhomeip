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

#include "ApplicationBasicManager.h"

using namespace std;
using namespace chip::app::Clusters::ApplicationBasic;

chip::CharSpan ApplicationBasicManager::HandleGetVendorName()
{
    return chip::CharSpan::fromCharString("exampleVendorName1");
}

uint16_t ApplicationBasicManager::HandleGetVendorId()
{
    return 1;
}

chip::CharSpan ApplicationBasicManager::HandleGetApplicationName()
{
    return chip::CharSpan::fromCharString("exampleName1");
}

uint16_t ApplicationBasicManager::HandleGetProductId()
{
    return 1;
}

chip::app::Clusters::ApplicationBasic::Structs::ApplicationBasicApplication::Type ApplicationBasicManager::HandleGetApplication()
{
    chip::app::Clusters::ApplicationBasic::Structs::ApplicationBasicApplication::Type application;
    application.catalogVendorId = 123;
    application.applicationId   = chip::CharSpan::fromCharString("applicationId");
    return application;
}

ApplicationStatusEnum ApplicationBasicManager::HandleGetStatus()
{
    return ApplicationStatusEnum::kStopped;
}

chip::CharSpan ApplicationBasicManager::HandleGetApplicationVersion()
{
    return chip::CharSpan::fromCharString("exampleVersion");
}

std::list<uint16_t> ApplicationBasicManager::HandleGetAllowedVendorList()
{
    return { 123, 456 };
}
