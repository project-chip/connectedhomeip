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
#include <app/InteractionModelEngine.h>
#include <app/persistence/AttributePersistence.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/OccupancySensing/Attributes.h>
#include <clusters/OccupancySensing/Metadata.h>
#include <cstring>
#include <lib/support/CodeUtils.h>

namespace chip::app::Clusters {

using namespace OccupancySensing::Attributes;
using namespace chip::app::Clusters::OccupancySensing;

namespace {

// Mapping table from spec:
//
//  | Feature Flag Value    | Value of OccupancySensorTypeBitmap    | Value of OccupancySensorType
//  | PHY | US | PIR        | ===================================== | ============================
//  | 0   | 0  | 0          | PIR ^*^                               | PIR
//  | 0   | 0  | 1          | PIR                                   | PIR
//  | 0   | 1  | 0          | Ultrasonic                            | Ultrasonic
//  | 0   | 1  | 1          | PIR + Ultrasonic                      | PIRAndUltrasonic
//  | 1   | 0  | 0          | PhysicalContact                       | PhysicalContact
//  | 1   | 0  | 1          | PhysicalContact + PIR                 | PIR
//  | 1   | 1  | 0          | PhysicalContact + Ultrasonic          | Ultrasonic
//  | 1   | 1  | 1          | PhysicalContact + PIR + Ultrasonic    | PIRAndUltrasonic

BitMask<OccupancySensing::OccupancySensorTypeBitmap>
FeaturesToOccupancySensorTypeBitmap(BitFlags<OccupancySensing::Feature> features)
{
    BitMask<OccupancySensing::OccupancySensorTypeBitmap> occupancySensorTypeBitmap{};

    occupancySensorTypeBitmap.Set(OccupancySensing::OccupancySensorTypeBitmap::kPir,
                                  features.Has(OccupancySensing::Feature::kPassiveInfrared));
    occupancySensorTypeBitmap.Set(OccupancySensing::OccupancySensorTypeBitmap::kUltrasonic,
                                  features.Has(OccupancySensing::Feature::kUltrasonic));
    occupancySensorTypeBitmap.Set(OccupancySensing::OccupancySensorTypeBitmap::kPhysicalContact,
                                  features.Has(OccupancySensing::Feature::kPhysicalContact));

    return occupancySensorTypeBitmap;
}

OccupancySensing::OccupancySensorTypeEnum FeaturesToOccupancySensorType(BitFlags<Feature> features)
{
    // Note how the truth table in the comment at the top of this section has
    // the PIR/US/PHY columns not in MSB-descending order. This is OK as we apply the correct
    // bit weighing to make the table equal below.
    unsigned maskFromFeatures = (features.Has(Feature::kPhysicalContact) ? (1 << 2) : 0) |
        (features.Has(Feature::kUltrasonic) ? (1 << 1) : 0) | (features.Has(Feature::kPassiveInfrared) ? (1 << 0) : 0);

    const OccupancySensorTypeEnum mappingTable[8] = {
        //  | PIR | US | PHY | Type value
        OccupancySensorTypeEnum::kPir,              //  | 0   | 0  | 0   | PIR
        OccupancySensorTypeEnum::kPir,              //  | 1   | 0  | 0   | PIR
        OccupancySensorTypeEnum::kUltrasonic,       //  | 0   | 1  | 0   | Ultrasonic
        OccupancySensorTypeEnum::kPIRAndUltrasonic, //  | 1   | 1  | 0   | PIRAndUltrasonic
        OccupancySensorTypeEnum::kPhysicalContact,  //  | 0   | 0  | 1   | PhysicalContact
        OccupancySensorTypeEnum::kPir,              //  | 1   | 0  | 1   | PIR
        OccupancySensorTypeEnum::kUltrasonic,       //  | 0   | 1  | 1   | Ultrasonic
        OccupancySensorTypeEnum::kPIRAndUltrasonic, //  | 1   | 1  | 1   | PIRAndUltrasonic
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
    DefaultServerCluster({ config.mEndpointId, OccupancySensing::Id }), mHoldTimeDelegate(config.mHoldTimeDelegate),
    mDelegate(config.mDelegate), mFeatureMap(config.mFeatureMap), mShowDeprecatedAttributes(config.mShowDeprecatedAttributes)
{
    SetHoldTimeLimits(config.mHoldTimeLimits);
    mHoldTime = std::clamp(config.mHoldTime, mHoldTimeLimits.holdTimeMin, mHoldTimeLimits.holdTimeMax);
}

CHIP_ERROR OccupancySensingCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    if (mHoldTimeDelegate)
    {
        AttributePersistence persistence(context.attributeStorage);
        uint16_t storedHoldTime;

        if (persistence.LoadNativeEndianValue({ mPath.mEndpointId, OccupancySensing::Id, Attributes::HoldTime::Id }, storedHoldTime,
                                              mHoldTime))
        {
            // A value was found in persistence. If stored value is not valid,
            // SetHoldTime will return an error and we keep the current mHoldTime.
            RETURN_SAFELY_IGNORED SetHoldTime(storedHoldTime);
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
        return encoder.Encode(mHoldTime);
    case Attributes::HoldTimeLimits::Id:
        return encoder.Encode(mHoldTimeLimits);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus OccupancySensingCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                      AttributeValueDecoder & aDecoder)
{
    VerifyOrDie(request.path.mClusterId == OccupancySensing::Id);

    switch (request.path.mAttributeId)
    {
    case Attributes::HoldTime::Id:
    case Attributes::PIROccupiedToUnoccupiedDelay::Id:
    case Attributes::UltrasonicOccupiedToUnoccupiedDelay::Id:
    case Attributes::PhysicalContactOccupiedToUnoccupiedDelay::Id: {
        if (!mHoldTimeDelegate)
        {
            return Protocols::InteractionModel::Status::UnsupportedAttribute;
        }
        uint16_t newHoldTime;
        ReturnErrorOnFailure(aDecoder.Decode(newHoldTime));
        return SetHoldTime(newHoldTime);
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
        { mHoldTimeDelegate != nullptr, Attributes::HoldTime::kMetadataEntry },
        { mHoldTimeDelegate != nullptr, Attributes::HoldTimeLimits::kMetadataEntry },
        { mHoldTimeDelegate != nullptr && mShowDeprecatedAttributes && mFeatureMap.Has(Feature::kPassiveInfrared),
          Attributes::PIROccupiedToUnoccupiedDelay::kMetadataEntry },
        { mHoldTimeDelegate != nullptr && mShowDeprecatedAttributes && mFeatureMap.Has(Feature::kUltrasonic),
          Attributes::UltrasonicOccupiedToUnoccupiedDelay::kMetadataEntry },
        { mHoldTimeDelegate != nullptr && mShowDeprecatedAttributes && mFeatureMap.Has(Feature::kPhysicalContact),
          Attributes::PhysicalContactOccupiedToUnoccupiedDelay::kMetadataEntry },
    };

    return listBuilder.Append(Span(OccupancySensing::Attributes::kMandatoryMetadata), Span(optionalAttributes));
}

DataModel::ActionReturnStatus OccupancySensingCluster::SetHoldTime(uint16_t holdTime)
{
    VerifyOrReturnError(mHoldTimeDelegate, Protocols::InteractionModel::Status::UnsupportedAttribute);
    VerifyOrReturnError(holdTime >= mHoldTimeLimits.holdTimeMin, Protocols::InteractionModel::Status::ConstraintError);
    VerifyOrReturnError(holdTime <= mHoldTimeLimits.holdTimeMax, Protocols::InteractionModel::Status::ConstraintError);

    if (mHoldTime == holdTime)
    {
        return DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp;
    }

    mHoldTime = holdTime;
    NotifyAttributeChanged(Attributes::HoldTime::Id);

    // If a timer is currently active, we need to adjust its duration to reflect the new hold time.
    if (mHoldTimeDelegate->IsTimerActive(this))
    {
        // Calculate the time that has already elapsed since the timer was started.
        System::Clock::Timestamp now                = mHoldTimeDelegate->GetCurrentMonotonicTimestamp();
        System::Clock::Timestamp elapsedTime        = now - mTimerStartedTimestamp;
        System::Clock::Seconds16 newHoldTimeSeconds = System::Clock::Seconds16(mHoldTime);

        // If the elapsed time is already greater than or equal to the new hold time,
        // the timer should have already fired. Cancel the current timer and immediately
        // set the state to unoccupied.
        if (elapsedTime >= newHoldTimeSeconds)
        {
            mHoldTimeDelegate->CancelTimer(this);
            DoSetOccupancy(false);
        }
        else
        {
            // Otherwise, restart the timer for the remaining duration.
            System::Clock::Timeout remainingTime = newHoldTimeSeconds - elapsedTime;
            // There's not much we can do it the timer fails to start. The Application is responsible for
            // providing a working timer delegate, so we ignore the return value here.
            RETURN_SAFELY_IGNORED mHoldTimeDelegate->StartTimer(this, remainingTime);
        }
    }

    if (mDelegate)
    {
        mDelegate->OnHoldTimeChanged(holdTime);
    }

    if (mContext != nullptr)
    {
        // There's not much we can do if persistence fails here, so we ignore the return value.
        // The application must ensure that persistence is working correctly.
        RETURN_SAFELY_IGNORED mContext->attributeStorage.WriteValue(
            { mPath.mEndpointId, OccupancySensing::Id, Attributes::HoldTime::Id },
            { reinterpret_cast<const uint8_t *>(&mHoldTime), sizeof(mHoldTime) });
    }

    return Protocols::InteractionModel::Status::Success;
}

void OccupancySensingCluster::SetOccupancy(bool occupied)
{
    if (mHoldTimeDelegate)
    {
        if (occupied)
        {
            mHoldTimeDelegate->CancelTimer(this);
            DoSetOccupancy(true);
        }
        else
        {
            mTimerStartedTimestamp = mHoldTimeDelegate->GetCurrentMonotonicTimestamp();
            mHoldTimeDelegate->StartTimer(this, System::Clock::Seconds16(mHoldTime));
        }
        return;
    }

    DoSetOccupancy(occupied);
}

void OccupancySensingCluster::TimerFired()
{
    DoSetOccupancy(false);
}

void OccupancySensingCluster::DoSetOccupancy(bool occupied)
{
    VerifyOrReturn(mOccupancy.Has(OccupancySensing::OccupancyBitmap::kOccupied) != occupied);
    mOccupancy.Set(OccupancySensing::OccupancyBitmap::kOccupied, occupied);
    NotifyAttributeChanged(Attributes::Occupancy::Id);

    if (mDelegate)
    {
        mDelegate->OnOccupancyChanged(occupied);
    }

    if (mContext != nullptr)
    {
        Events::OccupancyChanged::Type event;
        event.occupancy = mOccupancy;
        mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
    }
}

bool OccupancySensingCluster::IsOccupied() const
{
    return mOccupancy.Has(OccupancySensing::OccupancyBitmap::kOccupied);
}

void OccupancySensingCluster::SetHoldTimeLimits(const OccupancySensing::Structs::HoldTimeLimitsStruct::Type & holdTimeLimits)
{
    auto newHoldTimeLimits = holdTimeLimits;

    // Here we sanitize the input limits to ensure they are valid, in case the caller
    // provided invalid values.
    newHoldTimeLimits.holdTimeMin = std::max(static_cast<uint16_t>(1), newHoldTimeLimits.holdTimeMin);
    newHoldTimeLimits.holdTimeMax =
        std::max({ static_cast<uint16_t>(10), newHoldTimeLimits.holdTimeMin, newHoldTimeLimits.holdTimeMax });
    newHoldTimeLimits.holdTimeDefault =
        std::clamp(newHoldTimeLimits.holdTimeDefault, newHoldTimeLimits.holdTimeMin, newHoldTimeLimits.holdTimeMax);

    if (mHoldTimeLimits.holdTimeMin != newHoldTimeLimits.holdTimeMin ||
        mHoldTimeLimits.holdTimeMax != newHoldTimeLimits.holdTimeMax ||
        mHoldTimeLimits.holdTimeDefault != newHoldTimeLimits.holdTimeDefault)
    {
        mHoldTimeLimits = newHoldTimeLimits;
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
