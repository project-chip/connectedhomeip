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

#include <app/DeferredAttributePersistenceProvider.h>

#include <platform/CHIPDeviceLayer.h>

namespace chip {
namespace app {

CHIP_ERROR DeferredAttribute::PrepareWrite(AttributePersistenceProvider & persister, const EmberAfAttributeMetadata * metadata,
                                           const ByteSpan & value)
{
    mPersister = &persister;
    mMetadata  = metadata;

    if (mValue.AllocatedSize() != value.size())
    {
        mValue.Alloc(value.size());
        ReturnErrorCodeIf(!mValue, CHIP_ERROR_NO_MEMORY);
    }

    memcpy(mValue.Get(), value.data(), value.size());
    return CHIP_NO_ERROR;
}

void DeferredAttribute::Flush()
{
    VerifyOrReturn(mValue);
    mPersister->WriteValue(mPath, mMetadata, ByteSpan(mValue.Get(), mValue.AllocatedSize()));
    mValue.Release();
}

CHIP_ERROR DeferredAttributePersistenceProvider::WriteValue(const ConcreteAttributePath & path,
                                                            const EmberAfAttributeMetadata * metadata, const ByteSpan & value)
{
    for (DeferredAttribute & da : mDeferredAttributes)
    {
        if (da.Matches(path))
        {
            ReturnErrorOnFailure(da.PrepareWrite(mPersister, metadata, value));
            DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(mWriteDelay), Flush, &da);
            return CHIP_NO_ERROR;
        }
    }

    return mPersister.WriteValue(path, metadata, value);
}

CHIP_ERROR DeferredAttributePersistenceProvider::ReadValue(const ConcreteAttributePath & path,
                                                           const EmberAfAttributeMetadata * metadata, MutableByteSpan & value)
{
    return mPersister.ReadValue(path, metadata, value);
}

} // namespace app
} // namespace chip
