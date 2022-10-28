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
#pragma once

#include <app/AttributePersistenceProvider.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/Span.h>

namespace chip {
namespace app {

class DeferredAttribute
{
public:
    explicit DeferredAttribute(const ConcreteAttributePath & path) : mPath(path) {}

    bool Matches(const ConcreteAttributePath & path) const { return mPath == path; }
    bool IsArmed() const { return static_cast<bool>(mValue); }
    System::Clock::Timestamp GetFlushTime() const { return mFlushTime; }

    CHIP_ERROR PrepareWrite(System::Clock::Timestamp flushTime, const ByteSpan & value);
    void Flush(AttributePersistenceProvider & persister);

private:
    const ConcreteAttributePath mPath;
    System::Clock::Timestamp mFlushTime;
    Platform::ScopedMemoryBufferWithSize<uint8_t> mValue;
};

/**
 * Decorator class for the AttributePersistenceProvider implementation that
 * defers writes of selected attributes.
 *
 * This class is useful to increase the flash lifetime by reducing the number
 * of writes of fast-changing attributes, such as CurrentLevel attribute of the
 * LevelControl cluster.
 */
class DeferredAttributePersistenceProvider : public AttributePersistenceProvider
{
public:
    DeferredAttributePersistenceProvider(AttributePersistenceProvider & persister,
                                         const Span<DeferredAttribute> & deferredAttributes,
                                         System::Clock::Milliseconds32 writeDelay) :
        mPersister(persister),
        mDeferredAttributes(deferredAttributes), mWriteDelay(writeDelay)
    {}

    /*
     * If the written attribute is one of the deferred attributes specified in the constructor,
     * postpone the write operation by the configured delay. If this attribute changes within the
     * delay period, further postpone the operation so that the actual write happens once the
     * attribute has remained constant for the write delay period.
     *
     * For other attributes, immediately pass the write operation to the decorated persister.
     */
    CHIP_ERROR WriteValue(const ConcreteAttributePath & path, const ByteSpan & value) override;
    CHIP_ERROR ReadValue(const ConcreteAttributePath & path, const EmberAfAttributeMetadata * metadata,
                         MutableByteSpan & value) override;

private:
    void FlushAndScheduleNext();

    AttributePersistenceProvider & mPersister;
    const Span<DeferredAttribute> mDeferredAttributes;
    const System::Clock::Milliseconds32 mWriteDelay;
};

} // namespace app
} // namespace chip
