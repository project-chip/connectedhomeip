/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
    FixedLabelIterator * IterateFixedLabel(EndpointId endpoint) override;
    UserLabelIterator * IterateUserLabel(EndpointId endpoint) override;
    SupportedLocalesIterator * IterateSupportedLocales() override;
    SupportedCalendarTypesIterator * IterateSupportedCalendarTypes() override;

    static SampleDeviceProvider & GetDefaultInstance();

protected:
    class UserLabelIteratorImpl : public UserLabelIterator
    {
    public:
        UserLabelIteratorImpl(SampleDeviceProvider & provider, EndpointId endpoint);
        size_t Count() override { return mTotal; }
        bool Next(UserLabelType & output) override;
        void Release() override { chip::Platform::Delete(this); }

    private:
        size_t mTotal        = 0;
    };

    class SupportedLocalesIteratorImpl : public SupportedLocalesIterator
    {
    public:
        SupportedLocalesIteratorImpl() = default;
        size_t Count() override;
        bool Next(CharSpan & output) override;
        void Release() override { chip::Platform::Delete(this); }

    private:
        static constexpr size_t kNumSupportedLocales = 1;
    };

    class SupportedCalendarTypesIteratorImpl : public SupportedCalendarTypesIterator
    {
    public:
        SupportedCalendarTypesIteratorImpl() = default;
        size_t Count() override;
        bool Next(CalendarType & output) override;
        void Release() override { chip::Platform::Delete(this); }

    private:
        static constexpr size_t kNumSupportedCalendarTypes = 3;
        size_t mIndex                                      = 0;
    };

    CHIP_ERROR SetUserLabelLength(EndpointId endpoint, size_t val) override;
    CHIP_ERROR GetUserLabelLength(EndpointId endpoint, size_t & val) override;
    CHIP_ERROR SetUserLabelAt(EndpointId endpoint, size_t index, const UserLabelType & userLabel) override;
    CHIP_ERROR DeleteUserLabelAt(EndpointId endpoint, size_t index) override;

private:
    static constexpr size_t UserLabelTLVMaxSize() { return TLV::EstimateStructOverhead(kMaxLabelNameLength, kMaxLabelValueLength); }
};

} // namespace DeviceLayer
} // namespace chip
