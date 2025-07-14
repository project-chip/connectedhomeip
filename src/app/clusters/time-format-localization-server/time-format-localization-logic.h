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
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <app/AttributeValueEncoder.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <clusters/TimeFormatLocalization/Enums.h>
#include <lib/core/CHIPError.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <platform/DeviceInfoProvider.h>

namespace chip {
namespace app {
namespace Clusters {

class TimeFormatLocalizationLogic
{
public:
    TimeFormatLocalizationLogic(BitFlags<TimeFormatLocalization::Feature> features): mFeatures(features)
    {
        mHourFormat = TimeFormatLocalization::HourFormatEnum::k12hr;
        mCalendarType = TimeFormatLocalization::CalendarTypeEnum::kBuddhist;
    };
    virtual ~TimeFormatLocalizationLogic() = default;

    BitFlags<TimeFormatLocalization::Feature> GetFeatureMap() const;

    // Attributes handling for cluster
    TimeFormatLocalization::HourFormatEnum GetHourFormat();

    TimeFormatLocalization::CalendarTypeEnum GetActiveCalendarType();

    DataModel::ActionReturnStatus setHourFormat(TimeFormatLocalization::HourFormatEnum rHour);

    DataModel::ActionReturnStatus setActiveCalendarType(TimeFormatLocalization::CalendarTypeEnum rCalendar);

    CHIP_ERROR GetSupportedCalendarTypes(AttributeValueEncoder & aEncoder) const;

    CHIP_ERROR Attributes(ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder);

private:

    BitFlags<TimeFormatLocalization::Feature> mFeatures;

    TimeFormatLocalization::HourFormatEnum mHourFormat;
    TimeFormatLocalization::CalendarTypeEnum mCalendarType;

    bool IsCalendarSupported(TimeFormatLocalization::CalendarTypeEnum calendarType);

};

} // namespace Clusters
} // namespace app
} // namespace chip