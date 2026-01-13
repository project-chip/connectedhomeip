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
#include <AllClustersExampleDeviceInfoProviderImpl.h>

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

AllClustersExampleDeviceInfoProviderImpl & AllClustersExampleDeviceInfoProviderImpl::GetDefaultInstance()
{
    static AllClustersExampleDeviceInfoProviderImpl sInstance;
    return sInstance;
}

// See below for an example of FixedLabel and UserLabel setter/iterator.
// WARNING: DO NOT USE THESE DEFAULT IMPLEMENTATIONS WITH DEFAULT VALUES
// IN PRODUCTION PRODUCTS WITHOUT AUDITING THEM!
//
// The FixedLabel cluster, if used, should have values that have been vetted
// for correctness in the product.
DeviceInfoProvider::FixedLabelIterator * AllClustersExampleDeviceInfoProviderImpl::IterateFixedLabel(EndpointId endpoint)
{
    return chip::Platform::New<FixedLabelIteratorImpl>(endpoint);
}

AllClustersExampleDeviceInfoProviderImpl::FixedLabelIteratorImpl::FixedLabelIteratorImpl(EndpointId endpoint) : mEndpoint(endpoint)
{
    mIndex = 0;
}

size_t AllClustersExampleDeviceInfoProviderImpl::FixedLabelIteratorImpl::Count()
{
    // A hardcoded labelList on all endpoints.
    return kNumSupportedFixedLabels;
}

bool AllClustersExampleDeviceInfoProviderImpl::FixedLabelIteratorImpl::Next(FixedLabelType & output)
{
    bool retval = true;

    // A hardcoded list for testing only
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError(mIndex < kNumSupportedFixedLabels, false);

    switch (mIndex)
    {
    case 0:
        output.label = "direction"_span;
        // Values likely should differ by endpoint. Consider adding storage
        // for the spans to point to correct buffer+size elements in case
        // you dynamically generate these instead of using fixed const char*-backed CharSpans.
        if (mEndpoint == 1)
        {
            output.value = "up"_span;
        }
        else
        {
            output.value = "down"_span;
        }
        break;
    default:
        err = CHIP_ERROR_INTERNAL;
        break;
    }

    if (err == CHIP_NO_ERROR)
    {
        mIndex++;
        retval = true;
    }
    else
    {
        retval = false;
    }

    return retval;
}

CHIP_ERROR AllClustersExampleDeviceInfoProviderImpl::SetUserLabelLength(EndpointId endpoint, size_t val)
{
    return mStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::UserLabelLengthKey(endpoint).KeyName(), &val,
                                     static_cast<uint16_t>(sizeof(val)));
}

CHIP_ERROR AllClustersExampleDeviceInfoProviderImpl::GetUserLabelLength(EndpointId endpoint, size_t & val)
{
    uint16_t len = static_cast<uint16_t>(sizeof(val));

    return mStorage->SyncGetKeyValue(DefaultStorageKeyAllocator::UserLabelLengthKey(endpoint).KeyName(), &val, len);
}

CHIP_ERROR AllClustersExampleDeviceInfoProviderImpl::SetUserLabelAt(EndpointId endpoint, size_t index,
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

CHIP_ERROR AllClustersExampleDeviceInfoProviderImpl::DeleteUserLabelAt(EndpointId endpoint, size_t index)
{
    return mStorage->SyncDeleteKeyValue(
        DefaultStorageKeyAllocator::UserLabelIndexKey(endpoint, static_cast<uint32_t>(index)).KeyName());
}

DeviceInfoProvider::UserLabelIterator * AllClustersExampleDeviceInfoProviderImpl::IterateUserLabel(EndpointId endpoint)
{
    return chip::Platform::New<UserLabelIteratorImpl>(*this, endpoint);
}

AllClustersExampleDeviceInfoProviderImpl::UserLabelIteratorImpl::UserLabelIteratorImpl(
    AllClustersExampleDeviceInfoProviderImpl & provider, EndpointId endpoint) :
    mProvider(provider),
    mEndpoint(endpoint)
{
    size_t total = 0;

    ReturnOnFailure(mProvider.GetUserLabelLength(mEndpoint, total));
    mTotal = total;
    mIndex = 0;
}

bool AllClustersExampleDeviceInfoProviderImpl::UserLabelIteratorImpl::Next(UserLabelType & output)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError(mIndex < mTotal, false);
    VerifyOrReturnError(CanCastTo<uint32_t>(mIndex), false);

    uint8_t buf[UserLabelTLVMaxSize()];
    uint16_t len = static_cast<uint16_t>(sizeof(buf));

    err = mProvider.mStorage->SyncGetKeyValue(
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

DeviceInfoProvider::SupportedLocalesIterator * AllClustersExampleDeviceInfoProviderImpl::IterateSupportedLocales()
{
    return chip::Platform::New<SupportedLocalesIteratorImpl>();
}

size_t AllClustersExampleDeviceInfoProviderImpl::SupportedLocalesIteratorImpl::Count()
{
    // Hardcoded list of locales
    // {("en-US"), ("de-DE"), ("fr-FR"), ("en-GB"), ("es-ES"), ("zh-CN"), ("it-IT"), ("ja-JP")}

    return kNumSupportedLocales;
}

bool AllClustersExampleDeviceInfoProviderImpl::SupportedLocalesIteratorImpl::Next(CharSpan & output)
{
    // Hardcoded list of locales
    static const char * kAllSupportedLocales[kNumSupportedLocales] = { "en-US", "de-DE", "fr-FR", "en-GB",
                                                                       "es-ES", "zh-CN", "it-IT", "ja-JP" };

    VerifyOrReturnError(mIndex < kNumSupportedLocales, false);
    output = CharSpan::fromCharString(kAllSupportedLocales[mIndex]);
    mIndex++;

    return true;
}

DeviceInfoProvider::SupportedCalendarTypesIterator * AllClustersExampleDeviceInfoProviderImpl::IterateSupportedCalendarTypes()
{
    return chip::Platform::New<SupportedCalendarTypesIteratorImpl>();
}

size_t AllClustersExampleDeviceInfoProviderImpl::SupportedCalendarTypesIteratorImpl::Count()
{
    // Hardcoded list of strings
    // {("kGregorian")}

    return kNumSupportedCalendarTypes;
}

bool AllClustersExampleDeviceInfoProviderImpl::SupportedCalendarTypesIteratorImpl::Next(CalendarType & output)
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
