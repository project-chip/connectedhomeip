/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "time-synchronization-delegate.h"

namespace chip {
namespace app {
namespace Clusters {
namespace TimeSynchronization {

using namespace chip::app;
using chip::app::Clusters::TimeSynchronization::Delegate;

class TimeSyncManager : public Delegate
{

public:
    using TimeZoneList = DataModel::List<chip::app::Clusters::TimeSynchronization::Structs::TimeZoneStruct::Type>;

    TimeSyncManager() : Delegate(){};
    void HandleTimeZoneChanged(TimeZoneList timeZoneList) override;
    CHIP_ERROR HandleDstoffsetlookup() override;
    bool HandleDstoffsetavailable(chip::CharSpan name) override;
    CHIP_ERROR HandleGetdstoffset() override;
    bool isNTPAddressValid(chip::CharSpan ntp) override;
    bool isNTPAddressDomain(chip::CharSpan ntp) override;
};

} // namespace TimeSynchronization
} // namespace Clusters
} // namespace app
} // namespace chip