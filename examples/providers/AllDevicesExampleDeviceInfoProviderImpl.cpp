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
#include <AllDevicesExampleDeviceInfoProviderImpl.h>

#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <stdlib.h>
#include <string.h>

#include <cstring>

namespace chip {
namespace DeviceLayer {

AllDevicesExampleDeviceInfoProviderImpl & AllDevicesExampleDeviceInfoProviderImpl::GetDefaultInstance()
{
    static AllDevicesExampleDeviceInfoProviderImpl sInstance;
    return sInstance;
}

DeviceInfoProvider::SupportedLocalesIterator * AllDevicesExampleDeviceInfoProviderImpl::IterateSupportedLocales()
{
    return chip::Platform::New<AllDevicesSupportedLocalesIteratorImpl>();
}

size_t AllDevicesExampleDeviceInfoProviderImpl::AllDevicesSupportedLocalesIteratorImpl::Count()
{
    // Hardcoded list of locales
    // {("en-US")}

    return kNumSupportedLocales;
}

bool AllDevicesExampleDeviceInfoProviderImpl::AllDevicesSupportedLocalesIteratorImpl::Next(CharSpan & output)
{
    // Hardcoded list of locales
    static const char * kAllSupportedLocales[kNumSupportedLocales] = { "en-US" };

    VerifyOrReturnError(mIndex < kNumSupportedLocales, false);
    output = CharSpan::fromCharString(kAllSupportedLocales[mIndex]);
    mIndex++;

    return true;
}

} // namespace DeviceLayer
} // namespace chip
