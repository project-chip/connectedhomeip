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

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CommodityTariff;
using namespace chip::app::Clusters::CommodityTariff::Attributes;

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
    case TariffInfo::Id:
        return aEncoder.Encode(mDelegate.GetTariffInfo());
    case TariffUnit::Id:
        return aEncoder.Encode(mDelegate.GetTariffUnit());
    case StartDate::Id:
        return aEncoder.Encode(mDelegate.GetStartDate());
    case DayEntries::Id:
        return aEncoder.Encode(mDelegate.GetDayEntries());
    case DayPatterns::Id:
        return aEncoder.Encode(mDelegate.GetDayPatterns());
    case CalendarPeriods::Id:
        return aEncoder.Encode(mDelegate.GetCalendarPeriods());
    case IndividualDays::Id:
        return aEncoder.Encode(mDelegate.GetIndividualDays());
    case TariffComponents::Id:
        return aEncoder.Encode(mDelegate.GetTariffComponents());
    case TariffPeriods::Id:
        return aEncoder.Encode(mDelegate.GetTariffPeriods());
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
    case CurrentTariffComponents::Id:
        return aEncoder.Encode(GetCurrentTariffComponents());
    case NextTariffComponents::Id:
        return aEncoder.Encode(GetNextTariffComponents());
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

    /* FeatureMap - is held locally */
    case FeatureMap::Id:
        return aEncoder.Encode(mFeature);
    }

    /* Allow all other unhandled attributes to fall through to Ember */
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

    mCurrentTariffComponents_MgmtObj.Cleanup();
    mNextTariffComponents_MgmtObj.Cleanup();
}

static uint32_t GetCurrentTimestamp(void)
{
    System::Clock::Microseconds64 utcTimeUnix;
    uint64_t chipEpochTime;
    System::SystemClock().GetClock_RealTime(utcTimeUnix);
    UnixEpochToChipEpochMicros(utcTimeUnix.count(), chipEpochTime);

    return static_cast<uint32_t>(chipEpochTime / chip::kMicrosecondsPerSecond);
}

void Instance::TariffDataUpdatedCb(bool is_erased)
{
    if (is_erased)
    {
        UpdateCurrentAttrs(UpdateEventCode::TariffErased);
        return;
    }

    UpdateCurrentAttrs(UpdateEventCode::TariffUpdated);
}

void Instance::ScheduleTariffActivation(uint32_t delay)
{
    DeviceLayer::SystemLayer().StartTimer(
        System::Clock::Milliseconds32(delay * 1000),
        [](System::Layer *, void * context) {
            static_cast<Instance *>(context)->UpdateCurrentAttrs(UpdateEventCode::TariffUpdated);
        },
        this);
}

void Instance::ScheduleMidnightUpdate()
{
    uint32_t now                  = GetCurrentTimestamp();
    uint32_t secondsSinceMidnight = static_cast<uint32_t>(now % kSecondsPerDay);
    uint32_t delay                = (secondsSinceMidnight == 0) ? static_cast<uint32_t>(kSecondsPerDay)
                                                                : static_cast<uint32_t>(kSecondsPerDay - secondsSinceMidnight);

    DeviceLayer::SystemLayer().StartTimer(
        System::Clock::Milliseconds32(delay * 1000),
        [](System::Layer *, void * context) {
            static_cast<Instance *>(context)->UpdateCurrentAttrs(UpdateEventCode::DaysUpdating);
        },
        this);
}

void Instance::ScheduleDayEntryUpdate(uint16_t minutesSinceMidnight)
{
    uint32_t now                  = GetCurrentTimestamp();
    uint32_t secondsSinceMidnight = now % kSecondsPerDay;
    uint32_t triggerOffset        = minutesSinceMidnight * 60;

    // Calculate delay considering next day if needed
    uint32_t delay = (triggerOffset > secondsSinceMidnight) ? triggerOffset - secondsSinceMidnight
                                                            : kSecondsPerDay - secondsSinceMidnight + triggerOffset;

    DeviceLayer::SystemLayer().StartTimer(
        System::Clock::Milliseconds32(delay * 1000),
        [](System::Layer *, void * context) {
            static_cast<Instance *>(context)->UpdateCurrentAttrs(UpdateEventCode::DayEntryUpdating);
        },
        this);
}

namespace Utils {
template <typename T, auto X>
void ListToMap(const DataModel::List<T> & aList, std::map<uint32_t, const T *> & aMap)
{
    for (const auto & item : aList)
    {
        // Insert into map with specified entry as key
        aMap.emplace(item.*X, &item);
    }
}

template <typename T>
std::pair<const T *, const T *> GetCurrNextItemsById(const std::map<uint32_t, const T *> & aMap, uint32_t aId)
{
    auto current = aMap.find(aId);
    auto next    = aMap.upper_bound(aId); // Gets element after aId

    const T * currentPtr = (current != aMap.end()) ? current->second : nullptr;
    const T * nextPtr    = (next != aMap.end()) ? next->second : nullptr;

    return { currentPtr, nextPtr };
}

DayPatternDayOfWeekBitmap GetDayOfWeek(uint32_t timestamp)
{
    time_t time         = static_cast<time_t>(timestamp);
    struct tm * utcTime = gmtime(&time);
    return static_cast<DayPatternDayOfWeekBitmap>(1 << utcTime->tm_wday);
}

Structs::DayStruct::Type FindDay(CurrentTariffAttrsCtx & aCtx, uint32_t timestamp)
{
    Structs::DayStruct::Type defaultDay = { .date        = 0,
                                            .dayType     = DayTypeEnum::kUnknownEnumValue,
                                            .dayEntryIDs = DataModel::List<const uint32_t>() };
    uint32_t DayStartTS                 = timestamp - (timestamp % kSecondsPerDay);

    // First check IndividualDays
    if (!aCtx.TariffProvider->GetIndividualDays().IsNull())
    {
        for (const auto & day : aCtx.TariffProvider->GetIndividualDays().Value())
        {
            if ((day.date >= DayStartTS) && (day.date < (DayStartTS + kSecondsPerDay)))
            {
                return day;
            }
        }
    }

    if (!aCtx.TariffProvider->GetCalendarPeriods().IsNull())
    {
        for (const auto & period : aCtx.TariffProvider->GetCalendarPeriods().Value())
        {
            if (period.startDate.IsNull() ||
                ((period.startDate.Value() >= DayStartTS) && (period.startDate.Value() < (DayStartTS + kSecondsPerDay))))
            {
                for (const auto & patternID : period.dayPatternIDs)
                {
                    auto * pattern = GetCurrNextItemsById<Structs::DayPatternStruct::Type>(aCtx.DayPatternsMap, patternID).first;
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
             uint16_t * nextUpdInterval)
{
    const Structs::DayEntryStruct::Type * currentPtr = nullptr;
    const Structs::DayEntryStruct::Type * nextPtr    = nullptr;
    uint16_t duration                                = 0;

    for (const auto & entryID : dayEntryIDs)
    {
        auto [current, next] = GetCurrNextItemsById<Structs::DayEntryStruct::Type>(aCtx.DayEntriesMap, entryID);
        duration             = (CommodityTariffAttrsDataMgmt::kDayEntryDurationLimit - current->startTime);

        if (next != nullptr)
        {
            nextPtr = next;

            if (current->duration.HasValue())
            {
                duration = current->duration.Value();
            }
            else if (next->startTime > 0)
            {
                duration = next->startTime - current->startTime;
            }
        }

        if (current->startTime <= minutesSinceMidnight && (current->startTime + duration) > minutesSinceMidnight)
        {
            currentPtr = current;
            break;
        }
    }

    *nextUpdInterval = duration;

    return { currentPtr, nextPtr };
}

const Structs::TariffPeriodStruct::Type * FindTariffPeriodByDayEntryId(CurrentTariffAttrsCtx & aCtx, uint32_t dayEntryID)
{
    for (const auto & period : aCtx.TariffProvider->GetTariffPeriods().Value())
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

const Structs::TariffPeriodStruct::Type * FindTariffPeriodByTariffComponentId(CurrentTariffAttrsCtx & aCtx, uint32_t componentID)
{
    for (const auto & period : aCtx.TariffProvider->GetTariffPeriods().Value())
    {
        for (const auto & entryID : period.tariffComponentIDs)
        {
            if (entryID == componentID)
            {
                return &period;
            }
        }
    }

    return nullptr;
}

CHIP_ERROR UpdateTariffComponentAttrsDayEntryById(CurrentTariffAttrsCtx & aCtx, uint32_t dayEntryID,
                                                  TariffComponentsDataClass & mgmtObj)
{
    CHIP_ERROR err                                   = CHIP_NO_ERROR;
    const Structs::TariffPeriodStruct::Type * period = FindTariffPeriodByDayEntryId(aCtx, dayEntryID);
    std::vector<Structs::TariffComponentStruct::Type> tempList;

    if (period != nullptr)
    {
        const DataModel::List<const uint32_t> & componentIDs = period->tariffComponentIDs;

        tempList.reserve(componentIDs.size());

        for (const auto & entryID : componentIDs)
        {
            Structs::TariffComponentStruct::Type entry;
            auto current = GetCurrNextItemsById<Structs::TariffComponentStruct::Type>(aCtx.TariffComponentsMap, entryID).first;
            entry        = *current;
            if (current->label.HasValue())
            {
                DataModel::Nullable<chip::CharSpan> tmpNullLabel;
                tmpNullLabel.SetNull();

                if (!current->label.Value().IsNull())
                {
                    chip::CharSpan srcLabelSpan = current->label.Value().Value();
                    if (!CommodityTariffAttrsDataMgmt::SpanCopier<char>::Copy(current->label.Value().Value(), tmpNullLabel,
                                                                              srcLabelSpan.size()))
                    {
                        return CHIP_ERROR_NO_MEMORY;
                    }
                }

                entry.label = MakeOptional(tmpNullLabel);
            }

            tempList.push_back(entry);
        }

        if ((err == CHIP_NO_ERROR) && ((err = mgmtObj.CreateNewValue(tempList.size())) == CHIP_NO_ERROR))
        {
            std::copy(tempList.begin(), tempList.end(), mgmtObj.GetNewValueData());
            mgmtObj.MarkAsAssigned();
            mgmtObj.UpdateBegin(nullptr, nullptr);
            mgmtObj.UpdateCommit();
        }
        else
        {
            for (auto & entry : tempList)
                mgmtObj.CleanupExtEntry(entry);
        }
    }
    else
    {
        err = CHIP_ERROR_NOT_FOUND;
    }

    return err;
}
} // namespace Utils

static void AttrsCtxInit(Delegate & aTariffProvider, CurrentTariffAttrsCtx & aCtx)
{
    aCtx.TariffProvider = &aTariffProvider;

    Utils::ListToMap<Structs::DayPatternStruct::Type, &Structs::DayPatternStruct::Type::dayPatternID>(
        aTariffProvider.GetDayPatterns().Value(), aCtx.DayPatternsMap);
    Utils::ListToMap<Structs::DayEntryStruct::Type, &Structs::DayEntryStruct::Type::dayEntryID>(
        aTariffProvider.GetDayEntries().Value(), aCtx.DayEntriesMap);
    Utils::ListToMap<Structs::TariffComponentStruct::Type, &Structs::TariffComponentStruct::Type::tariffComponentID>(
        aTariffProvider.GetTariffComponents().Value(), aCtx.TariffComponentsMap);
}

static void AttrsCtxDeinit(CurrentTariffAttrsCtx & aCtx)
{
    aCtx.DayPatternsMap.clear();
    aCtx.DayEntriesMap.clear();
    aCtx.TariffComponentsMap.clear();
    aCtx.TariffProvider = nullptr;
}

void Instance::UpdateCurrentAttrs(UpdateEventCode aEvt)
{
    uint32_t timestampNow = GetCurrentTimestamp();

    ChipLogProgress(NotSpecified, "EGW-CTC: UpdateEventCode: %x", static_cast<std::underlying_type_t<UpdateEventCode>>(aEvt));

    // Only for test purposes
    if (mServerTariffAttrsCtx.forwardAlarmTriggerTime)
    {
        timestampNow                                  = mServerTariffAttrsCtx.forwardAlarmTriggerTime;
        mServerTariffAttrsCtx.forwardAlarmTriggerTime = 0;
    }

    if (mServerTariffAttrsCtx.TariffProvider == nullptr && aEvt != UpdateEventCode::TariffUpdated)
    {
        return;
    }

    switch (aEvt)
    {
    case UpdateEventCode::TariffErased: {
        [[fallthrough]];
    }
    case UpdateEventCode::TariffUpdated: {
        // Handle tariff structure updates
        // This would be triggered when primary attributes change
        // Reset all current attributes to null
        AttrsCtxDeinit(mServerTariffAttrsCtx);
        ResetCurrentAttributes();

        if (aEvt == UpdateEventCode::TariffErased)
        {
            return;
        }

        if (mDelegate.GetStartDate().Value() > timestampNow)
        {
            ScheduleTariffActivation(mDelegate.GetStartDate().Value() - timestampNow);
            return;
        }
        AttrsCtxInit(mDelegate, mServerTariffAttrsCtx);
        // Fall through to days updating
        [[fallthrough]];
    }
    case UpdateEventCode::DaysUpdating: {
        Structs::DayStruct::Type Day;
        if (mCurrentDay.IsNull())
        {
            Day = Utils::FindDay(mServerTariffAttrsCtx, timestampNow);
            // Find current day
            if (Utils::DayIsValid(&Day))
            {
                mCurrentDay.SetNonNull(Day);
                MatterReportingAttributeChangeCallback(mEndpointId, CommodityTariff::Id, CurrentDay::Id);
            }
        }
        else
        {
            // Move next day to current
            mCurrentDay = mNextDay;
        }

        if (mCurrentDay.IsNull())
        {
            // Something went wrong! The attribute can't be Null if tariff is active
            ChipLogError(NotSpecified, "The mCurrentDay can't be Null if tariff is active!");
            return;
        }

        Day = Utils::FindDay(mServerTariffAttrsCtx, timestampNow + kSecondsPerDay);
        if (Utils::DayIsValid(&Day))
        {
            mNextDay.SetNonNull(Day);
            MatterReportingAttributeChangeCallback(mEndpointId, CommodityTariff::Id, NextDay::Id);
        }

        ScheduleMidnightUpdate();
        // Fall through to DayEntries updating
        [[fallthrough]];
    }
    case UpdateEventCode::DayEntryUpdating: {
        uint16_t minutesSinceMidnight = static_cast<uint16_t>((timestampNow % kSecondsPerDay) / 60);
        uint16_t nextUpdInterval      = 0;
        auto & mCurrentDayEntryIDs    = mCurrentDay.Value().dayEntryIDs;
        auto [current, next] =
            Utils::FindDayEntry(mServerTariffAttrsCtx, mCurrentDayEntryIDs, minutesSinceMidnight, &nextUpdInterval);

        if (current != nullptr)
        {
            mCurrentDayEntry.SetNonNull(*current);
            mCurrentDayEntryDate.SetNonNull(mCurrentDay.Value().date);
            MatterReportingAttributeChangeCallback(mEndpointId, CommodityTariff::Id, CurrentDayEntry::Id);
            MatterReportingAttributeChangeCallback(mEndpointId, CommodityTariff::Id, CurrentDayEntryDate::Id);
            if (CHIP_NO_ERROR ==
                Utils::UpdateTariffComponentAttrsDayEntryById(mServerTariffAttrsCtx, current->dayEntryID,
                                                              mCurrentTariffComponents_MgmtObj))
            {
                MatterReportingAttributeChangeCallback(mEndpointId, CommodityTariff::Id, CurrentTariffComponents::Id);
            }
        }
        else
        {
            mCurrentDayEntry.SetNull();
            mCurrentDayEntryDate.SetNull();
            mCurrentTariffComponents_MgmtObj.Cleanup();
        }

        if (next != nullptr)
        {
            mNextDayEntry.SetNonNull(*next);
            mNextDayEntryDate.SetNonNull(mCurrentDay.Value().date);
            MatterReportingAttributeChangeCallback(mEndpointId, CommodityTariff::Id, NextDayEntry::Id);
            MatterReportingAttributeChangeCallback(mEndpointId, CommodityTariff::Id, NextDayEntryDate::Id);
            if (CHIP_NO_ERROR ==
                Utils::UpdateTariffComponentAttrsDayEntryById(mServerTariffAttrsCtx, next->dayEntryID,
                                                              mNextTariffComponents_MgmtObj))
            {
                MatterReportingAttributeChangeCallback(mEndpointId, CommodityTariff::Id, NextTariffComponents::Id);
            }
        }
        else
        {
            mNextDayEntry.SetNull();
            mNextDayEntryDate.SetNull();
            mNextTariffComponents_MgmtObj.Cleanup();
        }

        if (nextUpdInterval > 0)
        {
            ScheduleDayEntryUpdate(nextUpdInterval);
        }
    }
    }
}

void Instance::HandleGetTariffComponent(HandlerContext & ctx, const Commands::GetTariffComponent::DecodableType & commandData)
{
    Commands::GetTariffComponentResponse::Type response;
    Status status = Status::NotFound;

    auto component = Utils::GetCurrNextItemsById<Structs::TariffComponentStruct::Type>(mServerTariffAttrsCtx.TariffComponentsMap,
                                                                                       commandData.tariffComponentID)
                         .first;
    auto period = Utils::FindTariffPeriodByTariffComponentId(mServerTariffAttrsCtx, commandData.tariffComponentID);

    if (component != nullptr && period != nullptr)
    {
        response.label           = period->label;
        response.dayEntryIDs     = period->dayEntryIDs;
        response.tariffComponent = *component;
        status                   = Status::Success;
    }

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

void Instance::HandleGetDayEntry(HandlerContext & ctx, const Commands::GetDayEntry::DecodableType & commandData)
{
    Commands::GetDayEntryResponse::Type response;
    Status status = Status::NotFound;

    auto entry =
        Utils::GetCurrNextItemsById<Structs::DayEntryStruct::Type>(mServerTariffAttrsCtx.DayEntriesMap, commandData.dayEntryID)
            .first;

    if (entry != nullptr)
    {
        response.dayEntry = *entry;
        status            = Status::Success;
    }

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

void Instance::ForceDaysAttrsUpdate()
{
    uint32_t now                  = GetCurrentTimestamp();
    uint32_t secondsSinceMidnight = static_cast<uint32_t>(now % kSecondsPerDay);
    uint32_t delay                = (secondsSinceMidnight == 0) ? static_cast<uint32_t>(kSecondsPerDay)
                                                                : static_cast<uint32_t>(kSecondsPerDay - secondsSinceMidnight);

    mServerTariffAttrsCtx.forwardAlarmTriggerTime = now + delay;

    UpdateCurrentAttrs(UpdateEventCode::DaysUpdating);
}

void Instance::ForceDayEntriesAttrsUpdate()
{
    // Early return if we don't have valid current day data
    if (mCurrentDayEntryDate.IsNull() || mCurrentDayEntry.IsNull())
    {
        ChipLogDetail(NotSpecified, "ForceDayEntriesAttrsUpdate: No current day entry set");
        return;
    }

    const uint32_t now                  = GetCurrentTimestamp();
    const uint16_t minutesSinceMidnight = static_cast<uint16_t>((now % kSecondsPerDay) / 60);
    const auto & currentEntry           = mCurrentDayEntry.Value();

    // Calculate end time of current entry
    uint16_t entryEndTime = 0;
    if (currentEntry.duration.HasValue())
    {
        // Case 1: Entry has explicit duration
        entryEndTime = currentEntry.startTime + currentEntry.duration.Value();
    }
    else if (!mNextDayEntry.IsNull())
    {
        // Case 2: Use next entry's start time as our end time
        entryEndTime = mNextDayEntry.Value().startTime;
    }
    else
    {
        // Case 3: Default to end of day if no duration or next entry
        entryEndTime = CommodityTariffAttrsDataMgmt::kDayEntryDurationLimit;
    }

    // Calculate seconds remaining in current entry
    const uint16_t minutesRemaining = (entryEndTime >= minutesSinceMidnight) ? (entryEndTime - minutesSinceMidnight) : ((kSecondsPerDay / 60 - minutesSinceMidnight) + entryEndTime);
    mServerTariffAttrsCtx.forwardAlarmTriggerTime = now + (minutesRemaining * 60);

    // Determine update type based on whether we're crossing day boundary
    const bool isNewDayTransition =
        (mServerTariffAttrsCtx.forwardAlarmTriggerTime - mCurrentDayEntryDate.Value()) >= kSecondsPerDay;
    const UpdateEventCode updateType = isNewDayTransition ? UpdateEventCode::DaysUpdating : UpdateEventCode::DayEntryUpdating;

    ChipLogDetail(NotSpecified, "ForceDayEntriesAttrsUpdate: Triggering %s update in %u minutes",
                  isNewDayTransition ? "day" : "entry", minutesRemaining);

    UpdateCurrentAttrs(updateType);
}

} // namespace CommodityTariff
} // namespace Clusters
} // namespace app
} // namespace chip

void MatterCommodityTariffPluginServerInitCallback() {}
