/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/ambient-context-sensing-server/AmbientContextSensingCluster.h>
#include <app/clusters/ambient-context-sensing-server/ambient-context-sensing-namespace.h>
#include <app/persistence/AttributePersistence.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server/Server.h>
#include <chrono>
#include <clusters/AmbientContextSensing/Metadata.h>

namespace chip::app::Clusters {

using namespace AmbientContextSensing;
using namespace AmbientContextSensing::Attributes;

AmbientContextSensingCluster::AmbientContextSensingCluster(const Config & config) :
    DefaultServerCluster({ config.mEndpointId, AmbientContextSensing::Id }), mFeatureMap(config.mFeatureMap),
    mHoldTimeDelegate(config.mHoldTimeDelegate)
{
    TEMPORARY_RETURN_IGNORED SetAmbientContextTypeSupported(config.mAmbientContextTypeSupportedList);
    SetHoldTimeLimits(config.mHoldTimeLimits);
    mHoldTime = std::clamp(config.mHoldTime, mHoldTimeLimits.holdTimeMin, mHoldTimeLimits.holdTimeMax);
}

CHIP_ERROR AmbientContextSensingCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    AttributePersistence persistence(context.attributeStorage);

    // Read the CountThreshold from the persistence storage
    uint16_t storedCountThreshold;
    if (persistence.LoadNativeEndianValue({ mPath.mEndpointId, AmbientContextSensing::Id, Attributes::ObjectCountConfig::Id },
                                          storedCountThreshold, mObjectCountConfig.objectCountThreshold))
    {
        // A value was found in persistence.
        mObjectCountConfig.objectCountThreshold = storedCountThreshold;
    }

    uint16_t storedHoldTime;
    if (persistence.LoadNativeEndianValue({ mPath.mEndpointId, AmbientContextSensing::Id, Attributes::HoldTime::Id },
                                          storedHoldTime, mHoldTime))
    {
        // A value was found in persistence.
        RETURN_SAFELY_IGNORED SetHoldTime(storedHoldTime);
    }

    return CHIP_NO_ERROR;
}

void AmbientContextSensingCluster::Shutdown(ClusterShutdownType shutdownType)
{
    if (mHoldTimeDelegate)
    {
        mHoldTimeDelegate->CancelTimer(this);
    }
    DefaultServerCluster::Shutdown(shutdownType);
}

DataModel::ActionReturnStatus AmbientContextSensingCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                          AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case HumanActivityDetected::Id:
        return encoder.Encode(mHumanActivityDetected);
    case ObjectIdentified::Id:
        return encoder.Encode(mObjectIdentified);
    case AudioContextDetected::Id:
        return encoder.Encode(mAudioContextDetected);
    case AmbientContextType::Id:
        return ReadAmbientContextType(encoder);
    case AmbientContextTypeSupported::Id:
        return ReadAmbientContextTypeSupported(mFeatureMap, encoder);
    case ObjectCountReached::Id:
        return encoder.Encode(mObjectCountReached);
    case ObjectCountConfig::Id:
        return encoder.Encode(mObjectCountConfig);
    case ObjectCount::Id:
        return encoder.Encode(mObjectCount);
    case SimultaneousDetectionLimit::Id:
        return encoder.Encode(mSimultaneousDetectionLimit);
    case HoldTime::Id:
        return encoder.Encode(mHoldTime);
    case HoldTimeLimits::Id:
        return encoder.Encode(mHoldTimeLimits);
    case PredictedActivity::Id:
        return ReadPredictedActivity(encoder);
    case FeatureMap::Id:
        return encoder.Encode(mFeatureMap);
    case ClusterRevision::Id:
        return encoder.Encode(AmbientContextSensing::kRevision);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus AmbientContextSensingCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                           AttributeValueDecoder & decoder)
{
    switch (request.path.mAttributeId)
    {
    case ObjectCountConfig::Id: {
        ObjectCountConfigType newObjCountConfig;
        ReturnErrorOnFailure(decoder.Decode(newObjCountConfig));
        return SetObjectCountConfig(newObjCountConfig);
    }
    case SimultaneousDetectionLimit::Id: {
        uint16_t newSimultaneousDetectionLimit;
        ReturnErrorOnFailure(decoder.Decode(newSimultaneousDetectionLimit));
        return SetSimultaneousDetectionLimit(newSimultaneousDetectionLimit);
    }
    case HoldTime::Id: {
        uint16_t newHoldTime;
        ReturnErrorOnFailure(decoder.Decode(newHoldTime));
        return SetHoldTime(newHoldTime);
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedWrite;
    }
}

CHIP_ERROR AmbientContextSensingCluster::Attributes(const ConcreteClusterPath & path,
                                                    ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);
    const AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { mFeatureMap.Has(Feature::kHumanActivity), Attributes::HumanActivityDetected::kMetadataEntry },
        { mFeatureMap.Has(Feature::kObjectIdentification), Attributes::ObjectIdentified::kMetadataEntry },
        { mFeatureMap.Has(Feature::kSoundIdentification), Attributes::AudioContextDetected::kMetadataEntry },
        { mFeatureMap.Has(Feature::kHumanActivity) || mFeatureMap.Has(Feature::kObjectIdentification) ||
              mFeatureMap.Has(Feature::kSoundIdentification),
          Attributes::AmbientContextType::kMetadataEntry },
        { mFeatureMap.Has(Feature::kHumanActivity) || mFeatureMap.Has(Feature::kObjectIdentification) ||
              mFeatureMap.Has(Feature::kSoundIdentification),
          Attributes::AmbientContextTypeSupported::kMetadataEntry },
        { mFeatureMap.Has(Feature::kObjectCounting) && mFeatureMap.Has(Feature::kObjectIdentification),
          Attributes::ObjectCountReached::kMetadataEntry },
        { mFeatureMap.Has(Feature::kObjectCounting) && mFeatureMap.Has(Feature::kObjectIdentification),
          Attributes::ObjectCountConfig::kMetadataEntry },
        { mFeatureMap.Has(Feature::kObjectCounting) && mFeatureMap.Has(Feature::kObjectIdentification) && mObjectCountReached,
          Attributes::ObjectCount::kMetadataEntry },
        { false, Attributes::SimultaneousDetectionLimit::kMetadataEntry },
        { false, Attributes::HoldTime::kMetadataEntry },
        { false, Attributes::HoldTimeLimits::kMetadataEntry },
        { mFeatureMap.Has(Feature::kPredictedActivity), Attributes::PredictedActivity::kMetadataEntry },
    };

    return listBuilder.Append(Span(AmbientContextSensing::Attributes::kMandatoryMetadata), Span(optionalAttributes));
}

CHIP_ERROR AmbientContextSensingCluster::SetAmbientContextTypeSupported(const std::vector<SemanticTagType> & ACTypeList)
{
    VerifyOrReturnError(ACTypeList.size() <= kMaxACTypeSupported, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(CheckInputSupportedType(ACTypeList));
    mAmbientContextTypeSupportedList = ACTypeList;
    return CHIP_NO_ERROR;
}

CHIP_ERROR AmbientContextSensingCluster::AddDetection(const AmbientContextSensingType & sensedEvent)
{
    VerifyOrReturnError(sensedEvent.ambientContextSensed.size() > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(sensedEvent.ambientContextSensed.size() <= kMaxACSensed, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSupportedEvent(sensedEvent), CHIP_ERROR_INCORRECT_STATE);

    // If there have already been mSimultaneousDetectionLimit items in mAmbientContextTypeList => remove the oldest ones
    if (mAmbientContextTypeList.size() >= mSimultaneousDetectionLimit)
    {
        // One more space is required
        uint8_t itemsToRemove = mAmbientContextTypeList.size() - mSimultaneousDetectionLimit + 1;
        for (auto i = 0; i < itemsToRemove; i++)
        {
            mAmbientContextTypeList.pop_back();
        }
        // The detected status may be different
        UpdateDetectionAttributes();
        // The earliest end-time item may have been removed
        UpdateEventTimeout();
    }

    AmbientContextSensed item{};
    bool fromExisting = false;

    for (auto it = mAmbientContextTypeList.begin(); it != mAmbientContextTypeList.end(); ++it)
    {
        if (CompareAmbientContextSensed(it->mInfo, sensedEvent))
        {
            // Same detection occurs.
            // Need to reorder the event and hold-time
            item = std::move(*it);
            mAmbientContextTypeList.erase(it);
            fromExisting = true;
            break;
        }
    }
    System::Clock::Timestamp now = mHoldTimeDelegate->GetCurrentMonotonicTimestamp();
    System::Clock::Seconds16 newHoldTime;
    if (!fromExisting)
    {
        // The new detection event
        const auto & tags = sensedEvent.ambientContextSensed;
        item.mOwnedTags.assign(tags.begin(), tags.end());

        item.mInfo = sensedEvent;
        item.mInfo.ambientContextSensed =
            chip::app::DataModel::List<const SemanticTagType>(item.mOwnedTags.data(), item.mOwnedTags.size());
        item.mStartTimestamp = now;

        newHoldTime = System::Clock::Seconds16(mHoldTime);
    }
    else
    {
        // The event which is detected and whose hold-time has not elapsed
        const System::Clock::Seconds16 elapsedSec =
            std::chrono::duration_cast<System::Clock::Seconds16>(now - item.mStartTimestamp);

        System::Clock::Seconds16 holdTimeMaxSec = System::Clock::Seconds16(mHoldTimeLimits.holdTimeMax);
        if (elapsedSec + System::Clock::Seconds16(mHoldTime) <= holdTimeMaxSec)
        {
            newHoldTime = System::Clock::Seconds16(mHoldTime);
        }
        else
        {
            newHoldTime = holdTimeMaxSec - elapsedSec;
        }
    }
    item.mEndTimestamp = item.mStartTimestamp + newHoldTime;
    mAmbientContextTypeList.push_front(std::move(item));
    UpdateDetectionAttributes();

    // Update the timer if required.
    // Note: If the new detection has existed, the newHoldTime may be small that it may expire soon
    UpdateEventTimeout();

    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus AmbientContextSensingCluster::SetObjectCountConfig(const ObjectCountConfigType & objectCountConfig)
{
    auto newObjectCountConfig = objectCountConfig;

    VerifyOrReturnError(newObjectCountConfig.objectCountThreshold >= kMinObjectCount,
                        Protocols::InteractionModel::Status::ConstraintError);
    if (newObjectCountConfig.countingObject.mfgCode.IsNull())
    {
        VerifyOrReturnError(newObjectCountConfig.countingObject.namespaceID == kNamespaceIdentifiedObject,
                            Protocols::InteractionModel::Status::ConstraintError);
        bool inList = false;
        for (auto item : mAmbientContextTypeSupportedList)
        {
            if (item.tag == newObjectCountConfig.countingObject.tag)
            {
                inList = true;
                break;
            }
        }
        VerifyOrReturnError(inList, Protocols::InteractionModel::Status::ConstraintError);
    }

    if (newObjectCountConfig.countingObject.namespaceID != mObjectCountConfig.countingObject.namespaceID ||
        newObjectCountConfig.countingObject.tag != mObjectCountConfig.countingObject.tag ||
        newObjectCountConfig.objectCountThreshold != mObjectCountConfig.objectCountThreshold)
    {
        mObjectCountConfig = newObjectCountConfig;
        NotifyAttributeChanged(Attributes::ObjectCountConfig::Id);

        // Save the value to persistence
        if (mContext != nullptr)
        {
            RETURN_SAFELY_IGNORED mContext->attributeStorage.WriteValue(
                { mPath.mEndpointId, AmbientContextSensing::Id, Attributes::ObjectCountConfig::Id },
                { reinterpret_cast<const uint8_t *>(&mObjectCountConfig.objectCountThreshold),
                  sizeof(mObjectCountConfig.objectCountThreshold) });
        }
    }
    else
    {
        return DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp;
    }

    return Protocols::InteractionModel::Status::Success;
}

CHIP_ERROR AmbientContextSensingCluster::SetObjectCount(uint16_t objectCount)
{
    VerifyOrReturnError((mFeatureMap.Has(Feature::kObjectCounting) && mFeatureMap.Has(Feature::kObjectIdentification)),
                        CHIP_ERROR_INCORRECT_STATE);
    if (objectCount == mObjectCount)
    {
        return CHIP_NO_ERROR;
    }

    mObjectCount = objectCount;
    NotifyAttributeChanged(Attributes::ObjectCount::Id);
    mObjectCountEndTime = mHoldTimeDelegate->GetCurrentMonotonicTimestamp() + System::Clock::Seconds16(mHoldTime);
    UpdateDetectionAttributes();
    UpdateEventTimeout();

    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus AmbientContextSensingCluster::SetSimultaneousDetectionLimit(const uint16_t simultaneousDetectionLimit)
{
    VerifyOrReturnError((simultaneousDetectionLimit <= kMaxSimultaneousDetectionLimit),
                        Protocols::InteractionModel::Status::ConstraintError);
    VerifyOrReturnError((simultaneousDetectionLimit != mSimultaneousDetectionLimit),
                        DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp);

    mSimultaneousDetectionLimit = simultaneousDetectionLimit;
    NotifyAttributeChanged(Attributes::SimultaneousDetectionLimit::Id);

    if (mAmbientContextTypeList.size() <= mSimultaneousDetectionLimit)
    {
        return Protocols::InteractionModel::Status::Success;
    }
    // Resize the list of AmbientContextType list if the updated limitation becomes smaller than it's current length
    uint8_t itemsToRemove = mAmbientContextTypeList.size() - mSimultaneousDetectionLimit;
    for (auto i = 0; i < itemsToRemove; i++)
    {
        mAmbientContextTypeList.pop_back();
    }

    // The detected status may be different
    UpdateDetectionAttributes();
    // The earliest end-time item may have been removed
    UpdateEventTimeout();

    return Protocols::InteractionModel::Status::Success;
}

DataModel::ActionReturnStatus AmbientContextSensingCluster::SetHoldTime(uint16_t holdTime)
{
    VerifyOrReturnError(holdTime >= mHoldTimeLimits.holdTimeMin, Protocols::InteractionModel::Status::ConstraintError);
    VerifyOrReturnError(holdTime <= mHoldTimeLimits.holdTimeMax, Protocols::InteractionModel::Status::ConstraintError);
    VerifyOrReturnError(holdTime != mHoldTime, DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp);

    mHoldTime = holdTime;
    NotifyAttributeChanged(Attributes::HoldTime::Id);

    // Save the value to persistence
    if (mContext != nullptr)
    {
        RETURN_SAFELY_IGNORED mContext->attributeStorage.WriteValue(
            { mPath.mEndpointId, AmbientContextSensing::Id, Attributes::HoldTime::Id },
            { reinterpret_cast<const uint8_t *>(&mHoldTime), sizeof(mHoldTime) });
    }

    return Protocols::InteractionModel::Status::Success;
}

void AmbientContextSensingCluster::SetHoldTimeLimits(
    const AmbientContextSensing::Structs::HoldTimeLimitsStruct::Type & holdTimeLimits)
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

    if ((mHoldTime < mHoldTimeLimits.holdTimeMin) || (mHoldTimeLimits.holdTimeMax < mHoldTime))
    {
        // HoldTime is out of the new bounds. Resetting to the new default value
        SetHoldTime(mHoldTimeLimits.holdTimeDefault);
    }
}

CHIP_ERROR AmbientContextSensingCluster::SetPredictedActivity(const std::vector<PredictedActivityType> & predictedActivityList)
{
    VerifyOrReturnError(predictedActivityList.size() <= kMaxPredictedActivity, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(CheckPredictedActivity(predictedActivityList));

    // Destroy the old context
    mPredictedActivityList.clear();
    mPredictedActivityList.shrink_to_fit();
    // Copy the input vector into our local container.
    mPredictedActivityList.resize(predictedActivityList.size());
    for (size_t i = 0; i < predictedActivityList.size(); i++)
    {
        const auto & src = predictedActivityList[i];
        auto & dst       = mPredictedActivityList[i];
        dst.mInfo        = src;
        if (!src.ambientContextType.HasValue())
        {
            dst.mOwnedTags.clear();
            dst.mInfo.ambientContextType.ClearValue();
            continue;
        }

        const auto & acTypeList = src.ambientContextType.Value();

        dst.mOwnedTags.assign(acTypeList.begin(), acTypeList.end());
        dst.mInfo.ambientContextType.SetValue(
            DataModel::List<const SemanticTagType>(dst.mOwnedTags.data(), static_cast<uint16_t>(dst.mOwnedTags.size())));
    }
    return CHIP_NO_ERROR;
}

void AmbientContextSensingCluster::TimerFired()
{
    VerifyOrReturn(!mAmbientContextTypeList.empty() || (mObjectCount != 0));
    System::Clock::Timestamp now = mHoldTimeDelegate->GetCurrentMonotonicTimestamp();
    RemoveExpiredItems(mAmbientContextTypeList, now);

    // Update the detection attribute
    UpdateDetectionAttributes();

    // If more items in the queue => Re-fire the timer
    UpdateEventTimeout();
}

bool AmbientContextSensingCluster::CompareAmbientContextSensed(const AmbientContextSensingType & sensedEvent,
                                                               const AmbientContextSensingType & newEvent)
{
    const auto existEventTags = sensedEvent.ambientContextSensed;
    const auto newEventTags   = newEvent.ambientContextSensed;

    // If the size is different, it's the different event
    VerifyOrReturnError(existEventTags.size() == newEventTags.size(), false);
    for (size_t i = 0; i < existEventTags.size(); i++)
    {
        if ((existEventTags[i].namespaceID != newEventTags[i].namespaceID) || (existEventTags[i].tag != newEventTags[i].tag))
        {
            return false;
        }
    }
    return true;
}

CHIP_ERROR AmbientContextSensingCluster::ReadAmbientContextTypeSupported(BitFlags<AmbientContextSensing::Feature> features,
                                                                         AttributeValueEncoder & encoder)
{
    VerifyOrReturnValue(!mAmbientContextTypeSupportedList.empty(), encoder.EncodeEmptyList());
    return encoder.EncodeList([this](const auto & encode) -> CHIP_ERROR {
        for (const auto & item : mAmbientContextTypeSupportedList)
        {
            ReturnErrorOnFailure(encode.Encode(item));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR AmbientContextSensingCluster::ReadAmbientContextType(AttributeValueEncoder & encoder)
{
    VerifyOrReturnValue(!mAmbientContextTypeSupportedList.empty(), encoder.EncodeEmptyList());

    return encoder.EncodeList([this](const auto & encode) -> CHIP_ERROR {
        for (const auto & item : mAmbientContextTypeList)
        {
            ReturnErrorOnFailure(encode.Encode(item.mInfo));
        }
        return CHIP_NO_ERROR;
    });
}

void AmbientContextSensingCluster::UpdateDetectionAttributes()
{
    bool bHA                = false;
    bool bOI                = false;
    bool bAUD               = false;
    bool bOC                = false;
    bool needToSendEvent[4] = {};

    for (const auto & item : mAmbientContextTypeList)
    {
        const auto & tags = item.mInfo.ambientContextSensed;
        for (size_t i = 0; i < tags.size(); i++)
        {
            auto sense = tags[i];
            switch (sense.namespaceID)
            {
            case kNamespaceIdentifiedHumanActivity:
                bHA = true;
                break;
            case kNamespaceIdentifiedObject:
                bOI = true;
                break;
            case kNamespaceIdentifiedSound:
                bAUD = true;
                break;
            }
        }

        if (bHA && bOI && bAUD)
        {
            break;
        }
    }
    bOC = mObjectCount >= mObjectCountConfig.objectCountThreshold;
    if (bHA != mHumanActivityDetected)
    {
        mHumanActivityDetected = bHA;
        NotifyAttributeChanged(Attributes::HumanActivityDetected::Id);
        needToSendEvent[0] = true;
    }
    if (bOI != mObjectIdentified)
    {
        mObjectIdentified = bOI;
        NotifyAttributeChanged(Attributes::ObjectIdentified::Id);
        needToSendEvent[1] = true;
    }
    if (bAUD != mAudioContextDetected)
    {
        mAudioContextDetected = bAUD;
        NotifyAttributeChanged(Attributes::AudioContextDetected::Id);
        needToSendEvent[2] = true;
    }
    if (bOC != mObjectCountReached)
    {
        mObjectCountReached = bOC;
        NotifyAttributeChanged(Attributes::ObjectCountReached::Id);
        needToSendEvent[3] = true;
    }
    if (mContext == nullptr)
    {
        return;
    }
    for (auto i = 0; i < 4; i++)
    {
        if (needToSendEvent[i] == false)
        {
            continue;
        }

        if (mEventNum[i] == chip::EventNumber(0))
        {
            Events::AmbientContextDetectStarted::Type event;

            if (i == 3)
            {
                // Send the start-event of ObjectCount
                event.objectCount.SetValue(mObjectCount);
                auto opt     = mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
                mEventNum[i] = opt.value_or(static_cast<chip::EventNumber>(0));
                continue;
            }
            // Send the start-event of others
            std::vector<AmbientContextSensingType> tmp;
            tmp.reserve(mAmbientContextTypeList.size());
            for (const auto & sensed : mAmbientContextTypeList)
            {
                tmp.push_back(sensed.mInfo);
            }
            event.ambientContextType.SetValue(
                DataModel::List<const AmbientContextSensingType>(tmp.data(), static_cast<uint16_t>(tmp.size())));
            auto opt     = mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
            mEventNum[i] = opt.value_or(static_cast<chip::EventNumber>(0));
        }
        else
        {
            // Sending End-Event
            Events::AmbientContextDetectEnded::Type event;
            event.startEventNumber = mEventNum[i];
            mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
            mEventNum[i] = chip::EventNumber(0);
        }
    }
}

// Find the next-timeout to remove the item in mAmbientContextTypeList
void AmbientContextSensingCluster::UpdateEventTimeout()
{
    if (mAmbientContextTypeList.empty() && mObjectCount == 0)
    {
        if (mHoldTimeDelegate->IsTimerActive(this))
        {
            mHoldTimeDelegate->CancelTimer(this);
        }
        return;
    }

    // Find the earliest time out event
    const System::Clock::Timestamp now = mHoldTimeDelegate->GetCurrentMonotonicTimestamp();
    System::Clock::Timeout remainingTime;
    System::Clock::Timestamp earliestDetectEndTime = FindEarliestEndTimestamp(mAmbientContextTypeList);

    if (earliestDetectEndTime <= now)
    {
        // Already pass the time to pop out the event
        remainingTime = System::Clock::Timeout::zero();
    }
    else
    {
        remainingTime = std::chrono::duration_cast<System::Clock::Timeout>(earliestDetectEndTime - now);
    }

    // Update the timer
    if (mHoldTimeDelegate->IsTimerActive(this))
    {
        mHoldTimeDelegate->CancelTimer(this);
    }
    RETURN_SAFELY_IGNORED mHoldTimeDelegate->StartTimer(this, remainingTime);
}

constexpr TagIdentifiedObject TagIdentifiedObjectIds[] = {
    TagIdentifiedObject::kUnknown, TagIdentifiedObject::kAdult, TagIdentifiedObject::kChild,   TagIdentifiedObject::kPerson,
    TagIdentifiedObject::kRVC,     TagIdentifiedObject::kPet,   TagIdentifiedObject::kDog,     TagIdentifiedObject::kCat,
    TagIdentifiedObject::kAnimal,  TagIdentifiedObject::kCar,   TagIdentifiedObject::kVehicle, TagIdentifiedObject::kPackage,
    TagIdentifiedObject::kClothes,
};

constexpr TagIdentifiedSound TagIdentifiedSoundIds[] = {
    TagIdentifiedSound::kUnknown,       TagIdentifiedSound::kObjectFall,     TagIdentifiedSound::kSnoring,
    TagIdentifiedSound::kCoughing,      TagIdentifiedSound::kBarking,        TagIdentifiedSound::kShattering,
    TagIdentifiedSound::kBabyCrying,    TagIdentifiedSound::kUtilityAlarm,   TagIdentifiedSound::kUrgentShouting,
    TagIdentifiedSound::kDoorbell,      TagIdentifiedSound::kKnocking,       TagIdentifiedSound::kUrgentSiren,
    TagIdentifiedSound::kFaucetRunning, TagIdentifiedSound::kKettleBoiling,  TagIdentifiedSound::kFanDryer,
    TagIdentifiedSound::kClapping,      TagIdentifiedSound::kFingerSnapping, TagIdentifiedSound::kMeowing,
    TagIdentifiedSound::kLaughing,      TagIdentifiedSound::kGlassBreaking,  TagIdentifiedSound::kDoorKnocking,
    TagIdentifiedSound::kPersonTalking,
};

constexpr TagIdentifiedHumanActivity TagIdentifiedHumanActivityIds[] = {
    TagIdentifiedHumanActivity::kUnknown,
    TagIdentifiedHumanActivity::kPresence,
    TagIdentifiedHumanActivity::kFall,
    TagIdentifiedHumanActivity::kSleeping,
    TagIdentifiedHumanActivity::kWalking,
    TagIdentifiedHumanActivity::kWorkout,
    TagIdentifiedHumanActivity::kSitting,
    TagIdentifiedHumanActivity::kStanding,
    TagIdentifiedHumanActivity::kDancing,
    TagIdentifiedHumanActivity::kPackageDelivery,
    TagIdentifiedHumanActivity::kPackageRetrieval,
};

template <typename T>
static uint8_t TagToIndex(const chip::Span<const T> & SpanId, T tag)
{
    uint8_t i = 0;
    for (const auto iTag : SpanId)
    {
        if (iTag == tag)
        {
            return i;
        }
        i++;
    }
    return 0;
}

/*
    Return error if
    1. The feature of the AmbientContextTypeSupported item is not set
    2. The duplicate ones
*/
CHIP_ERROR AmbientContextSensingCluster::CheckInputSupportedType(const std::vector<SemanticTagType> & ACTSupportedList)
{
    // Remove the type if the feature is disabled or duplicated
    bool pickType_HA[std::size(TagIdentifiedHumanActivityIds)] = {};
    bool pickType_OI[std::size(TagIdentifiedObjectIds)]        = {};
    bool pickType_AUD[std::size(TagIdentifiedSoundIds)]        = {};

    for (auto it = ACTSupportedList.begin(); it != ACTSupportedList.end(); it++)
    {
        const auto & act = *it;
        bool isFeatureEnabled =
            (act.namespaceID == kNamespaceIdentifiedHumanActivity && mFeatureMap.Has(Feature::kHumanActivity)) ||
            (act.namespaceID == kNamespaceIdentifiedObject && mFeatureMap.Has(Feature::kObjectIdentification)) ||
            (act.namespaceID == kNamespaceIdentifiedSound && mFeatureMap.Has(Feature::kSoundIdentification));
        uint8_t id;

        // Return error if the feature is not enabled
        VerifyOrReturnError(isFeatureEnabled, CHIP_ERROR_INVALID_ARGUMENT);

        // Check and remove the duplicated items.
        // If the item has been set to true => it has existed.
        bool isSet = false;
        switch (act.namespaceID)
        {
        case kNamespaceIdentifiedHumanActivity:
            id = TagToIndex<TagIdentifiedHumanActivity>(Span(TagIdentifiedHumanActivityIds),
                                                        static_cast<TagIdentifiedHumanActivity>(act.tag));
            isSet |= pickType_HA[id];
            pickType_HA[id] = true;
            break;
        case kNamespaceIdentifiedObject:
            id = TagToIndex<TagIdentifiedObject>(Span(TagIdentifiedObjectIds), static_cast<TagIdentifiedObject>(act.tag));
            isSet |= pickType_OI[id];
            pickType_OI[id] = true;
            break;
        case kNamespaceIdentifiedSound:
            id = TagToIndex<TagIdentifiedSound>(Span(TagIdentifiedSoundIds), static_cast<TagIdentifiedSound>(act.tag));
            isSet |= pickType_AUD[id];
            pickType_AUD[id] = true;
            break;
        default:
            // Unsupported namespace
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        VerifyOrReturnError(isSet == false, CHIP_ERROR_INVALID_ARGUMENT);
    }
    return CHIP_NO_ERROR;
}

bool AmbientContextSensingCluster::IsSupportedEvent(const AmbientContextSensingType & sensedEvent)
{
    const auto tags = sensedEvent.ambientContextSensed;
    for (size_t i = 0; i < tags.size(); i++)
    {
        bool isSupported = false;

        for (const auto & supported : mAmbientContextTypeSupportedList)
        {
            if ((tags[i].namespaceID == supported.namespaceID) && (tags[i].tag == supported.tag))
            {
                // The event is supported.
                isSupported = true;
                break;
            }
        }
        if (isSupported == false)
        {
            return false;
        }
    }
    // All tags are supported
    return true;
}

void AmbientContextSensingCluster::RemoveExpiredItems(std::list<AmbientContextSensed> & eventList,
                                                      const System::Clock::Timestamp & now)
{
    // Remove the ones which expires
    for (auto it = eventList.begin(); it != eventList.end();)
    {
        if (it->mEndTimestamp <= now)
        {
            auto item = *it;
            it        = eventList.erase(it);
        }
        else
        {
            ++it;
        }
    }
    if (mObjectCountEndTime <= now)
    {
        mObjectCountEndTime = System::Clock::Timestamp(0);
        mObjectCount        = 0;
    }
}

System::Clock::Timestamp AmbientContextSensingCluster::FindEarliestEndTimestamp(const std::list<AmbientContextSensed> & eventList)
{
    System::Clock::Timestamp earliestTimestamp = System::Clock::Timestamp(0);

    if (!eventList.empty())
    {
        const auto it = std::min_element(
            eventList.begin(), eventList.end(),
            [](const AmbientContextSensed & a, const AmbientContextSensed & b) { return a.mEndTimestamp < b.mEndTimestamp; });
        earliestTimestamp = it->mEndTimestamp;
    }
    if (mObjectCount > 0)
    {
        if (earliestTimestamp == System::Clock::Timestamp(0) || (mObjectCountEndTime < earliestTimestamp))
        {
            earliestTimestamp = mObjectCountEndTime;
        }
    }
    return earliestTimestamp;
}

CHIP_ERROR AmbientContextSensingCluster::CheckPredictedActivity(const std::vector<PredictedActivityType> & predictedActivityList)
{
    VerifyOrReturnError(mFeatureMap.Has(Feature::kPredictedActivity), CHIP_ERROR_INCORRECT_STATE);

    // Sanitize the input parameters
    uint32_t lastCheckTime = 0u;
    for (auto item : predictedActivityList)
    {
        // Make sure the start-time > the end-time of the previous PredictedActivityStruct
        VerifyOrReturnError(item.startTimestamp < item.endTimestamp, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(lastCheckTime < item.startTimestamp, CHIP_ERROR_INCORRECT_STATE);
        lastCheckTime = item.endTimestamp;
        // Check ambientContextType
        if (item.ambientContextType.HasValue())
        {
            auto acsTypeList = item.ambientContextType.Value();
            VerifyOrReturnError(acsTypeList.size() <= 100, CHIP_ERROR_INCORRECT_STATE);
            for (auto acsType : acsTypeList)
            {
                switch (acsType.namespaceID)
                {
                case kNamespaceIdentifiedObject:
                    VerifyOrReturnError(mFeatureMap.Has(Feature::kObjectIdentification), CHIP_ERROR_INCORRECT_STATE);
                    break;
                case kNamespaceIdentifiedSound:
                    VerifyOrReturnError(mFeatureMap.Has(Feature::kSoundIdentification), CHIP_ERROR_INCORRECT_STATE);
                    break;
                case kNamespaceIdentifiedHumanActivity:
                    VerifyOrReturnError(mFeatureMap.Has(Feature::kHumanActivity), CHIP_ERROR_INCORRECT_STATE);
                    break;
                default:
                    return CHIP_ERROR_INCORRECT_STATE;
                }
            }
        }

        if (mFeatureMap.Has(Feature::kObjectCounting))
        {
            // Check CrowdDetected
            VerifyOrReturnError(item.crowdDetected.HasValue(), CHIP_ERROR_INCORRECT_STATE);

            // Check CrowdCount
            if (item.crowdCount.HasValue())
            {
                uint8_t value = item.crowdCount.Value();
                VerifyOrReturnError(((1 <= value) && (value <= 254)), CHIP_ERROR_INCORRECT_STATE);
            }
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR AmbientContextSensingCluster::ReadPredictedActivity(AttributeValueEncoder & encoder)
{
    VerifyOrReturnValue(!mPredictedActivityList.empty(), encoder.EncodeEmptyList());

    return encoder.EncodeList([this](const auto & encode) -> CHIP_ERROR {
        for (const auto & item : mPredictedActivityList)
        {
            ReturnErrorOnFailure(encode.Encode(item.mInfo));
        }
        return CHIP_NO_ERROR;
    });
}

} // namespace chip::app::Clusters
