/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <controller/python/chip/native/PyChipError.h>
#include <platform/PlatformManager.h>

#include <tracing/json/json_tracing.h>
#include <tracing/perfetto/event_storage.h>
#include <tracing/perfetto/file_output.h>
#include <tracing/perfetto/perfetto_tracing.h>
#include <tracing/perfetto/simple_initialize.h>
#include <tracing/registry.h>

namespace {

using chip::DeviceLayer::PlatformMgr;

class ScopedStackLock
{
public:
    ScopedStackLock() { PlatformMgr().LockChipStack(); }

    ~ScopedStackLock() { PlatformMgr().UnlockChipStack(); }
};

chip::Tracing::Json::JsonBackend gJsonBackend;

chip::Tracing::Perfetto::FileTraceOutput gPerfettoFileOutput;
chip::Tracing::Perfetto::PerfettoBackend gPerfettoBackend;

} // namespace

extern "C" void pychip_tracing_start_json_log(const char * file_name)
{

    ScopedStackLock lock;

    gJsonBackend.CloseFile(); // just in case, ensure no file output
    chip::Tracing::Register(gJsonBackend);
}

extern "C" PyChipError pychip_tracing_start_json_file(const char * file_name)
{
    ScopedStackLock lock;

    CHIP_ERROR err = gJsonBackend.OpenFile(file_name);
    if (err != CHIP_NO_ERROR)
    {
        return ToPyChipError(err);
    }
    chip::Tracing::Register(gJsonBackend);
    return ToPyChipError(CHIP_NO_ERROR);
}

extern "C" void pychip_tracing_start_perfetto_system()
{
    ScopedStackLock lock;

    chip::Tracing::Perfetto::Initialize(perfetto::kSystemBackend);
    chip::Tracing::Perfetto::RegisterEventTrackingStorage();
    chip::Tracing::Register(gPerfettoBackend);
}

extern "C" PyChipError pychip_tracing_start_perfetto_file(const char * file_name)
{
    ScopedStackLock lock;

    chip::Tracing::Perfetto::Initialize(perfetto::kInProcessBackend);
    chip::Tracing::Perfetto::RegisterEventTrackingStorage();

    CHIP_ERROR err = gPerfettoFileOutput.Open(file_name);
    if (err != CHIP_NO_ERROR)
    {
        return ToPyChipError(err);
    }
    chip::Tracing::Register(gPerfettoBackend);

    return ToPyChipError(CHIP_NO_ERROR);
}

extern "C" void pychip_tracing_stop()
{
    ScopedStackLock lock;

    chip::Tracing::Perfetto::FlushEventTrackingStorage();
    gPerfettoFileOutput.Close();
    chip::Tracing::Unregister(gPerfettoBackend);
    chip::Tracing::Unregister(gJsonBackend);
}
