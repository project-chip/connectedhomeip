/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/attribute-storage.h>
#include <jni.h>

/**
 * @brief Handles interfacing between java code and C++ code for the purposes of PowerSource clusters.
 */
class PowerSourceManager
{
public:
    // installed a bridege for a PowerSource cluster endpoint and java object
    static void NewManager(jint endpoint, jobject manager);
    static jboolean SetBatPercentRemaining(jint endpoint, jint value);

private:
    // init with java objects
    CHIP_ERROR InitializeWithObjects(jobject managerObject);
    jobject mPowerSourceManagerObject = nullptr;
};
