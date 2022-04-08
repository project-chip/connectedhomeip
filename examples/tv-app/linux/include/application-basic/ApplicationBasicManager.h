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

#include <app/clusters/application-basic-server/application-basic-server.h>

using chip::CharSpan;
using chip::app::AttributeValueEncoder;
using chip::Platform::CopyString;
using ApplicationBasicDelegate = chip::app::Clusters::ApplicationBasic::Delegate;

class ApplicationBasicManager : public ApplicationBasicDelegate
{
public:
    ApplicationBasicManager() :
        ApplicationBasicManager(123, "applicationId", "exampleVendorName1", 1, "exampleName1", 1, "exampleVersion"){};
    ApplicationBasicManager(uint16_t szCatalogVendorId, const char * szApplicationId, const char * szVendorName, uint16_t vendorId,
                            const char * szApplicationName, uint16_t productId, const char * szApplicationVersion) :
        ApplicationBasicDelegate(szCatalogVendorId, szApplicationId)
    {

        ChipLogProgress(DeviceLayer, "ApplicationBasic[%s]: Application Name=\"%s\"", szApplicationId, szApplicationName);

        CopyString(mApplicationName, sizeof(mApplicationName), szApplicationName);
        CopyString(mVendorName, sizeof(mVendorName), szVendorName);
        mVendorId = vendorId;
        CopyString(mApplicationVersion, sizeof(mApplicationVersion), szApplicationVersion);
        mProductId                          = productId;
        static const uint16_t kTestVendorId = 456; // CI test cases require this vendor id
        mAllowedVendorList.push_back(vendorId);
        mAllowedVendorList.push_back(kTestVendorId);
    };
    virtual ~ApplicationBasicManager(){};

    CHIP_ERROR HandleGetVendorName(AttributeValueEncoder & aEncoder) override;
    uint16_t HandleGetVendorId() override;
    CHIP_ERROR HandleGetApplicationName(AttributeValueEncoder & aEncoder) override;
    uint16_t HandleGetProductId() override;
    CHIP_ERROR HandleGetApplicationVersion(AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR HandleGetAllowedVendorList(AttributeValueEncoder & aEncoder) override;

    std::list<uint16_t> GetAllowedVendorList() override { return mAllowedVendorList; };

protected:
    static const int kVendorNameSize         = 32;
    static const int kApplicationNameSize    = 32;
    static const int kApplicationVersionSize = 32;

    char mVendorName[kVendorNameSize];
    uint16_t mVendorId;
    char mApplicationName[kApplicationNameSize];
    uint16_t mProductId;
    char mApplicationVersion[kApplicationVersionSize];
    std::list<uint16_t> mAllowedVendorList = {};
};
