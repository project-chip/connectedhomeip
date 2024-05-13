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
using namespace chip::app;
using namespace chip::app::Clusters::ApplicationBasic;

CHIP_ERROR ApplicationBasicManager::HandleGetVendorName(AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(CharSpan::fromCharString(mVendorName));
}

uint16_t ApplicationBasicManager::HandleGetVendorId()
{
    return mVendorId;
}

CHIP_ERROR ApplicationBasicManager::HandleGetApplicationName(AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(CharSpan::fromCharString(mApplicationName));
}

uint16_t ApplicationBasicManager::HandleGetProductId()
{
    return mProductId;
}

CHIP_ERROR ApplicationBasicManager::HandleGetApplicationVersion(AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(CharSpan::fromCharString(mApplicationVersion));
}

CHIP_ERROR ApplicationBasicManager::HandleGetAllowedVendorList(AttributeValueEncoder & aEncoder)
{
    std::list<uint16_t> allowedVendorList = GetAllowedVendorList();
    return aEncoder.EncodeList([allowedVendorList](const auto & encoder) -> CHIP_ERROR {
        for (const auto & allowedVendor : allowedVendorList)
        {
            ReturnErrorOnFailure(encoder.Encode(allowedVendor));
        }
        return CHIP_NO_ERROR;
    });
}
