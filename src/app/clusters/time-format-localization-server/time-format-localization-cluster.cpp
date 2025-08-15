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

#include <app/clusters/time-format-localization-server/time-format-localization-cluster.h>
#include <app/persistence/AttributePersistence.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/TimeFormatLocalization/Metadata.h>
#include <platform/DeviceInfoProvider.h>
#include <tracing/macros.h>

namespace chip {
namespace app {
namespace Clusters {

namespace {
constexpr DataModel::AttributeEntry kMandatoryAttributes[] = {
    TimeFormatLocalization::Attributes::HourFormat::kMetadataEntry,
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
    bool Next(TimeFormatLocalization::CalendarTypeEnum & value) { return (mIterator == nullptr) ? false : mIterator->Next(value); }

private:
    Iterator * mIterator;
};

bool IsSupportedCalendarType(TimeFormatLocalization::CalendarTypeEnum reqCalendar, TimeFormatLocalization::CalendarTypeEnum * validCalendar = nullptr)
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

} // namespace

TimeFormatLocalizationCluster::TimeFormatLocalizationCluster(EndpointId endpointId,
                                                             BitFlags<TimeFormatLocalization::Feature> features,
                                                             TimeFormatLocalization::HourFormatEnum defaultHourFormat, 
                                                             TimeFormatLocalization::CalendarTypeEnum defaultCalendarType) :
    DefaultServerCluster({ endpointId, TimeFormatLocalization::Id }),
    mFeatures(features), mHourFormat(defaultHourFormat), mCalendarType(defaultCalendarType)
{}

CHIP_ERROR TimeFormatLocalizationCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    AttributePersistence attrPersistence{ context.attributeStorage };

    // Store default values before attempting to load from persistence
    TimeFormatLocalization::CalendarTypeEnum defaultCalendarType = mCalendarType;
    TimeFormatLocalization::HourFormatEnum defaultHourFormat = mHourFormat;

    // Initialize the CalendarType if available
    attrPersistence.LoadNativeEndianValue<TimeFormatLocalization::CalendarTypeEnum>(
        { kRootEndpointId, TimeFormatLocalization::Id, TimeFormatLocalization::Attributes::ActiveCalendarType::Id }, mCalendarType,
        defaultCalendarType);

    // We could have an invalid calendar type value if an OTA update removed support for the value we were using.
    // If initial value is not one of the allowed values, pick one valid value and write it.
    TimeFormatLocalization::CalendarTypeEnum validCalendar = defaultCalendarType;
    if (!IsSupportedCalendarType(mCalendarType, &validCalendar))
    {
        mCalendarType = validCalendar;
    }

    // Initialize HourFormat
    attrPersistence.LoadNativeEndianValue<TimeFormatLocalization::HourFormatEnum>(
        { kRootEndpointId, TimeFormatLocalization::Id, TimeFormatLocalization::Attributes::HourFormat::Id }, mHourFormat,
        defaultHourFormat);


    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus TimeFormatLocalizationCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                            AttributeValueDecoder & decoder)
{
    return NotifyAttributeChangedIfSuccess(request.path.mAttributeId, WriteImpl(request, decoder));
}

DataModel::ActionReturnStatus TimeFormatLocalizationCluster::WriteImpl(const DataModel::WriteAttributeRequest & request, AttributeValueDecoder & decoder)
{
    AttributePersistence persistence { mContext->attributeStorage };

    switch (request.path.mAttributeId)
    {
    case TimeFormatLocalization::Attributes::HourFormat::Id: {
        return persistence.DecodeAndStoreNativeEndianValue(request.path, decoder, mHourFormat);
    }
    case TimeFormatLocalization::Attributes::ActiveCalendarType::Id: {
        TimeFormatLocalization::CalendarTypeEnum newCalendar;
        ReturnErrorOnFailure(decoder.Decode(newCalendar));

        if (!IsSupportedCalendarType(newCalendar))
        {
            return Protocols::InteractionModel::Status::ConstraintError;
        }

         mCalendarType = newCalendar;

        // Using WriteAttribute directly so we can check that the decoded value is in the supported list
        // before storing it.
        return mContext->attributeStorage.WriteValue({ kRootEndpointId, TimeFormatLocalization::Id, TimeFormatLocalization::Attributes::ActiveCalendarType::Id },
                                                    { reinterpret_cast<const uint8_t *>(&mCalendarType), sizeof(mCalendarType) });
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedWrite;
    }
}

DataModel::ActionReturnStatus TimeFormatLocalizationCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                           AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case TimeFormatLocalization::Attributes::HourFormat::Id:
        return encoder.Encode(mHourFormat);

    case TimeFormatLocalization::Attributes::ActiveCalendarType::Id:
        return encoder.Encode(mCalendarType);

    case TimeFormatLocalization::Attributes::SupportedCalendarTypes::Id:
        return GetSupportedCalendarTypes(encoder);

    case TimeFormatLocalization::Attributes::FeatureMap::Id:
        return encoder.Encode(mFeatures);

    case TimeFormatLocalization::Attributes::ClusterRevision::Id:
        return encoder.Encode(TimeFormatLocalization::kRevision);

    default:
        return Protocols::InteractionModel::Status::UnreportableAttribute;
    }
}

CHIP_ERROR TimeFormatLocalizationCluster::Attributes(const ConcreteClusterPath & path,
                                                     ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);
    

    const DataModel::AttributeEntry optionalAttributes[] = {
        TimeFormatLocalization::Attributes::ActiveCalendarType::kMetadataEntry,
        TimeFormatLocalization::Attributes::SupportedCalendarTypes::kMetadataEntry
    };

    chip::app::OptionalAttributeSet<TimeFormatLocalization::Attributes::ActiveCalendarType::Id,
                                    TimeFormatLocalization::Attributes::SupportedCalendarTypes::Id> optionalAttributeSet;

    if(mFeatures.Has(TimeFormatLocalization::Feature::kCalendarFormat)) 
    {
        optionalAttributeSet.Set<TimeFormatLocalization::Attributes::ActiveCalendarType::Id>();
        optionalAttributeSet.Set<TimeFormatLocalization::Attributes::SupportedCalendarTypes::Id>();
    }

    return listBuilder.Append(Span(kMandatoryAttributes), Span(optionalAttributes), optionalAttributeSet);
}


CHIP_ERROR TimeFormatLocalizationCluster::GetSupportedCalendarTypes(AttributeValueEncoder & aEncoder) const
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

} // namespace Clusters
} // namespace app
} // namespace chip
