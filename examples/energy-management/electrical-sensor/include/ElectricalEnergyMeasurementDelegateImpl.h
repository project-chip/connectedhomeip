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

#include <app/clusters/electrical-energy-measurement-server/ElectricalEnergyMeasurementCluster.h>
#include <app/clusters/electrical-energy-measurement-server/ElectricalEnergyMeasurementDelegate.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalEnergyMeasurement {

class ElectricalEnergyMeasurementDelegate : public Delegate
{
public:
    ~ElectricalEnergyMeasurementDelegate() = default;

    // Delegate interface -- pulled by the cluster on its reporting timer
    DataModel::Nullable<int64_t> GetCumulativeEnergyImported() override;
    DataModel::Nullable<int64_t> GetCumulativeEnergyExported() override;
    DataModel::Nullable<int64_t> GetPeriodicEnergyImported() override;
    DataModel::Nullable<int64_t> GetPeriodicEnergyExported() override;

    // Application API to push raw counter values
    void SetCumulativeEnergyImported(int64_t value) { mCumulativeImported = value; }
    void SetCumulativeEnergyExported(int64_t value) { mCumulativeExported = value; }
    void SetPeriodicEnergyImported(int64_t value) { mPeriodicImported = value; }
    void SetPeriodicEnergyExported(int64_t value) { mPeriodicExported = value; }

private:
    int64_t mCumulativeImported = 0;
    int64_t mCumulativeExported = 0;
    int64_t mPeriodicImported   = 0;
    int64_t mPeriodicExported   = 0;
};

} // namespace ElectricalEnergyMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
