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
namespace chip {
namespace app {
namespace Clusters {

namespace {

class AutoReleaseIterator
{
public:
    using Iterator = DeviceLayer::DeviceInfoProvider::SupportedCalendarTypesIterator;

    AutoReleaseIterator(Iterator * value) : mIterator(value) {}
    ~AutoReleaseIterator()
    {
        if (mIterator != nullptr)
        {
            mIterator->Release();
        }
    }

    bool IsValid() const { return mIterator != nullptr; }
    bool Next(TimeFormatLocalization::CalendarTypeEnum & value) { return (mIterator == nullptr) ? false : mIterator->Next(value); }

private:
    Iterator * mIterator;
};

} // namespace

void TimeFormatLocalizationLogic::Startup(AttributePersistenceProvider * attrProvider)
{
    VerifyOrReturn(mAttrProvider == nullptr);
    VerifyOrReturn(attrProvider != nullptr);

    mAttrProvider = attrProvider;

    // TODO: Consider moving the storage handling inside the Logic layer.
    // Get the current calendar type, if any.
    TimeFormatLocalization::CalendarTypeEnum calendarType;
    MutableByteSpan calendarBytes(reinterpret_cast<uint8_t *>(&calendarType), sizeof(calendarType));
    CHIP_ERROR error = mAttrProvider->ReadValue(
        { kRootEndpointId, TimeFormatLocalization::Id, TimeFormatLocalization::Attributes::ActiveCalendarType::Id }, calendarBytes);
    // We could have an invalid calendar type value if an OTA update removed support for the value we were using.
    // If initial value is not one of the allowed values, pick one valid value and write it.
    TimeFormatLocalization::CalendarTypeEnum validCalendar = TimeFormatLocalization::CalendarTypeEnum::kBuddhist;
    IsSupportedCalendarType(calendarType, &validCalendar);
    setActiveCalendarType(validCalendar);

    TimeFormatLocalization::HourFormatEnum hourFormat;
    MutableByteSpan hourBytes(reinterpret_cast<uint8_t *>(&hourFormat), sizeof(hourFormat));
    error = mAttrProvider->ReadValue(
        { kRootEndpointId, TimeFormatLocalization::Id, TimeFormatLocalization::Attributes::HourFormat::Id }, hourBytes);
    setHourFormat(hourFormat);
}

CHIP_ERROR TimeFormatLocalizationLogic::GetSupportedCalendarTypes(AttributeValueEncoder & aEncoder) const
{
    DeviceLayer::DeviceInfoProvider * provider = DeviceLayer::GetDeviceInfoProvider();
    VerifyOrReturnValue(provider != nullptr, aEncoder.EncodeEmptyList());

    AutoReleaseIterator it(provider->IterateSupportedCalendarTypes());
    VerifyOrReturnValue(it.IsValid(), aEncoder.EncodeEmptyList());

    return aEncoder.EncodeList([&it](const auto & encoder) -> CHIP_ERROR {
        TimeFormatLocalization::CalendarTypeEnum type;

        while (it.Next(type))
        {
            ReturnErrorOnFailure(encoder.Encode(type));
        }
        return CHIP_NO_ERROR;
    });
}

bool TimeFormatLocalizationLogic::IsSupportedCalendarType(TimeFormatLocalization::CalendarTypeEnum reqCalendar,
                                                          TimeFormatLocalization::CalendarTypeEnum * validCalendar)
{
    DeviceLayer::DeviceInfoProvider * provider = DeviceLayer::GetDeviceInfoProvider();
    VerifyOrReturnValue(provider != nullptr, false);

    AutoReleaseIterator it(provider->IterateSupportedCalendarTypes());
    VerifyOrReturnValue(it.IsValid(), false);

    TimeFormatLocalization::CalendarTypeEnum type;
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

TimeFormatLocalization::CalendarTypeEnum TimeFormatLocalizationLogic::GetActiveCalendarType()
{
    return mCalendarType;
}

DataModel::ActionReturnStatus TimeFormatLocalizationLogic::setHourFormat(TimeFormatLocalization::HourFormatEnum rHour)
{
    VerifyOrReturnValue(mAttrProvider != nullptr, Protocols::InteractionModel::Status::Failure);

    if (rHour == TimeFormatLocalization::HourFormatEnum::kUnknownEnumValue)
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }

    CHIP_ERROR result = mAttrProvider->WriteValue(
        { kRootEndpointId, TimeFormatLocalization::Id, TimeFormatLocalization::Attributes::HourFormat::Id },
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

TimeFormatLocalization::HourFormatEnum TimeFormatLocalizationLogic::GetHourFormat()
{
    return mHourFormat;
}

CHIP_ERROR TimeFormatLocalizationLogic::Attributes(ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    // Ensure capacity just in case
    ReturnErrorOnFailure(builder.EnsureAppendCapacity(3 + DefaultServerCluster::GlobalAttributes().size()));
    // Mandatory attributes
    ReturnErrorOnFailure(builder.Append(TimeFormatLocalization::Attributes::HourFormat::kMetadataEntry));

    // These attributes depend on the Feature CalendarFormat (CALFMT)
    if (mFeatures.Has(TimeFormatLocalization::Feature::kCalendarFormat))
    {
        ReturnErrorOnFailure(builder.Append(TimeFormatLocalization::Attributes::ActiveCalendarType::kMetadataEntry));
        ReturnErrorOnFailure(builder.Append(TimeFormatLocalization::Attributes::SupportedCalendarTypes::kMetadataEntry));
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
