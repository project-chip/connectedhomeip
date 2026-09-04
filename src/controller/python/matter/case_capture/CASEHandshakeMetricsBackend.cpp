/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <controller/python/matter/case_capture/CASEHandshakeMetricsBackend.h>

#include <controller/python/matter/native/ChipMainLoopWork.h>
#include <tracing/registry.h>

namespace chip {
namespace python {
namespace {

// Built on first use, so nothing depends on static initialisation order across translation
// units, and confined to this file so the class itself is not reachable from anywhere else.
CASEHandshakeMetricsBackend & Backend()
{
    static CASEHandshakeMetricsBackend backend;
    return backend;
}

bool gRegistered = false;

} // namespace

void StartCASEHandshakeMetricsBackend()
{
    // The tracing registry lives on the event loop, so registration goes through it.
    MainLoopWork::ExecuteInMainLoop([] {
        Backend().Reset();
        if (!gRegistered)
        {
            Tracing::Register(Backend());
            gRegistered = true;
        }
    });
}

void StopCASEHandshakeMetricsBackend()
{
    MainLoopWork::ExecuteInMainLoop([] {
        if (gRegistered)
        {
            Tracing::Unregister(Backend());
            gRegistered = false;
        }
    });
}

uint32_t AbandonedCASEHandshakeCount()
{
    uint32_t count = 0;
    MainLoopWork::ExecuteInMainLoop([&] { count = Backend().AbandonedCASEHandshakeCount(); });
    return count;
}

} // namespace python
} // namespace chip
