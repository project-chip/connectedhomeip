/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/occupancy-sensor-server/OccupancySensingCluster.h>

#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/OccupancySensing/Attributes.h>
#include <clusters/OccupancySensing/Commands.h>
#include <clusters/OccupancySensing/Metadata.h>

namespace chip::app::Clusters {

using namespace OccupancySensing::Attributes;
using namespace chip::app::Clusters::OccupancySensing;

namespace {
OccupancySensing::OccupancySensorTypeEnum OccupancySensorTypeEnumFromBitmap(BitMask<OccupancySensing::OccupancySensorTypeBitmap> bitmap)
{
    if (bitmap.Has(OccupancySensing::OccupancySensorTypeBitmap::kPir) && bitmap.Has(OccupancySensing::OccupancySensorTypeBitmap::kUltrasonic) && bitmap.Has(OccupancySensing::OccupancySensorTypeBitmap::kPhysicalContact))
    {
        return OccupancySensing::OccupancySensorTypeEnum::kPIRAndUltrasonic;
    }
    if (bitmap.Has(OccupancySensing::OccupancySensorTypeBitmap::kPir) && bitmap.Has(OccupancySensing::OccupancySensorTypeBitmap::kUltrasonic))
    {
        return OccupancySensing::OccupancySensorTypeEnum::kPIRAndUltrasonic;
    }
    if (bitmap.Has(OccupancySensing::OccupancySensorTypeBitmap::kPhysicalContact) && bitmap.Has(OccupancySensing::OccupancySensorTypeBitmap::kPir))
    {
        return OccupancySensing::OccupancySensorTypeEnum::kPir;
    }
    if (bitmap.Has(OccupancySensing::OccupancySensorTypeBitmap::kPhysicalContact) && bitmap.Has(OccupancySensing::OccupancySensorTypeBitmap::kUltrasonic))
    {
        return OccupancySensing::OccupancySensorTypeEnum::kUltrasonic;
    }
    if (bitmap.Has(OccupancySensing::OccupancySensorTypeBitmap::kPir))
    {
        return OccupancySensing::OccupancySensorTypeEnum::kPir;
    }
    if (bitmap.Has(OccupancySensing::OccupancySensorTypeBitmap::kUltrasonic))
    {
        return OccupancySensing::OccupancySensorTypeEnum::kUltrasonic;
    }
    if (bitmap.Has(OccupancySensing::OccupancySensorTypeBitmap::kPhysicalContact))
    {
        return OccupancySensing::OccupancySensorTypeEnum::kPhysicalContact;
    }
    return OccupancySensing::OccupancySensorTypeEnum::kPir; // Default to PIR if no bits are set, as per spec table for 000
}
} // namespace

OccupancySensingCluster::OccupancySensingCluster(const Config & config) :
    DefaultServerCluster({ config.endpointId, OccupancySensing::Id }), mFeature(config.feature), mHoldTime(config.holdTime), mHoldTimeLimits(config.holdTimeLimits), mOccupancySensorTypeBitmap(config.occupancySensorTypeBitmap), mPIRUnoccupiedToOccupiedDelay(config.pirUnoccupiedToOccupiedDelay), mPIRUnoccupiedToOccupiedThreshold(config.pirUnoccupiedToOccupiedThreshold), mUltrasonicUnoccupiedToOccupiedDelay(config.ultrasonicUnoccupiedToOccupiedDelay), mUltrasonicUnoccupiedToOccupiedThreshold(config.ultrasonicUnoccupiedToOccupiedThreshold), mPhysicalContactUnoccupiedToOccupiedDelay(config.physicalContactUnoccupiedToOccupiedDelay), mPhysicalContactUnoccupiedToOccupiedThreshold(config.physicalContactUnoccupiedToOccupiedThreshold)
{}

DataModel::ActionReturnStatus OccupancySensingCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                     AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Attributes::ClusterRevision::Id:
        return encoder.Encode(OccupancySensing::kRevision);
    case Attributes::FeatureMap::Id:
        return encoder.Encode(mFeature);
    case Attributes::HoldTime::Id:
    case Attributes::PIROccupiedToUnoccupiedDelay::Id:
    case Attributes::UltrasonicOccupiedToUnoccupiedDelay::Id:
    case Attributes::PhysicalContactOccupiedToUnoccupiedDelay::Id:
        return encoder.Encode(mHoldTime);
    case Attributes::HoldTimeLimits::Id:
        return encoder.Encode(mHoldTimeLimits);
    case Attributes::Occupancy::Id:
        return encoder.Encode(mOccupancy);
    case Attributes::OccupancySensorType::Id:
        return encoder.Encode(OccupancySensorTypeEnumFromBitmap(mOccupancySensorTypeBitmap));
    case Attributes::OccupancySensorTypeBitmap::Id:
        return encoder.Encode(mOccupancySensorTypeBitmap);
    case Attributes::PIRUnoccupiedToOccupiedDelay::Id:
        return encoder.Encode(mPIRUnoccupiedToOccupiedDelay);
    case Attributes::PIRUnoccupiedToOccupiedThreshold::Id:
        return encoder.Encode(mPIRUnoccupiedToOccupiedThreshold);
    case Attributes::UltrasonicUnoccupiedToOccupiedDelay::Id:
        return encoder.Encode(mUltrasonicUnoccupiedToOccupiedDelay);
    case Attributes::UltrasonicUnoccupiedToOccupiedThreshold::Id:
        return encoder.Encode(mUltrasonicUnoccupiedToOccupiedThreshold);
    case Attributes::PhysicalContactUnoccupiedToOccupiedDelay::Id:
        return encoder.Encode(mPhysicalContactUnoccupiedToOccupiedDelay);
    case Attributes::PhysicalContactUnoccupiedToOccupiedThreshold::Id:
        return encoder.Encode(mPhysicalContactUnoccupiedToOccupiedThreshold);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

} // namespace chip::app::Clusters