/*
 *   Copyright (c) 2023 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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
