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

#include <controller/python/chip/native/ChipMainLoopWork.h>

#include <lib/support/CHIPFaultInjection.h>

namespace {
// chip::Tracing::Json::JsonBackend gJsonBackend;

// chip::Tracing::Perfetto::FileTraceOutput gPerfettoFileOutput;
// chip::Tracing::Perfetto::PerfettoBackend gPerfettoBackend;

} // namespace

// #if CHIP_WITH_NLFAULTINJECTION

// Returns 0 if pass, -ve if failed
extern "C" int32_t pychip_faultinjection_fail_at_fault(uint32_t faultID, uint32_t numCallsToSkip, uint32_t numCallsToFail,
                                                       bool takeMutex)
{
    return chip::FaultInjection::GetManager().FailAtFault(faultID, numCallsToSkip, numCallsToFail, takeMutex);
    // return chip::FaultInjection::GetManager().FailAtFault(chip::FaultInjection::kFault_CASEInvalidDesintationID, 0, 1);
}

// extern "C" int32_t pychip_faultinjection_delete_Session_Resumption(uint32_t fabricIndex)
// {
//     chip::Server::GetInstance().GetSessionResumptionStorage()->DeleteAll(fabricIndex);
// }

/*
extern "C" void pychip_tracing_start_json_log()
{
    chip::MainLoopWork::ExecuteInMainLoop([] {
        gJsonBackend.CloseFile(); // just in case, ensure no file output
        chip::Tracing::Register(gJsonBackend);
    });
}

extern "C" PyChipError pychip_tracing_start_json_file(const char * file_name)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    chip::MainLoopWork::ExecuteInMainLoop([&err, file_name] {
        err = gJsonBackend.OpenFile(file_name);
        if (err != CHIP_NO_ERROR)
        {
            return;
        }
        chip::Tracing::Register(gJsonBackend);
    });

    return ToPyChipError(err);
}

extern "C" void pychip_tracing_start_perfetto_system()
{
    chip::MainLoopWork::ExecuteInMainLoop([] {
        chip::Tracing::Perfetto::Initialize(perfetto::kSystemBackend);
        chip::Tracing::Perfetto::RegisterEventTrackingStorage();
        chip::Tracing::Register(gPerfettoBackend);
    });
}

extern "C" PyChipError pychip_tracing_start_perfetto_file(const char * file_name)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::MainLoopWork::ExecuteInMainLoop([&err, file_name] {
        chip::Tracing::Perfetto::Initialize(perfetto::kInProcessBackend);
        chip::Tracing::Perfetto::RegisterEventTrackingStorage();

        err = gPerfettoFileOutput.Open(file_name);
        if (err != CHIP_NO_ERROR)
        {
            return;
        }
        chip::Tracing::Register(gPerfettoBackend);
    });

    return ToPyChipError(err);
}

extern "C" void pychip_tracing_stop()
{
    chip::MainLoopWork::ExecuteInMainLoop([] {
        chip::Tracing::Perfetto::FlushEventTrackingStorage();
        gPerfettoFileOutput.Close();
        chip::Tracing::Unregister(gPerfettoBackend);
        chip::Tracing::Unregister(gJsonBackend);
    });
}
*/

// #endif // CHIP_WITH_NLFAULTINJECTION
