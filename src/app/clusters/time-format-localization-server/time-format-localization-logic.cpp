/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/time-format-localization-server/time-format-localization-logic.h>

#include <app/EventLogging.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/TimeFormatLocalization/Metadata.h>

using namespace chip::app::Clusters::TimeFormatLocalization;
using namespace chip::app::Clusters::TimeFormatLocalization::Attributes;

namespace chip {
namespace app {
namespace Clusters {

namespace {

class AutoReleaseIterator
{
public:
    using Iterator = DeviceLayer::DeviceInfoProvider::SupportedCalendarTypesIterator;

    explicit AutoReleaseIterator(Iterator * value) : mIterator(value) {}
    ~AutoReleaseIterator()
    {
        if (mIterator != nullptr)
        {
            mIterator->Release();
        }
    }

    // Delete copy constuctor and assignement
    AutoReleaseIterator(const AutoReleaseIterator &)             = delete;
    AutoReleaseIterator & operator=(const AutoReleaseIterator &) = delete;

    bool IsValid() const { return mIterator != nullptr; }
    bool Next(CalendarTypeEnum & value) { return (mIterator == nullptr) ? false : mIterator->Next(value); }

private:
    Iterator * mIterator;
};

constexpr CalendarTypeEnum DEFAULT_CALENDAR_TYPE = CalendarTypeEnum::kBuddhist;
constexpr HourFormatEnum DEFAULT_HOUR_FORMAT     = HourFormatEnum::k12hr;
constexpr size_t MAX_EXPECTED_ATTRIBUTE_COUNT    = 3;

} // namespace

void TimeFormatLocalizationLogic::Startup(AttributePersistenceProvider * attrProvider)
{
    VerifyOrReturn(mAttrProvider == nullptr);
    VerifyOrReturn(attrProvider != nullptr);

    mAttrProvider = attrProvider;

    InitializeCalendarType();
    InitializeHourFormat();
}

void TimeFormatLocalizationLogic::InitializeCalendarType()
{
    CalendarTypeEnum calendarType = DEFAULT_CALENDAR_TYPE;

    // Try to read existing calendar type from persistence
    MutableByteSpan calendarBytes(reinterpret_cast<uint8_t *>(&calendarType), sizeof(calendarType));
    CHIP_ERROR error =
        mAttrProvider->ReadValue({ kRootEndpointId, TimeFormatLocalization::Id, ActiveCalendarType::Id }, calendarBytes);

    // If read failed or value is invalid, use default
    // Can't tell for sure if ReadValue will not change previous variable value
    // so will set it again to default.
    if (error != CHIP_NO_ERROR)
    {
        calendarType = DEFAULT_CALENDAR_TYPE;
    }

    // Ensure the calendar type is within the supported CalendarList, otherwise choose one from that list.
    CalendarTypeEnum validCalendar = DEFAULT_CALENDAR_TYPE;
    if (!IsSupportedCalendarType(calendarType, &validCalendar))
    {
        calendarType = validCalendar;
    }

    setActiveCalendarType(calendarType);
}

void TimeFormatLocalizationLogic::InitializeHourFormat()
{
    HourFormatEnum hourFormat = DEFAULT_HOUR_FORMAT;

    MutableByteSpan hourBytes(reinterpret_cast<uint8_t *>(&hourFormat), sizeof(hourFormat));
    CHIP_ERROR error = mAttrProvider->ReadValue({ kRootEndpointId, TimeFormatLocalization::Id, HourFormat::Id }, hourBytes);

    // If read failed or value is invalid, use default
    // Can't tell for sure if ReadValue will not change previous variable value
    // so will set it again to default.
    if (error != CHIP_NO_ERROR)
    {
        hourFormat = DEFAULT_HOUR_FORMAT;
    }

    setHourFormat(hourFormat);
}

CHIP_ERROR TimeFormatLocalizationLogic::GetSupportedCalendarTypes(AttributeValueEncoder & aEncoder) const
{
    DeviceLayer::DeviceInfoProvider * provider = DeviceLayer::GetDeviceInfoProvider();
    VerifyOrReturnValue(provider != nullptr, aEncoder.EncodeEmptyList());

    AutoReleaseIterator it(provider->IterateSupportedCalendarTypes());
    VerifyOrReturnValue(it.IsValid(), aEncoder.EncodeEmptyList());

    return aEncoder.EncodeList([&it](const auto & encoder) -> CHIP_ERROR {
        CalendarTypeEnum type;

        while (it.Next(type))
        {
            ReturnErrorOnFailure(encoder.Encode(type));
        }
        return CHIP_NO_ERROR;
    });
}

bool TimeFormatLocalizationLogic::IsSupportedCalendarType(CalendarTypeEnum reqCalendar, CalendarTypeEnum * validCalendar)
{
    DeviceLayer::DeviceInfoProvider * provider = DeviceLayer::GetDeviceInfoProvider();
    VerifyOrReturnValue(provider != nullptr, false);

    AutoReleaseIterator it(provider->IterateSupportedCalendarTypes());
    VerifyOrReturnValue(it.IsValid(), false);

    CalendarTypeEnum type;
    bool found = false;

    while (it.Next(type))
    {
        // Update the optional validCalendar to a value from the SupportedList.
        // This will return either the last element of the list or the requested
        // value if exists.
        if (validCalendar != nullptr)
        {
            *validCalendar = type;
        }

        if (type == reqCalendar)
        {
            found = true;
            break;
        }
    }

    return found;
}

TimeFormatLocalization::CalendarTypeEnum TimeFormatLocalizationLogic::GetActiveCalendarType() const
{
    return mCalendarType;
}

DataModel::ActionReturnStatus TimeFormatLocalizationLogic::setHourFormat(TimeFormatLocalization::HourFormatEnum rHour)
{
    VerifyOrReturnValue(mAttrProvider != nullptr, Protocols::InteractionModel::Status::Failure);

    if (rHour == HourFormatEnum::kUnknownEnumValue)
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }

    CHIP_ERROR result = mAttrProvider->WriteValue({ kRootEndpointId, TimeFormatLocalization::Id, HourFormat::Id },
                                                  { reinterpret_cast<const uint8_t *>(&rHour), sizeof(rHour) });
    if (result == CHIP_NO_ERROR)
    {
        mHourFormat = rHour;
        return Protocols::InteractionModel::Status::Success;
    }

    return Protocols::InteractionModel::Status::WriteIgnored;
}

DataModel::ActionReturnStatus TimeFormatLocalizationLogic::setActiveCalendarType(TimeFormatLocalization::CalendarTypeEnum rCalendar)
{
    VerifyOrReturnValue(mAttrProvider != nullptr, Protocols::InteractionModel::Status::Failure);
    // TODO: Confirm error values for this operation.
    if (!mFeatures.Has(TimeFormatLocalization::Feature::kCalendarFormat))
    {
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }

    // Verify that the requested value is in the supported calendars.
    if (!IsSupportedCalendarType(rCalendar))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }

    // Now try to write the value using the AttributeProvider.
    CHIP_ERROR result = mAttrProvider->WriteValue(
        { kRootEndpointId, TimeFormatLocalization::Id, TimeFormatLocalization::Attributes::ActiveCalendarType::Id },
        { reinterpret_cast<const uint8_t *>(&rCalendar), sizeof(rCalendar) });
    if (result == CHIP_NO_ERROR)
    {
        mCalendarType = rCalendar;
        return Protocols::InteractionModel::Status::Success;
    }

    return Protocols::InteractionModel::Status::WriteIgnored;
}

TimeFormatLocalization::HourFormatEnum TimeFormatLocalizationLogic::GetHourFormat() const
{
    return mHourFormat;
}

CHIP_ERROR TimeFormatLocalizationLogic::Attributes(ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) const
{
    // Ensure capacity just in case
    ReturnErrorOnFailure(
        builder.EnsureAppendCapacity(MAX_EXPECTED_ATTRIBUTE_COUNT + DefaultServerCluster::GlobalAttributes().size()));
    // Mandatory attributes
    ReturnErrorOnFailure(builder.Append(HourFormat::kMetadataEntry));

    // These attributes depend on the Feature CalendarFormat (CALFMT)
    if (mFeatures.Has(TimeFormatLocalization::Feature::kCalendarFormat))
    {
        ReturnErrorOnFailure(builder.Append(ActiveCalendarType::kMetadataEntry));
        ReturnErrorOnFailure(builder.Append(SupportedCalendarTypes::kMetadataEntry));
    }

    // Finally, the global attributes
    return builder.AppendElements(DefaultServerCluster::GlobalAttributes());
}

BitFlags<TimeFormatLocalization::Feature> TimeFormatLocalizationLogic::GetFeatureMap() const
{
    return mFeatures;
}

} // namespace Clusters
} // namespace app
} // namespace chip
