/*

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
#include <lib/core/TLV.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <platform/ESP32/ESP32Config.h>
#include <platform/ESP32/ESP32DeviceInfoProvider.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <string.h>

namespace chip {
namespace DeviceLayer {

namespace {
constexpr TLV::Tag kLabelNameTag  = TLV::ContextTag(0);
constexpr TLV::Tag kLabelValueTag = TLV::ContextTag(1);
} // anonymous namespace

using namespace Internal;

ESP32DeviceInfoProvider & ESP32DeviceInfoProvider::GetDefaultInstance(void)
{
    static ESP32DeviceInfoProvider sInstance;
    return sInstance;
}

DeviceInfoProvider::FixedLabelIterator * ESP32DeviceInfoProvider::IterateFixedLabel(EndpointId endpoint)
{
    return chip::Platform::New<FixedLabelIteratorImpl>(endpoint, mFixedLabels);
}

ESP32DeviceInfoProvider::FixedLabelIteratorImpl::FixedLabelIteratorImpl(EndpointId endpoint, const Span<FixedLabelEntry> & labels) :
    mEndpoint(endpoint), mLabels(labels)
{
    mIndex = 0;
}

size_t ESP32DeviceInfoProvider::FixedLabelIteratorImpl::Count()
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

bool ESP32DeviceInfoProvider::FixedLabelIteratorImpl::Next(FixedLabelType & output)
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

CHIP_ERROR ESP32DeviceInfoProvider::SetUserLabelLength(EndpointId endpoint, size_t val)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return mStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::UserLabelLengthKey(endpoint).KeyName(), &val,
                                     static_cast<uint16_t>(sizeof(val)));
}

CHIP_ERROR ESP32DeviceInfoProvider::GetUserLabelLength(EndpointId endpoint, size_t & val)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    uint16_t len = static_cast<uint16_t>(sizeof(val));
    return mStorage->SyncGetKeyValue(DefaultStorageKeyAllocator::UserLabelLengthKey(endpoint).KeyName(), &val, len);
}

CHIP_ERROR ESP32DeviceInfoProvider::SetUserLabelAt(EndpointId endpoint, size_t index, const UserLabelType & userLabel)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    uint8_t buf[UserLabelTLVMaxSize()];
    TLV::TLVWriter writer;
    writer.Init(buf);

    TLV::TLVType outerType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
    ReturnErrorOnFailure(writer.PutString(kLabelNameTag, userLabel.label));
    ReturnErrorOnFailure(writer.PutString(kLabelValueTag, userLabel.value));
    ReturnErrorOnFailure(writer.EndContainer(outerType));

    return mStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::UserLabelIndexKey(endpoint, index).KeyName(), buf,
                                     static_cast<uint16_t>(writer.GetLengthWritten()));
}

CHIP_ERROR ESP32DeviceInfoProvider::DeleteUserLabelAt(EndpointId endpoint, size_t index)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return mStorage->SyncDeleteKeyValue(DefaultStorageKeyAllocator::UserLabelIndexKey(endpoint, index).KeyName());
}

DeviceInfoProvider::UserLabelIterator * ESP32DeviceInfoProvider::IterateUserLabel(EndpointId endpoint)
{
    return chip::Platform::New<UserLabelIteratorImpl>(*this, endpoint);
}

ESP32DeviceInfoProvider::UserLabelIteratorImpl::UserLabelIteratorImpl(ESP32DeviceInfoProvider & provider, EndpointId endpoint) :
    mProvider(provider), mEndpoint(endpoint)
{
    size_t total = 0;

    ReturnOnFailure(mProvider.GetUserLabelLength(mEndpoint, total));
    mTotal = total;
    mIndex = 0;
}

bool ESP32DeviceInfoProvider::UserLabelIteratorImpl::Next(UserLabelType & output)
{
    VerifyOrReturnError(mProvider.mStorage != nullptr, false);
    VerifyOrReturnError(mIndex < mTotal, false);

    uint8_t buf[UserLabelTLVMaxSize()];
    uint16_t len = static_cast<uint16_t>(sizeof(buf));

    CHIP_ERROR err =
        mProvider.mStorage->SyncGetKeyValue(DefaultStorageKeyAllocator::UserLabelIndexKey(mEndpoint, mIndex).KeyName(), buf, len);
    VerifyOrReturnError(err == CHIP_NO_ERROR, false);

    TLV::ContiguousBufferTLVReader reader;
    reader.Init(buf);
    err = reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag());
    VerifyOrReturnError(err == CHIP_NO_ERROR, false);

    TLV::TLVType containerType;
    VerifyOrReturnError(reader.EnterContainer(containerType) == CHIP_NO_ERROR, false);

    chip::CharSpan label;
    chip::CharSpan value;

    VerifyOrReturnError(reader.Next(kLabelNameTag) == CHIP_NO_ERROR, false);
    VerifyOrReturnError(reader.Get(label) == CHIP_NO_ERROR, false);

    VerifyOrReturnError(reader.Next(kLabelValueTag) == CHIP_NO_ERROR, false);
    VerifyOrReturnError(reader.Get(value) == CHIP_NO_ERROR, false);

    VerifyOrReturnError(reader.VerifyEndOfContainer() == CHIP_NO_ERROR, false);
    VerifyOrReturnError(reader.ExitContainer(containerType) == CHIP_NO_ERROR, false);

    Platform::CopyString(mUserLabelNameBuf, label);
    Platform::CopyString(mUserLabelValueBuf, value);

    output.label = CharSpan::fromCharString(mUserLabelNameBuf);
    output.value = CharSpan::fromCharString(mUserLabelValueBuf);

    mIndex++;

    return true;
}

DeviceInfoProvider::SupportedLocalesIterator * ESP32DeviceInfoProvider::IterateSupportedLocales()
{
    return chip::Platform::New<SupportedLocalesIteratorImpl>(mSupportedLocales);
}

ESP32DeviceInfoProvider::SupportedLocalesIteratorImpl::SupportedLocalesIteratorImpl(const Span<CharSpan> & locales)
{
    mLocales = locales;
}

size_t ESP32DeviceInfoProvider::SupportedLocalesIteratorImpl::Count()
{
    return mLocales.empty() ? 0 : mLocales.size();
}

bool ESP32DeviceInfoProvider::SupportedLocalesIteratorImpl::Next(CharSpan & output)
{
    VerifyOrReturnValue(mIndex < mLocales.size(), false);
    output = mLocales.data()[mIndex++];
    return true;
}

DeviceInfoProvider::SupportedCalendarTypesIterator * ESP32DeviceInfoProvider::IterateSupportedCalendarTypes()
{
    return chip::Platform::New<SupportedCalendarTypesIteratorImpl>(mSupportedCalendarTypes);
}

ESP32DeviceInfoProvider::SupportedCalendarTypesIteratorImpl::SupportedCalendarTypesIteratorImpl(
    const Span<CalendarType> & calendarTypes)
{
    mCalendarTypes = calendarTypes;
}

size_t ESP32DeviceInfoProvider::SupportedCalendarTypesIteratorImpl::Count()
{
    return mCalendarTypes.empty() ? 0 : mCalendarTypes.size();
}

bool ESP32DeviceInfoProvider::SupportedCalendarTypesIteratorImpl::Next(CalendarType & output)
{
    VerifyOrReturnValue(mIndex < mCalendarTypes.size(), false);
    output = mCalendarTypes.data()[mIndex++];
    return true;
}

} // namespace DeviceLayer
} // namespace chip
