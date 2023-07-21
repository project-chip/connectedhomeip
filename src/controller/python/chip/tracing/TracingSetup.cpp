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

#include "tracing/enabled_features.h"

#include <tracing/json/json_tracing.h>
#include <tracing/registry.h>

#if ENABLE_PERFETTO_TRACING
#include <tracing/perfetto/event_storage.h>     // nogncheck
#include <tracing/perfetto/file_output.h>       // nogncheck
#include <tracing/perfetto/perfetto_tracing.h>  // nogncheck
#include <tracing/perfetto/simple_initialize.h> // nogncheck
#endif

namespace {

::chip::Tracing::Json::JsonBackend mJsonBackend;

#if ENABLE_PERFETTO_TRACING
chip::Tracing::Perfetto::FileTraceOutput mPerfettoFileOutput;
chip::Tracing::Perfetto::PerfettoBackend mPerfettoBackend;
#endif

} // namespace

extern "C" void pychip_tracing_start_json_log(const char * file_name)
{
    mJsonBackend.CloseFile(); // just in case, ensure no file output
    chip::Tracing::Register(mJsonBackend);
}

extern "C" PyChipError pychip_tracing_start_json_file(const char * file_name)
{
    CHIP_ERROR err = mJsonBackend.OpenFile(file_name);
    if (err != CHIP_NO_ERROR)
    {
        return ToPyChipError(err);
    }
    chip::Tracing::Register(mJsonBackend);
    return ToPyChipError(CHIP_NO_ERROR);
}

#if ENABLE_PERFETTO_TRACING

extern "C" PyChipError pychip_tracing_start_perfetto_system()
{
    chip::Tracing::Perfetto::Initialize(perfetto::kSystemBackend);
    chip::Tracing::Perfetto::RegisterEventTrackingStorage();
    chip::Tracing::Register(mPerfettoBackend);

    return ToPyChipError(CHIP_NO_ERROR);
}

extern "C" PyChipError pychip_tracing_start_perfetto_file(const char * file_name)
{
    chip::Tracing::Perfetto::Initialize(perfetto::kInProcessBackend);
    chip::Tracing::Perfetto::RegisterEventTrackingStorage();

    CHIP_ERROR err = mPerfettoFileOutput.Open(file_name);
    if (err != CHIP_NO_ERROR)
    {
        return ToPyChipError(err);
    }
    chip::Tracing::Register(mPerfettoBackend);

    return ToPyChipError(CHIP_NO_ERROR);
}

#else  // ENABLE_PERFETTO_TRACING

extern "C" PyChipError pychip_tracing_start_perfetto_system()
{
    return ToPyChipError(CHIP_ERROR_NOT_IMPLEMENTED);
}

extern "C" PyChipError pychip_tracing_start_perfetto_file(const char * file_name)
{
    return ToPyChipError(CHIP_ERROR_NOT_IMPLEMENTED);
}

#endif // ENABLE_PERFETTO_TRACING

extern "C" void pychip_tracing_stop()
{
#if ENABLE_PERFETTO_TRACING
    chip::Tracing::Perfetto::FlushEventTrackingStorage();
    mPerfettoFileOutput.Close();
    chip::Tracing::Unregister(mPerfettoBackend);

#endif

    chip::Tracing::Unregister(mJsonBackend);
}
