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
    chip::CharSpan HandleGetVendorName() override;
    uint16_t HandleGetVendorId() override;
    chip::CharSpan HandleGetApplicationName() override;
    uint16_t HandleGetProductId() override;
    chip::app::Clusters::ApplicationBasic::Structs::ApplicationBasicApplication::Type HandleGetApplication() override;
    chip::app::Clusters::ApplicationBasic::ApplicationStatusEnum HandleGetStatus() override;
    chip::CharSpan HandleGetApplicationVersion() override;
    std::list<uint16_t> HandleGetAllowedVendorList() override;
};
