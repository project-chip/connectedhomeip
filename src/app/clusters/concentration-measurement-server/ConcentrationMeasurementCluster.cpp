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

#include "ConcentrationMeasurementCluster.h"
#include "ConcentrationMeasurementDelegate.h"
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <lib/core/CHIPError.h>

using namespace chip::app::Clusters::ConcentrationMeasurement::Attributes;
using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace ConcentrationMeasurement {

ConcentrationMeasurementCluster::ConcentrationMeasurementCluster(EndpointId endpointId, ClusterId clusterId,
                                                                 BitFlags<Feature> features, Delegate & delegate) :
    DefaultServerCluster({ endpointId, clusterId }),
    mFeatures(features), mDelegate(delegate)
{}

ConcentrationMeasurementCluster::~ConcentrationMeasurementCluster() {}

CHIP_ERROR ConcentrationMeasurementCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    return mDelegate.Init();
}
void ConcentrationMeasurementCluster::Shutdown(ClusterShutdownType shutdownType)
{
    DefaultServerCluster::Shutdown(shutdownType);
}

DataModel::ActionReturnStatus ConcentrationMeasurementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                             AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case MeasurementMedium::Id:
        return encoder.Encode(mDelegate.GetMeasurementMedium());

    case Attributes::FeatureMap::Id:
        // mFeatures was set at construction and never changes.
        // Raw() returns it as a uint32_t as required by the wire format.
        return encoder.Encode(mFeatures.Raw());

    case Attributes::ClusterRevision::Id:
        return encoder.Encode(kClusterRevision);

    // ── Feature::kNumericMeasurement
    case MeasuredValue::Id:
        VerifyOrReturnError(mFeatures.Has(Feature::kNumericMeasurement), Status::UnsupportedAttribute);
        return encoder.Encode(mDelegate.GetMeasuredValue());

    case MinMeasuredValue::Id:
        VerifyOrReturnError(mFeatures.Has(Feature::kNumericMeasurement), Status::UnsupportedAttribute);
        return encoder.Encode(mDelegate.GetMinMeasuredValue());

    case MaxMeasuredValue::Id:
        VerifyOrReturnError(mFeatures.Has(Feature::kNumericMeasurement), Status::UnsupportedAttribute);
        return encoder.Encode(mDelegate.GetMaxMeasuredValue());

    case Uncertainty::Id:
        VerifyOrReturnError(mFeatures.Has(Feature::kNumericMeasurement), Status::UnsupportedAttribute);
        return encoder.Encode(mDelegate.GetUncertainty());

    case MeasurementUnit::Id:
        VerifyOrReturnError(mFeatures.Has(Feature::kNumericMeasurement), Status::UnsupportedAttribute);
        return encoder.Encode(mDelegate.GetMeasurementUnit());

    // ── Feature::kPeakMeasurement
    case PeakMeasuredValue::Id:
        VerifyOrReturnError(mFeatures.Has(Feature::kPeakMeasurement), Status::UnsupportedAttribute);
        return encoder.Encode(mDelegate.GetPeakMeasuredValue());

    case PeakMeasuredValueWindow::Id:
        VerifyOrReturnError(mFeatures.Has(Feature::kPeakMeasurement), Status::UnsupportedAttribute);
        return encoder.Encode(mDelegate.GetPeakMeasuredValueWindow());

    // ── Feature::kAverageMeasurement
    case AverageMeasuredValue::Id:
        VerifyOrReturnError(mFeatures.Has(Feature::kAverageMeasurement), Status::UnsupportedAttribute);
        return encoder.Encode(mDelegate.GetAverageMeasuredValue());

    case AverageMeasuredValueWindow::Id:
        VerifyOrReturnError(mFeatures.Has(Feature::kAverageMeasurement), Status::UnsupportedAttribute);
        return encoder.Encode(mDelegate.GetAverageMeasuredValueWindow());

    // ── Feature::kLevelIndication
    case LevelValue::Id:
        VerifyOrReturnError(mFeatures.Has(Feature::kLevelIndication), Status::UnsupportedAttribute);
        return encoder.Encode(mDelegate.GetLevelValue());

    default:
        return Status::UnsupportedAttribute;
    }
}

CHIP_ERROR ConcentrationMeasurementCluster::Attributes(const ConcreteClusterPath & path,
                                                       ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    // The base class appends the global attributes that every cluster must
    // expose: AttributeList, AcceptedCommandList, GeneratedCommandList,
    // EventList, FeatureMap, ClusterRevision.
    ReturnErrorOnFailure(DefaultServerCluster::Attributes(path, builder));

    // Shorthand — all concentration attributes are read-only with kView privilege.
    using Flags = BitFlags<DataModel::AttributeQualityFlags>;
    using Priv  = chip::Access::Privilege;

    // MeasurementMedium is mandatory for all concentration measurement clusters.
    ReturnErrorOnFailure(builder.Append(DataModel::AttributeEntry(MeasurementMedium::Id, Flags{}, Priv::kView, std::nullopt)));

    if (mFeatures.Has(Feature::kNumericMeasurement))
    {
        ReturnErrorOnFailure(builder.Append(DataModel::AttributeEntry(MeasuredValue::Id, Flags{}, Priv::kView, std::nullopt)));
        ReturnErrorOnFailure(builder.Append(DataModel::AttributeEntry(MinMeasuredValue::Id, Flags{}, Priv::kView, std::nullopt)));
        ReturnErrorOnFailure(builder.Append(DataModel::AttributeEntry(MaxMeasuredValue::Id, Flags{}, Priv::kView, std::nullopt)));
        ReturnErrorOnFailure(builder.Append(DataModel::AttributeEntry(Uncertainty::Id, Flags{}, Priv::kView, std::nullopt)));
        ReturnErrorOnFailure(builder.Append(DataModel::AttributeEntry(MeasurementUnit::Id, Flags{}, Priv::kView, std::nullopt)));
    }

    if (mFeatures.Has(Feature::kPeakMeasurement))
    {
        ReturnErrorOnFailure(builder.Append(DataModel::AttributeEntry(PeakMeasuredValue::Id, Flags{}, Priv::kView, std::nullopt)));
        ReturnErrorOnFailure(
            builder.Append(DataModel::AttributeEntry(PeakMeasuredValueWindow::Id, Flags{}, Priv::kView, std::nullopt)));
    }

    if (mFeatures.Has(Feature::kAverageMeasurement))
    {
        ReturnErrorOnFailure(
            builder.Append(DataModel::AttributeEntry(AverageMeasuredValue::Id, Flags{}, Priv::kView, std::nullopt)));
        ReturnErrorOnFailure(
            builder.Append(DataModel::AttributeEntry(AverageMeasuredValueWindow::Id, Flags{}, Priv::kView, std::nullopt)));
    }

    if (mFeatures.Has(Feature::kLevelIndication))
    {
        ReturnErrorOnFailure(builder.Append(DataModel::AttributeEntry(LevelValue::Id, Flags{}, Priv::kView, std::nullopt)));
    }

    return CHIP_NO_ERROR;
}

} // namespace ConcentrationMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
