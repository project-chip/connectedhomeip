/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/door-lock-server/door-lock-server.h>
#include <app/util/attribute-storage.h>
#include <jni.h>

/**
 * @brief Handles interfacing between java code and C++ code for the purposes of DoorLock clusters.
 */
class DoorLockManager
{
public:
    // installed a bridege for a DoorLock cluster endpoint and java object
    static void NewManager(jint endpoint, jobject manager);

    // helps for java to set attributes::LockType of DoorLock cluster
    static jboolean SetLockType(jint endpoint, jint value);

    // helps for java to set attributes::LockState of DoorLock cluster
    static jboolean SetLockState(jint endpoint, jint value);

    // helps for java to set attributes::ActuatorEnabled of DoorLock cluster
    static jboolean SetActuatorEnabled(jint endpoint, jboolean value);

    // helps for java to set attributes::AutoRelockTime of DoorLock cluster
    static jboolean SetAutoRelockTime(jint endpoint, jint value);

    // helps for java to set attributes::OperatingMode of DoorLock cluster
    static jboolean SetOperatingMode(jint endpoint, jint value);

    // helps for java to set attributes::SupportedOperatingMode of DoorLock cluster
    static jboolean SetSupportedOperatingModes(jint endpoint, jint value);

    // helps for java to send LockAlarmEvent of DoorLock cluster
    static jboolean SendLockAlarmEvent(jint endpoint);

    // posts a LockStateChanged event to suitable DoorLockManager
    static void PostLockStateChanged(chip::EndpointId endpoint, int value);

    // handles `Changed` callbacks by calling the java `void HandleLockStateChanged()` method
    void HandleLockStateChanged(int endpoint, int value);

private:
    // init with java objects
    CHIP_ERROR InitializeWithObjects(jobject managerObject);
    jobject mDoorLockManagerObject          = nullptr;
    jmethodID mHandleLockStateChangedMethod = nullptr;
};
