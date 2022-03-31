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
    return new FixedLabelIteratorImpl(*this, endpoint);
}

DeviceInfoProviderImpl::FixedLabelIteratorImpl::FixedLabelIteratorImpl(DeviceInfoProviderImpl & provider, EndpointId endpoint) :
    mProvider(provider), mEndpoint(endpoint)
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

} // namespace DeviceLayer
} // namespace chip
