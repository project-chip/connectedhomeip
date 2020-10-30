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

} // namespace

AndroidDeviceControllerWrapper::~AndroidDeviceControllerWrapper()
{
    if ((mJavaEnv != nullptr) && (mJavaObjectRef != nullptr))
    {
        mJavaEnv->DeleteGlobalRef(mJavaObjectRef);
        mController->AppState = nullptr;
    }
    mController->Shutdown();
}

void AndroidDeviceControllerWrapper::SetJavaObjectRef(JNIEnv * env, jobject obj)
{
    mJavaEnv       = env;
    mJavaObjectRef = env->NewGlobalRef(obj);
}

AndroidDeviceControllerWrapper * AndroidDeviceControllerWrapper::AllocateNew(chip::NodeId nodeId, chip::System::Layer * systemLayer,
                                                                             chip::Inet::InetLayer * inetLayer,
                                                                             CHIP_ERROR * errInfoOnFailure)
{
    if (errInfoOnFailure == nullptr)
    {
        ChipLogProgress(Controller, "Missing error info");
        return nullptr;
    }
    if (systemLayer == nullptr)
    {
        ChipLogProgress(Controller, "Missing system layer");
        *errInfoOnFailure = CHIP_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }
    if (inetLayer == nullptr)
    {
        ChipLogProgress(Controller, "Missing inet layer");
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

void AndroidDeviceControllerWrapper::OnNetworkCredentialsRequested(chip::RendezvousDeviceCredentialsDelegate * callback)
{
    mCredentialsDelegate = callback;

    jmethodID method;
    if (!FindMethod(mJavaEnv, mJavaObjectRef, "onNetworkCredentialsRequested", "()V", &method))
    {
        return;
    }

    mJavaEnv->ExceptionClear();
    mJavaEnv->CallVoidMethod(mJavaObjectRef, method);
}

void AndroidDeviceControllerWrapper::OnOperationalCredentialsRequested(const char * csr, size_t csr_length,
                                                                       chip::RendezvousDeviceCredentialsDelegate * callback)
{
    mCredentialsDelegate = callback;

    ChipLogProgress(Controller, "NOT YET IMPLEMENTED: %s", __PRETTY_FUNCTION__);
}

void AndroidDeviceControllerWrapper::OnStatusUpdate(chip::RendezvousSessionDelegate::Status status)
{
    CallVoidInt(mJavaEnv, mJavaObjectRef, "onStatusUpdate", static_cast<jint>(status));
}

void AndroidDeviceControllerWrapper::OnPairingComplete(CHIP_ERROR error)
{
    CallVoidInt(mJavaEnv, mJavaObjectRef, "onPairingComplete", static_cast<jint>(error));
}

void AndroidDeviceControllerWrapper::OnPairingDeleted(CHIP_ERROR error)
{
    CallVoidInt(mJavaEnv, mJavaObjectRef, "onPairingDeleted", static_cast<jint>(error));
}