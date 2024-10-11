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
class DefaultTimeSyncDelegate : public Delegate
{

public:
    DefaultTimeSyncDelegate() : Delegate(){};
    bool IsNTPAddressValid(CharSpan ntp) override;
    bool IsNTPAddressDomain(CharSpan ntp) override;
    CHIP_ERROR UpdateTimeFromPlatformSource(chip::Callback::Callback<OnTimeSyncCompletion> * callback) override;
};

} // namespace TimeSynchronization
} // namespace Clusters
} // namespace app
} // namespace chip
