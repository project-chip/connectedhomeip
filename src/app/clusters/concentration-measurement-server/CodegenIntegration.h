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

#include "ConcentrationMeasurementCluster.h"
#include "ConcentrationMeasurementDelegate.h"
#include "concentration-measurement-cluster-objects.h"
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ConcentrationMeasurement {

/**
 * Instance — thin convenience wrapper that owns DefaultDelegate +
 * ConcentrationMeasurementCluster + ServerClusterRegistration in one allocation.
 *
 * Migration from old template-based Instance<bool, bool, bool, bool, bool, bool>:
 *   Old: Instance<true, false, false, false, true, false> gCO2(ep, ..., kAir, kPpm);
 *        gCO2.Init();
 *        // (no explicit registration step)
 *
 *   New: static Instance gCO2(ep, CarbonDioxideConcentrationMeasurement::Id,
 *            BitFlags<Feature>(Feature::kNumericMeasurement, Feature::kPeakMeasurement),
 *            MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
 *        provider.RegisterCluster(gCO2.Registration());  // registers + calls Startup()
 *        gCO2.GetDelegate().HandleNewMeasuredValue(DataModel::MakeNullable(412.5f));
 *
 * For production code where RAM matters, skip Instance and use the three objects
 * directly with a custom Delegate that only stores the fields your features need:
 *
 *   static MyCO2Delegate    gDelegate;                        // only your fields
 *   static ConcentrationMeasurementCluster gCluster(ep, CarbonDioxideConcentrationMeasurement::Id,
 *                BitFlags<Feature>(Feature::kNumericMeasurement), gDelegate);
 *   static ServerClusterRegistration gReg(gCluster);
 *   provider.RegisterCluster(gReg);
 */
class Instance
{
public:
    Instance(EndpointId endpointId,
             ClusterId clusterId,
             BitFlags<Feature> features,
             MeasurementMediumEnum medium,
             MeasurementUnitEnum unit)
        : mDelegate(medium, unit),
          mCluster(endpointId, clusterId, features, mDelegate),
          mRegistration(mCluster)
    {}

    // Returns the delegate to push new sensor readings.
    // Example: instance.GetDelegate().HandleNewMeasuredValue(DataModel::MakeNullable(42.0f));
    DefaultDelegate & GetDelegate() { return mDelegate; }

    // Returns the registration entry — pass this to your data model provider
    // to register the cluster and trigger Startup().
    ServerClusterRegistration & Registration() { return mRegistration; }

private:
    // Construction order matters:
    //   1. delegate  — cluster stores a reference, must be alive first
    //   2. cluster   — registration stores a pointer, must be alive first
    //   3. registration — just holds the pointer; Startup() is called by the provider
    DefaultDelegate                  mDelegate;
    ConcentrationMeasurementCluster  mCluster;
    ServerClusterRegistration        mRegistration;
};

} // namespace ConcentrationMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
