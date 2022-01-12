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

ApplicationBasicManager::ApplicationBasicManager(const char * szVendorName, uint16_t vendorId, const char * szApplicationName,
                                                 uint16_t productId, const char * szApplicationVersion)
{
    ChipLogProgress(DeviceLayer, "ApplicationBasic[%s]: Application Name=\"%s\"", szApplicationName, szApplicationName);

    strncpy(mApplicationName, szApplicationName, sizeof(mApplicationName));
    strncpy(mVendorName, szVendorName, sizeof(mVendorName));
    mVendorId = vendorId;
    strncpy(mApplicationVersion, szApplicationVersion, sizeof(mApplicationVersion));
    mProductId = productId;
}

chip::CharSpan ApplicationBasicManager::HandleGetVendorName()
{
    return chip::CharSpan(mVendorName, strlen(mVendorName));
}

uint16_t ApplicationBasicManager::HandleGetVendorId()
{
    return mVendorId;
}

chip::CharSpan ApplicationBasicManager::HandleGetApplicationName()
{
    return chip::CharSpan(mApplicationName, strlen(mApplicationName));
}

uint16_t ApplicationBasicManager::HandleGetProductId()
{
    return mProductId;
}

chip::app::Clusters::ApplicationBasic::Structs::Application::Type ApplicationBasicManager::HandleGetApplication()
{
    chip::app::Clusters::ApplicationBasic::Structs::Application::Type application;
    application.catalogVendorId = 123;
    application.applicationId   = chip::CharSpan(mApplicationName, strlen(mApplicationName));
    return application;
}

ApplicationStatusEnum ApplicationBasicManager::HandleGetStatus()
{
    return mApplicationStatus;
}

chip::CharSpan ApplicationBasicManager::HandleGetApplicationVersion()
{
    return chip::CharSpan(mApplicationVersion, strlen(mApplicationVersion));
}

std::list<uint16_t> ApplicationBasicManager::HandleGetAllowedVendorList()
{
    return { mVendorId, 456 };
}
