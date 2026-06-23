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
#include <cassert>
#include <chrono>
#include <clusters/AmbientContextSensing/Metadata.h>

namespace chip::app::Clusters {

using namespace AmbientContextSensing;
using namespace AmbientContextSensing::Attributes;

AmbientContextSensingCluster::AmbientContextSensingCluster(const Config & config) :
    DefaultServerCluster({ config.mEndpointId, AmbientContextSensing::Id }), mFeatureMap(config.mFeatureMap),
    mOptionalAttributeSet(config.mOptionalAttributeBits), mACSDelegate(config.mDelegate),
    mHoldTimeDelegate(config.mHoldTimeDelegate)
{
    assert(mFeatureMap.Has(Feature::kHumanActivity) || mFeatureMap.Has(Feature::kObjectIdentification) ||
           mFeatureMap.Has(Feature::kSoundIdentification) || mFeatureMap.Has(Feature::kObjectCounting));
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
        if ((SetHoldTime(storedHoldTime) == Protocols::InteractionModel::Status::ConstraintError) && (mContext != nullptr))
        {
            // A value was found in persistence and if stored value is not valid, replace it
            LogErrorOnFailure(
                mContext->attributeStorage.WriteValue({ mPath.mEndpointId, AmbientContextSensing::Id, Attributes::HoldTime::Id },
                                                      { reinterpret_cast<const uint8_t *>(&mHoldTime), sizeof(mHoldTime) }));
        }
    }

    return CHIP_NO_ERROR;
}

void AmbientContextSensingCluster::Shutdown(ClusterShutdownType shutdownType)
{
    mAmbientContextTypeSupportedList = {};
    mAmbientContextTypeList.Clear();
    mAmbientContextTypeListSize = 0;
    mHoldTimeDelegate.CancelTimer(this);
    DefaultServerCluster::Shutdown(shutdownType);
}

DataModel::ActionReturnStatus AmbientContextSensingCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                          AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case HumanActivityDetected::Id:
        return encoder.Encode(GetHumanActivityDetected());
    case ObjectIdentified::Id:
        return encoder.Encode(GetObjectIdentified());
    case AudioContextDetected::Id:
        return encoder.Encode(GetAudioContextDetected());
    case AmbientContextType::Id:
        return ReadAmbientContextType(encoder);
    case AmbientContextTypeSupported::Id:
        return ReadAmbientContextTypeSupported(encoder);
    case ObjectCountThresholdReached::Id:
        return encoder.Encode(GetObjectCountThresholdReached());
    case ObjectCountConfig::Id:
        return encoder.Encode(GetObjectCountConfig());
    case ObjectCount::Id:
        return encoder.Encode(GetObjectCount());
    case SimultaneousDetectionLimit::Id:
        return encoder.Encode(GetSimultaneousDetectionLimit());
    case HoldTime::Id:
        return encoder.Encode(GetHoldTime());
    case HoldTimeLimits::Id:
        return encoder.Encode(GetHoldTimeLimits());
    case PredictedActivity::Id:
        return ReadPredictedActivity(encoder);
    case FeatureMap::Id:
        return encoder.Encode(GetFeatures());
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
    case HoldTime::Id: {
        uint16_t newHoldTime;
        ReturnErrorOnFailure(decoder.Decode(newHoldTime));
        VerifyOrReturnError((newHoldTime != mHoldTime), Protocols::InteractionModel::Status::Success);
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
          Attributes::ObjectCountThresholdReached::kMetadataEntry },
        { mFeatureMap.Has(Feature::kObjectCounting) && mFeatureMap.Has(Feature::kObjectIdentification),
          Attributes::ObjectCountConfig::kMetadataEntry },
        { mOptionalAttributeSet.IsSet(ObjectCount::Id) &&
              (mFeatureMap.Has(Feature::kObjectCounting) && mFeatureMap.Has(Feature::kObjectIdentification)),
          Attributes::ObjectCount::kMetadataEntry },
        { mFeatureMap.Has(Feature::kPredictedActivity), Attributes::PredictedActivity::kMetadataEntry },
        { mFeatureMap.Has(Feature::kSensorFusion), Attributes::SensorFusionSupported::kMetadataEntry },
    };

    return listBuilder.Append(Span(AmbientContextSensing::Attributes::kMandatoryMetadata), Span(optionalAttributes));
}

CHIP_ERROR AmbientContextSensingCluster::SetAmbientContextTypeSupported(const Span<SemanticTagType> & ACTypeList)
{
    ReturnErrorOnFailure(CheckInputSupportedType(ACTypeList));
    size_t acTypeListSize = ACTypeList.size();
    VerifyOrReturnError((0 < acTypeListSize) && (acTypeListSize <= kMaxACTypeSupported), CHIP_ERROR_INVALID_ARGUMENT);

    auto * ambientContextTypeSupportedBuf = mACSDelegate.GetAmbientContextTypeSupportedBuf(acTypeListSize);
    VerifyOrReturnError(ambientContextTypeSupportedBuf != nullptr, CHIP_ERROR_INCORRECT_STATE);
    std::copy(ACTypeList.begin(), ACTypeList.end(), ambientContextTypeSupportedBuf);
    mAmbientContextTypeSupportedList = Span<SemanticTagType>(ambientContextTypeSupportedBuf, ACTypeList.size());
    NotifyAttributeChanged(Attributes::AmbientContextTypeSupported::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR AmbientContextSensingCluster::AddDetection(const AmbientContextSensingType & sensedEvent)
{
    size_t acsSize = sensedEvent.ambientContextSensed.size();
    VerifyOrReturnError((0 < acsSize) && (acsSize <= kMaxACSensed), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSupportedEvent(sensedEvent), CHIP_ERROR_INCORRECT_STATE);

    // If there have already been mSimultaneousDetectionLimit items in mAmbientContextTypeList => remove the oldest ones
    AmbientContextSensed * item;
    if (mAmbientContextTypeListSize >= mSimultaneousDetectionLimit)
    {
        // One more space is required
        uint8_t itemsToRemove = static_cast<uint8_t>(mAmbientContextTypeListSize - mSimultaneousDetectionLimit + 1);

        for (auto i = 0; i < itemsToRemove; i++)
        {
            auto iter = mAmbientContextTypeList.end();
            --iter;
            item = &*iter;
            mAmbientContextTypeList.Remove(item);
            mAmbientContextTypeListSize--;
            LogErrorOnFailure(mACSDelegate.DelDetection(item->id));
        }

        // The detected status may be different
        UpdateDetectionAttributes();
        // The earliest end-time item may have been removed
        UpdateEventTimeout();
    }

    bool fromExisting = false;
    for (auto it = mAmbientContextTypeList.begin(); it != mAmbientContextTypeList.end(); ++it)
    {
        if (CompareAmbientContextSensed((*it).mInfo, sensedEvent))
        {
            // Same detection occurs.
            // Need to reorder the event and hold-time
            item = &*it;
            mAmbientContextTypeList.Remove(item);
            mAmbientContextTypeListSize--;
            fromExisting = true;
            break;
        }
    }
    System::Clock::Timestamp now = mHoldTimeDelegate.GetCurrentMonotonicTimestamp();
    System::Clock::Seconds16 newHoldTime;
    if (!fromExisting)
    {
        // The new detection event
        DetectFuncResult res = mACSDelegate.FindAndUseAvailableDetection();
        ReturnErrorOnFailure(res.res);

        item                = mACSDelegate.GetAllocedDetection(res.id);
        item->id            = res.id;
        const auto & tags   = sensedEvent.ambientContextSensed;
        const auto tagCount = tags.size();
        for (size_t t = 0; t < tagCount; t++)
        {
            item->mOwnedTags[t] = tags[t];
        }
        item->mInfo                      = sensedEvent;
        item->mInfo.ambientContextSensed = chip::app::DataModel::List<const SemanticTagType>(item->mOwnedTags, tagCount);
        item->mStartTimestamp            = now;
        item->mStartEpoch                = mACSDelegate.GetEpochNow();
        newHoldTime                      = System::Clock::Seconds16(mHoldTime);
    }
    else
    {
        // The event which is detected and whose hold-time has not elapsed
        const System::Clock::Seconds16 elapsedSec =
            std::chrono::duration_cast<System::Clock::Seconds16>(now - item->mStartTimestamp);

        System::Clock::Seconds16 holdTimeMaxSec = System::Clock::Seconds16(mHoldTimeLimits.holdTimeMax);
        if (elapsedSec + System::Clock::Seconds16(mHoldTime) <= holdTimeMaxSec)
        {
            newHoldTime = System::Clock::Seconds16(mHoldTime);
        }
        else
        {
            newHoldTime = (elapsedSec < holdTimeMaxSec) ? (holdTimeMaxSec - elapsedSec) : System::Clock::Seconds16(0);
        }
    }
    item->mEndTimestamp = item->mStartTimestamp + newHoldTime;
    if (!fromExisting)
    {
        SendDetectStartEvent(*item);
    }
    mAmbientContextTypeList.PushFront(item);
    mAmbientContextTypeListSize++;

    UpdateDetectionAttributes();

    // Update the timer if required.
    // Note: If the new detection has existed, the newHoldTime may be small that it may expire soon
    UpdateEventTimeout();

    NotifyAttributeChanged(Attributes::AmbientContextType::Id);

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
        bool inList             = false;
        auto & ACSSupportedList = mAmbientContextTypeSupportedList;
        for (const auto & item : ACSSupportedList)
        {
            if ((item.tag == newObjectCountConfig.countingObject.tag) &&
                (item.namespaceID == newObjectCountConfig.countingObject.namespaceID))
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
            LogErrorOnFailure(mContext->attributeStorage.WriteValue(
                { mPath.mEndpointId, AmbientContextSensing::Id, Attributes::ObjectCountConfig::Id },
                { reinterpret_cast<const uint8_t *>(&mObjectCountConfig.objectCountThreshold),
                  sizeof(mObjectCountConfig.objectCountThreshold) }));
        }
    }
    return Protocols::InteractionModel::Status::Success;
}

CHIP_ERROR AmbientContextSensingCluster::SetObjectCount(uint16_t objectCount)
{
    VerifyOrReturnError((mFeatureMap.Has(Feature::kObjectCounting) && mFeatureMap.Has(Feature::kObjectIdentification)),
                        CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(objectCount >= 1, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnValue(SetAttributeValue(mObjectCount, objectCount, Attributes::ObjectCount::Id), CHIP_NO_ERROR);
    mObjectCountStartTime  = mHoldTimeDelegate.GetCurrentMonotonicTimestamp();
    mObjectCountEndTime    = mObjectCountStartTime + System::Clock::Seconds16(mHoldTime);
    mObjectCountStartEpoch = mACSDelegate.GetEpochNow();
    UpdateDetectionAttributes();
    UpdateEventTimeout();
    SendDetectStartEvent(mObjectCountThresholdReached, mObjectCount);

    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus AmbientContextSensingCluster::SetSimultaneousDetectionLimit(const uint8_t simultaneousDetectionLimit)
{
    VerifyOrReturnError((simultaneousDetectionLimit <= kMaxSimultaneousDetectionLimit),
                        Protocols::InteractionModel::Status::ConstraintError);
    VerifyOrReturnValue(
        SetAttributeValue(mSimultaneousDetectionLimit, simultaneousDetectionLimit, Attributes::SimultaneousDetectionLimit::Id),
        DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp);

    if (mAmbientContextTypeListSize <= mSimultaneousDetectionLimit)
    {
        return Protocols::InteractionModel::Status::Success;
    }
    // Resize the list of AmbientContextType list if the updated limitation becomes smaller than it's current length
    uint8_t itemsToRemove = static_cast<uint8_t>(mAmbientContextTypeListSize - mSimultaneousDetectionLimit);
    for (auto i = 0; i < itemsToRemove; i++)
    {
        auto iter = mAmbientContextTypeList.end();
        --iter;
        AmbientContextSensed * item = &*iter;
        mAmbientContextTypeList.Remove(item);
        mAmbientContextTypeListSize--;
        LogErrorOnFailure(mACSDelegate.DelDetection(item->id));
    }

    // The detected status may be different
    UpdateDetectionAttributes();
    // The earliest end-time item may have been removed
    UpdateEventTimeout();

    return Protocols::InteractionModel::Status::Success;
}

DataModel::ActionReturnStatus AmbientContextSensingCluster::SetHoldTime(uint16_t holdTime)
{
    VerifyOrReturnError((mHoldTimeLimits.holdTimeMin <= holdTime) && (holdTime <= mHoldTimeLimits.holdTimeMax),
                        Protocols::InteractionModel::Status::ConstraintError);
    VerifyOrReturnValue(SetAttributeValue(mHoldTime, holdTime, Attributes::HoldTime::Id),
                        DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp);

    // Save the value to persistence
    if (mContext != nullptr)
    {
        LogErrorOnFailure(
            mContext->attributeStorage.WriteValue({ mPath.mEndpointId, AmbientContextSensing::Id, Attributes::HoldTime::Id },
                                                  { reinterpret_cast<const uint8_t *>(&mHoldTime), sizeof(mHoldTime) }));
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
        ChipLogProgress(Zcl, "HoldTime is out of the new bounds. Resetting to the new default value");
        // mHoldTimeLimits.holdTimeDefault was verified at the beginning of the function
        RETURN_SAFELY_IGNORED SetHoldTime(mHoldTimeLimits.holdTimeDefault);
    }
}

CHIP_ERROR AmbientContextSensingCluster::SetPredictedActivity(const Span<PredictedActivityType> & predictedActivityList)
{
    VerifyOrReturnError(predictedActivityList.size() <= kMaxPredictedActivity, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(CheckPredictedActivity(predictedActivityList));

    ReturnErrorOnFailure(mACSDelegate.SetPredictedActivity(predictedActivityList));
    NotifyAttributeChanged(Attributes::PredictedActivity::Id);
    return CHIP_NO_ERROR;
}

void AmbientContextSensingCluster::TimerFired()
{
    VerifyOrReturn((mAmbientContextTypeListSize != 0) || (mObjectCount != 0));
    System::Clock::Timestamp now = mHoldTimeDelegate.GetCurrentMonotonicTimestamp();
    RemoveExpiredItems(mAmbientContextTypeList, mAmbientContextTypeListSize, now);

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

CHIP_ERROR AmbientContextSensingCluster::ReadAmbientContextTypeSupported(AttributeValueEncoder & encoder)
{
    auto & ACSSupportedList = mAmbientContextTypeSupportedList;
    VerifyOrReturnValue(!ACSSupportedList.empty(), encoder.EncodeEmptyList());

    return encoder.EncodeList([&ACSSupportedList](const auto & encode) -> CHIP_ERROR {
        for (const auto & item : ACSSupportedList)
        {
            ReturnErrorOnFailure(encode.Encode(item));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR AmbientContextSensingCluster::ReadAmbientContextType(AttributeValueEncoder & encoder)
{
    // If the supported_list is empty => No detection can be added
    auto & ACSSupportedList = mAmbientContextTypeSupportedList;
    VerifyOrReturnValue(!ACSSupportedList.empty(), encoder.EncodeEmptyList());

    return encoder.EncodeList([this](const auto & encode) -> CHIP_ERROR {
        for (const auto & item : mAmbientContextTypeList)
        {
            ReturnErrorOnFailure(encode.Encode(item.mInfo));
        }
        return CHIP_NO_ERROR;
    });
}

void AmbientContextSensingCluster::SendDetectStartEvent(const AmbientContextSensed & ACSItem)
{
    VerifyOrReturn(mContext != nullptr);
    Events::AmbientContextDetectStarted::Type event;
    event.ambientContextDetected.SetValue(ACSItem.mInfo);
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

void AmbientContextSensingCluster::SendDetectStartEvent(const bool objectCountReached, const uint16_t objectCount)
{
    VerifyOrReturn(mContext != nullptr);
    VerifyOrReturn(mFeatureMap.Has(Feature::kObjectCounting) && mFeatureMap.Has(Feature::kObjectIdentification));
    // Constraint of ObjectCount in AmbientContextDetectedStarted Event
    VerifyOrReturn(objectCount >= 1);

    Events::AmbientContextDetectStarted::Type event;
    SemanticTagType tag = { .namespaceID = mObjectCountConfig.countingObject.namespaceID,
                            .tag         = mObjectCountConfig.countingObject.tag };
    chip::app::DataModel::List<const SemanticTagType> tagList(&tag, 1);
    AmbientContextSensingType countACS = { .ambientContextSensed = tagList };
    event.ambientContextDetected.SetValue(countACS);
    event.objectCountThresholdReached.SetValue(objectCountReached);
    event.objectCount.SetValue(objectCount);
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

void AmbientContextSensingCluster::SendDetectEndEvent(const uint64_t eventStartTimePos, const uint64_t eventStartTimeSys)
{
    VerifyOrReturn(mContext != nullptr);
    Events::AmbientContextDetectEnded::Type event;
    event.eventStartTimePos.SetValue(eventStartTimePos);
    event.eventStartTimeSys.SetValue(eventStartTimeSys);
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

/*
    Update attribute:
    - HumanActivityDetected
    - ObjectIdentified
    - AudioContextDetected
    - ObjectCountThresholdReached
*/
void AmbientContextSensingCluster::UpdateDetectionAttributes()
{
    // We need to detect HumanActivity, ObjectIdentified, Audio and ObjectCountThresholdReached.
    // => Reuse the existing definition of feature to avoid duplicate logic
    BitFlags<Feature> bDetect(0);

    // Check if a detetion is found in AmbientContextType list
    for (const auto & item : mAmbientContextTypeList)
    {
        const auto & tags = item.mInfo.ambientContextSensed;
        for (size_t i = 0; i < tags.size(); i++)
        {
            const auto sense = tags[i];
            switch (sense.namespaceID)
            {
            case kNamespaceIdentifiedHumanActivity:
                bDetect.Set(Feature::kHumanActivity);
                break;
            case kNamespaceIdentifiedObject:
                bDetect.Set(Feature::kObjectIdentification);
                break;
            case kNamespaceIdentifiedSound:
                bDetect.Set(Feature::kSoundIdentification);
                break;
            }
        }

        if (bDetect.Has(Feature::kHumanActivity) && bDetect.Has(Feature::kObjectIdentification) &&
            bDetect.Has(Feature::kSoundIdentification))
        {
            // All types have been found in mAmbientContextTypeList => No further checking is required
            break;
        }
    }

    // Check if ObjectCountThresholdReached should be set or not
    if (mObjectCount >= mObjectCountConfig.objectCountThreshold)
    {
        bDetect.Set(Feature::kObjectCounting);
    }
    // If the status is different, set the attribute & notify
    SetAttributeValue(mHumanActivityDetected, bDetect.Has(Feature::kHumanActivity), Attributes::HumanActivityDetected::Id);
    SetAttributeValue(mObjectIdentified, bDetect.Has(Feature::kObjectIdentification), Attributes::ObjectIdentified::Id);
    SetAttributeValue(mAudioContextDetected, bDetect.Has(Feature::kSoundIdentification), Attributes::AudioContextDetected::Id);
    SetAttributeValue(mObjectCountThresholdReached, bDetect.Has(Feature::kObjectCounting),
                      Attributes::ObjectCountThresholdReached::Id);
}

// Find the next-timeout to remove the item in mAmbientContextTypeList
void AmbientContextSensingCluster::UpdateEventTimeout()
{
    if (mAmbientContextTypeListSize == 0 && mObjectCount == 0)
    {
        if (mHoldTimeDelegate.IsTimerActive(this))
        {
            mHoldTimeDelegate.CancelTimer(this);
        }
        return;
    }

    // Find the earliest time out event
    const System::Clock::Timestamp now = mHoldTimeDelegate.GetCurrentMonotonicTimestamp();
    System::Clock::Timeout remainingTime;
    System::Clock::Timestamp earliestDetectEndTime = FindEarliestEndTimestamp();

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
    if (mHoldTimeDelegate.IsTimerActive(this))
    {
        mHoldTimeDelegate.CancelTimer(this);
    }
    LogErrorOnFailure(mHoldTimeDelegate.StartTimer(this, remainingTime));
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
    TagIdentifiedHumanActivity::kUnknown,         TagIdentifiedHumanActivity::kFall,
    TagIdentifiedHumanActivity::kSleeping,        TagIdentifiedHumanActivity::kWalking,
    TagIdentifiedHumanActivity::kWorkout,         TagIdentifiedHumanActivity::kSitting,
    TagIdentifiedHumanActivity::kStanding,        TagIdentifiedHumanActivity::kDancing,
    TagIdentifiedHumanActivity::kPackageDelivery, TagIdentifiedHumanActivity::kPackageRetrieval,
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
    // 0 is the index of kUnknown for all the used tags. Return that if not found.
    return 0;
}

/*
    Return error if
    1. The feature of the AmbientContextTypeSupported item is not set
    2. A SemanticTagType is duplicated
    3. The namespaceId is not known
*/
CHIP_ERROR AmbientContextSensingCluster::CheckInputSupportedType(const Span<SemanticTagType> & ACTSupportedList)
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
    const auto & tags          = sensedEvent.ambientContextSensed;
    const auto & supportedList = mAmbientContextTypeSupportedList;

    return std::all_of(tags.begin(), tags.end(), [&supportedList](const auto & tag) {
        return std::any_of(supportedList.begin(), supportedList.end(), [&tag](const auto & supported) {
            return tag.namespaceID == supported.namespaceID && tag.tag == supported.tag;
        });
    });
}

void AmbientContextSensingCluster::RemoveExpiredItems(IntrusiveList<AmbientContextSensed> & eventList, uint8_t & listSize,
                                                      const System::Clock::Timestamp & now)
{
    // Remove the ones which expires
    AmbientContextSensed * pitem;

    for (auto it = eventList.begin(); it != eventList.end();)
    {
        pitem = &*it;
        ++it;
        if (pitem->mEndTimestamp <= now)
        {
            eventList.Remove(pitem);
            listSize--;
            SendDetectEndEvent(pitem->mStartEpoch, pitem->mStartTimestamp.count());
            LogErrorOnFailure(mACSDelegate.DelDetection(pitem->id));
            NotifyAttributeChanged(Attributes::AmbientContextType::Id);
        }
    }
    if ((mObjectCount > 0) && (mObjectCountEndTime <= now))
    {
        mObjectCountEndTime = System::Clock::Timestamp(0);
        SetAttributeValue(mObjectCount, uint16_t{ 0 }, Attributes::ObjectCount::Id);
        // Send the DetectEndEvent
        SendDetectEndEvent(mObjectCountStartEpoch, mObjectCountStartTime.count());
        mObjectCountStartTime = System::Clock::Timestamp(0);
    }
}

System::Clock::Timestamp AmbientContextSensingCluster::FindEarliestEndTimestamp()
{
    System::Clock::Timestamp earliestTimestamp      = System::Clock::Timestamp(0);
    IntrusiveList<AmbientContextSensed> & eventList = mAmbientContextTypeList;

    if (mAmbientContextTypeListSize > 0)
    {
        auto it = std::min_element(
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

CHIP_ERROR AmbientContextSensingCluster::CheckPredictedActivity(const Span<PredictedActivityType> & predictedActivityList)
{
    VerifyOrReturnError(mFeatureMap.Has(Feature::kPredictedActivity), CHIP_ERROR_INCORRECT_STATE);

    // Sanitize the input parameters
    uint32_t lastCheckTime = 0u;
    for (auto item : predictedActivityList)
    {
        // Make sure the start-time > the end-time of the previous PredictedActivityStruct
        VerifyOrReturnError(item.startTimestamp < item.endTimestamp, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(lastCheckTime < item.startTimestamp, CHIP_ERROR_INVALID_ARGUMENT);
        lastCheckTime = item.endTimestamp;
        // Check ambientContextType
        if (item.ambientContextType.HasValue())
        {
            auto & acsTypeList = item.ambientContextType.Value();
            VerifyOrReturnError(acsTypeList.size() <= kMaxPredictedACType, CHIP_ERROR_INVALID_ARGUMENT);
            for (const auto & acsType : acsTypeList)
            {
                switch (acsType.namespaceID)
                {
                case kNamespaceIdentifiedObject:
                    VerifyOrReturnError(mFeatureMap.Has(Feature::kObjectIdentification), CHIP_ERROR_INVALID_ARGUMENT);
                    break;
                case kNamespaceIdentifiedSound:
                    VerifyOrReturnError(mFeatureMap.Has(Feature::kSoundIdentification), CHIP_ERROR_INVALID_ARGUMENT);
                    break;
                case kNamespaceIdentifiedHumanActivity:
                    VerifyOrReturnError(mFeatureMap.Has(Feature::kHumanActivity), CHIP_ERROR_INVALID_ARGUMENT);
                    break;
                default:
                    return CHIP_ERROR_INVALID_ARGUMENT;
                }
            }
        }

        if (mFeatureMap.Has(Feature::kObjectCounting))
        {
            // Check CrowdCount
            if (item.crowdCount.HasValue())
            {
                uint8_t value = item.crowdCount.Value();
                VerifyOrReturnError(((1 <= value) && (value <= 254)), CHIP_ERROR_INVALID_ARGUMENT);
            }
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR AmbientContextSensingCluster::ReadPredictedActivity(AttributeValueEncoder & encoder)
{
    return encoder.EncodeList([this](const auto & encode) -> CHIP_ERROR {
        auto predictedActivityList = mACSDelegate.GetPredictedActivity();
        for (const auto & item : predictedActivityList)
        {
            ReturnErrorOnFailure(encode.Encode(item.mInfo));
        }
        return CHIP_NO_ERROR;
    });
}

} // namespace chip::app::Clusters
