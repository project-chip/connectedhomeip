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

#include "AllDevicesExampleDeviceInfoProviderImpl.h"

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

namespace {
constexpr TLV::Tag kLabelNameTag  = TLV::ContextTag(0);
constexpr TLV::Tag kLabelValueTag = TLV::ContextTag(1);
} // anonymous namespace

DeviceInfoProvider::FixedLabelIterator * AllDevicesExampleDeviceInfoProviderImpl::IterateFixedLabel(EndpointId endpoint)
{
    return FixedLabelIteratorImpl::Create(endpoint);
}

AllDevicesExampleDeviceInfoProviderImpl::FixedLabelIteratorImpl::FixedLabelIteratorImpl(EndpointId endpoint) : mEndpoint(endpoint)
{
    mIndex = 0;
}

size_t AllDevicesExampleDeviceInfoProviderImpl::FixedLabelIteratorImpl::Count()
{
    return kNumSupportedFixedLabels;
}

bool AllDevicesExampleDeviceInfoProviderImpl::FixedLabelIteratorImpl::Next(FixedLabelType & output)
{
    VerifyOrReturnError(mIndex < kNumSupportedFixedLabels, false);

    output.label = "direction"_span;
    if (mEndpoint == 1)
    {
        output.value = "up"_span;
    }
    else
    {
        output.value = "down"_span;
    }

    mIndex++;
    return true;
}

CHIP_ERROR AllDevicesExampleDeviceInfoProviderImpl::SetUserLabelLength(EndpointId endpoint, size_t val)
{
    return mStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::UserLabelLengthKey(endpoint).KeyName(), &val,
                                     static_cast<uint16_t>(sizeof(val)));
}

CHIP_ERROR AllDevicesExampleDeviceInfoProviderImpl::GetUserLabelLength(EndpointId endpoint, size_t & val)
{
    uint16_t len = static_cast<uint16_t>(sizeof(val));
    return mStorage->SyncGetKeyValue(DefaultStorageKeyAllocator::UserLabelLengthKey(endpoint).KeyName(), &val, len);
}

CHIP_ERROR AllDevicesExampleDeviceInfoProviderImpl::SetUserLabelAt(EndpointId endpoint, size_t index,
                                                                   const UserLabelType & userLabel)
{
    VerifyOrReturnError(CanCastTo<uint32_t>(index), CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t buf[UserLabelTLVMaxSize()];
    TLV::TLVWriter writer;
    writer.Init(buf);

    TLV::TLVType outerType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
    ReturnErrorOnFailure(writer.PutString(kLabelNameTag, userLabel.label));
    ReturnErrorOnFailure(writer.PutString(kLabelValueTag, userLabel.value));
    ReturnErrorOnFailure(writer.EndContainer(outerType));

    return mStorage->SyncSetKeyValue(
        DefaultStorageKeyAllocator::UserLabelIndexKey(endpoint, static_cast<uint32_t>(index)).KeyName(), buf,
        static_cast<uint16_t>(writer.GetLengthWritten()));
}

CHIP_ERROR AllDevicesExampleDeviceInfoProviderImpl::DeleteUserLabelAt(EndpointId endpoint, size_t index)
{
    return mStorage->SyncDeleteKeyValue(
        DefaultStorageKeyAllocator::UserLabelIndexKey(endpoint, static_cast<uint32_t>(index)).KeyName());
}

DeviceInfoProvider::UserLabelIterator * AllDevicesExampleDeviceInfoProviderImpl::IterateUserLabel(EndpointId endpoint)
{
    return UserLabelIteratorImpl::Create(*this, endpoint);
}

AllDevicesExampleDeviceInfoProviderImpl::UserLabelIteratorImpl::UserLabelIteratorImpl(
    AllDevicesExampleDeviceInfoProviderImpl & provider, EndpointId endpoint) :
    mProvider(provider),
    mEndpoint(endpoint)
{
    size_t total = 0;
    ReturnOnFailure(mProvider.GetUserLabelLength(mEndpoint, total));
    mTotal = total;
    mIndex = 0;
}

bool AllDevicesExampleDeviceInfoProviderImpl::UserLabelIteratorImpl::Next(UserLabelType & output)
{
    VerifyOrReturnError(mIndex < mTotal, false);
    VerifyOrReturnError(CanCastTo<uint32_t>(mIndex), false);

    uint8_t buf[UserLabelTLVMaxSize()];
    uint16_t len = static_cast<uint16_t>(sizeof(buf));

    CHIP_ERROR err = mProvider.mStorage->SyncGetKeyValue(
        DefaultStorageKeyAllocator::UserLabelIndexKey(mEndpoint, static_cast<uint32_t>(mIndex)).KeyName(), buf, len);
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

DeviceInfoProvider::SupportedLocalesIterator * AllDevicesExampleDeviceInfoProviderImpl::IterateSupportedLocales()
{
    return SupportedLocalesIteratorImpl::Create();
}

size_t AllDevicesExampleDeviceInfoProviderImpl::SupportedLocalesIteratorImpl::Count()
{
    return kNumSupportedLocales;
}

bool AllDevicesExampleDeviceInfoProviderImpl::SupportedLocalesIteratorImpl::Next(CharSpan & output)
{
    static const char * kAllSupportedLocales[kNumSupportedLocales] = { "en-US" };

    VerifyOrReturnError(mIndex < kNumSupportedLocales, false);
    output = CharSpan::fromCharString(kAllSupportedLocales[mIndex]);
    mIndex++;

    return true;
}

DeviceInfoProvider::SupportedCalendarTypesIterator * AllDevicesExampleDeviceInfoProviderImpl::IterateSupportedCalendarTypes()
{
    return SupportedCalendarTypesIteratorImpl::Create();
}

size_t AllDevicesExampleDeviceInfoProviderImpl::SupportedCalendarTypesIteratorImpl::Count()
{
    return kNumSupportedCalendarTypes;
}

bool AllDevicesExampleDeviceInfoProviderImpl::SupportedCalendarTypesIteratorImpl::Next(CalendarType & output)
{
    static const CalendarType kAllSupportedCalendarTypes[kNumSupportedCalendarTypes] = {
        app::Clusters::TimeFormatLocalization::CalendarTypeEnum::kGregorian
    };

    VerifyOrReturnError(mIndex < kNumSupportedCalendarTypes, false);
    output = kAllSupportedCalendarTypes[mIndex];
    mIndex++;
    return true;
}

} // namespace DeviceLayer
} // namespace chip
