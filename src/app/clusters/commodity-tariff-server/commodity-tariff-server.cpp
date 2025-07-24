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

#include "CommodityTariffAttrsDataMgmt.h"
#include "CommodityTariffConsts.h"
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
        using chip::app::Clusters::CommodityTariff::Structs::operator!=;
        hasChanged = (currValue.Value() != newValue.Value());
    }

    if (hasChanged)
    {
        currValue = newValue;
        MatterReportingAttributeChangeCallback(mEndpointId, CommodityTariff::Id, attrId);
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

    mCurrentTariffComponents_MgmtObj.Cleanup();
    mNextTariffComponents_MgmtObj.Cleanup();
}

static uint32_t GetCurrentTimestamp()
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

static void TariffComponentUpd_AttrChangeCb(uint32_t aAttrId, void * CbCtx)
{
    if (CbCtx != nullptr)
    {
        EndpointId * pEndpointId = (EndpointId *) CbCtx;
        ChipLogProgress(NotSpecified, "EGW-CTC: The value for attribute (Id %d) updated", aAttrId);
        MatterReportingAttributeChangeCallback(*pEndpointId, CommodityTariff::Id, aAttrId);
    }
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
        for (const auto & period : aCtx.mTariffProvider->GetCalendarPeriods().Value())
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
    *nextUpdInterval                                 = 0;

    for (const auto & entryID : dayEntryIDs)
    {
        auto [current, next] = GetCurrNextItemsById<Structs::DayEntryStruct::Type>(aCtx.DayEntriesMap, entryID);
        if (current == nullptr)
        {
            continue;
        }

        // Default: Current entry lasts until end of day
        uint16_t duration = (kDayEntryDurationLimit - current->startTime);

        if (current->duration.HasValue())
        {
            duration = current->duration.Value();
        }
        else if (next != nullptr && next->startTime < kDayEntryDurationLimit)
        {
            if (next->startTime <= current->startTime)
            {
                // Next entry is on the following day
                duration = static_cast<uint16_t>((kDayEntryDurationLimit - current->startTime) + next->startTime);
            }
            else
            {
                // Next entry is on the same day
                duration = next->startTime - current->startTime;
            }
        }

        // Check if current entry matches the current time
        if (current->startTime <= minutesSinceMidnight && (current->startTime + duration) > minutesSinceMidnight)
        {
            currentPtr       = current;
            nextPtr          = next;
            *nextUpdInterval = static_cast<uint16_t>(duration - (minutesSinceMidnight - current->startTime));
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

const Structs::TariffPeriodStruct::Type * FindTariffPeriodByTariffComponentId(CurrentTariffAttrsCtx & aCtx, uint32_t componentID)
{
    for (const auto & period : aCtx.mTariffProvider->GetTariffPeriods().Value())
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

        if (err == CHIP_NO_ERROR)
        {
            if ((err = mgmtObj.CreateNewValue(tempList.size())) == CHIP_NO_ERROR)
            {
                std::copy(tempList.begin(), tempList.end(), mgmtObj.GetNewValueData());
                mgmtObj.MarkAsAssigned();
                if ((err = mgmtObj.UpdateBegin(&aCtx.mEndpointId, TariffComponentUpd_AttrChangeCb, false)) == CHIP_NO_ERROR)
                {
                    mgmtObj.UpdateCommit(); // Success path
                }
                else
                {
                    mgmtObj.UpdateEnd(); // Clean up on validation failure
                }
            }
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

static void AttrsCtxInit(Delegate & aTariffProvider, CurrentTariffAttrsCtx & aCtx, EndpointId aEndpointId)
{
    aCtx.mTariffProvider = &aTariffProvider;
    aCtx.mEndpointId     = aEndpointId;

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
    aCtx.mTariffProvider = nullptr;
}

void Instance::UpdateCurrentAttrs(UpdateEventCode aEvt)
{
    uint32_t timestampNow = GetCurrentTimestamp();

    ChipLogProgress(NotSpecified, "EGW-CTC: UpdateEventCode: %x", static_cast<std::underlying_type_t<UpdateEventCode>>(aEvt));

    // Only for test purposes
    if (mServerTariffAttrsCtx.mForwardAlarmTriggerTime)
    {
        timestampNow                                   = mServerTariffAttrsCtx.mForwardAlarmTriggerTime;
        mServerTariffAttrsCtx.mForwardAlarmTriggerTime = 0;
    }

    if (mServerTariffAttrsCtx.mTariffProvider == nullptr && aEvt != UpdateEventCode::TariffUpdated)
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
        AttrsCtxInit(mDelegate, mServerTariffAttrsCtx, mEndpointId);
        // Fall through to days updating
        [[fallthrough]];
    }
    case UpdateEventCode::DaysUpdating: {
        DataModel::Nullable<Structs::DayStruct::Type> Day;
        if (mCurrentDay.IsNull())
        {
            Day.SetNonNull(Utils::FindDay(mServerTariffAttrsCtx, timestampNow));
            // Find current day
            if (Utils::DayIsValid(&Day.Value()))
            {
                SetCurrentDay(Day);
            }
        }
        else
        {
            // Move next day to current
            SetCurrentDay(mNextDay);
        }

        if (mCurrentDay.IsNull())
        {
            // Something went wrong! The attribute can't be Null if tariff is active
            ChipLogError(NotSpecified, "The mCurrentDay can't be Null if tariff is active!");
            return;
        }

        Day = Utils::FindDay(mServerTariffAttrsCtx, timestampNow + kSecondsPerDay);
        if (Utils::DayIsValid(&Day.Value()))
        {
            SetNextDay(Day);
        }

        ScheduleMidnightUpdate();
        // Fall through to DayEntries updating
        [[fallthrough]];
    }
    case UpdateEventCode::DayEntryUpdating: {
        uint16_t minutesSinceMidnight = static_cast<uint16_t>((timestampNow % kSecondsPerDay) / 60);
        uint16_t nextUpdInterval      = 0;
        auto & mCurrentDayEntryIDs    = mCurrentDay.Value().dayEntryIDs;
        DataModel::Nullable<Structs::DayEntryStruct::Type> tmpDayEntry;
        DataModel::Nullable<uint32_t> tmpDate;
        auto [current, next] =
            Utils::FindDayEntry(mServerTariffAttrsCtx, mCurrentDayEntryIDs, minutesSinceMidnight, &nextUpdInterval);

        tmpDayEntry.SetNull();
        tmpDate.SetNull();

        if (current != nullptr)
        {
            tmpDayEntry.SetNonNull(*current);
            tmpDate.SetNonNull(mCurrentDay.Value().date);
            if (CHIP_NO_ERROR !=
                Utils::UpdateTariffComponentAttrsDayEntryById(mServerTariffAttrsCtx, current->dayEntryID,
                                                              mCurrentTariffComponents_MgmtObj))
            {
                ChipLogError(NotSpecified, "Unable to update the CurrentTariffComponents attribute!");
            }
            ChipLogDetail(NotSpecified, "UpdateCurrentAttrs: current day entry: %u", tmpDayEntry.Value().dayEntryID);
        }

        SetCurrentDayEntry(tmpDayEntry);
        SetCurrentDayEntryDate(tmpDate);

        tmpDayEntry.SetNull();
        tmpDate.SetNull();

        if (next != nullptr)
        {
            tmpDayEntry.SetNonNull(*next);
            if (CHIP_NO_ERROR !=
                Utils::UpdateTariffComponentAttrsDayEntryById(mServerTariffAttrsCtx, next->dayEntryID,
                                                              mNextTariffComponents_MgmtObj))
            {
                ChipLogError(NotSpecified, "Unable to update the NextTariffComponents attribute!");
            }
            ChipLogDetail(NotSpecified, "UpdateCurrentAttrs: next day entry: %u", tmpDayEntry.Value().dayEntryID);
        }

        SetNextDayEntry(tmpDayEntry);

        if (nextUpdInterval > 0)
        {
            if ((nextUpdInterval >= (kDayEntryDurationLimit - minutesSinceMidnight)) && !mNextDay.IsNull())
            {
                tmpDate.SetNonNull(mNextDay.Value().date);
            }
            else
            {
                tmpDate.SetNonNull(mCurrentDay.Value().date);
            }
            SetNextDayEntryDate(tmpDate);
            ScheduleDayEntryUpdate(nextUpdInterval);
        }
    }
    }
}

void Instance::HandleGetTariffComponent(HandlerContext & ctx, const Commands::GetTariffComponent::DecodableType & commandData)
{
    Commands::GetTariffComponentResponse::Type response;
    Status status = Status::Failure;

    if (mServerTariffAttrsCtx.mTariffProvider == nullptr)
    {
        ChipLogError(NotSpecified, "The tariff is not active");
    }
    else
    {
        status         = Status::NotFound;
        auto component = Utils::GetCurrNextItemsById<Structs::TariffComponentStruct::Type>(
                             mServerTariffAttrsCtx.TariffComponentsMap, commandData.tariffComponentID)
                             .first;
        auto period = Utils::FindTariffPeriodByTariffComponentId(mServerTariffAttrsCtx, commandData.tariffComponentID);

        if (component != nullptr && period != nullptr)
        {
            response.label           = period->label;
            response.dayEntryIDs     = period->dayEntryIDs;
            response.tariffComponent = *component;
            status                   = Status::Success;
        }
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
    Status status = Status::Failure;

    if (mServerTariffAttrsCtx.mTariffProvider == nullptr)
    {
        ChipLogError(NotSpecified, "The tariff is not active");
    }
    else
    {
        status = Status::NotFound;
        auto entry =
            Utils::GetCurrNextItemsById<Structs::DayEntryStruct::Type>(mServerTariffAttrsCtx.DayEntriesMap, commandData.dayEntryID)
                .first;

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

/**
 * @brief Passes the specified time offset value to the context variable that is used to override the real-time stamp.
 * In depens on the time shift value may triggered DaysUpdating or DayEntryUpdating event handling.
 */
void Instance::SetupTimeShiftOffset(uint32_t offset)
{
    mServerTariffAttrsCtx.mForwardAlarmTriggerTime = offset;

    // Determine if this update crosses day boundary
    const bool crossesMidnight      = (mServerTariffAttrsCtx.mForwardAlarmTriggerTime) % kSecondsPerDay == 0;
    const UpdateEventCode eventType = crossesMidnight ? UpdateEventCode::DaysUpdating : UpdateEventCode::DayEntryUpdating;

    UpdateCurrentAttrs(eventType);
}

} // namespace CommodityTariff
} // namespace Clusters
} // namespace app
} // namespace chip

void MatterCommodityTariffPluginServerInitCallback() {}
