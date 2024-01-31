/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app/clusters/time-synchronization-server/DefaultTimeSyncDelegate.h>
#include <app/clusters/valve-configuration-and-control-server/valve-configuration-and-control-delegate.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ValveConfigurationAndControl {

class ValveControlDelegate : public Delegate
{
public:
    DataModel::Nullable<chip::Percent> HandleOpenValve(DataModel::Nullable<chip::Percent> level) override;
    CHIP_ERROR HandleCloseValve() override;
    void HandleRemainingDurationTick(uint32_t duration) override;
};

} // namespace ValveConfigurationAndControl

namespace TimeSynchronization {

class ExtendedTimeSyncDelegate : public DefaultTimeSyncDelegate
{
public:
    void UTCTimeAvailabilityChanged(uint64_t time) override;
};

} // namespace TimeSynchronization
} // namespace Clusters
} // namespace app
} // namespace chip
