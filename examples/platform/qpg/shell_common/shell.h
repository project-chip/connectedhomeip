/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "FreeRTOS.h"
#include "task.h"

class ShellTask
{

public:
    static int Start(void);
    static void Main(void * args);

private:
    friend ShellTask & GetShellTask(void);

    int Init(void);

    static ShellTask sShellTask;
};

inline ShellTask & GetShellTask(void)
{
    return ShellTask::sShellTask;
}
