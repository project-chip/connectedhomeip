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

#include <clusters/TimeFormatLocalization/Metadata.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/EventLogging.h>
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

}
CHIP_ERROR TimeFormatLocalizationLogic::GetSupportedCalendarTypes(AttributeValueEncoder & aEncoder) const 
{
    DeviceLayer::DeviceInfoProvider * provider = DeviceLayer::GetDeviceInfoProvider();
    VerifyOrReturnValue(provider != nullptr, aEncoder.EncodeEmptyList());

    AutoReleaseIterator it(provider->IterateSupportedCalendarTypes());
    VerifyOrReturnValue(it.IsValid(), aEncoder.EncodeEmptyList());

    return aEncoder.EncodeList([&it](const auto & encoder) -> CHIP_ERROR {
        TimeFormatLocalization::CalendarTypeEnum type;

        while(it.Next(type))
        {
            ReturnErrorOnFailure(encoder.Encode(type));
        }
        return CHIP_NO_ERROR;
    });
}

bool TimeFormatLocalizationLogic::IsCalendarSupported(TimeFormatLocalization::CalendarTypeEnum calendar)
{
    DeviceLayer::DeviceInfoProvider * provider = DeviceLayer::GetDeviceInfoProvider();
    VerifyOrReturnValue(provider != nullptr, false);

    AutoReleaseIterator it(provider->IterateSupportedCalendarTypes());
    VerifyOrReturnValue(it.IsValid(), false);

    TimeFormatLocalization::CalendarTypeEnum type;

    while (it.Next(type))
    {
        if(type == calendar) 
        {
            return true;
        }
    }
    return false;
}

TimeFormatLocalization::CalendarTypeEnum TimeFormatLocalizationLogic::GetActiveCalendarType() 
{
    return mCalendarType;
}
// TODO: Missing persisten storage for Writable attributes, also should check here for a valid value for HourFormat and CalendarType?
DataModel::ActionReturnStatus TimeFormatLocalizationLogic::setHourFormat(TimeFormatLocalization::HourFormatEnum rHour)
{
    mHourFormat = rHour;
    return Protocols::InteractionModel::Status::Success;
}

DataModel::ActionReturnStatus TimeFormatLocalizationLogic::setActiveCalendarType(TimeFormatLocalization::CalendarTypeEnum rCalendar)
{
    // TODO: Confirm error values for this operation.
    if(!mFeatures.Has(TimeFormatLocalization::Feature::kCalendarFormat))
    {
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }

    if(IsCalendarSupported(rCalendar))
    {
        mCalendarType = rCalendar;
        return Protocols::InteractionModel::Status::Success;
    }

    return Protocols::InteractionModel::Status::ConstraintError;
    
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
    if(mFeatures.Has(TimeFormatLocalization::Feature::kCalendarFormat))
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
