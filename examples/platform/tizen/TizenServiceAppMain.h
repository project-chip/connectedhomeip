/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
