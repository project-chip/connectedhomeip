/*
 *   Copyright (c) 2020 Project CHIP Authors
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
#include "AndroidDeviceControllerWrapper.h"

#include <memory>

using chip::DeviceController::ChipDeviceController;

namespace {

bool FindMethod(JNIEnv * env, jobject object, const char * methodName, const char * methodSignature, jmethodID * methodId)
{
    if ((env == nullptr) || (object == nullptr))
    {
        ChipLogError(Controller, "Missing java object for %s", methodName);
        return false;
    }

    jclass javaClass = env->GetObjectClass(object);
    if (javaClass == NULL)
    {
        ChipLogError(Controller, "Failed to get class for %s", methodName);
        return false;
    }

    *methodId = env->GetMethodID(javaClass, methodName, methodSignature);
    if (*methodId == NULL)
    {
        ChipLogError(Controller, "Failed to find method %s", methodName);
        return false;
    }

    return true;
}

void CallVoidInt(JNIEnv * env, jobject object, const char * methodName, jint argument)
{
    jmethodID method;

    if (!FindMethod(env, object, methodName, "(I)V", &method))
    {
        return;
    }

    env->ExceptionClear();
    env->CallVoidMethod(object, method, argument);
}

bool N2J_ByteArray(JNIEnv * env, const uint8_t * inArray, uint32_t inArrayLen, jbyteArray & outArray)
{
    outArray = env->NewByteArray((int) inArrayLen);
    if (outArray == nullptr)
    {
        return false;
    }

    env->ExceptionClear();
    env->SetByteArrayRegion(outArray, 0, inArrayLen, (jbyte *) inArray);
    return !env->ExceptionCheck();
}

} // namespace

AndroidDeviceControllerWrapper::~AndroidDeviceControllerWrapper()
{
    if ((mJavaVM != nullptr) && (mJavaObjectRef != nullptr))
    {
        GetJavaEnv()->DeleteGlobalRef(mJavaObjectRef);
        mController->AppState = nullptr;
    }
    mController->Shutdown();
}

void AndroidDeviceControllerWrapper::SetJavaObjectRef(JavaVM * vm, jobject obj)
{
    mJavaVM        = vm;
    mJavaObjectRef = GetJavaEnv()->NewGlobalRef(obj);
}

JNIEnv * AndroidDeviceControllerWrapper::GetJavaEnv()
{
    if (mJavaVM == nullptr)
    {
        return nullptr;
    }

    JNIEnv * env = nullptr;
    mJavaVM->GetEnv((void **) &env, JNI_VERSION_1_6);

    return env;
}

AndroidDeviceControllerWrapper * AndroidDeviceControllerWrapper::AllocateNew(chip::NodeId nodeId, chip::System::Layer * systemLayer,
                                                                             chip::Inet::InetLayer * inetLayer,
                                                                             CHIP_ERROR * errInfoOnFailure)
{
    if (errInfoOnFailure == nullptr)
    {
        ChipLogError(Controller, "Missing error info");
        return nullptr;
    }
    if (systemLayer == nullptr)
    {
        ChipLogError(Controller, "Missing system layer");
        *errInfoOnFailure = CHIP_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }
    if (inetLayer == nullptr)
    {
        ChipLogError(Controller, "Missing inet layer");
        *errInfoOnFailure = CHIP_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    *errInfoOnFailure = CHIP_NO_ERROR;

    std::unique_ptr<ChipDeviceController> controller(new ChipDeviceController());

    if (!controller)
    {
        *errInfoOnFailure = CHIP_ERROR_NO_MEMORY;
        return nullptr;
    }
    std::unique_ptr<AndroidDeviceControllerWrapper> wrapper(new AndroidDeviceControllerWrapper(std::move(controller)));

    *errInfoOnFailure = wrapper->Controller()->Init(nodeId, systemLayer, inetLayer, wrapper.get());
    if (*errInfoOnFailure != CHIP_NO_ERROR)
    {
        return nullptr;
    }

    return wrapper.release();
}

void AndroidDeviceControllerWrapper::SendNetworkCredentials(const char * ssid, const char * password)
{
    if (mCredentialsDelegate == nullptr)
    {
        ChipLogError(Controller, "No credential callback available to send credentials.");
        return;
    }

    ChipLogProgress(Controller, "Sending network credentials for %s...", ssid);
    mCredentialsDelegate->SendNetworkCredentials(ssid, password);
}

void AndroidDeviceControllerWrapper::OnNetworkCredentialsRequested(chip::RendezvousDeviceCredentialsDelegate * callback)
{
    mCredentialsDelegate = callback;

    JNIEnv * env = GetJavaEnv();

    jmethodID method;
    if (!FindMethod(env, mJavaObjectRef, "onNetworkCredentialsRequested", "()V", &method))
    {
        return;
    }

    env->ExceptionClear();
    env->CallVoidMethod(mJavaObjectRef, method);
}

void AndroidDeviceControllerWrapper::OnOperationalCredentialsRequested(const char * csr, size_t csr_length,
                                                                       chip::RendezvousDeviceCredentialsDelegate * callback)
{
    mCredentialsDelegate = callback;

    JNIEnv * env = GetJavaEnv();

    jbyteArray jCsr;
    if (!N2J_ByteArray(env, reinterpret_cast<const uint8_t *>(csr), csr_length, jCsr))
    {
        ChipLogError(Controller, "Failed to build byte array for operational credential request");
        return;
    }

    jmethodID method;
    if (!FindMethod(env, mJavaObjectRef, "onNetworkCredentialsRequested", "([B)V", &method))
    {
        return;
    }

    env->ExceptionClear();
    env->CallVoidMethod(mJavaObjectRef, method, jCsr);
}

void AndroidDeviceControllerWrapper::OnStatusUpdate(chip::RendezvousSessionDelegate::Status status)
{
    CallVoidInt(GetJavaEnv(), mJavaObjectRef, "onStatusUpdate", static_cast<jint>(status));
}

void AndroidDeviceControllerWrapper::OnPairingComplete(CHIP_ERROR error)
{
    CallVoidInt(GetJavaEnv(), mJavaObjectRef, "onPairingComplete", static_cast<jint>(error));
}

void AndroidDeviceControllerWrapper::OnPairingDeleted(CHIP_ERROR error)
{
    CallVoidInt(GetJavaEnv(), mJavaObjectRef, "onPairingDeleted", static_cast<jint>(error));
}

void AndroidDeviceControllerWrapper::DeprecatedHardcodeThreadCredentials()
{
    if (mCredentialsDelegate == nullptr)
    {
        ChipLogError(Controller, "No credential callback available to send thread credentials.");
        return;
    }

    using namespace chip::DeviceLayer::Internal;

    // This is a dummy implementation of Thread provisioning which allows to test Rendezvous over BLE with
    // Thread-enabled devices by sending OpenThread Border Router default credentials.
    //
    // TODO:
    // 1. Figure out whether WiFi or Thread provisioning should be performed
    // 2. Call Java code to prompt a user for credentials or use the commissioner component of the app

    constexpr uint8_t XPAN_ID[kThreadExtendedPANIdLength]  = { 0x11, 0x11, 0x11, 0x11, 0x22, 0x22, 0x22, 0x22 };
    constexpr uint8_t MESH_PREFIX[kThreadMeshPrefixLength] = { 0xFD, 0x11, 0x11, 0x11, 0x11, 0x22, 0x00, 0x00 };
    constexpr uint8_t NETWORK_KEY[kThreadMasterKeyLength]  = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                                              0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };

    DeviceNetworkInfo threadData = {};
    memcpy(threadData.ThreadExtendedPANId, XPAN_ID, sizeof(XPAN_ID));
    memcpy(threadData.ThreadMeshPrefix, MESH_PREFIX, sizeof(MESH_PREFIX));
    memcpy(threadData.ThreadMasterKey, NETWORK_KEY, sizeof(NETWORK_KEY));
    threadData.ThreadPANId                      = 0x1234;
    threadData.ThreadChannel                    = 15;
    threadData.FieldPresent.ThreadExtendedPANId = 1;
    threadData.FieldPresent.ThreadMeshPrefix    = 1;

    mCredentialsDelegate->SendThreadCredentials(threadData);
}
