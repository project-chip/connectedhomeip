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

#include <app/util/af.h>
#include <list>

namespace chip {
namespace app {
namespace Clusters {
namespace ApplicationBasic {

/** @brief
 *    Defines methods for implementing application-specific logic for the Application Basic Cluster.
 */
class Delegate
{
public:
    virtual chip::CharSpan HandleGetVendorName()                                                                     = 0;
    virtual uint16_t HandleGetVendorId()                                                                             = 0;
    virtual chip::CharSpan HandleGetApplicationName()                                                                = 0;
    virtual uint16_t HandleGetProductId()                                                                            = 0;
    virtual chip::app::Clusters::ApplicationBasic::Structs::ApplicationBasicApplication::Type HandleGetApplication() = 0;
    virtual ApplicationStatusEnum HandleGetStatus()                                                                  = 0;
    virtual chip::CharSpan HandleGetApplicationVersion()                                                             = 0;
    virtual std::list<uint16_t> HandleGetAllowedVendorList()                                                         = 0;

    virtual ~Delegate() = default;
};

} // namespace ApplicationBasic
} // namespace Clusters
} // namespace app
} // namespace chip
