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

/**
 *    @file
 *      Implementation of JNI bridge for CHIP Device Controller for Android apps
 *
 */

#include "CHIPDeviceController.h"

#include <jni.h>
#include <pthread.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/logging/CHIPLogging.h>

extern "C" {
#include "chip-zcl-zpro-codec.h"
} // extern "C"

using namespace chip;
using namespace chip::DeviceController;

#define JNI_METHOD(RETURN, METHOD_NAME)                                                                                            \
    extern "C" JNIEXPORT RETURN JNICALL Java_chip_devicecontroller_ChipDeviceController_##METHOD_NAME

#define CDC_JNI_ERROR_MIN 10000
#define CDC_JNI_ERROR_MAX 10999

#define _CDC_JNI_ERROR(e) (CDC_JNI_ERROR_MIN + (e))

#define CDC_JNI_ERROR_EXCEPTION_THROWN _CDC_JNI_ERROR(0)
#define CDC_JNI_ERROR_TYPE_NOT_FOUND _CDC_JNI_ERROR(1)
#define CDC_JNI_ERROR_METHOD_NOT_FOUND _CDC_JNI_ERROR(2)
#define CDC_JNI_ERROR_FIELD_NOT_FOUND _CDC_JNI_ERROR(3)

#define CDC_JNI_CALLBACK_LOCAL_REF_COUNT 256

static void HandleKeyExchange(ChipDeviceController * deviceController, Transport::PeerConnectionState * state, void * appReqState);
static void HandleEchoResponse(ChipDeviceController * deviceController, void * appReqState, System::PacketBuffer * payload);
static void HandleSimpleOperationComplete(ChipDeviceController * deviceController, void * appReqState);
static void HandleError(ChipDeviceController * deviceController, void * appReqState, CHIP_ERROR err, const IPPacketInfo * pktInfo);
static void ThrowError(JNIEnv * env, CHIP_ERROR errToThrow);
static void * IOThreadMain(void * arg);
static CHIP_ERROR GetClassRef(JNIEnv * env, const char * clsType, jclass & outCls);
static CHIP_ERROR N2J_ByteArray(JNIEnv * env, const uint8_t * inArray, uint32_t inArrayLen, jbyteArray & outArray);
static CHIP_ERROR N2J_Error(JNIEnv * env, CHIP_ERROR inErr, jthrowable & outEx);
static CHIP_ERROR N2J_NewStringUTF(JNIEnv * env, const char * inStr, jstring & outString);
static CHIP_ERROR N2J_NewStringUTF(JNIEnv * env, const char * inStr, size_t inStrLen, jstring & outString);

static JavaVM * sJVM;
static System::Layer sSystemLayer;
static InetLayer sInetLayer;
static pthread_mutex_t sStackLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_t sIOThread        = PTHREAD_NULL;
static bool sShutdown             = false;

static jclass sChipDeviceControllerCls          = NULL;
static jclass sChipDeviceControllerExceptionCls = NULL;

// NOTE: Remote device ID is in sync with the echo server device id
// At some point, we may want to add an option to connect to a device without
// knowing its id, because the ID can be learned on the first response that is received.
chip::NodeId kLocalDeviceId  = 112233;
chip::NodeId kRemoteDeviceId = 12344321;

static const unsigned char local_private_key[] = { 0x00, 0xd1, 0x90, 0xd9, 0xb3, 0x95, 0x1c, 0x5f, 0xa4, 0xe7, 0x47,
                                                   0x92, 0x5b, 0x0a, 0xa9, 0xa7, 0xc1, 0x1c, 0xe7, 0x06, 0x10, 0xe2,
                                                   0xdd, 0x16, 0x41, 0x52, 0x55, 0xb7, 0xb8, 0x80, 0x8d, 0x87, 0xa1 };

static const unsigned char remote_public_key[] = { 0x04, 0xe2, 0x07, 0x64, 0xff, 0x6f, 0x6a, 0x91, 0xd9, 0xc2, 0xc3, 0x0a, 0xc4,
                                                   0x3c, 0x56, 0x4b, 0x42, 0x8a, 0xf3, 0xb4, 0x49, 0x29, 0x39, 0x95, 0xa2, 0xf7,
                                                   0x02, 0x8c, 0xa5, 0xce, 0xf3, 0xc9, 0xca, 0x24, 0xc5, 0xd4, 0x5c, 0x60, 0x79,
                                                   0x48, 0x30, 0x3c, 0x53, 0x86, 0xd9, 0x23, 0xe6, 0x61, 0x1f, 0x5a, 0x3d, 0xdf,
                                                   0x9f, 0xdc, 0x35, 0xea, 0xd0, 0xde, 0x16, 0x7e, 0x64, 0xde, 0x7f, 0x3c, 0xa6 };

jint JNI_OnLoad(JavaVM * jvm, void * reserved)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env;
    int pthreadErr = 0;
    pthread_mutexattr_t stackLockAttrs;

    ChipLogProgress(Controller, "JNI_OnLoad() called");

    // Save a reference to the JVM.  Will need this to call back into Java.
    sJVM = jvm;

    // Get a JNI environment object.
    sJVM->GetEnv((void **) &env, JNI_VERSION_1_6);

    ChipLogProgress(Controller, "Loading Java class references.");

    // Get various class references need by the API.
    err = GetClassRef(env, "chip/devicecontroller/ChipDeviceController", sChipDeviceControllerCls);
    SuccessOrExit(err);
    err = GetClassRef(env, "chip/devicecontroller/ChipDeviceControllerException", sChipDeviceControllerExceptionCls);
    SuccessOrExit(err);
    ChipLogProgress(Controller, "Java class references loaded.");

    // Initialize the lock that will be used to protect the stack.
    // Note that this needs to allow recursive acquisition.
    pthread_mutexattr_init(&stackLockAttrs);
    pthread_mutexattr_settype(&stackLockAttrs, PTHREAD_MUTEX_RECURSIVE);
    pthreadErr = pthread_mutex_init(&sStackLock, &stackLockAttrs);
    VerifyOrExit(pthreadErr == 0, err = System::MapErrorPOSIX(pthreadErr));

    // Initialize the CHIP System Layer.
    err = sSystemLayer.Init(NULL);
    SuccessOrExit(err);

    // Initialize the CHIP Inet layer.
    err = sInetLayer.Init(sSystemLayer, NULL);
    SuccessOrExit(err);

    // Create and start the IO thread.
    sShutdown  = false;
    pthreadErr = pthread_create(&sIOThread, NULL, IOThreadMain, NULL);
    VerifyOrExit(pthreadErr == 0, err = System::MapErrorPOSIX(pthreadErr));

exit:
    if (err != CHIP_NO_ERROR)
    {
        ThrowError(env, err);
        JNI_OnUnload(jvm, reserved);
    }

    return (err == CHIP_NO_ERROR) ? JNI_VERSION_1_6 : JNI_ERR;
}

void JNI_OnUnload(JavaVM * jvm, void * reserved)
{
    ChipLogProgress(Controller, "JNI_OnUnload() called");

    // If the IO thread has been started, shut it down and wait for it to exit.
    if (sIOThread != PTHREAD_NULL)
    {
        sShutdown = true;
        sSystemLayer.WakeSelect();
        pthread_join(sIOThread, NULL);
    }

    sSystemLayer.Shutdown();
    sInetLayer.Shutdown();

    pthread_mutex_destroy(&sStackLock);
    sJVM = NULL;
}

JNI_METHOD(jlong, newDeviceController)(JNIEnv * env, jobject self)
{
    CHIP_ERROR err                          = CHIP_NO_ERROR;
    ChipDeviceController * deviceController = NULL;
    long result                             = 0;

    ChipLogProgress(Controller, "newDeviceController() called");

    deviceController = new ChipDeviceController();
    VerifyOrExit(deviceController != NULL, err = CHIP_ERROR_NO_MEMORY);

    err = deviceController->Init(kLocalDeviceId, &sSystemLayer, &sInetLayer);
    SuccessOrExit(err);

    deviceController->AppState = (void *) env->NewGlobalRef(self);

    result = (long) deviceController;

exit:
    if (err != CHIP_NO_ERROR)
    {
        if (deviceController != NULL)
        {
            if (deviceController->AppState != NULL)
            {
                env->DeleteGlobalRef((jobject) deviceController->AppState);
            }
            deviceController->Shutdown();
            delete deviceController;
        }

        if (err != CDC_JNI_ERROR_EXCEPTION_THROWN)
        {
            ThrowError(env, err);
        }
    }

    return result;
}

JNI_METHOD(void, beginConnectDevice)(JNIEnv * env, jobject self, jlong deviceControllerPtr, jstring deviceAddr)
{
    CHIP_ERROR err                          = CHIP_NO_ERROR;
    ChipDeviceController * deviceController = (ChipDeviceController *) deviceControllerPtr;
    chip::Inet::IPAddress deviceIPAddr;

    ChipLogProgress(Controller, "beginConnectDevice() called with IP Address");

    const char * deviceAddrStr = env->GetStringUTFChars(deviceAddr, 0);
    chip::Inet::IPAddress::FromString(deviceAddrStr, deviceIPAddr);
    env->ReleaseStringUTFChars(deviceAddr, deviceAddrStr);

    pthread_mutex_lock(&sStackLock);
    err = deviceController->ConnectDevice(kRemoteDeviceId, deviceIPAddr, (void *) "ConnectDevice", HandleKeyExchange,
                                          HandleEchoResponse, HandleError, CHIP_PORT);
    pthread_mutex_unlock(&sStackLock);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to connect to device.");
        ThrowError(env, err);
    }
}

JNI_METHOD(void, beginSendMessage)(JNIEnv * env, jobject self, jlong deviceControllerPtr, jstring messageObj)
{
    CHIP_ERROR err                          = CHIP_NO_ERROR;
    ChipDeviceController * deviceController = (ChipDeviceController *) deviceControllerPtr;

    ChipLogProgress(Controller, "beginSendMessage() called");

    const char * messageStr = env->GetStringUTFChars(messageObj, 0);
    size_t messageLen       = strlen(messageStr);

    pthread_mutex_lock(&sStackLock);

    auto * buffer = System::PacketBuffer::NewWithAvailableSize(messageLen);
    memcpy(buffer->Start(), messageStr, messageLen);
    buffer->SetDataLength(messageLen);
    err = deviceController->SendMessage((void *) "SendMessage", buffer);

    pthread_mutex_unlock(&sStackLock);

    env->ReleaseStringUTFChars(messageObj, messageStr);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to send echo message.");
        ThrowError(env, err);
    }
}

JNI_METHOD(void, beginSendCommand)(JNIEnv * env, jobject self, jlong deviceControllerPtr, jobject commandObj)
{
    CHIP_ERROR err                          = CHIP_NO_ERROR;
    ChipDeviceController * deviceController = (ChipDeviceController *) deviceControllerPtr;

    ChipLogProgress(Controller, "beginSendCommand() called");

    jclass commandCls         = env->GetObjectClass(commandObj);
    jmethodID commandMethodID = env->GetMethodID(commandCls, "getValue", "()I");
    jint commandID            = env->CallIntMethod(commandObj, commandMethodID);

    pthread_mutex_lock(&sStackLock);

    // Make sure our buffer is big enough, but this will need a better setup!
    static const size_t bufferSize = 1024;
    auto * buffer                  = System::PacketBuffer::NewWithAvailableSize(bufferSize);

    // Hardcode endpoint to 1 for now
    uint8_t endpoint = 1;

    uint16_t dataLength = 0;
    switch (commandID)
    {
    case 0:
        dataLength = encodeOffCommand(buffer->Start(), bufferSize, endpoint);
        break;
    case 1:
        dataLength = encodeOnCommand(buffer->Start(), bufferSize, endpoint);
        break;
    case 2:
        dataLength = encodeToggleCommand(buffer->Start(), bufferSize, endpoint);
        break;
    default:
        ChipLogError(Controller, "Unknown command: %d", commandID);
        return;
    }

    buffer->SetDataLength(dataLength);

    // Hardcode endpoint to 1 for now
    err = deviceController->SendMessage((void *) "SendMessage", buffer);

    pthread_mutex_unlock(&sStackLock);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to send CHIP command.");
        ThrowError(env, err);
    }
}

JNI_METHOD(jboolean, isConnected)(JNIEnv * env, jobject self, jlong deviceControllerPtr)
{
    ChipLogProgress(Controller, "isConnected() called");
    ChipDeviceController * deviceController = (ChipDeviceController *) deviceControllerPtr;
    return deviceController->IsConnected() ? JNI_TRUE : JNI_FALSE;
}

JNI_METHOD(jboolean, disconnectDevice)(JNIEnv * env, jobject self, jlong deviceControllerPtr)
{
    ChipLogProgress(Controller, "disconnectDevice() called");

    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipDeviceController * deviceController = (ChipDeviceController *) deviceControllerPtr;

    pthread_mutex_lock(&sStackLock);
    err = deviceController->DisconnectDevice();
    pthread_mutex_unlock(&sStackLock);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to disconnect ChipDeviceController");
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

JNI_METHOD(void, deleteDeviceController)(JNIEnv * env, jobject self, jlong deviceControllerPtr)
{
    ChipDeviceController * deviceController = (ChipDeviceController *) deviceControllerPtr;

    ChipLogProgress(Controller, "deleteDeviceController() called");

    if (deviceController != NULL)
    {
        if (deviceController->AppState != NULL)
        {
            env->DeleteGlobalRef((jobject) deviceController->AppState);
        }
        deviceController->Shutdown();
        delete deviceController;
    }
}

void HandleSimpleOperationComplete(ChipDeviceController * deviceController, void * appReqState)
{
    JNIEnv * env;
    jclass deviceControllerCls;
    jmethodID methodID;
    char methodName[128];
    jobject self   = (jobject) deviceController->AppState;
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogProgress(Controller, "Received response to %s request", (const char *) appReqState);

    sJVM->GetEnv((void **) &env, JNI_VERSION_1_6);

    deviceControllerCls = env->GetObjectClass(self);
    VerifyOrExit(deviceControllerCls != NULL, err = CDC_JNI_ERROR_TYPE_NOT_FOUND);

    snprintf(methodName, sizeof(methodName) - 1, "on%sComplete", (const char *) appReqState);
    methodName[sizeof(methodName) - 1] = 0;
    methodID                           = env->GetMethodID(deviceControllerCls, methodName, "()V");
    VerifyOrExit(methodID != NULL, err = CDC_JNI_ERROR_METHOD_NOT_FOUND);

    ChipLogProgress(Controller, "Calling Java %s method", methodName);

    env->ExceptionClear();
    env->CallVoidMethod(self, methodID);
    VerifyOrExit(!env->ExceptionCheck(), err = CDC_JNI_ERROR_EXCEPTION_THROWN);

exit:
    if (err != CHIP_NO_ERROR)
    {
        const char * functName = __FUNCTION__;

        if (err == CDC_JNI_ERROR_EXCEPTION_THROWN)
        {
            ChipLogError(Controller, "Java Exception thrown in %s", functName);
            env->ExceptionDescribe();
        }
        else
        {
            const char * errStr;
            switch (err)
            {
            case CDC_JNI_ERROR_TYPE_NOT_FOUND:
                errStr = "JNI type not found";
                break;
            case CDC_JNI_ERROR_METHOD_NOT_FOUND:
                errStr = "JNI method not found";
                break;
            case CDC_JNI_ERROR_FIELD_NOT_FOUND:
                errStr = "JNI field not found";
                break;
            default:
                errStr = ErrorStr(err);
                break;
            }
            ChipLogError(Controller, "Error in %s : %s", functName, errStr);
        }
    }
    env->ExceptionClear();
}

void HandleKeyExchange(ChipDeviceController * deviceController, Transport::PeerConnectionState * state, void * appReqState)
{
    JNIEnv * env;

    sJVM->GetEnv((void **) &env, JNI_VERSION_1_6);

    CHIP_ERROR err = deviceController->ManualKeyExchange(state, remote_public_key, sizeof(remote_public_key), local_private_key,
                                                         sizeof(local_private_key));

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to exchange keys");
        ThrowError(env, err);
    }
    else
    {
        HandleSimpleOperationComplete(deviceController, appReqState);
    }
}

void HandleEchoResponse(ChipDeviceController * deviceController, void * appReqState, System::PacketBuffer * payload)
{
    JNIEnv * env;
    jclass deviceControllerCls;
    jmethodID methodID;
    jobject self      = (jobject) deviceController->AppState;
    jstring msgString = NULL;
    CHIP_ERROR err    = CHIP_NO_ERROR;

    sJVM->GetEnv((void **) &env, JNI_VERSION_1_6);

    int dataLen = payload->DataLength();
    char msgBuffer[dataLen];
    msgBuffer[dataLen] = 0;
    memcpy(msgBuffer, payload->Start(), dataLen);

    err = N2J_NewStringUTF(env, msgBuffer, msgString);

    ChipLogProgress(Controller, "Echo response %s", msgBuffer);

    deviceControllerCls = env->GetObjectClass(self);
    VerifyOrExit(deviceControllerCls != NULL, err = CDC_JNI_ERROR_TYPE_NOT_FOUND);

    methodID = env->GetMethodID(deviceControllerCls, "onSendMessageComplete", "(Ljava/lang/String;)V");
    VerifyOrExit(methodID != NULL, err = CDC_JNI_ERROR_METHOD_NOT_FOUND);

    env->ExceptionClear();
    env->CallVoidMethod(self, methodID, msgString);
    VerifyOrExit(!env->ExceptionCheck(), err = CDC_JNI_ERROR_EXCEPTION_THROWN);

exit:
    env->ExceptionClear();
    System::PacketBuffer::Free(payload);
}

void HandleError(ChipDeviceController * deviceController, void * appReqState, CHIP_ERROR err, const IPPacketInfo * pktInfo)
{
    JNIEnv * env;
    jclass cls;
    jmethodID method;
    jthrowable ex;
    jobject self = (jobject) deviceController->AppState;

    ChipLogError(Controller, "HandleError");

    sJVM->GetEnv((void **) &env, JNI_VERSION_1_6);

    err = N2J_Error(env, err, ex);
    SuccessOrExit(err);

    cls = env->GetObjectClass(self);
    VerifyOrExit(cls != NULL, err = CDC_JNI_ERROR_TYPE_NOT_FOUND);

    method = env->GetMethodID(cls, "onError", "(Ljava/lang/Throwable;)V");
    VerifyOrExit(method != NULL, err = CDC_JNI_ERROR_METHOD_NOT_FOUND);

    env->ExceptionClear();
    env->CallVoidMethod(self, method, ex);
    VerifyOrExit(!env->ExceptionCheck(), err = CDC_JNI_ERROR_EXCEPTION_THROWN);

exit:
    env->ExceptionClear();
}

void * IOThreadMain(void * arg)
{
    JNIEnv * env;
    JavaVMAttachArgs attachArgs;
    struct timeval sleepTime;
    fd_set readFDs, writeFDs, exceptFDs;
    int numFDs = 0;

    // Attach the IO thread to the JVM as a daemon thread.
    // This allows the JVM to shutdown without waiting for this thread to exit.
    attachArgs.version = JNI_VERSION_1_6;
    attachArgs.name    = (char *) "CHIP Device Controller IO Thread";
    attachArgs.group   = NULL;
#ifdef __ANDROID__
    sJVM->AttachCurrentThreadAsDaemon(&env, (void *) &attachArgs);
#else
    sJVM->AttachCurrentThreadAsDaemon((void **) &env, (void *) &attachArgs);
#endif

    ChipLogProgress(Controller, "IO thread starting");

    // Lock the stack to prevent collisions with Java threads.
    pthread_mutex_lock(&sStackLock);

    // Loop until we are told to exit.
    while (true)
    {
        numFDs = 0;
        FD_ZERO(&readFDs);
        FD_ZERO(&writeFDs);
        FD_ZERO(&exceptFDs);

        sleepTime.tv_sec  = 10;
        sleepTime.tv_usec = 0;

        // Collect the currently active file descriptors.
        sSystemLayer.PrepareSelect(numFDs, &readFDs, &writeFDs, &exceptFDs, sleepTime);
        sInetLayer.PrepareSelect(numFDs, &readFDs, &writeFDs, &exceptFDs, sleepTime);

        // Unlock the stack so that Java threads can make API calls.
        pthread_mutex_unlock(&sStackLock);

        // Wait for for I/O or for the next timer to expire.
        int selectRes = select(numFDs, &readFDs, &writeFDs, &exceptFDs, &sleepTime);

        // Break the loop if requested to shutdown.
        // if (sShutdown)
        // break;

        // Re-lock the stack.
        pthread_mutex_lock(&sStackLock);

        // Perform I/O and/or dispatch timers.
        sSystemLayer.HandleSelectResult(selectRes, &readFDs, &writeFDs, &exceptFDs);
        sInetLayer.HandleSelectResult(selectRes, &readFDs, &writeFDs, &exceptFDs);
    }

    // Detach the thread from the JVM.
    sJVM->DetachCurrentThread();

    return NULL;
}

void ThrowError(JNIEnv * env, CHIP_ERROR errToThrow)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    jthrowable ex;

    err = N2J_Error(env, errToThrow, ex);
    if (err == CHIP_NO_ERROR)
    {
        env->Throw(ex);
    }
}

CHIP_ERROR GetClassRef(JNIEnv * env, const char * clsType, jclass & outCls)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    jclass cls     = NULL;

    cls = env->FindClass(clsType);
    VerifyOrExit(cls != NULL, err = CDC_JNI_ERROR_TYPE_NOT_FOUND);

    outCls = (jclass) env->NewGlobalRef((jobject) cls);
    VerifyOrExit(outCls != NULL, err = CDC_JNI_ERROR_TYPE_NOT_FOUND);

exit:
    env->DeleteLocalRef(cls);
    return err;
}

CHIP_ERROR N2J_NewStringUTF(JNIEnv * env, const char * inStr, jstring & outString)
{
    return N2J_NewStringUTF(env, inStr, strlen(inStr), outString);
}

CHIP_ERROR N2J_ByteArray(JNIEnv * env, const uint8_t * inArray, uint32_t inArrayLen, jbyteArray & outArray)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    outArray = env->NewByteArray((int) inArrayLen);
    VerifyOrExit(outArray != NULL, err = CHIP_ERROR_NO_MEMORY);

    env->ExceptionClear();
    env->SetByteArrayRegion(outArray, 0, inArrayLen, (jbyte *) inArray);
    VerifyOrExit(!env->ExceptionCheck(), err = CDC_JNI_ERROR_EXCEPTION_THROWN);

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
    VerifyOrExit(java_lang_String != NULL, err = CDC_JNI_ERROR_TYPE_NOT_FOUND);

    ctor = env->GetMethodID(java_lang_String, "<init>", "([BLjava/lang/String;)V");
    VerifyOrExit(ctor != NULL, err = CDC_JNI_ERROR_METHOD_NOT_FOUND);

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

CHIP_ERROR N2J_Error(JNIEnv * env, CHIP_ERROR inErr, jthrowable & outEx)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    const char * errStr = NULL;
    jstring errStrObj   = NULL;
    jmethodID constructor;

    constructor = env->GetMethodID(sChipDeviceControllerExceptionCls, "<init>", "(ILjava/lang/String;)V");
    VerifyOrExit(constructor != NULL, err = CDC_JNI_ERROR_METHOD_NOT_FOUND);

    switch (inErr)
    {
    case CDC_JNI_ERROR_TYPE_NOT_FOUND:
        errStr = "CHIP Device Controller Error: JNI type not found";
        break;
    case CDC_JNI_ERROR_METHOD_NOT_FOUND:
        errStr = "CHIP Device Controller Error: JNI method not found";
        break;
    case CDC_JNI_ERROR_FIELD_NOT_FOUND:
        errStr = "CHIP Device Controller Error: JNI field not found";
        break;
    default:
        errStr = ErrorStr(inErr);
        break;
    }
    errStrObj = (errStr != NULL) ? env->NewStringUTF(errStr) : NULL;

    env->ExceptionClear();
    outEx = (jthrowable) env->NewObject(sChipDeviceControllerExceptionCls, constructor, (jint) inErr, errStrObj);
    VerifyOrExit(!env->ExceptionCheck(), err = CDC_JNI_ERROR_EXCEPTION_THROWN);

exit:
    env->DeleteLocalRef(errStrObj);
    return err;
}
