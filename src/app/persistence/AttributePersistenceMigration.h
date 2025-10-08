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

#pragma once
#include <app/DefaultSafeAttributePersistenceProvider.h>
#include <app/persistence/DefaultAttributePersistenceProvider.h>

#include <app/persistence/AttributePersistence.h>

namespace chip::app {

CHIP_ERROR MigrateFromSafeAttributePersistenceProvider(SafeAttributePersistenceProvider & safeProvider,
    AttributePersistenceProvider & normProvider, ConcreteClusterPath path,
    Span<AttributeId> attributes, MutableByteSpan & buffer);

template <int attributeBufferSize = 255>
CHIP_ERROR MigrateFromSafeAttributePersistenceProvider(ConcreteClusterPath path, Span<AttributeId> attributes,
    PersistentStorageDelegate & storageDelegate)
{
    DefaultSafeAttributePersistenceProvider safeProvider;
    safeProvider.Init(&storageDelegate);
    DefaultAttributePersistenceProvider normProvider;
    normProvider.Init(&storageDelegate);

    unsigned char attributeBuffer[attributeBufferSize] = { };
    MutableByteSpan buffer(attributeBuffer);

    return MigrateFromSafeAttributePersistenceProvider(safeProvider, normProvider, path, attributes, buffer);
};

}
