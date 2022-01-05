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

class ApplicationBasicManager : public chip::app::Clusters::ApplicationBasic::Delegate
{
public:
    ApplicationBasicManager() : ApplicationBasicManager("exampleVendorName1", 1, "exampleName1", 1, "exampleVersion"){};
    ApplicationBasicManager(const char * szVendorName, uint16_t vendorId, const char * szApplicationName, uint16_t productId,
                            const char * szApplicationVersion);
    virtual ~ApplicationBasicManager(){};

    chip::CharSpan HandleGetVendorName() override;
    uint16_t HandleGetVendorId() override;
    chip::CharSpan HandleGetApplicationName() override;
    uint16_t HandleGetProductId() override;
    chip::app::Clusters::ApplicationBasic::Structs::Application::Type HandleGetApplication() override;
    chip::app::Clusters::ApplicationBasic::ApplicationStatusEnum HandleGetStatus() override;
    chip::CharSpan HandleGetApplicationVersion() override;
    std::list<uint16_t> HandleGetAllowedVendorList() override;

    inline void SetApplicationStatus(chip::app::Clusters::ApplicationBasic::ApplicationStatusEnum applicationStatus)
    {
        mApplicationStatus = applicationStatus;
    };

    inline const char * GetApplicationName() override { return mApplicationName; }

protected:
    static const int kVendorNameSize         = 32;
    static const int kApplicationNameSize    = 32;
    static const int kApplicationVersionSize = 32;

    char mVendorName[kVendorNameSize];
    uint16_t mVendorId;
    char mApplicationName[kApplicationNameSize];
    uint16_t mProductId;
    chip::app::Clusters::ApplicationBasic::ApplicationStatusEnum mApplicationStatus =
        chip::app::Clusters::ApplicationBasic::ApplicationStatusEnum::kStopped;
    char mApplicationVersion[kApplicationVersionSize];
};
