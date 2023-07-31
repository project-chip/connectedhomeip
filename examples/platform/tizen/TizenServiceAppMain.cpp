/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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

#include "TizenServiceAppMain.h"

#include <lib/support/logging/CHIPLogging.h>

#include <service_app.h>
#include <tizen.h>

#include <AppMain.h>

namespace {
bool service_app_create(void * data)
{
    auto app = reinterpret_cast<TizenServiceAppMain *>(data);
    return app->AppCreated();
}

void service_app_terminate(void * data)
{
    auto app = reinterpret_cast<TizenServiceAppMain *>(data);
    app->AppTerminated();
}

void service_app_control(app_control_h app_control, void * data)
{
    auto app = reinterpret_cast<TizenServiceAppMain *>(data);
    app->AppControl(app_control);
}
}; // namespace

int TizenServiceAppMain::Init(int argc, char ** argv)
{
    mArgc = argc;
    mArgv = argv;
    return 0;
}

app_error_e TizenServiceAppMain::RunMainLoop()
{
    service_app_lifecycle_callback_s callbacks;
    callbacks.create      = service_app_create;
    callbacks.terminate   = service_app_terminate;
    callbacks.app_control = service_app_control;

    auto err = service_app_main(mArgc, mArgv, &callbacks, this);
    return static_cast<app_error_e>(err);
}

void TizenServiceAppMain::Exit()
{
    service_app_exit();
}

bool TizenServiceAppMain::AppCreated()
{
    ChipLogProgress(NotSpecified, "Tizen app created");
    return true;
}

void TizenServiceAppMain::AppTerminated()
{
    ChipLogProgress(NotSpecified, "Tizen app terminated");
}

static void TizenMainLoopWrapper()
{
    ChipLinuxAppMainLoop();
}

void TizenServiceAppMain::AppControl(app_control_h app_control)
{
    ChipLogProgress(NotSpecified, "Tizen app control");
    if (!initialized)
    {
        mLinuxArgs.Parse(mArgc > 0 ? mArgv[0] : nullptr, app_control);
        if (ChipLinuxAppInit(mLinuxArgs.argc(), const_cast<char **>(mLinuxArgs.argv())) != 0)
        {
            service_app_exit();
            return;
        }

        mLinuxThread = std::thread(TizenMainLoopWrapper);
        initialized  = true;
    }
}
