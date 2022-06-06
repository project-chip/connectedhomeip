/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <thread>

#include <service_app.h>
#include <tizen.h>

#include "OptionsProxy.h"

class TizenServiceAppMain
{
public:
    TizenServiceAppMain() = default;
    virtual ~TizenServiceAppMain(){};

    int Init(int argc, char ** argv);
    app_error_e RunMainLoop();
    void Exit();

    virtual bool AppCreated();
    virtual void AppTerminated();
    virtual void AppControl(app_control_h app_control);

private:
    int mArgc     = 0;
    char ** mArgv = nullptr;
    OptionsProxy mLinuxArgs;
    std::thread mLinuxThread;
    bool initialized = false;
};
