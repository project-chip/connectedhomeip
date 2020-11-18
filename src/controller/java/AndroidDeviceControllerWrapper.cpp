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
    }
    mController->AppState = nullptr;
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
        ChipLogError(Controller, "No credential callback available to send Wi-Fi credentials.");
        return;
    }

    ChipLogProgress(Controller, "Sending network credentials for %s...", ssid);
    mCredentialsDelegate->SendNetworkCredentials(ssid, password);
}

void AndroidDeviceControllerWrapper::SendThreadCredentials(const chip::DeviceLayer::Internal::DeviceNetworkInfo & threadData)
{
    if (mCredentialsDelegate == nullptr)
    {
        ChipLogError(Controller, "No credential callback available to send Thread credentials.");
        return;
    }

    ChipLogProgress(Controller, "Sending Thread credentials for channel %u, PAN ID %x...", threadData.ThreadChannel,
                    threadData.ThreadPANId);
    mCredentialsDelegate->SendThreadCredentials(threadData);
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
    if (!FindMethod(env, mJavaObjectRef, "onOperationalCredentialsRequested", "([B)V", &method))
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
