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
#include <app/persistence/AttributePersistence.h>
#include <app/server-cluster/AttributeListBuilder.h>

using namespace chip::app::Clusters::TimeFormatLocalization;
using namespace chip::app::Clusters::TimeFormatLocalization::Attributes;

namespace chip {
namespace app {
namespace Clusters {

namespace {
constexpr DataModel::AttributeEntry kMandatoryAttributes[] = {
    HourFormat::kMetadataEntry,
};
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

} // namespace

void TimeFormatLocalizationLogic::Startup(AttributePersistenceProvider * attrProvider)
{
    VerifyOrReturn(attrProvider != nullptr);

    InitializeCalendarType(attrProvider);
    InitializeHourFormat(attrProvider);
}

void TimeFormatLocalizationLogic::InitializeCalendarType(AttributePersistenceProvider * attrProvider)
{
    VerifyOrReturn(attrProvider != nullptr);

    // Try to read existing calendar type from persistence
    AttributePersistence attrPersistence {*attrProvider};

    attrPersistence.LoadNativeEndianValue<CalendarTypeEnum>({ kRootEndpointId, TimeFormatLocalization::Id, 
        TimeFormatLocalization::Attributes::ActiveCalendarType::Id }, mCalendarType, kDefaultCalendarType);

    ChipLogError(Zcl, "Initial value for Calendar: %d", (int)mCalendarType);

    // We could have an invalid calendar type value if an OTA update removed support for the value we were using.
    // If initial value is not one of the allowed values, pick one valid value and write it.
    CalendarTypeEnum validCalendar = kDefaultCalendarType;
    if (!IsSupportedCalendarType(mCalendarType, &validCalendar))
    {
        mCalendarType = validCalendar;
    }

}

void TimeFormatLocalizationLogic::InitializeHourFormat(AttributePersistenceProvider * attrProvider)
{
    VerifyOrReturn(attrProvider != nullptr);
    AttributePersistence attrPersistence {*attrProvider};

    attrPersistence.LoadNativeEndianValue<HourFormatEnum>({ kRootEndpointId, TimeFormatLocalization::Id, 
        TimeFormatLocalization::Attributes::HourFormat::Id }, mHourFormat, kDefaultHourFormat);

    ChipLogError(Zcl, "Initial value for Hour: %d", (int)mHourFormat);

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

DataModel::ActionReturnStatus TimeFormatLocalizationLogic::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                               AttributePersistenceProvider * attrProvider,
                                                                               AttributeValueDecoder & decoder)
{
    VerifyOrReturnValue(attrProvider != nullptr, Protocols::InteractionModel::Status::Failure);
    AttributePersistence attrPersistence {*attrProvider};

    switch (request.path.mAttributeId)
    {
    case TimeFormatLocalization::Attributes::HourFormat::Id: {
        CHIP_ERROR result = attrPersistence.DecodeAndStoreNativeEndianValue(
            { kRootEndpointId, TimeFormatLocalization::Id, HourFormat::Id }, decoder, mHourFormat);
        return result == CHIP_NO_ERROR ? Protocols::InteractionModel::Status::Success 
                                     : Protocols::InteractionModel::Status::ConstraintError;
    }

    case TimeFormatLocalization::Attributes::ActiveCalendarType::Id: {
        if (!mFeatures.Has(TimeFormatLocalization::Feature::kCalendarFormat))
        {
            return Protocols::InteractionModel::Status::UnsupportedAttribute;
        }

        TimeFormatLocalization::CalendarTypeEnum newCalendar;
        ReturnErrorOnFailure(decoder.Decode(newCalendar));

        if (!IsSupportedCalendarType(newCalendar))
        {
            return Protocols::InteractionModel::Status::ConstraintError;
        }

        CHIP_ERROR result = attrPersistence.DecodeAndStoreNativeEndianValue(
            { kRootEndpointId, TimeFormatLocalization::Id, ActiveCalendarType::Id }, decoder, mCalendarType);

        return result == CHIP_NO_ERROR ? Protocols::InteractionModel::Status::Success 
                                     : Protocols::InteractionModel::Status::WriteIgnored;
    }

    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

TimeFormatLocalization::HourFormatEnum TimeFormatLocalizationLogic::GetHourFormat() const
{
    return mHourFormat;
}

CHIP_ERROR TimeFormatLocalizationLogic::Attributes(ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) const
{
    AttributeListBuilder listBuilder(builder);

    const AttributeListBuilder::OptionalAttributeEntry optionalAttributeEntries[] = {
        {mFeatures.Has(Feature::kCalendarFormat), ActiveCalendarType::kMetadataEntry},
        {mFeatures.Has(Feature::kCalendarFormat), SupportedCalendarTypes::kMetadataEntry}
    };

    return listBuilder.Append(Span(kMandatoryAttributes), Span(optionalAttributeEntries));
}

BitFlags<TimeFormatLocalization::Feature> TimeFormatLocalizationLogic::GetFeatureMap() const
{
    return mFeatures;
}

} // namespace Clusters
} // namespace app
} // namespace chip
