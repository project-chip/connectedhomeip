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
    CHIP_ERROR PrepareWrite(AttributePersistenceProvider & persister, const EmberAfAttributeMetadata * metadata,
                            const ByteSpan & value);
    void Flush();

private:
    const ConcreteAttributePath mPath;
    AttributePersistenceProvider * mPersister;
    const EmberAfAttributeMetadata * mMetadata;
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

    CHIP_ERROR WriteValue(const ConcreteAttributePath & path, const EmberAfAttributeMetadata * metadata, const ByteSpan & value);
    CHIP_ERROR ReadValue(const ConcreteAttributePath & path, const EmberAfAttributeMetadata * metadata, MutableByteSpan & value);

private:
    static void Flush(System::Layer *, void * deferredAttr) { static_cast<DeferredAttribute *>(deferredAttr)->Flush(); }

    AttributePersistenceProvider & mPersister;
    const Span<DeferredAttribute> mDeferredAttributes;
    const System::Clock::Milliseconds32 mWriteDelay;
};

} // namespace app
} // namespace chip
