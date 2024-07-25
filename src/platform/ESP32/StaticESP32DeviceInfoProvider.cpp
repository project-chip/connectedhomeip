/*

 *    Copyright (c) 2024 Project CHIP Authors
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
#include <lib/support/CodeUtils.h>
#include <platform/ESP32/StaticESP32DeviceInfoProvider.h>

namespace chip {
namespace DeviceLayer {

StaticESP32DeviceInfoProvider & StaticESP32DeviceInfoProvider::GetDefaultInstance(void)
{
    static StaticESP32DeviceInfoProvider sInstance;
    return sInstance;
}

DeviceInfoProvider::FixedLabelIterator * StaticESP32DeviceInfoProvider::IterateFixedLabel(EndpointId endpoint)
{
    return chip::Platform::New<StaticFixedLabelIteratorImpl>(endpoint, mFixedLabels);
}

StaticESP32DeviceInfoProvider::StaticFixedLabelIteratorImpl::StaticFixedLabelIteratorImpl(EndpointId endpoint,
                                                                                          const Span<FixedLabelEntry> & labels)
{
    mEndpoint = endpoint;
    mLabels   = labels;
    mIndex    = 0;
}

size_t StaticESP32DeviceInfoProvider::StaticFixedLabelIteratorImpl::Count()
{
    size_t count = 0;
    for (size_t i = 0; i < mLabels.size(); i++)
    {
        const FixedLabelEntry & entry = mLabels.data()[i];

        if (entry.endpointId == mEndpoint)
        {
            count++;
        }
    }
    return count;
}

bool StaticESP32DeviceInfoProvider::StaticFixedLabelIteratorImpl::Next(FixedLabelType & output)
{
    ChipLogDetail(DeviceLayer, "Get the fixed label with index:%u at endpoint:%d", static_cast<unsigned>(mIndex), mEndpoint);

    while (mIndex < mLabels.size())
    {
        const FixedLabelEntry & entry = mLabels.data()[mIndex++];
        if (entry.endpointId == mEndpoint)
        {
            output.label = entry.label;
            output.value = entry.value;
            return true;
        }
    }

    return false;
}

DeviceInfoProvider::SupportedLocalesIterator * StaticESP32DeviceInfoProvider::IterateSupportedLocales()
{
    return chip::Platform::New<StaticSupportedLocalesIteratorImpl>(mSupportedLocales);
}

StaticESP32DeviceInfoProvider::StaticSupportedLocalesIteratorImpl::StaticSupportedLocalesIteratorImpl(
    const Span<CharSpan> & locales)
{
    mLocales = locales;
}

size_t StaticESP32DeviceInfoProvider::StaticSupportedLocalesIteratorImpl::Count()
{
    return mLocales.empty() ? 0 : mLocales.size();
}

bool StaticESP32DeviceInfoProvider::StaticSupportedLocalesIteratorImpl::Next(CharSpan & output)
{
    VerifyOrReturnValue(mIndex < mLocales.size(), false);
    output = mLocales.data()[mIndex++];
    return true;
}

DeviceInfoProvider::SupportedCalendarTypesIterator * StaticESP32DeviceInfoProvider::IterateSupportedCalendarTypes()
{
    return chip::Platform::New<StaticSupportedCalendarTypesIteratorImpl>(mSupportedCalendarTypes);
}

StaticESP32DeviceInfoProvider::StaticSupportedCalendarTypesIteratorImpl::StaticSupportedCalendarTypesIteratorImpl(
    const Span<CalendarType> & calendarTypes)
{
    mCalendarTypes = calendarTypes;
}

size_t StaticESP32DeviceInfoProvider::StaticSupportedCalendarTypesIteratorImpl::Count()
{
    return mCalendarTypes.empty() ? 0 : mCalendarTypes.size();
}

bool StaticESP32DeviceInfoProvider::StaticSupportedCalendarTypesIteratorImpl::Next(CalendarType & output)
{
    VerifyOrReturnValue(mIndex < mCalendarTypes.size(), false);
    output = mCalendarTypes.data()[mIndex++];
    return true;
}

} // namespace DeviceLayer
} // namespace chip
