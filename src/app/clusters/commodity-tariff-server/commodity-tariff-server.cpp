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

#include "commodity-tariff-server.h"

#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <cstdint>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CommodityTariff;
using namespace chip::app::Clusters::CommodityTariff::Structs;
using namespace chip::app::Clusters::CommodityTariff::Attributes;
using namespace CommodityTariffConsts;

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace CommodityTariff {

CHIP_ERROR Instance::Init()
{
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));
    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INCORRECT_STATE);

    return CHIP_NO_ERROR;
}

void Instance::Shutdown()
{
    CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

bool Instance::HasFeature(Feature aFeature) const
{
    return mFeature.Has(aFeature);
}

// AttributeAccessInterface
CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    // Simple attributes (non-list)
    case TariffInfo::Id:
        return aEncoder.Encode(mDelegate.GetTariffInfo());
    case TariffUnit::Id:
        return aEncoder.Encode(mDelegate.GetTariffUnit());
    case StartDate::Id:
        return aEncoder.Encode(mDelegate.GetStartDate());
    case CurrentDay::Id:
        return aEncoder.Encode(GetCurrentDay());
    case NextDay::Id:
        return aEncoder.Encode(GetNextDay());
    case CurrentDayEntry::Id:
        return aEncoder.Encode(GetCurrentDayEntry());
    case CurrentDayEntryDate::Id:
        return aEncoder.Encode(GetCurrentDayEntryDate());
    case NextDayEntry::Id:
        return aEncoder.Encode(GetNextDayEntry());
    case NextDayEntryDate::Id:
        return aEncoder.Encode(GetNextDayEntryDate());
    case DefaultRandomizationOffset::Id:
        if (!HasFeature(Feature::kRandomization))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        return aEncoder.Encode(mDelegate.GetDefaultRandomizationOffset());
    case DefaultRandomizationType::Id:
        if (!HasFeature(Feature::kRandomization))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        return aEncoder.Encode(mDelegate.GetDefaultRandomizationType());
    case FeatureMap::Id:
        return aEncoder.Encode(mFeature);

    // List attributes (using proper chunking)
    case DayEntries::Id: {
        const auto & entries = mDelegate.GetDayEntries();
        if (entries.IsNull())
        {
            return aEncoder.EncodeNull();
        }
        return aEncoder.EncodeList([&entries](const auto & encoder) {
            for (const auto & entry : entries.Value())
            {
                ReturnErrorOnFailure(encoder.Encode(entry));
            }
            return CHIP_NO_ERROR;
        });
    }
    case DayPatterns::Id: {
        const auto & patterns = mDelegate.GetDayPatterns();
        if (patterns.IsNull())
        {
            return aEncoder.EncodeNull();
        }
        return aEncoder.EncodeList([&patterns](const auto & encoder) {
            for (const auto & pattern : patterns.Value())
            {
                ReturnErrorOnFailure(encoder.Encode(pattern));
            }
            return CHIP_NO_ERROR;
        });
    }
    case CalendarPeriods::Id: {
        const auto & periods = mDelegate.GetCalendarPeriods();
        if (periods.IsNull())
        {
            return aEncoder.EncodeNull();
        }
        return aEncoder.EncodeList([&periods](const auto & encoder) {
            for (const auto & period : periods.Value())
            {
                ReturnErrorOnFailure(encoder.Encode(period));
            }
            return CHIP_NO_ERROR;
        });
    }
    case IndividualDays::Id: {
        const auto & days = mDelegate.GetIndividualDays();
        if (days.IsNull())
        {
            return aEncoder.EncodeNull();
        }
        return aEncoder.EncodeList([&days](const auto & encoder) {
            for (const auto & day : days.Value())
            {
                ReturnErrorOnFailure(encoder.Encode(day));
            }
            return CHIP_NO_ERROR;
        });
    }
    case TariffComponents::Id: {
        const auto & components = mDelegate.GetTariffComponents();
        if (components.IsNull())
        {
            return aEncoder.EncodeNull();
        }
        return aEncoder.EncodeList([&components](const auto & encoder) {
            for (const auto & component : components.Value())
            {
                ReturnErrorOnFailure(encoder.Encode(component));
            }
            return CHIP_NO_ERROR;
        });
    }
    case TariffPeriods::Id: {
        const auto & periods = mDelegate.GetTariffPeriods();
        if (periods.IsNull())
        {
            return aEncoder.EncodeNull();
        }
        return aEncoder.EncodeList([&periods](const auto & encoder) {
            for (const auto & period : periods.Value())
            {
                ReturnErrorOnFailure(encoder.Encode(period));
            }
            return CHIP_NO_ERROR;
        });
    }
    case CurrentTariffComponents::Id: {
        const auto & components = GetCurrentTariffComponents();
        if (components.IsNull())
        {
            return aEncoder.EncodeNull();
        }
        return aEncoder.EncodeList([&components](const auto & encoder) {
            for (const auto & component : components.Value())
            {
                ReturnErrorOnFailure(encoder.Encode(component));
            }
            return CHIP_NO_ERROR;
        });
    }
    case NextTariffComponents::Id: {
        const auto & components = GetNextTariffComponents();
        if (components.IsNull())
        {
            return aEncoder.EncodeNull();
        }
        return aEncoder.EncodeList([&components](const auto & encoder) {
            for (const auto & component : components.Value())
            {
                ReturnErrorOnFailure(encoder.Encode(component));
            }
            return CHIP_NO_ERROR;
        });
    }
    }

    return CHIP_NO_ERROR;
}

template <typename T>
CHIP_ERROR Instance::SetValue(T & currValue, T & newValue, uint32_t attrId)
{
    bool hasChanged = false;

    if (currValue.IsNull() || newValue.IsNull())
    {
        hasChanged = currValue.IsNull() != newValue.IsNull();
    }
    else
    {
        hasChanged = (currValue.Value() != newValue.Value());
    }

    if (hasChanged)
    {
        currValue = newValue;
        AttributeUpdCb(attrId);
    }

    return CHIP_NO_ERROR;
}

void Instance::InvokeCommand(HandlerContext & handlerContext)
{
    using namespace Commands;

    switch (handlerContext.mRequestPath.mCommandId)
    {
    case GetTariffComponent::Id:
        HandleCommand<GetTariffComponent::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleGetTariffComponent(ctx, commandData); });
        return;
    case GetDayEntry::Id:
        HandleCommand<GetDayEntry::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleGetDayEntry(ctx, commandData); });
        return;
    }
}

void Instance::ResetCurrentAttributes()
{
    mCurrentDay.SetNull();
    mNextDay.SetNull();
    mCurrentDayEntry.SetNull();
    mNextDayEntry.SetNull();
    mCurrentDayEntryDate.SetNull();
    mNextDayEntryDate.SetNull();

    mCurrentTariffComponents_MgmtObj.Cleanup();
    mNextTariffComponents_MgmtObj.Cleanup();
}

void Instance::TariffDataUpdatedCb(bool is_erased, const AttributeId * aUpdatedAttrIds, size_t aCount)
{
    // Process each updated attribute
    for (size_t i = 0; i < aCount; i++)
    {
        AttributeUpdCb(aUpdatedAttrIds[i]);
    }

    DeinitCurrentAttrs();

    // Check if essential tariff data is available
    if (mDelegate.GetTariffUnit().IsNull() || mDelegate.GetStartDate().IsNull() || mDelegate.GetTariffInfo().IsNull() ||
        mDelegate.GetDayEntries().IsNull() || mDelegate.GetTariffPeriods().IsNull() || mDelegate.GetTariffComponents().IsNull())
    {
        ChipLogError(AppServer, "Seems the new tariff is unavailable - skip the current/next attrs init");
        return;
    }

    if (!is_erased)
    {
        InitCurrentAttrs();
    }
}

namespace Utils {

using CurrentTariffAttrsCtx = CommodityTariff::Instance::CurrentTariffAttrsCtx;

static constexpr uint32_t kNotFoundPattern = UINT32_MAX;

template <typename T>
uint32_t GetIdFromEntry(const T & aEntry)
{
    return kNotFoundPattern;
}

template <>
uint32_t GetIdFromEntry<Structs::DayEntryStruct::Type>(const Structs::DayEntryStruct::Type & aEntry)
{
    return aEntry.dayEntryID;
}

template <>
uint32_t GetIdFromEntry<Structs::DayPatternStruct::Type>(const Structs::DayPatternStruct::Type & aEntry)
{
    return aEntry.dayPatternID;
}

template <>
uint32_t GetIdFromEntry<Structs::TariffComponentStruct::Type>(const Structs::TariffComponentStruct::Type & aEntry)
{
    return aEntry.tariffComponentID;
}

template <typename T>
const T * GetListEntryById(const DataModel::List<const T> & aList, uint32_t aId)
{
    for (size_t i = 0; i < aList.size(); ++i)
    {
        const T & item  = aList[i];
        uint32_t itemId = GetIdFromEntry(item);

        if (itemId == aId) // Replace 'id' with the actual field name
        {
            return &item;
        }
        // If list is sorted by ID and we've passed the target, break early
        if (itemId > aId)
        {
            break;
        }
    }
    return nullptr;
}

DayPatternDayOfWeekBitmap GetDayOfWeek(uint32_t timestamp)
{
    time_t time = static_cast<time_t>(timestamp);
    struct tm utcTimeStruct;
    struct tm * utcTime = gmtime_r(&time, &utcTimeStruct);
    return static_cast<DayPatternDayOfWeekBitmap>(1 << utcTime->tm_wday);
}

Structs::DayStruct::Type FindDay(CurrentTariffAttrsCtx & aCtx, uint32_t timestamp)
{
    Structs::DayStruct::Type defaultDay = { .date        = 0,
                                            .dayType     = DayTypeEnum::kUnknownEnumValue,
                                            .dayEntryIDs = DataModel::List<const uint32_t>() };
    uint32_t DayStartTS                 = timestamp - (timestamp % kSecondsPerDay);

    // First check IndividualDays
    if (!aCtx.mTariffProvider->GetIndividualDays().IsNull())
    {
        for (const auto & day : aCtx.mTariffProvider->GetIndividualDays().Value())
        {
            if ((day.date >= DayStartTS) && (day.date < (DayStartTS + kSecondsPerDay)))
            {
                return day;
            }
        }
    }

    if (!aCtx.mTariffProvider->GetCalendarPeriods().IsNull())
    {
        const Structs::CalendarPeriodStruct::Type * period = nullptr;
        bool first_item                                    = true;

        for (const auto & entry : aCtx.mTariffProvider->GetCalendarPeriods().Value())
        {
            if (first_item && aCtx.mTariffProvider->GetStartDate().Value() == 0 &&
                (entry.startDate.IsNull() || entry.startDate.Value() == 0))
            {
                period     = &entry; // Use this entry as fallback
                first_item = false;
                continue; // Keep looking for a better match
            }

            // Check if entry matches our target day
            const auto entryStart = entry.startDate.Value();
            if (entryStart >= DayStartTS && entryStart < (DayStartTS + kSecondsPerDay))
            {
                period = &entry; // Found perfect match
                break;           // No need to continue searching
            }
        }

        if (period != nullptr)
        {
            for (const auto & patternID : period->dayPatternIDs)
            {
                auto * pattern =
                    GetListEntryById<Structs::DayPatternStruct::Type>(aCtx.mTariffProvider->GetDayPatterns().Value(), patternID);
                if ((pattern != nullptr) && pattern->daysOfWeek.Has(GetDayOfWeek(timestamp)))
                {
                    defaultDay.date        = DayStartTS;
                    defaultDay.dayType     = DayTypeEnum::kStandard;
                    defaultDay.dayEntryIDs = pattern->dayEntryIDs;
                    break;
                }
            }
        }
    }

    return defaultDay;
}

bool DayIsValid(Structs::DayStruct::Type * aDay)
{
    if ((aDay->date == 0) || (aDay->dayType == DayTypeEnum::kUnknownEnumValue))
    {
        return false;
    }
    return true;
}

std::pair<const Structs::DayEntryStruct::Type *, const Structs::DayEntryStruct::Type *>
FindDayEntry(CurrentTariffAttrsCtx & aCtx, const DataModel::List<const uint32_t> & dayEntryIDs, uint16_t minutesSinceMidnight,
             uint16_t * currentEntryMinutesRemain)
{
    const Structs::DayEntryStruct::Type * currentPtr = nullptr;
    const Structs::DayEntryStruct::Type * nextPtr    = nullptr;
    *currentEntryMinutesRemain                       = 0;

    for (size_t i = 0; i < dayEntryIDs.size(); i++)
    {
        nextPtr    = nullptr;
        currentPtr = GetListEntryById<Structs::DayEntryStruct::Type>(aCtx.mTariffProvider->GetDayEntries().Value(), dayEntryIDs[i]);
        if (currentPtr == nullptr)
        {
            continue;
        }

        if (i + 1 < dayEntryIDs.size())
        {
            nextPtr =
                GetListEntryById<Structs::DayEntryStruct::Type>(aCtx.mTariffProvider->GetDayEntries().Value(), dayEntryIDs[i + 1]);
        }
        else
        {
            nextPtr = nullptr;
        }

        // Default: Current entry lasts until end of day
        uint16_t duration = (kDayEntryDurationLimit - currentPtr->startTime);

        if (currentPtr->duration.HasValue())
        {
            duration = currentPtr->duration.Value();
        }
        else if (nextPtr != nullptr && nextPtr->startTime < kDayEntryDurationLimit)
        {
            if (nextPtr->startTime <= currentPtr->startTime)
            {
                // Next entry is on the following day
                duration = static_cast<uint16_t>((kDayEntryDurationLimit - currentPtr->startTime) + nextPtr->startTime);
            }
            else
            {
                // Next entry is on the same day
                duration = nextPtr->startTime - currentPtr->startTime;
            }
        }

        // Check if current entry matches the current time
        if (currentPtr->startTime <= minutesSinceMidnight && (currentPtr->startTime + duration) > minutesSinceMidnight)
        {
            *currentEntryMinutesRemain = static_cast<uint16_t>(duration - (minutesSinceMidnight - currentPtr->startTime));
            break;
        }
    }

    return { currentPtr, nextPtr };
}

const Structs::TariffPeriodStruct::Type * FindTariffPeriodByDayEntryId(CurrentTariffAttrsCtx & aCtx, uint32_t dayEntryID)
{
    for (const auto & period : aCtx.mTariffProvider->GetTariffPeriods().Value())
    {
        for (const auto & entryID : period.dayEntryIDs)
        {
            if (entryID == dayEntryID)
            {
                return &period;
            }
        }
    }

    return nullptr;
}

std::unordered_set<const Structs::TariffPeriodStruct::Type *> FindTariffPeriodsByTariffComponentId(CurrentTariffAttrsCtx & aCtx,
                                                                                                   uint32_t componentID)
{
    std::unordered_set<const Structs::TariffPeriodStruct::Type *> matchingPeriods;

    for (const auto & period : aCtx.mTariffProvider->GetTariffPeriods().Value())
    {
        if (std::find(period.tariffComponentIDs.begin(), period.tariffComponentIDs.end(), componentID) !=
            period.tariffComponentIDs.end())
        {
            matchingPeriods.insert(&period);
        }
    }

    return matchingPeriods;
}

CHIP_ERROR UpdateTariffComponentAttrsDayEntryById(Instance * aInstance, CurrentTariffAttrsCtx & aCtx, uint32_t dayEntryID,
                                                  TariffComponentsDataClass & mgmtObj)
{
    CHIP_ERROR err                                   = CHIP_NO_ERROR;
    const Structs::TariffPeriodStruct::Type * period = FindTariffPeriodByDayEntryId(aCtx, dayEntryID);

    // Use a fixed-size array with maximum expected components
    Platform::ScopedMemoryBufferWithSize<Structs::TariffComponentStruct::Type> tempBuffer;
    Platform::ScopedMemoryBufferWithSize<char> tempLabelBuffers[kTariffPeriodItemMaxIDs];

    if (period == nullptr)
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    const DataModel::List<const uint32_t> & componentIDs = period->tariffComponentIDs;
    const size_t componentCount                          = componentIDs.size();

    // Validate component count
    if (componentCount == 0 || componentCount > kTariffPeriodItemMaxIDs)
    {
        return CHIP_ERROR_INVALID_LIST_LENGTH;
    }

    // Allocate memory for the component array
    if (!tempBuffer.Calloc(componentCount))
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    for (size_t i = 0; i < componentIDs.size(); i++)
    {
        Structs::TariffComponentStruct::Type entry;
        auto current = GetListEntryById<Structs::TariffComponentStruct::Type>(aCtx.mTariffProvider->GetTariffComponents().Value(),
                                                                              componentIDs[i]);
        if (current == nullptr)
        {
            err = CHIP_ERROR_NOT_FOUND;
            break;
        }
        entry = *current;
        if (current->label.HasValue())
        {
            DataModel::Nullable<chip::CharSpan> tmpNullLabel;
            tmpNullLabel.SetNull();
            if (!current->label.Value().IsNull())
            {
                chip::CharSpan srcLabelSpan = current->label.Value().Value();
                tempLabelBuffers[i].CopyFromSpan(srcLabelSpan);
                tmpNullLabel.SetNonNull(chip::CharSpan(tempLabelBuffers[i].Get(), srcLabelSpan.size()));
            }
            entry.label = MakeOptional(tmpNullLabel);
        }
        tempBuffer[i] = entry;
    }
    SuccessOrExit(err);

    err =
        mgmtObj.SetNewValue(MakeNullable(DataModel::List<Structs::TariffComponentStruct::Type>(tempBuffer.Get(), componentCount)));
    SuccessOrExit(err);

    err = mgmtObj.UpdateBegin(nullptr);
    SuccessOrExit(err);

    if (mgmtObj.UpdateFinish(err == CHIP_NO_ERROR)) // Success path
    {
        aInstance->AttributeUpdCb(mgmtObj.GetAttrId());
    }

exit:
    return err;
}
} // namespace Utils

using CurrentTariffAttrsCtx = CommodityTariff::Instance::CurrentTariffAttrsCtx;

static void AttrsCtxInit(Delegate & aTariffProvider, CurrentTariffAttrsCtx & aCtx)
{
    aCtx.mTariffProvider = &aTariffProvider;
}

static void AttrsCtxDeinit(CurrentTariffAttrsCtx & aCtx)
{
    aCtx.mTariffProvider = nullptr;
}

void Instance::InitCurrentAttrs()
{
    AttrsCtxInit(mDelegate, mServerTariffAttrsCtx);
    CHIP_ERROR err = UpdateCurrentAttrs();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to initialize current attributes: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void Instance::TariffTimeAttrsSync()
{
    CHIP_ERROR err = UpdateCurrentAttrs();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to sync tariff time attributes: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

CHIP_ERROR Instance::UpdateCurrentAttrs()
{
    uint32_t matterEpochNow_s = GetCurrentTimestamp();
    if (!matterEpochNow_s)
    {
        ChipLogError(AppServer, "The timestamp value can't be zero!");
        return CHIP_ERROR_INVALID_TIME;
    }

    if (mServerTariffAttrsCtx.mTariffProvider == nullptr)
    {
        ChipLogError(AppServer, "The tariff is not available");
        return CHIP_ERROR_NOT_FOUND;
    }

    // Update day information
    ReturnErrorOnFailure(UpdateDayInformation(matterEpochNow_s));

    // Update day entry information
    ReturnErrorOnFailure(UpdateDayEntryInformation(matterEpochNow_s));

    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::UpdateDayInformation(uint32_t matterEpochNow_s)
{
    DataModel::Nullable<Structs::DayStruct::Type> currentDay;
    DataModel::Nullable<Structs::DayStruct::Type> nextDay;

    currentDay.SetNonNull(Utils::FindDay(mServerTariffAttrsCtx, matterEpochNow_s));

    // Find current day
    if (!Utils::DayIsValid(&currentDay.Value()))
    {
        ChipLogError(AppServer, "The mCurrentDay data is invalid");
        return CHIP_ERROR_INTERNAL;
    }

    ChipLogDetail(AppServer, "UpdateCurrentAttrs: current day date: %u", currentDay.Value().date);
    SetCurrentDay(currentDay);

    nextDay.SetNonNull(
        Utils::FindDay(mServerTariffAttrsCtx, (matterEpochNow_s + (kSecondsPerDay - matterEpochNow_s % kSecondsPerDay)) + 1));

    if (Utils::DayIsValid(&nextDay.Value()))
    {
        ChipLogDetail(AppServer, "UpdateCurrentAttrs: next day date: %u", nextDay.Value().date);
        SetNextDay(nextDay);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::UpdateDayEntryInformation(uint32_t matterEpochNow_s)
{
    if (mCurrentDay.IsNull())
    {
        return CHIP_ERROR_INTERNAL;
    }

    const uint16_t minutesSinceMidnight = static_cast<uint16_t>((matterEpochNow_s % kSecondsPerDay) / 60);
    uint16_t currentEntryMinutesRemain  = 0;
    auto & currentDayEntryIDs           = mCurrentDay.Value().dayEntryIDs;

    auto [currentEntry, nextEntry] =
        Utils::FindDayEntry(mServerTariffAttrsCtx, currentDayEntryIDs, minutesSinceMidnight, &currentEntryMinutesRemain);

    // Handle current day entry
    DataModel::Nullable<Structs::DayEntryStruct::Type> tmpDayEntry;
    DataModel::Nullable<uint32_t> tmpDate;

    if (currentEntry != nullptr)
    {
        tmpDayEntry.SetNonNull(*currentEntry);
        tmpDate.SetNonNull(mCurrentDay.Value().date + (currentEntry->startTime * 60));

        ReturnErrorOnFailure(Utils::UpdateTariffComponentAttrsDayEntryById(this, mServerTariffAttrsCtx, currentEntry->dayEntryID,
                                                                           mCurrentTariffComponents_MgmtObj));
        ChipLogDetail(AppServer, "UpdateCurrentAttrs: current day entry: %u", tmpDayEntry.Value().dayEntryID);
    }

    SetCurrentDayEntry(tmpDayEntry);
    SetCurrentDayEntryDate(tmpDate);

    // Handle next day entry
    tmpDayEntry.SetNull();
    tmpDate.SetNull();

    if (nextEntry != nullptr)
    {
        tmpDayEntry.SetNonNull(*nextEntry);
        ReturnErrorOnFailure(Utils::UpdateTariffComponentAttrsDayEntryById(this, mServerTariffAttrsCtx, nextEntry->dayEntryID,
                                                                           mNextTariffComponents_MgmtObj));
        ChipLogDetail(AppServer, "UpdateCurrentAttrs: next day entry: %u", tmpDayEntry.Value().dayEntryID);
        tmpDate.SetNonNull(mCurrentDayEntryDate.Value() + currentEntryMinutesRemain * 60);
    }

    SetNextDayEntry(tmpDayEntry);
    SetNextDayEntryDate(tmpDate);

    return CHIP_NO_ERROR;
}

void Instance::DeinitCurrentAttrs()
{
    AttrsCtxDeinit(mServerTariffAttrsCtx);
    ResetCurrentAttributes();
}

void Instance::HandleGetTariffComponent(HandlerContext & ctx, const Commands::GetTariffComponent::DecodableType & commandData)
{
    Commands::GetTariffComponentResponse::Type response;
    Status status = Status::Failure;

    if (mServerTariffAttrsCtx.mTariffProvider == nullptr)
    {
        ChipLogError(AppServer, "The tariff is not available");
    }
    else
    {
        status         = Status::NotFound;
        auto component = Utils::GetListEntryById<Structs::TariffComponentStruct::Type>(
            mServerTariffAttrsCtx.mTariffProvider->GetTariffComponents().Value(), commandData.tariffComponentID);

        if (component != nullptr)
        {
            std::array<uint32_t, CommodityTariffConsts::kDayEntriesAttrMaxLength> deIDsArray;
            size_t deIDsCount = 0;
            std::array<char, kDefaultStringValuesMaxBufLength> labelBuffer;
            size_t labelLength = 0;

            auto matchingPeriods =
                Utils::FindTariffPeriodsByTariffComponentId(mServerTariffAttrsCtx, commandData.tariffComponentID);

            if (!matchingPeriods.empty())
            {
                bool firstLabel = true;

                for (const auto * period : matchingPeriods)
                {
                    if (!period->label.IsNull())
                    {
                        const auto & periodLabel       = period->label.Value();
                        const size_t periodLabelLength = periodLabel.size();

                        // Add separator if not the first label
                        if (!firstLabel)
                        {
                            if (labelLength + 2 < CommodityTariffConsts::kDefaultStringValuesMaxBufLength) // Space for "; "
                            {
                                labelBuffer[labelLength++] = ';';
                                labelBuffer[labelLength++] = ' ';
                            }
                            else
                            {
                                ChipLogError(AppServer, "Label buffer full, truncating");
                                break;
                            }
                        }

                        // Copy period label to buffer
                        size_t copyLength =
                            std::min(periodLabelLength, CommodityTariffConsts::kDefaultStringValuesMaxBufLength - labelLength);
                        if (copyLength > 0)
                        {
                            memcpy(labelBuffer.data() + labelLength, periodLabel.data(), copyLength);
                            labelLength += copyLength;
                        }

                        firstLabel = false;

                        if (labelLength >= CommodityTariffConsts::kDefaultStringValuesMaxBufLength)
                        {
                            ChipLogError(AppServer, "Label buffer full, truncating");
                            break;
                        }
                    }

                    if (!period->dayEntryIDs.empty())
                    {
                        for (const auto & deID : period->dayEntryIDs)
                        {
                            if (deIDsCount >= CommodityTariffConsts::kDayEntriesAttrMaxLength)
                            {
                                ChipLogError(AppServer, "Day entry ID buffer full, truncating");
                                break;
                            }
                            deIDsArray[deIDsCount++] = deID;
                        }
                    }
                }
            }

            std::sort(deIDsArray.begin(), std::next(deIDsArray.begin(), static_cast<std::ptrdiff_t>(deIDsCount)));

            response.label.SetNull();
            response.dayEntryIDs = DataModel::List<uint32_t>();

            if (labelLength > 0)
            {
                response.label.SetNonNull(chip::CharSpan(labelBuffer.data(), labelLength));
            }

            if (deIDsCount > 0)
            {
                response.dayEntryIDs = DataModel::List<uint32_t>(deIDsArray.data(), deIDsCount);
            }

            response.tariffComponent = *component;
            status                   = Status::Success;

            ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        }
    }

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }
}

void Instance::HandleGetDayEntry(HandlerContext & ctx, const Commands::GetDayEntry::DecodableType & commandData)
{
    Commands::GetDayEntryResponse::Type response;
    Status status = Status::Failure;

    if (mServerTariffAttrsCtx.mTariffProvider == nullptr)
    {
        ChipLogError(AppServer, "The tariff is not available");
    }
    else
    {
        status     = Status::NotFound;
        auto entry = Utils::GetListEntryById<Structs::DayEntryStruct::Type>(
            mServerTariffAttrsCtx.mTariffProvider->GetDayEntries().Value(), commandData.dayEntryID);

        if (entry != nullptr)
        {
            response.dayEntry = *entry;
            status            = Status::Success;
        }
    }

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

CommodityTariffAttrsDataMgmt::CTC_BaseDataClassBase & Delegate::GetMgmtObj(CommodityTariffAttrTypeEnum aType)
{
    switch (aType)
    {
    case CommodityTariffAttrTypeEnum::kTariffUnit:
        return mTariffUnit_MgmtObj;
    case CommodityTariffAttrTypeEnum::kStartDate:
        return mStartDate_MgmtObj;
    case CommodityTariffAttrTypeEnum::kDefaultRandomizationOffset:
        return mDefaultRandomizationOffset_MgmtObj;
    case CommodityTariffAttrTypeEnum::kDefaultRandomizationType:
        return mDefaultRandomizationType_MgmtObj;
    case CommodityTariffAttrTypeEnum::kTariffInfo:
        return mTariffInfo_MgmtObj;
    case CommodityTariffAttrTypeEnum::kDayEntries:
        return mDayEntries_MgmtObj;
    case CommodityTariffAttrTypeEnum::kDayPatterns:
        return mDayPatterns_MgmtObj;
    case CommodityTariffAttrTypeEnum::kTariffComponents:
        return mTariffComponents_MgmtObj;
    case CommodityTariffAttrTypeEnum::kTariffPeriods:
        return mTariffPeriods_MgmtObj;
    case CommodityTariffAttrTypeEnum::kIndividualDays:
        return mIndividualDays_MgmtObj;
    case CommodityTariffAttrTypeEnum::kCalendarPeriods:
        return mCalendarPeriods_MgmtObj;
    default:
        VerifyOrDieWithMsg(false, AppServer, "Unknown management attribute type");
        return mTariffUnit_MgmtObj; // return something to satisfy compiler
    }
}

} // namespace CommodityTariff
} // namespace Clusters
} // namespace app
} // namespace chip
