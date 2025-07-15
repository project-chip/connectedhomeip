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

#include "time-format-localization-cluster.h"
#include <clusters/TimeFormatLocalization/Metadata.h>
#include <tracing/macros.h>

namespace chip {
namespace app {
namespace Clusters {

TimeFormatLocalizationCluster::TimeFormatLocalizationCluster(EndpointId endpointId, BitFlags<TimeFormatLocalization::Feature> features) :
DefaultServerCluster ({endpointId, TimeFormatLocalization::Id}), mLogic(features) { }

CHIP_ERROR TimeFormatLocalizationCluster::Startup(ServerClusterContext & context) 
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    TimeFormatLocalization::CalendarTypeEnum calendarType;

    MutableByteSpan calendarBytes(reinterpret_cast<uint8_t *>(&calendarType), sizeof(calendarType));

    CHIP_ERROR error = context.attributeStorage->ReadValue({kRootEndpointId, TimeFormatLocalization::Id, TimeFormatLocalization::Attributes::ActiveCalendarType::Id},
                                                            calendarBytes);
    ChipLogError(Zcl, "Got this message: %" CHIP_ERROR_FORMAT, error.Format());
    mLogic.setActiveCalendarType(calendarType);

    TimeFormatLocalization::HourFormatEnum hourFormat;

    MutableByteSpan hourBytes(reinterpret_cast<uint8_t *>(&hourFormat), sizeof(hourFormat));

    error = context.attributeStorage->ReadValue({kRootEndpointId, TimeFormatLocalization::Id, TimeFormatLocalization::Attributes::HourFormat::Id},
                                                            hourBytes);
    ChipLogError(Zcl, "Got this message: %" CHIP_ERROR_FORMAT, error.Format());
    mLogic.setHourFormat(hourFormat);

    if(error == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        return CHIP_NO_ERROR;
    }

    return error;
}

DataModel::ActionReturnStatus TimeFormatLocalizationCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request, AttributeValueDecoder & decoder)
{
    switch(request.path.mAttributeId)
    {
    case TimeFormatLocalization::Attributes::ActiveCalendarType::Id:
    {
        TimeFormatLocalization::CalendarTypeEnum tCalendar;
        ReturnErrorOnFailure(decoder.Decode(tCalendar));
        CHIP_ERROR result = mContext->attributeStorage->WriteValue({kRootEndpointId, TimeFormatLocalization::Id, TimeFormatLocalization::Attributes::ActiveCalendarType::Id},
        {reinterpret_cast<const uint8_t *>(&tCalendar), sizeof(tCalendar)});
        if(result == CHIP_NO_ERROR)
        {
            NotifyAttributeChanged(TimeFormatLocalization::Attributes::ActiveCalendarType::Id);
            mLogic.setActiveCalendarType(tCalendar);
            return Protocols::InteractionModel::Status::Success;
        }
        return Protocols::InteractionModel::Status::Failure;
        //return mLogic.setActiveCalendarType(tCalendar);
    }
    case TimeFormatLocalization::Attributes::HourFormat::Id:
    {
        TimeFormatLocalization::HourFormatEnum tHour;
        ReturnErrorOnFailure(decoder.Decode(tHour));
        CHIP_ERROR result = mContext->attributeStorage->WriteValue({kRootEndpointId, TimeFormatLocalization::Id, TimeFormatLocalization::Attributes::HourFormat::Id},
        {reinterpret_cast<const uint8_t *>(&tHour), sizeof(tHour)});
        if(result == CHIP_NO_ERROR)
        {
            NotifyAttributeChanged(TimeFormatLocalization::Attributes::HourFormat::Id);
            mLogic.setHourFormat(tHour);
            return Protocols::InteractionModel::Status::Success;
        }
        return Protocols::InteractionModel::Status::Failure;
    }
    default:
    {
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
    }
}

DataModel::ActionReturnStatus TimeFormatLocalizationCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request, AttributeValueEncoder & encoder)
{
    switch(request.path.mAttributeId)
    {
    case TimeFormatLocalization::Attributes::HourFormat::Id:
    {
        return encoder.Encode(mLogic.GetHourFormat());
    }
    case TimeFormatLocalization::Attributes::ActiveCalendarType::Id:
    {
        return encoder.Encode(mLogic.GetActiveCalendarType());
    }
    case TimeFormatLocalization::Attributes::SupportedCalendarTypes::Id:
    {
        return mLogic.GetSupportedCalendarTypes(encoder);
    }
    case TimeFormatLocalization::Attributes::FeatureMap::Id:
    {
        return encoder.Encode(mLogic.GetFeatureMap().Raw());
    }
    case TimeFormatLocalization::Attributes::ClusterRevision::Id:
    {
        return encoder.Encode(TimeFormatLocalization::kRevision);
    }
    default:
        return Protocols::InteractionModel::Status::UnreportableAttribute;
    }
}

CHIP_ERROR TimeFormatLocalizationCluster::Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) 
{
    return mLogic.Attributes(builder);
}

} // namespace Clusters
} // namespace app
} // namespace chip