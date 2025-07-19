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
#include "SampleDeviceProvider.h"

#include <lib/core/TLV.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/SafeInt.h>
#include <lib/support/Span.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <stdlib.h>
#include <string.h>

#include <cstring>

namespace chip {
namespace DeviceLayer {

SampleDeviceProvider & SampleDeviceProvider::GetDefaultInstance()
{
    static SampleDeviceProvider sInstance;
    return sInstance;
}

// !!!!!!!!!!!!!!!!!!!!!!!! WARNING WARNING WARNING !!!!!!!!!!!!!!!!!!!!
// WARNING: DO NOT USE THESE DEFAULT IMPLEMENTATIONS WITH DEFAULT VALUES
// IN PRODUCTION PRODUCTS WITHOUT AUDITING THEM! See
// `AllClustersExampleDeviceInforProviderImpl.cpp` for an example provider
// that has constant values. Here, all providers have empty implementations
// to force empty lists which prevent bad values from leaking into products
// like happened before Matter 1.5. If you really are using these clusters,
// then please re-implement the provider as needed.
//
// The FixedLabel, LocalizationConfigurationand and Time Format localization
// clusters, if used, should have values that have been vetted
// for correctness in the product !!! DO NOT USE SAMPLE DEFAULTS IN PRODUCTS.
// !!!!!!!!!!!!!!!!!!!!!!!! WARNING WARNING WARNING !!!!!!!!!!!!!!!!!!!!

DeviceInfoProvider::FixedLabelIterator * SampleDeviceProvider::IterateFixedLabel(EndpointId endpoint)
{
    // We don't include fixed label data in this sample one. Returning nullptr returns empty list.
    (void) endpoint;
    return nullptr;
}

CHIP_ERROR SampleDeviceProvider::SetUserLabelLength(EndpointId endpoint, size_t val)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR SampleDeviceProvider::GetUserLabelLength(EndpointId endpoint, size_t & val)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR SampleDeviceProvider::SetUserLabelAt(EndpointId endpoint, size_t index, const UserLabelType & userLabel)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR SampleDeviceProvider::DeleteUserLabelAt(EndpointId endpoint, size_t index)
{
    return mStorage->SyncDeleteKeyValue(
        DefaultStorageKeyAllocator::UserLabelIndexKey(endpoint, static_cast<uint32_t>(index)).KeyName());
}

DeviceInfoProvider::UserLabelIterator * SampleDeviceProvider::IterateUserLabel(EndpointId endpoint)
{
    return chip::Platform::New<UserLabelIteratorImpl>(*this, endpoint);
}

SampleDeviceProvider::UserLabelIteratorImpl::UserLabelIteratorImpl(SampleDeviceProvider & provider, EndpointId endpoint)
{

}

bool SampleDeviceProvider::UserLabelIteratorImpl::Next(UserLabelType & output)
{
    return true;
}

DeviceInfoProvider::SupportedLocalesIterator * SampleDeviceProvider::IterateSupportedLocales()
{
    return chip::Platform::New<SupportedLocalesIteratorImpl>();
}

size_t SampleDeviceProvider::SupportedLocalesIteratorImpl::Count()
{
    return kNumSupportedLocales;
}

bool SampleDeviceProvider::SupportedLocalesIteratorImpl::Next(CharSpan & output)
{
    return true;
}

DeviceInfoProvider::SupportedCalendarTypesIterator * SampleDeviceProvider::IterateSupportedCalendarTypes()
{
    return chip::Platform::New<SupportedCalendarTypesIteratorImpl>();
}

size_t SampleDeviceProvider::SupportedCalendarTypesIteratorImpl::Count()
{

    return kNumSupportedCalendarTypes;
}

bool SampleDeviceProvider::SupportedCalendarTypesIteratorImpl::Next(CalendarType & output)
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
}

} // namespace DeviceLayer
} // namespace chip
