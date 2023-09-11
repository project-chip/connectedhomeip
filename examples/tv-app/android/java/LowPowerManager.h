/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/clusters/low-power-server/low-power-server.h>

#include <jni.h>
#include <lib/core/CHIPError.h>

class LowPowerManager : public chip::app::Clusters::LowPower::Delegate
{
public:
    static void NewManager(jint endpoint, jobject manager);
    void InitializeWithObjects(jobject managerObject);

    bool HandleSleep() override;

private:
    jobject mLowPowerManagerObject = nullptr;
    jmethodID mSleepMethod         = nullptr;
};
