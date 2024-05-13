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

CHIP_ERROR DeferredAttribute::PrepareWrite(System::Clock::Timestamp flushTime, const ByteSpan & value)
{
    mFlushTime = flushTime;

    if (mValue.AllocatedSize() != value.size())
    {
        mValue.Alloc(value.size());
        ReturnErrorCodeIf(!mValue, CHIP_ERROR_NO_MEMORY);
    }

    memcpy(mValue.Get(), value.data(), value.size());
    return CHIP_NO_ERROR;
}

void DeferredAttribute::Flush(AttributePersistenceProvider & persister)
{
    VerifyOrReturn(IsArmed());
    persister.WriteValue(mPath, ByteSpan(mValue.Get(), mValue.AllocatedSize()));
    mValue.Free();
}

CHIP_ERROR DeferredAttributePersistenceProvider::WriteValue(const ConcreteAttributePath & aPath, const ByteSpan & aValue)
{
    for (DeferredAttribute & da : mDeferredAttributes)
    {
        if (da.Matches(aPath))
        {
            ReturnErrorOnFailure(da.PrepareWrite(System::SystemClock().GetMonotonicTimestamp() + mWriteDelay, aValue));
            FlushAndScheduleNext();
            return CHIP_NO_ERROR;
        }
    }

    return mPersister.WriteValue(aPath, aValue);
}

CHIP_ERROR DeferredAttributePersistenceProvider::ReadValue(const ConcreteAttributePath & aPath,
                                                           const EmberAfAttributeMetadata * aMetadata, MutableByteSpan & aValue)
{
    return mPersister.ReadValue(aPath, aMetadata, aValue);
}

void DeferredAttributePersistenceProvider::FlushAndScheduleNext()
{
    const System::Clock::Timestamp now     = System::SystemClock().GetMonotonicTimestamp();
    System::Clock::Timestamp nextFlushTime = System::Clock::Timestamp::max();

    for (DeferredAttribute & da : mDeferredAttributes)
    {
        if (!da.IsArmed())
        {
            continue;
        }

        if (da.GetFlushTime() <= now)
        {
            da.Flush(mPersister);
        }
        else
        {
            nextFlushTime = chip::min(nextFlushTime, da.GetFlushTime());
        }
    }

    if (nextFlushTime != System::Clock::Timestamp::max())
    {
        DeviceLayer::SystemLayer().StartTimer(
            nextFlushTime - now,
            [](System::Layer *, void * me) { static_cast<DeferredAttributePersistenceProvider *>(me)->FlushAndScheduleNext(); },
            this);
    }
}

} // namespace app
} // namespace chip
