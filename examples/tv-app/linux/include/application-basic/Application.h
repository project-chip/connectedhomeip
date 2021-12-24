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

#include <app-common/zap-generated/cluster-objects.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ApplicationBasic {
struct Application
{
    using ApplicationBasicStatus   = chip::app::Clusters::ApplicationBasic::ApplicationStatusEnum;
    char vendorName[32]            = "";
    char name[32]                  = "";
    char id[32]                    = "";
    char version[32]               = "";
    uint16_t vendorId              = 0;
    uint16_t productId             = 0;
    uint16_t catalogVendorId       = 0;
    uint16_t allowedVendorList[32] = { 123, 456 };
    ApplicationBasicStatus status  = ApplicationBasicStatus::kStopped;
};
} // namespace ApplicationBasic
} // namespace Clusters
} // namespace app
} // namespace chip
