/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
