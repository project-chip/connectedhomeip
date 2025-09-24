/*
 *
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
#pragma once

#include <lib/support/CodeUtils.h>
#include <lib/support/EnforceFormat.h>
#include <platform/DeviceInfoProvider.h>

namespace chip {
namespace DeviceLayer {

// TODO: Reduce this test case if possible since it is only used for the Calendars iterator
class SampleDeviceProvider : public DeviceInfoProvider
{
public:
    SampleDeviceProvider() = default;
    ~SampleDeviceProvider() override {}

    // Iterators
    FixedLabelIterator * IterateFixedLabel(EndpointId endpoint) override { return nullptr; };
    UserLabelIterator * IterateUserLabel(EndpointId endpoint) override { return nullptr; };
    SupportedLocalesIterator * IterateSupportedLocales() override { return nullptr; };
    SupportedCalendarTypesIterator * IterateSupportedCalendarTypes() override
    {
        return chip::Platform::New<SupportedCalendarTypesIteratorImpl>();
    }

protected:
    class UserLabelIteratorImpl : public UserLabelIterator
    {
    public:
        UserLabelIteratorImpl() = default;
        size_t Count() override { return 0; }
        bool Next(UserLabelType & output) override { return false; }
        void Release() override {}
    };

    class SupportedLocalesIteratorImpl : public SupportedLocalesIterator
    {
    public:
        SupportedLocalesIteratorImpl() = default;
        size_t Count() override { return 0; }
        bool Next(CharSpan & output) override { return false; }
        void Release() override {}
    };

    class SupportedCalendarTypesIteratorImpl : public SupportedCalendarTypesIterator
    {
    public:
        SupportedCalendarTypesIteratorImpl() = default;
        size_t Count() override { return kNumSupportedCalendarTypes; }
        bool Next(CalendarType & output) override
        {
            static const CalendarType kAllSupportedCalendarTypes[kNumSupportedCalendarTypes] = {
                app::Clusters::TimeFormatLocalization::CalendarTypeEnum::kGregorian,
                app::Clusters::TimeFormatLocalization::CalendarTypeEnum::kChinese,
                app::Clusters::TimeFormatLocalization::CalendarTypeEnum::kJapanese
            };

            VerifyOrReturnError(mIndex < kNumSupportedCalendarTypes, false);
            output = kAllSupportedCalendarTypes[mIndex];
            mIndex++;
            return true;
        };
        void Release() override { chip::Platform::Delete(this); }

    private:
        static constexpr size_t kNumSupportedCalendarTypes = 3;
        size_t mIndex                                      = 0;
    };

    CHIP_ERROR SetUserLabelLength(EndpointId endpoint, size_t val) override { return CHIP_NO_ERROR; };
    CHIP_ERROR GetUserLabelLength(EndpointId endpoint, size_t & val) override { return CHIP_NO_ERROR; };
    CHIP_ERROR SetUserLabelAt(EndpointId endpoint, size_t index, const UserLabelType & userLabel) override
    {
        return CHIP_NO_ERROR;
    };
    CHIP_ERROR DeleteUserLabelAt(EndpointId endpoint, size_t index) override { return CHIP_NO_ERROR; };
};

} // namespace DeviceLayer
} // namespace chip
