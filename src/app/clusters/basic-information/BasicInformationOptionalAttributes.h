/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/BasicInformation/Attributes.h>

namespace chip::app::Clusters {

using BasicInformationOptionalAttributesSet = OptionalAttributeSet< //
    BasicInformation::Attributes::ManufacturingDate::Id,            //
    BasicInformation::Attributes::PartNumber::Id,                   //
    BasicInformation::Attributes::ProductURL::Id,                   //
    BasicInformation::Attributes::ProductLabel::Id,                 //
    BasicInformation::Attributes::SerialNumber::Id,                 //
    BasicInformation::Attributes::LocalConfigDisabled::Id,          //
    BasicInformation::Attributes::Reachable::Id,                    //
    BasicInformation::Attributes::ProductAppearance::Id,            //
    // Old specification versions had UniqueID as optional, so this
    // appears here even though MANDATORY in the latest spec. We
    // default it enabled (to decrease chances of error)
    BasicInformation::Attributes::UniqueID::Id //
    >;

} // namespace chip::app::Clusters
