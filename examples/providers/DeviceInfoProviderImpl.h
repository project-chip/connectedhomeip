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

// !!!!!!!!!!!!!!!!!!!!!!!! WARNING WARNING WARNING !!!!!!!!!!!!!!!!!!!!
// WARNING: DO NOT USE THESE DEFAULT IMPLEMENTATIONS WITH DEFAULT VALUES
// IN PRODUCTION PRODUCTS WITHOUT AUDITING THEM! See
// `AllClustersExampleDeviceInforProviderImpl.h` for an example provider
// that has constant values. Here, all providers have empty implementations
// to force empty lists which prevent bad values from leaking into products
// like happened before Matter 1.5. If you really are using these clusters,
// then please re-implement the provider as needed.
//
// The FixedLabel, LocalizationConfigurationand and Time Format localization
// clusters, if used, should have values that have been vetted
// for correctness in the product !!! DO NOT USE SAMPLE DEFAULTS IN PRODUCTS.
// !!!!!!!!!!!!!!!!!!!!!!!! WARNING WARNING WARNING !!!!!!!!!!!!!!!!!!!!

class DeviceInfoProviderImpl : public DeviceInfoProvider
{
public:
    DeviceInfoProviderImpl() = default;
    ~DeviceInfoProviderImpl() override {}

    // Iterators
    FixedLabelIterator * IterateFixedLabel(EndpointId endpoint) override;
    UserLabelIterator * IterateUserLabel(EndpointId endpoint) override;
    SupportedLocalesIterator * IterateSupportedLocales() override;
    SupportedCalendarTypesIterator * IterateSupportedCalendarTypes() override;

    static DeviceInfoProviderImpl & GetDefaultInstance();

protected:
    class UserLabelIteratorImpl : public UserLabelIterator
    {
    public:
        UserLabelIteratorImpl(DeviceInfoProviderImpl & provider, EndpointId endpoint);
        size_t Count() override { return mTotal; }
        bool Next(UserLabelType & output) override;
        void Release() override { chip::Platform::Delete(this); }

    private:
        DeviceInfoProviderImpl & mProvider;
        EndpointId mEndpoint = 0;
        size_t mIndex        = 0;
        size_t mTotal        = 0;
        char mUserLabelNameBuf[kMaxLabelNameLength + 1];
        char mUserLabelValueBuf[kMaxLabelValueLength + 1];
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
        size_t mIndex                                = 0;
    };

    class SupportedCalendarTypesIteratorImpl : public SupportedCalendarTypesIterator
    {
    public:
        SupportedCalendarTypesIteratorImpl() = default;
        size_t Count() override;
        bool Next(CalendarType & output) override;
        void Release() override { chip::Platform::Delete(this); }

    private:
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
