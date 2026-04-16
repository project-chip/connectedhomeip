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

#include "ConcentrationMeasurementDelegate.h"
#include "concentration-measurement-cluster-objects.h"
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model/Nullable.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <lib/support/BitFlags.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ConcentrationMeasurement {

/**
 * ConcentrationMeasurementCluster mplementation serves for ALL 10 aliased Concentration Measurement clusters:
 *   CarbonDioxideConcentrationMeasurement, CarbonMonoxideConcentrationMeasurement,
 *   NitrogenDioxideConcentrationMeasurement, OzoneConcentrationMeasurement,
 *   PM2_5ConcentrationMeasurement, PM1ConcentrationMeasurement,
 *   PM10ConcentrationMeasurement, RadonConcentrationMeasurement,
 *   TotalVolatileOrganicCompoundsConcentrationMeasurement,
 *   FormaldehydeConcentrationMeasurement
 *
 * They all share an identical attribute structure. The only difference between
 * them is the ClusterId, which is passed to the constructor.
 *

 * This cluster stores NO attribute data. Every Read request is forwarded
 * to the Delegate, which provides values on demand.
 * 
 * Cluster takes BitFlags<Feature> at construction. The same flags drive three things:
 *   1. Attributes() — which attribute IDs are advertised to the DataModel layer
 *   2. ReadAttribute() — which attributes are readable (returns UnsupportedAttribute
 *      for any attribute whose feature flag is not set)
 *   3. FeatureMap attribute value (mFeatures.Raw())
 *
 *  USAGE
 *   // 1. Implement the delegate (see ConcentrationMeasuremntDelegate.h)
 *   class MyCO2Sensor : public Delegate { ... };
 *
 *   // 2. Instantiate — one cluster per ConcentrationType per endpoint
 *   static MyCO2Sensor gCO2Delegate;
 *   static ConcentrationMeasurementCluster gCO2(
 *       kAirQualityEndpointId,
 *       CarbonDioxideConcentrationMeasurement::Id,       // cluster ID for CO2
 *       BitFlags<Feature>(Feature::kNumericMeasurement, Feature::kLevelIndication),
 *       gCO2Delegate);
 *
 *   // 3. Register with the data model (this also calls Startup() automatically)
 *   static ServerClusterRegistration gCO2Registration(gCO2);
 *
 *   // 4. Push a new reading from your sensor driver
 *   gCO2Delegate.Update(412.5f);  // calls NotifyChanged() internally
 */
class ConcentrationMeasurementCluster : public DefaultServerCluster
{
public:
    /**
     * @param endpointId  Endpoint this cluster lives on. Must match ZAP configuration.
     * @param clusterId   Which aliased cluster ID to use (e.g. CarbonDioxideConcentrationMeasurement::Id).
     *                    Pass a different ID per pollutant — the attribute structure is identical.
     * @param features    Bitmask of optional feature groups this instance supports.
     *                    Example: BitFlags<Feature>(Feature::kNumericMeasurement, Feature::kPeakMeasurement)
     *                    This value is also returned as the FeatureMap attribute.
     * @param delegate    Application-supplied delegate. Must outlive this cluster instance.
     */
    ConcentrationMeasurementCluster(EndpointId endpointId, ClusterId clusterId,
                                    BitFlags<Feature> features, Delegate & delegate);

    ~ConcentrationMeasurementCluster() override;

    /**
     * Called once by the framework when the cluster is registered (via ServerClusterRegistration).
     * Calls DefaultServerCluster::Startup(), then delegate.SetCluster(this) + delegate.Init().
     */
    CHIP_ERROR Startup(ServerClusterContext & context) override;

    /**
     * Fills 'builder' with the AttributeEntry IDs this instance exposes.
     * Called by the DataModel layer to enumerate supported attributes.
     *
     * Always appended: MeasurementMedium, FeatureMap, ClusterRevision.
     * Feature-conditional:
     *   kNumericMeasurement → MeasuredValue, MinMeasuredValue, MaxMeasuredValue,
     *                         Uncertainty, MeasurementUnit
     *   kPeakMeasurement    → PeakMeasuredValue, PeakMeasuredValueWindow
     *   kAverageMeasurement → AverageMeasuredValue, AverageMeasuredValueWindow
     *   kLevelIndication    → LevelValue
     */
    CHIP_ERROR Attributes(const ConcreteClusterPath & path,
                          ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    /**
     * Reads an attribute value and encodes it into 'encoder'.
     * Returns UnsupportedAttribute for any attribute whose feature flag is not set.
     * All values are forwarded to the corresponding Delegate getter — this cluster
     * stores no data of its own.
     */
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;


    /** Returns true if the given feature flag was set at construction. */
    bool HasFeature(Feature f) const { return mFeatures.Has(f); }

private:
    // Per-spec maximum window for PeakMeasuredValueWindow / AverageMeasuredValueWindow: 7 days
    static constexpr uint32_t kWindowMaxSeconds = 604800;

    // Cluster revision per Matter spec
    static constexpr uint16_t kClusterRevision = 3;

    BitFlags<Feature> mFeatures;
    Delegate &        mDelegate;

   
    static bool CheckConstraintMinMax(DataModel::Nullable<float> value,
                                      DataModel::Nullable<float> minValue,
                                      DataModel::Nullable<float> maxValue)
    {
        return (minValue.IsNull() || value.IsNull() || (value.Value() >= minValue.Value())) &&
               (maxValue.IsNull() || value.IsNull() || (value.Value() <= maxValue.Value()));
    }

    static bool CheckConstraintsLessThanOrEqualTo(DataModel::Nullable<float> value,
                                                  DataModel::Nullable<float> valueToBeLessThanOrEqualTo)
    {
        return valueToBeLessThanOrEqualTo.IsNull() || value.IsNull() ||
               (value.Value() <= valueToBeLessThanOrEqualTo.Value());
    }

    static bool CheckConstraintsGreaterThanOrEqualTo(DataModel::Nullable<float> value,
                                                     DataModel::Nullable<float> valueToBeGreaterThanOrEqualTo)
    {
        return valueToBeGreaterThanOrEqualTo.IsNull() || value.IsNull() ||
               (value.Value() >= valueToBeGreaterThanOrEqualTo.Value());
    }
};

} // namespace ConcentrationMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
