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
#include <clusters/OccupancySensing/Events.h>


#include <algorithm>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/InteractionModelEngine.h>
#include <app/persistence/AttributePersistence.h>
#include <clusters/OccupancySensing/Attributes.h>
#include <clusters/OccupancySensing/Commands.h>
#include <clusters/OccupancySensing/Metadata.h>
#include <lib/support/CodeUtils.h>

namespace chip::app::Clusters {

using namespace OccupancySensing::Attributes;
using namespace chip::app::Clusters::OccupancySensing;

namespace {

// Mapping table from spec:
//
//  | Feature Flag Value    | Value of OccupancySensorTypeBitmap    | Value of OccupancySensorType
//  | PIR | US | PHY        | ===================================== | ============================
//  | 0   | 0  | 0          | PIR ^*^                               | PIR
//  | 1   | 0  | 0          | PIR                                   | PIR
//  | 0   | 1  | 0          | Ultrasonic                            | Ultrasonic
//  | 1   | 1  | 0          | PIR + Ultrasonic                      | PIRAndUltrasonic
//  | 0   | 0  | 1          | PhysicalContact                       | PhysicalContact
//  | 1   | 0  | 1          | PhysicalContact + PIR                 | PIR
//  | 0   | 1  | 1          | PhysicalContact + Ultrasonic          | Ultrasonic
//  | 1   | 1  | 1          | PhysicalContact + PIR + Ultrasonic    | PIRAndUltrasonic

BitMask<OccupancySensing::OccupancySensorTypeBitmap> FeaturesToOccupancySensorTypeBitmap(BitFlags<OccupancySensing::Feature> features)
{
    BitMask<OccupancySensing::OccupancySensorTypeBitmap> occupancySensorTypeBitmap{};

    if (!features.HasAny(OccupancySensing::Feature::kPassiveInfrared, OccupancySensing::Feature::kUltrasonic, OccupancySensing::Feature::kPhysicalContact))
    {
        // If no features are set, default to PIR
        occupancySensorTypeBitmap.Set(OccupancySensing::OccupancySensorTypeBitmap::kPir);
    }
    else
    {
        occupancySensorTypeBitmap.Set(OccupancySensing::OccupancySensorTypeBitmap::kPir, features.Has(OccupancySensing::Feature::kPassiveInfrared));
        occupancySensorTypeBitmap.Set(OccupancySensing::OccupancySensorTypeBitmap::kUltrasonic, features.Has(OccupancySensing::Feature::kUltrasonic));
        occupancySensorTypeBitmap.Set(OccupancySensing::OccupancySensorTypeBitmap::kPhysicalContact, features.Has(OccupancySensing::Feature::kPhysicalContact));
    }

    return occupancySensorTypeBitmap;
}

OccupancySensing::OccupancySensorTypeEnum FeaturesToOccupancySensorType(BitFlags<Feature> features)
{
    // Note how the truth table in the comment at the top of this section has
    // the PIR/US/PHY columns not in MSB-descending order. This is OK as we apply the correct
    // bit weighing to make the table equal below.
    unsigned maskFromFeatures = (features.Has(Feature::kPhysicalContact) ? (1 << 2) : 0) |
    (features.Has(Feature::kUltrasonic) ? (1 << 1) : 0) |
    (features.Has(Feature::kPassiveInfrared) ? (1 << 0) : 0);

    const OccupancySensorTypeEnum mappingTable[8] = {  //  | PIR | US | PHY | Type value
      OccupancySensorTypeEnum::kPir,                   //  | 0   | 0  | 0   | PIR
      OccupancySensorTypeEnum::kPir,                   //  | 1   | 0  | 0   | PIR
      OccupancySensorTypeEnum::kUltrasonic,            //  | 0   | 1  | 0   | Ultrasonic
      OccupancySensorTypeEnum::kPIRAndUltrasonic,      //  | 1   | 1  | 0   | PIRAndUltrasonic
      OccupancySensorTypeEnum::kPhysicalContact,       //  | 0   | 0  | 1   | PhysicalContact
      OccupancySensorTypeEnum::kPir,                   //  | 1   | 0  | 1   | PIR
      OccupancySensorTypeEnum::kUltrasonic,            //  | 0   | 1  | 1   | Ultrasonic
      OccupancySensorTypeEnum::kPIRAndUltrasonic,      //  | 1   | 1  | 1   | PIRAndUltrasonic
    };

    // This check is to ensure that no changes to the mapping table in the future can overrun.
    if (maskFromFeatures >= sizeof(mappingTable))
    {
        return OccupancySensorTypeEnum::kPir;
    }

    return mappingTable[maskFromFeatures];
}

} // namespace

OccupancySensingCluster::OccupancySensingCluster(const Config & config) :
    DefaultServerCluster({ config.mEndpointId, OccupancySensing::Id }),
                         mFeatureMap(config.mFeatureMap),
                         mHasHoldTime(config.mHasHoldTime),
                         mHoldTime(config.mHoldTime),
                         mHoldTimeLimits(config.mHoldTimeLimits)
{
    if (mHasHoldTime)
    {
        mHoldTimeLimits.holdTimeMin = std::max(static_cast<uint16_t>(1), mHoldTimeLimits.holdTimeMin);
        mHoldTimeLimits.holdTimeMax = std::max(mHoldTimeLimits.holdTimeMin, mHoldTimeLimits.holdTimeMax);
        mHoldTimeLimits.holdTimeDefault = std::clamp(mHoldTimeLimits.holdTimeDefault, mHoldTimeLimits.holdTimeMin, mHoldTimeLimits.holdTimeMax);
    }
}

CHIP_ERROR OccupancySensingCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    if (mHasHoldTime)
    {
        AttributePersistence persistence(context.attributeStorage);
        uint16_t storedHoldTime;

        if (persistence.LoadNativeEndianValue({ mPath.mEndpointId, OccupancySensing::Id, Attributes::HoldTime::Id }, storedHoldTime, mHoldTime))
        {
            // A value was found in persistence, so we'll use it.
            // We ignore the return status as we don't want to fail startup if the value is invalid.
            // The value will be coerced in SetHoldTime if it's out of bounds.
            (void) SetHoldTime(storedHoldTime);
        }
        else
        {
            // No value in persistence, so store the default value.
            ReturnErrorOnFailure(
                context.attributeStorage.WriteValue({ mPath.mEndpointId, OccupancySensing::Id, Attributes::HoldTime::Id }, { reinterpret_cast<const uint8_t *>(&mHoldTime), sizeof(mHoldTime) }));
        }
    }

    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus OccupancySensingCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                     AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Attributes::FeatureMap::Id:
        return encoder.Encode(mFeatureMap);
    case Attributes::ClusterRevision::Id:
        return encoder.Encode(OccupancySensing::kRevision);
    case Attributes::Occupancy::Id:
        return encoder.Encode(mOccupancy);
    case Attributes::OccupancySensorType::Id:
    return encoder.Encode(FeaturesToOccupancySensorType(mFeatureMap));
    case Attributes::OccupancySensorTypeBitmap::Id:
        return encoder.Encode(FeaturesToOccupancySensorTypeBitmap(mFeatureMap));
    case Attributes::HoldTime::Id:
    case Attributes::PIROccupiedToUnoccupiedDelay::Id:
    case Attributes::UltrasonicOccupiedToUnoccupiedDelay::Id:
    case Attributes::PhysicalContactOccupiedToUnoccupiedDelay::Id:
        if (!mHasHoldTime)
        {
            return Protocols::InteractionModel::Status::UnsupportedAttribute;
        }
        return encoder.Encode(mHoldTime);
    case Attributes::HoldTimeLimits::Id:
        if (!mHasHoldTime)
        {
            return Protocols::InteractionModel::Status::UnsupportedAttribute;
        }
        return encoder.Encode(mHoldTimeLimits);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus OccupancySensingCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request, AttributeValueDecoder & aDecoder)
{
    VerifyOrDie(request.path.mClusterId == OccupancySensing::Id);

    switch (request.path.mAttributeId)
    {
    case Attributes::HoldTime::Id:
    case Attributes::PIROccupiedToUnoccupiedDelay::Id:
    case Attributes::UltrasonicOccupiedToUnoccupiedDelay::Id:
    case Attributes::PhysicalContactOccupiedToUnoccupiedDelay::Id: {
        if (!mHasHoldTime)
        {
            return Protocols::InteractionModel::Status::UnsupportedAttribute;
        }
        uint16_t newHoldTime;
        ReturnErrorOnFailure(aDecoder.Decode(newHoldTime));
        return NotifyAttributeChangedIfSuccess(request.path.mAttributeId, SetHoldTime(newHoldTime));
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR OccupancySensingCluster::Attributes(const ConcreteClusterPath & clusterPath,
                                             ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    const AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { mHasHoldTime, Attributes::HoldTime::kMetadataEntry },
        { mHasHoldTime, Attributes::HoldTimeLimits::kMetadataEntry },
        { mHasHoldTime, Attributes::PIROccupiedToUnoccupiedDelay::kMetadataEntry },
        { mHasHoldTime, Attributes::UltrasonicOccupiedToUnoccupiedDelay::kMetadataEntry },
        { mHasHoldTime, Attributes::PhysicalContactOccupiedToUnoccupiedDelay::kMetadataEntry },
    };

    return listBuilder.Append(Span(OccupancySensing::Attributes::kMandatoryMetadata), Span(optionalAttributes));
}

DataModel::ActionReturnStatus OccupancySensingCluster::SetHoldTime(uint16_t holdTime)
{
    VerifyOrReturnError(holdTime >= mHoldTimeLimits.holdTimeMin, Protocols::InteractionModel::Status::ConstraintError);
    VerifyOrReturnError(holdTime <= mHoldTimeLimits.holdTimeMax, Protocols::InteractionModel::Status::ConstraintError);

    if (mHoldTime == holdTime)
    {
        return DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp;
    }

    mHoldTime = holdTime;

    if (mContext != nullptr)
    {
        CHIP_ERROR err = mContext->attributeStorage.WriteValue({ mPath.mEndpointId, OccupancySensing::Id, Attributes::HoldTime::Id }, { reinterpret_cast<const uint8_t *>(&mHoldTime), sizeof(mHoldTime) });
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "Failed to store holdTime in persistence: %" CHIP_ERROR_FORMAT, err.Format());
        }
    }

    return Protocols::InteractionModel::Status::Success;
}

void OccupancySensingCluster::SetOccupancy(bool occupied)
{
    BitMask<OccupancySensing::OccupancyBitmap> newOccupancy =
        occupied ? OccupancySensing::OccupancyBitmap::kOccupied : static_cast<OccupancySensing::OccupancyBitmap>(0);

    if (mOccupancy == newOccupancy)
    {
        return;
    }

    mOccupancy = newOccupancy;
    NotifyAttributeChanged(Attributes::Occupancy::Id);

    if (mContext != nullptr)
    {
        Events::OccupancyChanged::Type event;
        event.occupancy = mOccupancy;
                    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);    }
}

void OccupancySensingCluster::SetHoldTimeLimits(const OccupancySensing::Structs::HoldTimeLimitsStruct::Type & holdTimeLimits)
{
    if (mHoldTimeLimits.holdTimeMin != holdTimeLimits.holdTimeMin || mHoldTimeLimits.holdTimeMax != holdTimeLimits.holdTimeMax ||
        mHoldTimeLimits.holdTimeDefault != holdTimeLimits.holdTimeDefault)
    {
        mHoldTimeLimits = holdTimeLimits;
        NotifyAttributeChanged(Attributes::HoldTimeLimits::Id);
    }
}

uint16_t OccupancySensingCluster::GetHoldTime() const
{
    return mHoldTime;
}

const OccupancySensing::Structs::HoldTimeLimitsStruct::Type & OccupancySensingCluster::GetHoldTimeLimits() const
{
    return mHoldTimeLimits;
}

} // namespace chip::app::Clusters
