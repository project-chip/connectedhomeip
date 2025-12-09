/*
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
#include <app/persistence/AttributePersistence.h>

#include <app/ConcreteAttributePath.h>
#include <app/data-model/Nullable.h>
#include <app/persistence/AttributePersistenceProvider.h>
#include <app/persistence/String.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

namespace chip::app {

namespace {

bool VerifySuccessLogOnFailure(const ConcreteAttributePath & path, CHIP_ERROR err)
{
    VerifyOrReturnValue(err != CHIP_NO_ERROR, true);

    // Value not found is typical. Not an error worth logging.
    VerifyOrReturnValue(err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND, false);

    ChipLogError(Zcl, "Failed to load attribute %u/" ChipLogFormatMEI "/" ChipLogFormatMEI ": %" CHIP_ERROR_FORMAT,
                 path.mEndpointId, ChipLogValueMEI(path.mClusterId), ChipLogValueMEI(path.mAttributeId), err.Format());
    return false;
}

} // namespace

bool AttributePersistence::InternalRawLoadNativeEndianValue(const ConcreteAttributePath & path, void * data,
                                                            const void * valueOnLoadFailure, size_t size)
{
    MutableByteSpan rawBytes(reinterpret_cast<uint8_t *>(data), size);
    if (!VerifySuccessLogOnFailure(path, mProvider.ReadValue(path, rawBytes)))
    {
        // in case of failure, set the default value
        memcpy(data, valueOnLoadFailure, size);
        return false;
    }

    if (rawBytes.size() != size)
    {
        // short read: the value is not valid
        memcpy(data, valueOnLoadFailure, size);
        return false;
    }

    return true;
}

bool AttributePersistence::LoadString(const ConcreteAttributePath & path, Storage::Internal::ShortString & value)
{
    Storage::Internal::ShortStringInputAdapter io(value);
    MutableByteSpan rawBytes = io.ReadBuffer();

    if (!VerifySuccessLogOnFailure(path, mProvider.ReadValue(path, rawBytes)))
    {
        value.SetContent(""_span);
        return false;
    }
    return io.FinalizeRead(rawBytes);
}

CHIP_ERROR AttributePersistence::StoreString(const ConcreteAttributePath & path, const Storage::Internal::ShortString & value)
{
    Storage::Internal::ShortStringOutputAdapter io(value);
    return mProvider.WriteValue(path, io.ContentWithPrefix());
}

} // namespace chip::app
