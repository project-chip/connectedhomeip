/*
 *
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

#include <app/data-model/Nullable.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalEnergyMeasurement {

/// Abstract interface for the EEM cluster to pull energy readings from hardware.
///
/// The cluster calls these methods on its reporting timer (or on a manual GenerateReport() call).
/// Implementations should return Nullable null when the value cannot currently be determined.
/// The cluster only calls getters matching its enabled features.
/// All values are in milli-watt-hours (mWh).
class Delegate
{
public:
    virtual ~Delegate() = default;

    virtual DataModel::Nullable<int64_t> GetCumulativeEnergyImported() = 0;
    virtual DataModel::Nullable<int64_t> GetCumulativeEnergyExported() = 0;
    virtual DataModel::Nullable<int64_t> GetPeriodicEnergyImported()   = 0;
    virtual DataModel::Nullable<int64_t> GetPeriodicEnergyExported()   = 0;
};

} // namespace ElectricalEnergyMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
