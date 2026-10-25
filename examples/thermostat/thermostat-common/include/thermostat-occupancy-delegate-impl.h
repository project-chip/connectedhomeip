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

#include <app/persistence/AttributePersistenceProvider.h>
#include <app/persistence/AttributePersistenceProviderInstance.h>

#include <app/clusters/thermostat-server/ThermostatClusterOccupancy.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

/*
 * A simple implementation of ThermostatOccupancy::Delegate.
 * It reports and persists the basic state of the thermostat's occupancy attributes.
 */
class ThermostatOccupancyDelegate : public ThermostatOccupancy::Delegate
{
public:
    ThermostatOccupancyDelegate() = default;

    BitMask<OccupancyBitmap> GetOccupancy() const override;
    Protocols::InteractionModel::Status SetOccupancy(BitMask<OccupancyBitmap> occupancy, bool & changed) override;

private:
    BitMask<OccupancyBitmap> mOccupancy{ OccupancyBitmap::kOccupied };
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
