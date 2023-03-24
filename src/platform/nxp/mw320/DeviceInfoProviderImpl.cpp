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

#include <lib/core/TLV.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/nxp/mw320/DeviceInfoProviderImpl.h>

#include <stdlib.h>
#include <string.h>

namespace chip {
namespace DeviceLayer {

namespace {
constexpr TLV::Tag kLabelNameTag  = TLV::ContextTag(0);
constexpr TLV::Tag kLabelValueTag = TLV::ContextTag(1);
} // anonymous namespace

DeviceInfoProviderImpl & DeviceInfoProviderImpl::GetDefaultInstance()
{
    static DeviceInfoProviderImpl sInstance;
    return sInstance;
}

DeviceInfoProvider::FixedLabelIterator * DeviceInfoProviderImpl::IterateFixedLabel(EndpointId endpoint)
{
    return new FixedLabelIteratorImpl(endpoint);
}

DeviceInfoProviderImpl::FixedLabelIteratorImpl::FixedLabelIteratorImpl(EndpointId endpoint) : mEndpoint(endpoint)
{
    mIndex = 0;
}

size_t DeviceInfoProviderImpl::FixedLabelIteratorImpl::Count()
{
    // TODO: Need to provide a script which can generate a binary file which contains device information and
    // update the DeviceInfoProvider which can read the information from it.
    // Now we use the hardcoded labellist.
    return 4;
}

bool DeviceInfoProviderImpl::FixedLabelIteratorImpl::Next(FixedLabelType & output)
{
    bool retval = true;

    // TODO: Need to provide a script which can generate a binary file which contains device information and
    // update the DeviceInfoProvider which can read the information from it.
    // Now we use the hardcoded labellist.
    CHIP_ERROR err = CHIP_NO_ERROR;

    const char * labelPtr = nullptr;
    const char * valuePtr = nullptr;

    VerifyOrReturnError(mIndex < 4, false);

    ChipLogProgress(DeviceLayer, "Get the fixed label with index:%d at endpoint:%d", mIndex, mEndpoint);

    switch (mIndex)
    {
    case 0:
        labelPtr = "room";
        valuePtr = "bedroom 2";
        break;
    case 1:
        labelPtr = "orientation";
        valuePtr = "North";
        break;
    case 2:
        labelPtr = "floor";
        valuePtr = "2";
        break;
    case 3:
        labelPtr = "direction";
        valuePtr = "up";
        break;
    default:
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
        break;
    }

    if (err == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(strlen(labelPtr) <= kMaxLabelNameLength, false);
        VerifyOrReturnError(strlen(valuePtr) <= kMaxLabelValueLength, false);

        Platform::CopyString(mFixedLabelNameBuf, kMaxLabelNameLength + 1, labelPtr);
        Platform::CopyString(mFixedLabelValueBuf, kMaxLabelValueLength + 1, valuePtr);

        output.label = CharSpan::fromCharString(mFixedLabelNameBuf);
        output.value = CharSpan::fromCharString(mFixedLabelValueBuf);

        mIndex++;

        retval = true;
    }
    else
    {
        retval = false;
    }

    return retval;
}

CHIP_ERROR DeviceInfoProviderImpl::SetUserLabelLength(EndpointId endpoint, size_t val)
{
    return mStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::UserLabelLengthKey(endpoint).KeyName(), &val,
                                     static_cast<uint16_t>(sizeof(val)));
}

CHIP_ERROR DeviceInfoProviderImpl::GetUserLabelLength(EndpointId endpoint, size_t & val)
{
    uint16_t len = static_cast<uint16_t>(sizeof(val));

    return mStorage->SyncGetKeyValue(DefaultStorageKeyAllocator::UserLabelLengthKey(endpoint).KeyName(), &val, len);
}

CHIP_ERROR DeviceInfoProviderImpl::SetUserLabelAt(EndpointId endpoint, size_t index, const UserLabelType & userLabel)
{
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

CHIP_ERROR DeviceInfoProviderImpl::DeleteUserLabelAt(EndpointId endpoint, size_t index)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return mStorage->SyncDeleteKeyValue(DefaultStorageKeyAllocator::UserLabelIndexKey(endpoint, index).KeyName());
}

DeviceInfoProvider::UserLabelIterator * DeviceInfoProviderImpl::IterateUserLabel(EndpointId endpoint)
{
    return new UserLabelIteratorImpl(*this, endpoint);
}

DeviceInfoProviderImpl::UserLabelIteratorImpl::UserLabelIteratorImpl(DeviceInfoProviderImpl & provider, EndpointId endpoint) :
    mProvider(provider), mEndpoint(endpoint)
{
    size_t total = 0;

    ReturnOnFailure(mProvider.GetUserLabelLength(mEndpoint, total));
    mTotal = total;
    mIndex = 0;
}

bool DeviceInfoProviderImpl::UserLabelIteratorImpl::Next(UserLabelType & output)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError(mIndex < mTotal, false);

    uint8_t buf[UserLabelTLVMaxSize()];
    uint16_t len = static_cast<uint16_t>(sizeof(buf));

    err = mProvider.mStorage->SyncGetKeyValue(DefaultStorageKeyAllocator::UserLabelIndexKey(mEndpoint, mIndex).KeyName(), buf, len);
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

DeviceInfoProvider::SupportedLocalesIterator * DeviceInfoProviderImpl::IterateSupportedLocales()
{
    return new SupportedLocalesIteratorImpl();
}

size_t DeviceInfoProviderImpl::SupportedLocalesIteratorImpl::Count()
{
    // TODO: Need to provide a script which can generate a binary file which contains device information and
    // update the DeviceInfoProvider which can read the information from it.
    // Now we use the hardcoded SupportedLocales.
    // {("en-US"), ("de-DE"), ("fr-FR"), ("en-GB"), ("es-ES"), ("zh-CN"), ("it-IT"), ("ja-JP")}

    return 8;
}

bool DeviceInfoProviderImpl::SupportedLocalesIteratorImpl::Next(CharSpan & output)
{
    bool retval = true;

    // TODO: Need to provide a script which can generate a binary file which contains device information and
    // update the DeviceInfoProvider which can read the information from it.
    // Now we use the hardcoded SupportedLocales.
    CHIP_ERROR err = CHIP_NO_ERROR;

    const char * activeLocalePtr = nullptr;

    VerifyOrReturnError(mIndex < 8, false);

    switch (mIndex)
    {
    case 0:
        activeLocalePtr = "en-US";
        break;
    case 1:
        activeLocalePtr = "de-DE";
        break;
    case 2:
        activeLocalePtr = "fr-FR";
        break;
    case 3:
        activeLocalePtr = "en-GB";
        break;
    case 4:
        activeLocalePtr = "es-ES";
        break;
    case 5:
        activeLocalePtr = "zh-CN";
        break;
    case 6:
        activeLocalePtr = "it-IT";
        break;
    case 7:
        activeLocalePtr = "ja-JP";
        break;
    default:
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
        break;
    }

    if (err == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(strlen(activeLocalePtr) <= kMaxActiveLocaleLength, false);

        Platform::CopyString(mActiveLocaleBuf, kMaxActiveLocaleLength + 1, activeLocalePtr);

        output = CharSpan::fromCharString(mActiveLocaleBuf);

        mIndex++;

        retval = true;
    }
    else
    {
        retval = false;
    }

    return retval;
}

DeviceInfoProvider::SupportedCalendarTypesIterator * DeviceInfoProviderImpl::IterateSupportedCalendarTypes()
{
    return new SupportedCalendarTypesIteratorImpl();
}

size_t DeviceInfoProviderImpl::SupportedCalendarTypesIteratorImpl::Count()
{
    // TODO: Need to provide a script which can generate a binary file which contains device information and
    // update the DeviceInfoProvider which can read the information from it.
    // Now we use the hardcoded SupportedCalendarTypes.
    // {("kBuddhist"), ("kChinese"), ("kCoptic"), ("kEthiopian"), ("kGregorian"), ("kHebrew"), ("kIndian"), ("kJapanese"),
    //  ("kKorean"), ("kPersian"), ("kTaiwanese"), ("kIslamic")}

    return 12;
}

bool DeviceInfoProviderImpl::SupportedCalendarTypesIteratorImpl::Next(CalendarType & output)
{
    bool retval = true;

    // TODO: Need to provide a script which can generate a binary file which contains device information and
    // update the DeviceInfoProvider which can read the information from it.
    // Now we use the hardcoded SupportedCalendarTypes.
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError(mIndex < 12, false);

    switch (mIndex)
    {
    case 0:
        output = app::Clusters::TimeFormatLocalization::CalendarTypeEnum::kBuddhist;
        break;
    case 1:
        output = app::Clusters::TimeFormatLocalization::CalendarTypeEnum::kChinese;
        break;
    case 2:
        output = app::Clusters::TimeFormatLocalization::CalendarTypeEnum::kCoptic;
        break;
    case 3:
        output = app::Clusters::TimeFormatLocalization::CalendarTypeEnum::kEthiopian;
        break;
    case 4:
        output = app::Clusters::TimeFormatLocalization::CalendarTypeEnum::kGregorian;
        break;
    case 5:
        output = app::Clusters::TimeFormatLocalization::CalendarTypeEnum::kHebrew;
        break;
    case 6:
        output = app::Clusters::TimeFormatLocalization::CalendarTypeEnum::kIndian;
        break;
    case 7:
        output = app::Clusters::TimeFormatLocalization::CalendarTypeEnum::kJapanese;
        break;
    case 8:
        output = app::Clusters::TimeFormatLocalization::CalendarTypeEnum::kKorean;
        break;
    case 9:
        output = app::Clusters::TimeFormatLocalization::CalendarTypeEnum::kPersian;
        break;
    case 10:
        output = app::Clusters::TimeFormatLocalization::CalendarTypeEnum::kTaiwanese;
        break;
    case 11:
        output = app::Clusters::TimeFormatLocalization::CalendarTypeEnum::kIslamic;
        break;
    default:
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
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

} // namespace DeviceLayer
} // namespace chip
