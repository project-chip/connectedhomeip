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

#include <lib/core/CHIPTLV.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <platform/Linux/DeviceInfoProviderImpl.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <stdlib.h>
#include <string.h>

namespace chip {
namespace DeviceLayer {

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
    // In Linux Simulation, return the size of the hardcoded labelList on all endpoints.
    return 4;
}

bool DeviceInfoProviderImpl::FixedLabelIteratorImpl::Next(FixedLabelType & output)
{
    // In Linux Simulation, use the following hardcoded labelList on all endpoints.
    CHIP_ERROR err = CHIP_NO_ERROR;

    const char * labelPtr = nullptr;
    const char * valuePtr = nullptr;

    VerifyOrReturnError(mIndex < 4, false);

    ChipLogProgress(DeviceLayer, "Get the fixed label with index:%ld at endpoint:%d", mIndex, mEndpoint);

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
        VerifyOrReturnError(std::strlen(labelPtr) <= kMaxLabelNameLength, false);
        VerifyOrReturnError(std::strlen(valuePtr) <= kMaxLabelValueLength, false);

        Platform::CopyString(mFixedLabelNameBuf, kMaxLabelNameLength + 1, labelPtr);
        Platform::CopyString(mFixedLabelValueBuf, kMaxLabelValueLength + 1, valuePtr);

        output.label = CharSpan::fromCharString(mFixedLabelNameBuf);
        output.value = CharSpan::fromCharString(mFixedLabelValueBuf);

        mIndex++;

        return true;
    }
    else
    {
        return false;
    }
}

CHIP_ERROR DeviceInfoProviderImpl::SetUserLabelLength(EndpointId endpoint, size_t val)
{
    // TODO:: store the user label count.
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR DeviceInfoProviderImpl::GetUserLabelLength(EndpointId endpoint, size_t & val)
{
    // TODO:: read the user label count. temporarily return the size of hardcoded labelList.
    val = 4;

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceInfoProviderImpl::SetUserLabelAt(EndpointId endpoint, size_t index, const UserLabelType & userLabel)
{
    // TODO:: store the user labelList, and read back stored user labelList if it has been set.
    // Add yaml test to verify this.
    return CHIP_ERROR_NOT_IMPLEMENTED;
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
    // TODO:: get the user labelList from persistent storage, temporarily, use the following
    // hardcoded labelList on all endpoints.
    CHIP_ERROR err = CHIP_NO_ERROR;

    const char * labelPtr = nullptr;
    const char * valuePtr = nullptr;

    VerifyOrReturnError(mIndex < mTotal, false);

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
        VerifyOrReturnError(std::strlen(labelPtr) <= kMaxLabelNameLength, false);
        VerifyOrReturnError(std::strlen(valuePtr) <= kMaxLabelValueLength, false);

        Platform::CopyString(mUserLabelNameBuf, kMaxLabelNameLength + 1, labelPtr);
        Platform::CopyString(mUserLabelValueBuf, kMaxLabelValueLength + 1, valuePtr);

        output.label = CharSpan::fromCharString(mUserLabelNameBuf);
        output.value = CharSpan::fromCharString(mUserLabelValueBuf);

        mIndex++;

        return true;
    }
    else
    {
        return false;
    }
}

DeviceInfoProvider::SupportedLocalesIterator * DeviceInfoProviderImpl::IterateSupportedLocales()
{
    return new SupportedLocalesIteratorImpl();
}

size_t DeviceInfoProviderImpl::SupportedLocalesIteratorImpl::Count()
{
    // In Linux Simulation, return the size of the hardcoded list of Strings that are valid values for the ActiveLocale.
    // {("en-US"), ("de-DE"), ("fr-FR"), ("en-GB"), ("es-ES"), ("zh-CN"), ("it-IT"), ("ja-JP")}

    return 8;
}

bool DeviceInfoProviderImpl::SupportedLocalesIteratorImpl::Next(CharSpan & output)
{
    // In Linux simulation, return following hardcoded list of Strings that are valid values for the ActiveLocale.
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
        VerifyOrReturnError(std::strlen(activeLocalePtr) <= kMaxActiveLocaleLength, false);

        Platform::CopyString(mActiveLocaleBuf, kMaxActiveLocaleLength + 1, activeLocalePtr);

        output = CharSpan::fromCharString(mActiveLocaleBuf);

        mIndex++;

        return true;
    }
    else
    {
        return false;
    }
}

DeviceInfoProvider::SupportedCalendarTypesIterator * DeviceInfoProviderImpl::IterateSupportedCalendarTypes()
{
    return new SupportedCalendarTypesIteratorImpl();
}

size_t DeviceInfoProviderImpl::SupportedCalendarTypesIteratorImpl::Count()
{
    // In Linux Simulation, return the size of the hardcoded list of Strings that are valid values for the Calendar Types.
    // {("kBuddhist"), ("kChinese"), ("kCoptic"), ("kEthiopian"), ("kGregorian"), ("kHebrew"), ("kIndian"), ("kJapanese"),
    //  ("kKorean"), ("kPersian"), ("kTaiwanese"), ("kIslamic")}

    return 12;
}

bool DeviceInfoProviderImpl::SupportedCalendarTypesIteratorImpl::Next(CalendarType & output)
{
    // In Linux Simulation, return following hardcoded list of Strings that are valid values for the Calendar Types.
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError(mIndex < 12, false);

    switch (mIndex)
    {
    case 0:
        output = app::Clusters::TimeFormatLocalization::CalendarType::kBuddhist;
        break;
    case 1:
        output = app::Clusters::TimeFormatLocalization::CalendarType::kChinese;
        break;
    case 2:
        output = app::Clusters::TimeFormatLocalization::CalendarType::kCoptic;
        break;
    case 3:
        output = app::Clusters::TimeFormatLocalization::CalendarType::kEthiopian;
        break;
    case 4:
        output = app::Clusters::TimeFormatLocalization::CalendarType::kGregorian;
        break;
    case 5:
        output = app::Clusters::TimeFormatLocalization::CalendarType::kHebrew;
        break;
    case 6:
        output = app::Clusters::TimeFormatLocalization::CalendarType::kIndian;
        break;
    case 7:
        output = app::Clusters::TimeFormatLocalization::CalendarType::kJapanese;
        break;
    case 8:
        output = app::Clusters::TimeFormatLocalization::CalendarType::kKorean;
        break;
    case 9:
        output = app::Clusters::TimeFormatLocalization::CalendarType::kPersian;
        break;
    case 10:
        output = app::Clusters::TimeFormatLocalization::CalendarType::kTaiwanese;
        break;
    case 11:
        output = app::Clusters::TimeFormatLocalization::CalendarType::kIslamic;
        break;
    default:
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
        break;
    }

    if (err == CHIP_NO_ERROR)
    {
        mIndex++;
        return true;
    }
    else
    {
        return false;
    }
}

} // namespace DeviceLayer
} // namespace chip
