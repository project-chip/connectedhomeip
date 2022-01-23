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

CharSpan ApplicationBasicManager::HandleGetVendorName()
{
    return CharSpan(mVendorName, strlen(mVendorName));
}

uint16_t ApplicationBasicManager::HandleGetVendorId()
{
    return mVendorId;
}

CharSpan ApplicationBasicManager::HandleGetApplicationName()
{
    return CharSpan(mApplicationName, strlen(mApplicationName));
}

uint16_t ApplicationBasicManager::HandleGetProductId()
{
    return mProductId;
}

CharSpan ApplicationBasicManager::HandleGetApplicationVersion()
{
    return CharSpan(mApplicationVersion, strlen(mApplicationVersion));
}

std::list<uint16_t> ApplicationBasicManager::HandleGetAllowedVendorList()
{
    return { mVendorId, 456 };
}
