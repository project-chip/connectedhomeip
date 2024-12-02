/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <AppMain.h>
#include <CommissionerMain.h>
#include <admin/FabricAdmin.h>
#include <admin/PairingManager.h>
#include <bridge/include/Bridge.h>

#if defined(ENABLE_CHIP_SHELL)
#include "ShellCommands.h"
#endif

using namespace chip;

namespace {

constexpr char kFabricSyncLogFilePath[] = "/tmp/fabric_sync.log";

// File pointer for the log file
FILE * sLogFile = nullptr;

void OpenLogFile(const char * filePath)
{
    sLogFile = fopen(filePath, "a");
    if (sLogFile == nullptr)
    {
        perror("Failed to open log file");
    }
}

void CloseLogFile()
{
    if (sLogFile != nullptr)
    {
        fclose(sLogFile);
        sLogFile = nullptr;
    }
}

void ENFORCE_FORMAT(3, 0) LoggingCallback(const char * module, uint8_t category, const char * msg, va_list args)
{
    if (sLogFile == nullptr)
    {
        return;
    }

    uint64_t timeMs       = System::SystemClock().GetMonotonicMilliseconds64().count();
    uint64_t seconds      = timeMs / 1000;
    uint64_t milliseconds = timeMs % 1000;

    flockfile(sLogFile);

    fprintf(sLogFile, "[%llu.%06llu] CHIP:%s: ", static_cast<unsigned long long>(seconds),
            static_cast<unsigned long long>(milliseconds), module);
    vfprintf(sLogFile, msg, args);
    fprintf(sLogFile, "\n");
    fflush(sLogFile);

    funlockfile(sLogFile);
}

} // namespace

void ApplicationInit()
{
    ChipLogProgress(NotSpecified, "Fabric-Sync: ApplicationInit()");

    OpenLogFile(kFabricSyncLogFilePath);

    // Redirect logs to the custom logging callback
    Logging::SetLogRedirectCallback(LoggingCallback);

    CHIP_ERROR err = bridge::BridgeInit(&admin::FabricAdmin::Instance());
    VerifyOrDieWithMsg(err == CHIP_NO_ERROR, NotSpecified, "Fabric-Sync: Failed to initialize bridge, error: %s", ErrorStr(err));

    err = admin::FabricAdmin::Instance().Init();
    VerifyOrDieWithMsg(err == CHIP_NO_ERROR, NotSpecified, "Fabric-Sync: Failed to initialize admin, error: %s", ErrorStr(err));
}

void ApplicationShutdown()
{
    ChipLogDetail(NotSpecified, "Fabric-Sync: ApplicationShutdown()");
    CloseLogFile();

    CHIP_ERROR err = bridge::BridgeShutdown();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Fabric-Sync: Failed to shutdown bridge, error: %s", ErrorStr(err));
    }
}

int main(int argc, char * argv[])
{

    VerifyOrDie(ChipLinuxAppInit(argc, argv) == 0);

#if defined(ENABLE_CHIP_SHELL)
    Shell::RegisterCommands();
#endif

    ChipLinuxAppMainLoop();

    return 0;
}
