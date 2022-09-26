/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *    All rights reserved.
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

#include <nlunit-test.h>

#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>

namespace chip {

/// Performs a memory Init/Shutdown in a controlled manner
/// (i.e. guarantees shutdown is called).
class ScopedMemoryInit
{
public:
    ScopedMemoryInit() { VerifyOrDie(chip::Platform::MemoryInit() == CHIP_NO_ERROR); }

    ~ScopedMemoryInit() { chip::Platform::MemoryShutdown(); }
};

/// Executes nlTestRunner for the given test suite while
/// allocating the given context type on the heap
template <class Context, typename... Args>
inline int ExecuteTestsWithContext(struct _nlTestSuite * suite, Args &&... args)
{
    {
        ScopedMemoryInit ensureHeapIsInitialized;
        auto ctx = chip::Platform::MakeUnique<Context>(std::forward<Args>(args)...);
        nlTestRunner(suite, ctx.get());
    }

    return nlTestRunnerStats(suite);
}

inline int ExecuteTestsWithoutContext(struct _nlTestSuite * suite)
{
    nlTestRunner(suite, nullptr);
    return nlTestRunnerStats(suite);
}

} // namespace chip
