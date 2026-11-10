/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <memory>

#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>
#include <oob-accessors/OOBAccessor.h>
#include <oob-accessors/OOBAccessorDependency.h>

namespace chip::app {

/// A no-op implementation of a registry, the api should fully match `InMemoryOOBAccessorRegistry`.
class NoopOOBAccessorRegistry
{
public:
    class ScopedDependency
    {
    public:
        ScopedDependency(NoopOOBAccessorRegistry & /* registry */, OOBAccessorDependency /* dependency */) {}
        ~ScopedDependency() {}

        ScopedDependency(const ScopedDependency &)             = delete;
        ScopedDependency & operator=(const ScopedDependency &) = delete;
        ScopedDependency(ScopedDependency && /* other */)      = delete;
        ScopedDependency & operator=(ScopedDependency &&)      = delete;
    };

    static NoopOOBAccessorRegistry & Instance()
    {
        static NoopOOBAccessorRegistry instance;
        return instance;
    }

    CHIP_ERROR Register(std::unique_ptr<OOBAccessor> /* accessor */) { return CHIP_NO_ERROR; }
    void Unregister(OOBAccessorDependency /* dependency */) {}
    CHIP_ERROR HandleAction(CharSpan /* action */, ByteSpan /* tlvData */) { return CHIP_ERROR_NOT_FOUND; }
    [[nodiscard]] ScopedDependency WithDependency(OOBAccessorDependency /* dependency */)
    {
        return ScopedDependency(*this, NoDependency{});
    }
    void Clear() {}
    size_t Size() const { return 0; }
};

} // namespace chip::app
