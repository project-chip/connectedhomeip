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
#include <vector>

#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>
#include <oob-accessors/OOBAccessor.h>
#include <oob-accessors/OOBAccessorDependency.h>

namespace chip::app {

/**
 * A registry for managing OOB accessors in memory.
 *
 * Usage:
 *
 * ```
 * InMemoryOOBAccessorRegistry registry;
 * registry.Register(std::make_unique<MyOOBAccessor>());
 * registry.HandleAction("MyAction"_span, myTlvData);
 * ```
 *
 * If at some point unregistration of accessors will be needed, register with some dependency that will be used to unregister them.
 *
 * ```
 * OOBAccessorDependency myDependency = {...};
 * InMemoryOOBAccessorRegistry registry;
 * {
 *    auto scopedDep = registry.WithDependency(myDependency);
 *    registry.Register(std::make_unique<MyOOBAccessor>());
 * }
 * ```
 *
 * Later, unregister with the same dependency.
 * ```
 * registry.Unregister(myDependency);
 * ```
 */
class InMemoryOOBAccessorRegistry
{
public:
    class ScopedDependency
    {
    public:
        ScopedDependency(InMemoryOOBAccessorRegistry & registry, OOBAccessorDependency dependency) :
            mRegistry(registry), mPreviousDependency(registry.mCurrentDependency)
        {
            mRegistry.mCurrentDependency = dependency;
        }

        ~ScopedDependency() { mRegistry.mCurrentDependency = mPreviousDependency; }

        ScopedDependency(const ScopedDependency &)             = delete;
        ScopedDependency & operator=(const ScopedDependency &) = delete;
        ScopedDependency(ScopedDependency && other)            = delete;
        ScopedDependency & operator=(ScopedDependency &&)      = delete;

    private:
        InMemoryOOBAccessorRegistry & mRegistry;
        OOBAccessorDependency mPreviousDependency;
    };

    static InMemoryOOBAccessorRegistry & Instance()
    {
        static InMemoryOOBAccessorRegistry instance;
        return instance;
    }

    InMemoryOOBAccessorRegistry()                                                = default;
    ~InMemoryOOBAccessorRegistry()                                               = default;
    InMemoryOOBAccessorRegistry(const InMemoryOOBAccessorRegistry &)             = delete;
    InMemoryOOBAccessorRegistry & operator=(const InMemoryOOBAccessorRegistry &) = delete;

    /**
     * @brief Registers an OOB accessor instance.
     * @param accessor The accessor instance to register.
     */
    CHIP_ERROR Register(std::unique_ptr<OOBAccessor> accessor);

    /**
     * @brief Unregisters an OOB accessor instance.
     * @param dependency The dependency of the accessor(s) to unregister.
     */
    void Unregister(OOBAccessorDependency dependency);

    /**
     * @brief Dispatches an action to registered accessors in order.
     * @return CHIP_NO_ERROR on success, CHIP_ERROR_NOT_FOUND if no handler is found that can handle the action,
     *         or a specific error on execution failure.
     */
    CHIP_ERROR HandleAction(CharSpan action, ByteSpan tlvData);

    /**
     * @brief Creates RAII object for managing a dependency of oob-accessors for the scope.
     * @param dependency The dependency to set.
     * @return A ScopedDependency object that will restore the previous dependency when it goes out of scope.
     */
    [[nodiscard]] ScopedDependency WithDependency(OOBAccessorDependency dependency) { return ScopedDependency(*this, dependency); }

    /**
     * @brief Clears all registered accessors during device teardown.
     */
    void Clear() { mAccessors.clear(); }

    /**
     * @brief Returns the number of registered accessors.
     */
    size_t Size() const { return mAccessors.size(); }

private:
    std::vector<std::pair<OOBAccessorDependency, std::unique_ptr<OOBAccessor>>> mAccessors;
    OOBAccessorDependency mCurrentDependency = NoDependency{};
};

} // namespace chip::app
