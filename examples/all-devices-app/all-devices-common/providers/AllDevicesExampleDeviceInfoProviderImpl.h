/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <lib/core/DataModelTypes.h>
#include <lib/support/CHIPMem.h>
#include <platform/DeviceInfoProvider.h>

namespace chip {
namespace DeviceLayer {

class AllDevicesExampleDeviceInfoProviderImpl : public DeviceInfoProvider
{
public:
    AllDevicesExampleDeviceInfoProviderImpl() = default;
    ~AllDevicesExampleDeviceInfoProviderImpl() override {}

    // Iterators
    FixedLabelIterator * IterateFixedLabel(EndpointId endpoint) override;
    UserLabelIterator * IterateUserLabel(EndpointId endpoint) override;
    SupportedLocalesIterator * IterateSupportedLocales() override;
    SupportedCalendarTypesIterator * IterateSupportedCalendarTypes() override;

protected:
    class FixedLabelIteratorImpl : public FixedLabelIterator
    {
    public:
        static FixedLabelIteratorImpl * Create(EndpointId endpoint)
        {
            return chip::Platform::New<FixedLabelIteratorImpl>(endpoint);
        }
        size_t Count() override;
        bool Next(FixedLabelType & output) override;
        void Release() override { chip::Platform::Delete(this); }

    private:
        template <typename U, typename... UArgs>
        friend U * chip::Platform::New(UArgs &&... args);

        FixedLabelIteratorImpl(EndpointId endpoint);
        static constexpr size_t kNumSupportedFixedLabels = 1;
        EndpointId mEndpoint                             = 0;
        size_t mIndex                                    = 0;
    };

    class UserLabelIteratorImpl : public UserLabelIterator
    {
    public:
        static UserLabelIteratorImpl * Create(AllDevicesExampleDeviceInfoProviderImpl & provider, EndpointId endpoint)
        {
            return chip::Platform::New<UserLabelIteratorImpl>(provider, endpoint);
        }
        size_t Count() override { return mTotal; }
        bool Next(UserLabelType & output) override;
        void Release() override { chip::Platform::Delete(this); }

    private:
        template <typename U, typename... UArgs>
        friend U * chip::Platform::New(UArgs &&... args);

        UserLabelIteratorImpl(AllDevicesExampleDeviceInfoProviderImpl & provider, EndpointId endpoint);
        AllDevicesExampleDeviceInfoProviderImpl & mProvider;
        EndpointId mEndpoint = 0;
        size_t mIndex        = 0;
        size_t mTotal        = 0;
        char mUserLabelNameBuf[kMaxLabelNameLength + 1];
        char mUserLabelValueBuf[kMaxLabelValueLength + 1];
    };

    class SupportedLocalesIteratorImpl : public SupportedLocalesIterator
    {
    public:
        static SupportedLocalesIteratorImpl * Create() { return chip::Platform::New<SupportedLocalesIteratorImpl>(); }
        size_t Count() override;
        bool Next(CharSpan & output) override;
        void Release() override { chip::Platform::Delete(this); }

    private:
        template <typename U, typename... UArgs>
        friend U * chip::Platform::New(UArgs &&... args);

        SupportedLocalesIteratorImpl()               = default;
        static constexpr size_t kNumSupportedLocales = 1;
        size_t mIndex                                = 0;
    };

    class SupportedCalendarTypesIteratorImpl : public SupportedCalendarTypesIterator
    {
    public:
        static SupportedCalendarTypesIteratorImpl * Create() { return chip::Platform::New<SupportedCalendarTypesIteratorImpl>(); }
        size_t Count() override;
        bool Next(CalendarType & output) override;
        void Release() override { chip::Platform::Delete(this); }

    private:
        template <typename U, typename... UArgs>
        friend U * chip::Platform::New(UArgs &&... args);

        SupportedCalendarTypesIteratorImpl()               = default;
        static constexpr size_t kNumSupportedCalendarTypes = 1;
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
