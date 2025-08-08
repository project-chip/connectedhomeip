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

#include <app/AttributeValueDecoder.h>
#include <app/AttributeValueEncoder.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model-provider/OperationTypes.h>
#include <app/persistence/AttributePersistenceProvider.h>
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
    static constexpr TimeFormatLocalization::CalendarTypeEnum kDefaultCalendarType = TimeFormatLocalization::CalendarTypeEnum::kBuddhist;
    static constexpr TimeFormatLocalization::HourFormatEnum kDefaultHourFormat = TimeFormatLocalization::HourFormatEnum::k12hr;
    static constexpr size_t kMaxExpectedAttributeCount = 3;

    TimeFormatLocalizationLogic(BitFlags<TimeFormatLocalization::Feature> features) :
        mFeatures(features),
        mHourFormat(kDefaultHourFormat),
        mCalendarType(kDefaultCalendarType) {};
    virtual ~TimeFormatLocalizationLogic() = default;

    void Startup(AttributePersistenceProvider * attrStorage);

    BitFlags<TimeFormatLocalization::Feature> GetFeatureMap() const;

    // Attributes handling for cluster
    TimeFormatLocalization::HourFormatEnum GetHourFormat() const;
    TimeFormatLocalization::CalendarTypeEnum GetActiveCalendarType() const;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                               AttributePersistenceProvider * attrProvider,
                                               AttributeValueDecoder & decoder);
    CHIP_ERROR GetSupportedCalendarTypes(AttributeValueEncoder & aEncoder) const;
    CHIP_ERROR Attributes(ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) const;

private:
    // Method used to validate if the requested calendar is valid, optionally can return a valid calendar from the
    // supported calendars list.
    bool IsSupportedCalendarType(TimeFormatLocalization::CalendarTypeEnum reqCalendar,
                                 TimeFormatLocalization::CalendarTypeEnum * validCalendar = nullptr);

    void InitializeCalendarType(AttributePersistenceProvider * attrProvider);
    void InitializeHourFormat(AttributePersistenceProvider * attrProvider);

    BitFlags<TimeFormatLocalization::Feature> mFeatures;
    TimeFormatLocalization::HourFormatEnum mHourFormat;
    TimeFormatLocalization::CalendarTypeEnum mCalendarType;
};

} // namespace Clusters
} // namespace app
} // namespace chip
