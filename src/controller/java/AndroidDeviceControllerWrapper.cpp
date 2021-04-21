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
#include "CHIPJNIError.h"

#include <memory>

#include <support/ThreadOperationalDataset.h>

using chip::PersistentStorageResultDelegate;
using chip::Controller::DeviceCommissioner;

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

CHIP_ERROR N2J_ByteArray(JNIEnv * env, const uint8_t * inArray, uint32_t inArrayLen, jbyteArray & outArray)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    outArray = env->NewByteArray((int) inArrayLen);
    VerifyOrExit(outArray != NULL, err = CHIP_ERROR_NO_MEMORY);

    env->ExceptionClear();
    env->SetByteArrayRegion(outArray, 0, inArrayLen, (jbyte *) inArray);
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);

exit:
    return err;
}

CHIP_ERROR N2J_NewStringUTF(JNIEnv * env, const char * inStr, size_t inStrLen, jstring & outString)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    jbyteArray charArray    = NULL;
    jstring utf8Encoding    = NULL;
    jclass java_lang_String = NULL;
    jmethodID ctor          = NULL;

    err = N2J_ByteArray(env, reinterpret_cast<const uint8_t *>(inStr), inStrLen, charArray);
    SuccessOrExit(err);

    utf8Encoding = env->NewStringUTF("UTF-8");
    VerifyOrExit(utf8Encoding != NULL, err = CHIP_ERROR_NO_MEMORY);

    java_lang_String = env->FindClass("java/lang/String");
    VerifyOrExit(java_lang_String != NULL, err = CHIP_JNI_ERROR_TYPE_NOT_FOUND);

    ctor = env->GetMethodID(java_lang_String, "<init>", "([BLjava/lang/String;)V");
    VerifyOrExit(ctor != NULL, err = CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    outString = (jstring) env->NewObject(java_lang_String, ctor, charArray, utf8Encoding);
    VerifyOrExit(outString != NULL, err = CHIP_ERROR_NO_MEMORY);

exit:
    // error code propagated from here, so clear any possible
    // exceptions that arose here
    env->ExceptionClear();

    if (utf8Encoding != NULL)
        env->DeleteLocalRef(utf8Encoding);
    if (charArray != NULL)
        env->DeleteLocalRef(charArray);

    return err;
}

CHIP_ERROR N2J_NewStringUTF(JNIEnv * env, const char * inStr, jstring & outString)
{
    return N2J_NewStringUTF(env, inStr, strlen(inStr), outString);
}

} // namespace

AndroidDeviceControllerWrapper::~AndroidDeviceControllerWrapper()
{
    if ((mJavaVM != nullptr) && (mJavaObjectRef != nullptr))
    {
        GetJavaEnv()->DeleteGlobalRef(mJavaObjectRef);
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

AndroidDeviceControllerWrapper * AndroidDeviceControllerWrapper::AllocateNew(JavaVM * vm, jobject deviceControllerObj,
                                                                             chip::NodeId nodeId, chip::System::Layer * systemLayer,
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

    std::unique_ptr<DeviceCommissioner> controller(new DeviceCommissioner());

    if (!controller)
    {
        *errInfoOnFailure = CHIP_ERROR_NO_MEMORY;
        return nullptr;
    }
    std::unique_ptr<AndroidDeviceControllerWrapper> wrapper(new AndroidDeviceControllerWrapper(std::move(controller)));

    wrapper->SetJavaObjectRef(vm, deviceControllerObj);
    wrapper->Controller()->SetUdpListenPort(CHIP_PORT + 1);
    *errInfoOnFailure = wrapper->Controller()->Init(nodeId, wrapper.get(), wrapper.get(), systemLayer, inetLayer);

    if (*errInfoOnFailure != CHIP_NO_ERROR)
    {
        return nullptr;
    }

    *errInfoOnFailure = wrapper->Controller()->ServiceEvents();

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

void AndroidDeviceControllerWrapper::SendThreadCredentials(chip::ByteSpan threadData)
{
    if (mCredentialsDelegate == nullptr)
    {
        ChipLogError(Controller, "No credential callback available to send Thread credentials.");
        return;
    }

    chip::Thread::OperationalDataset dataset{};

    if (!dataset.Init(threadData))
    {
        ChipLogError(Controller, "Failed to parse Thread credentials.");
        return;
    }

    uint16_t channel = chip::Thread::kChannel_NotSpecified;
    uint16_t panid   = chip::Thread::kPANId_NotSpecified;

    dataset.GetChannel(channel);
    dataset.GetPanId(panid);

    ChipLogProgress(Controller, "Sending Thread credentials for channel %u, PAN ID 0x%04x...", channel, panid);
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

void AndroidDeviceControllerWrapper::OnMessage(chip::System::PacketBufferHandle msg) {}

void AndroidDeviceControllerWrapper::OnStatusChange(void) {}

void AndroidDeviceControllerWrapper::SetStorageDelegate(PersistentStorageResultDelegate * delegate)
{
    mStorageResultDelegate = delegate;
}

CHIP_ERROR AndroidDeviceControllerWrapper::SyncGetKeyValue(const char * key, char * value, uint16_t & size)
{
    jstring keyString       = NULL;
    jstring valueString     = NULL;
    const char * valueChars = nullptr;
    CHIP_ERROR err          = CHIP_NO_ERROR;
    jclass storageCls       = GetPersistentStorageClass();
    jmethodID method        = GetJavaEnv()->GetStaticMethodID(storageCls, "getKeyValue", "(Ljava/lang/String;)Ljava/lang/String;");

    GetJavaEnv()->ExceptionClear();

    err = N2J_NewStringUTF(GetJavaEnv(), key, keyString);
    SuccessOrExit(err);

    valueString = (jstring) GetJavaEnv()->CallStaticObjectMethod(storageCls, method, keyString);

    if (valueString != NULL)
    {
        size_t stringLength = GetJavaEnv()->GetStringUTFLength(valueString);
        if (stringLength > UINT16_MAX - 1)
        {
            err = CHIP_ERROR_BUFFER_TOO_SMALL;
        }
        else
        {
            if (value != nullptr)
            {
                valueChars = GetJavaEnv()->GetStringUTFChars(valueString, 0);
                size       = strlcpy(value, valueChars, size);
                if (size < stringLength)
                {
                    err = CHIP_ERROR_NO_MEMORY;
                }
            }
            else
            {
                size = stringLength;
                err  = CHIP_ERROR_NO_MEMORY;
            }
            // Increment size to account for null termination
            size += 1;
        }
    }
    else
    {
        err = CHIP_ERROR_INVALID_ARGUMENT;
    }

exit:
    GetJavaEnv()->ExceptionClear();
    if (valueChars != nullptr)
    {
        GetJavaEnv()->ReleaseStringUTFChars(valueString, valueChars);
    }
    GetJavaEnv()->DeleteLocalRef(keyString);
    GetJavaEnv()->DeleteLocalRef(valueString);
    return err;
}

void AndroidDeviceControllerWrapper::AsyncSetKeyValue(const char * key, const char * value)
{
    jclass storageCls = GetPersistentStorageClass();
    jmethodID method  = GetJavaEnv()->GetStaticMethodID(storageCls, "setKeyValue", "(Ljava/lang/String;Ljava/lang/String;)V");

    GetJavaEnv()->ExceptionClear();

    jstring keyString   = NULL;
    jstring valueString = NULL;
    CHIP_ERROR err      = CHIP_NO_ERROR;

    err = N2J_NewStringUTF(GetJavaEnv(), key, keyString);
    SuccessOrExit(err);
    err = N2J_NewStringUTF(GetJavaEnv(), value, valueString);
    SuccessOrExit(err);

    GetJavaEnv()->CallStaticVoidMethod(storageCls, method, keyString, valueString);

    if (mStorageResultDelegate)
    {
        mStorageResultDelegate->OnPersistentStorageStatus(key, PersistentStorageResultDelegate::Operation::kSET, CHIP_NO_ERROR);
    }

exit:
    GetJavaEnv()->ExceptionClear();
    GetJavaEnv()->DeleteLocalRef(keyString);
    GetJavaEnv()->DeleteLocalRef(valueString);
}

void AndroidDeviceControllerWrapper::AsyncDeleteKeyValue(const char * key)
{
    jclass storageCls = GetPersistentStorageClass();
    jmethodID method  = GetJavaEnv()->GetStaticMethodID(storageCls, "deleteKeyValue", "(Ljava/lang/String;)V");

    GetJavaEnv()->ExceptionClear();

    jstring keyString = NULL;
    CHIP_ERROR err    = CHIP_NO_ERROR;

    err = N2J_NewStringUTF(GetJavaEnv(), key, keyString);
    SuccessOrExit(err);

    GetJavaEnv()->CallStaticVoidMethod(storageCls, method, keyString);

    if (mStorageResultDelegate)
    {
        mStorageResultDelegate->OnPersistentStorageStatus(key, PersistentStorageResultDelegate::Operation::kDELETE, CHIP_NO_ERROR);
    }

exit:
    GetJavaEnv()->ExceptionClear();
    GetJavaEnv()->DeleteLocalRef(keyString);
}
