/*
 *    Copyright (c) 2021-2025 Project CHIP Authors
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

#include <app/clusters/time-format-localization-server/TimeFormatLocalizationCluster.h>
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
class AutoReleaseIterator
{
public:
    using Iterator = DeviceLayer::DeviceInfoProvider::SupportedCalendarTypesIterator;

    explicit AutoReleaseIterator(DeviceLayer::DeviceInfoProvider * provider) :
        mIterator(provider != nullptr ? provider->IterateSupportedCalendarTypes() : nullptr)
    {}
    ~AutoReleaseIterator()
    {
        if (mIterator != nullptr)
        {
            mIterator->Release();
        }
    }

    // Delete copy constructor and assignment
    AutoReleaseIterator(const AutoReleaseIterator &)             = delete;
    AutoReleaseIterator & operator=(const AutoReleaseIterator &) = delete;

    bool IsValid() const { return mIterator != nullptr; }
    bool Next(TimeFormatLocalization::CalendarTypeEnum & value) { return (mIterator == nullptr) ? false : mIterator->Next(value); }

private:
    Iterator * mIterator;
};

bool IsSupportedCalendarType(TimeFormatLocalization::CalendarTypeEnum reqCalendar,
                             TimeFormatLocalization::CalendarTypeEnum * aValidCalendar = nullptr)
{
    AutoReleaseIterator it(DeviceLayer::GetDeviceInfoProvider());
    VerifyOrReturnValue(it.IsValid(), false);
    TimeFormatLocalization::CalendarTypeEnum type;

    while (it.Next(type))
    {
        // During the first loop, set some valid calendar value if requested
        if (aValidCalendar != nullptr)
        {
            *aValidCalendar = type;
            aValidCalendar  = nullptr;
        }
        if (type == reqCalendar)
        {
            return true;
        }
    }

    return false;
}

CHIP_ERROR GetSupportedCalendarTypes(AttributeValueEncoder & aEncoder)
{
    AutoReleaseIterator it(DeviceLayer::GetDeviceInfoProvider());
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

} // namespace

TimeFormatLocalizationCluster::TimeFormatLocalizationCluster(EndpointId endpointId,
                                                             BitFlags<TimeFormatLocalization::Feature> features,
                                                             TimeFormatLocalization::HourFormatEnum defaultHourFormat,
                                                             TimeFormatLocalization::CalendarTypeEnum defaultCalendarType) :
    DefaultServerCluster({ endpointId, TimeFormatLocalization::Id }),
    mFeatures(features), mHourFormat(defaultHourFormat), mCalendarType(defaultCalendarType)
{
    TimeFormatLocalization::CalendarTypeEnum validCalendar = defaultCalendarType;

    // Enforce a valid calendar. Historically codegen zap configuration default to 0
    // which is kBudhist calendar and is not typically supported. In that case
    // this selects a valid calendar (usually Gregorian).
    if (!IsSupportedCalendarType(defaultCalendarType, &validCalendar))
    {
        mCalendarType = validCalendar;
    }
}

CHIP_ERROR TimeFormatLocalizationCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    AttributePersistence attrPersistence{ context.attributeStorage };

    const TimeFormatLocalization::CalendarTypeEnum defaultCalendarType = mCalendarType;
    if (attrPersistence.LoadNativeEndianValue<TimeFormatLocalization::CalendarTypeEnum>(
            { mPath.mEndpointId, TimeFormatLocalization::Id, TimeFormatLocalization::Attributes::ActiveCalendarType::Id },
            mCalendarType, defaultCalendarType))
    {
        if (!IsSupportedCalendarType(mCalendarType))
        {
            mCalendarType = defaultCalendarType; // reset to the default if we fail to load
        }
    }

    const TimeFormatLocalization::HourFormatEnum defaultHourFormat = mHourFormat;
    attrPersistence.LoadNativeEndianValue<TimeFormatLocalization::HourFormatEnum>(
        { mPath.mEndpointId, TimeFormatLocalization::Id, TimeFormatLocalization::Attributes::HourFormat::Id }, mHourFormat,
        defaultHourFormat);

    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus TimeFormatLocalizationCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                            AttributeValueDecoder & decoder)
{
    return NotifyAttributeChangedIfSuccess(request.path.mAttributeId, WriteImpl(request, decoder));
}

DataModel::ActionReturnStatus TimeFormatLocalizationCluster::WriteImpl(const DataModel::WriteAttributeRequest & request,
                                                                       AttributeValueDecoder & decoder)
{
    // Use a couple of if statements instead of a switch case to try to reduce memory footprint.

    if (request.path.mAttributeId == TimeFormatLocalization::Attributes::HourFormat::Id)
    {
        AttributePersistence persistence{ mContext->attributeStorage };
        return persistence.DecodeAndStoreNativeEndianValue(request.path, decoder, mHourFormat);
    }

    if (request.path.mAttributeId == TimeFormatLocalization::Attributes::ActiveCalendarType::Id)
    {
        TimeFormatLocalization::CalendarTypeEnum newCalendar;
        ReturnErrorOnFailure(decoder.Decode(newCalendar));

        if (!IsSupportedCalendarType(newCalendar))
        {
            return Protocols::InteractionModel::Status::ConstraintError;
        }

        mCalendarType = newCalendar;

        // Using WriteValue directly so we can check that the decoded value is in the supported list
        // before storing it.
        return mContext->attributeStorage.WriteValue(request.path,
                                                     { reinterpret_cast<const uint8_t *>(&mCalendarType), sizeof(mCalendarType) });
    }

    return Protocols::InteractionModel::Status::UnsupportedWrite;
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
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
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

    OptionalAttributeSet<TimeFormatLocalization::Attributes::ActiveCalendarType::Id,
                         TimeFormatLocalization::Attributes::SupportedCalendarTypes::Id>
        optionalAttributeSet;

    if (mFeatures.Has(TimeFormatLocalization::Feature::kCalendarFormat))
    {
        optionalAttributeSet.Set<TimeFormatLocalization::Attributes::ActiveCalendarType::Id>();
        optionalAttributeSet.Set<TimeFormatLocalization::Attributes::SupportedCalendarTypes::Id>();
    }

    return listBuilder.Append(Span(TimeFormatLocalization::Attributes::kMandatoryMetadata), Span(optionalAttributes),
                              optionalAttributeSet);
}

} // namespace Clusters
} // namespace app
} // namespace chip
