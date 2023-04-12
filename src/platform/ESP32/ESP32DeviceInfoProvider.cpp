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
#include <lib/core/CHIPTLV.h>
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
    return chip::Platform::New<FixedLabelIteratorImpl>(endpoint);
}

ESP32DeviceInfoProvider::FixedLabelIteratorImpl::FixedLabelIteratorImpl(EndpointId endpoint) : mEndpoint(endpoint)
{
    mIndex = 0;
}

size_t ESP32DeviceInfoProvider::FixedLabelIteratorImpl::Count()
{
    char keyBuf[ESP32Config::kMaxConfigKeyNameLength];
    uint32_t count = 0;

    VerifyOrReturnValue(ESP32Config::KeyAllocator::FixedLabelCount(keyBuf, sizeof(keyBuf), mEndpoint) == CHIP_NO_ERROR, 0);
    ESP32Config::Key key(ESP32Config::kConfigNamespace_ChipFactory, keyBuf);
    VerifyOrReturnValue(ESP32Config::ReadConfigValue(key, count) == CHIP_NO_ERROR, 0);
    return count;
}

bool ESP32DeviceInfoProvider::FixedLabelIteratorImpl::Next(FixedLabelType & output)
{
    ChipLogDetail(DeviceLayer, "Get the fixed label with index:%u at endpoint:%d", static_cast<unsigned>(mIndex), mEndpoint);

    char keyBuf[ESP32Config::kMaxConfigKeyNameLength];
    size_t keyOutLen   = 0;
    size_t valueOutLen = 0;

    memset(mFixedLabelNameBuf, 0, sizeof(mFixedLabelNameBuf));
    memset(mFixedLabelValueBuf, 0, sizeof(mFixedLabelValueBuf));

    VerifyOrReturnValue(ESP32Config::KeyAllocator::FixedLabelKey(keyBuf, sizeof(keyBuf), mEndpoint, mIndex) == CHIP_NO_ERROR,
                        false);
    ESP32Config::Key keyKey(ESP32Config::kConfigNamespace_ChipFactory, keyBuf);
    VerifyOrReturnValue(
        ESP32Config::ReadConfigValueStr(keyKey, mFixedLabelNameBuf, sizeof(mFixedLabelNameBuf), keyOutLen) == CHIP_NO_ERROR, false);

    VerifyOrReturnValue(ESP32Config::KeyAllocator::FixedLabelValue(keyBuf, sizeof(keyBuf), mEndpoint, mIndex) == CHIP_NO_ERROR,
                        false);
    ESP32Config::Key valueKey(ESP32Config::kConfigNamespace_ChipFactory, keyBuf);
    VerifyOrReturnValue(ESP32Config::ReadConfigValueStr(valueKey, mFixedLabelValueBuf, sizeof(mFixedLabelValueBuf), valueOutLen) ==
                            CHIP_NO_ERROR,
                        false);

    output.label = CharSpan::fromCharString(mFixedLabelNameBuf);
    output.value = CharSpan::fromCharString(mFixedLabelValueBuf);
    ChipLogDetail(DeviceLayer, "Fixed label with index:%u at endpoint:%d, %s:%s", static_cast<unsigned>(mIndex), mEndpoint,
                  mFixedLabelNameBuf, mFixedLabelValueBuf);

    mIndex++;
    return true;
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
    return chip::Platform::New<SupportedLocalesIteratorImpl>();
}

size_t ESP32DeviceInfoProvider::SupportedLocalesIteratorImpl::Count()
{
    uint32_t count = 0;
    CHIP_ERROR err = ESP32Config::ReadConfigValue(ESP32Config::kConfigKey_SupportedLocaleSize, count);
    if (err != CHIP_NO_ERROR)
    {
        return 0;
    }
    return count;
}

bool ESP32DeviceInfoProvider::SupportedLocalesIteratorImpl::Next(CharSpan & output)
{
    char keyBuf[ESP32Config::kMaxConfigKeyNameLength];
    size_t keyOutLen = 0;
    memset(mLocaleBuf, 0, sizeof(mLocaleBuf));

    VerifyOrReturnValue(ESP32Config::KeyAllocator::Locale(keyBuf, sizeof(keyBuf), mIndex) == CHIP_NO_ERROR, false);
    ESP32Config::Key keyKey(ESP32Config::kConfigNamespace_ChipFactory, keyBuf);
    VerifyOrReturnValue(ESP32Config::ReadConfigValueStr(keyKey, mLocaleBuf, sizeof(mLocaleBuf), keyOutLen) == CHIP_NO_ERROR, false);

    output = CharSpan::fromCharString(mLocaleBuf);
    mIndex++;
    return true;
}

void ESP32DeviceInfoProvider::SupportedLocalesIteratorImpl::Release()
{
    chip::Platform::Delete(this);
}

DeviceInfoProvider::SupportedCalendarTypesIterator * ESP32DeviceInfoProvider::IterateSupportedCalendarTypes()
{
    return chip::Platform::New<SupportedCalendarTypesIteratorImpl>();
}

ESP32DeviceInfoProvider::SupportedCalendarTypesIteratorImpl::SupportedCalendarTypesIteratorImpl()
{
    CHIP_ERROR err = ESP32Config::ReadConfigValue(ESP32Config::kConfigKey_SupportedCalTypes, mSupportedCalendarTypes);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to read supported calendar types: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

size_t ESP32DeviceInfoProvider::SupportedCalendarTypesIteratorImpl::Count()
{
    size_t count = 0;
    for (uint8_t i = 0; i < to_underlying(CalendarType::kUnknownEnumValue); i++)
    {
        if (mSupportedCalendarTypes & (1 << i))
        {
            count++;
        }
    }
    ChipLogDetail(DeviceLayer, "Supported calendar types count:%u", count);
    return count;
}

bool ESP32DeviceInfoProvider::SupportedCalendarTypesIteratorImpl::Next(CalendarType & output)
{
    while (mIndex < to_underlying(CalendarType::kUnknownEnumValue))
    {
        if (mSupportedCalendarTypes & (1 << mIndex))
        {
            output = static_cast<CalendarType>(mIndex);
            mIndex++;
            return true;
        }
        mIndex++;
    }
    return false;
}

} // namespace DeviceLayer
} // namespace chip
