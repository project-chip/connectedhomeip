/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include <jni.h>
#include <lib/core/CHIPError.h>
#include <platform/ConnectivityManager.h>

#include "BLEManagerImpl.h"

using namespace chip;
using namespace chip::DeviceLayer::Internal;

#define JNI_METHOD(RETURN, METHOD_NAME) extern "C" JNIEXPORT RETURN JNICALL Java_chip_platform_BleConnectCallback_##METHOD_NAME

CHIP_ERROR BleConnectCallbackJNI_OnLoad(JavaVM * jvm, void * reserved)
{
    ChipLogProgress(DeviceLayer, "BleConnectCallbackJNI_OnLoad");
    return CHIP_NO_ERROR;
}

void BleConnectCallbackJNI_OnUnload(JavaVM * jvm, void * reserved) {}

JNI_METHOD(void, onConnectSuccess)(JNIEnv * env, jobject self, jlong managerImplPtr, jlong appStatePtr, jint connId)
{
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    BLEManagerImpl * impl = reinterpret_cast<BLEManagerImpl *>(managerImplPtr);
    void * appState       = reinterpret_cast<void *>(appStatePtr);
    impl->OnConnectSuccess(appState, reinterpret_cast<BLE_CONNECTION_OBJECT>(connId));
#endif
}

JNI_METHOD(void, onConnectFailed)(JNIEnv * env, jobject self, jlong managerImplPtr, jlong appStatePtr)
{
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    BLEManagerImpl * impl = reinterpret_cast<BLEManagerImpl *>(managerImplPtr);
    void * appState       = reinterpret_cast<void *>(appStatePtr);
    impl->OnConnectFailed(appState, BLE_ERROR_NO_CONNECTION_RECEIVED_CALLBACK);
#endif
}
