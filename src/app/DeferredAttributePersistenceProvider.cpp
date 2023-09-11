/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
    mValue.Release();
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
