/**
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

#include <app/storage/AttributeStorage.h>

namespace chip {
namespace app {
namespace Storage {

/// Provides an attribute storage implementation that is using
/// AttributePersistenceProvider to implement read/write operations.
class EmberAttributeStorageImpl : public AttributeStorage
{
public:
    ~EmberAttributeStorageImpl() override = default;
    CHIP_ERROR Write(const ConcreteAttributePath & path, const Value & value) override;
    CHIP_ERROR Read(const ConcreteAttributePath & path, Buffer buffer) override;
};

} // namespace Storage
} // namespace app
} // namespace chip
