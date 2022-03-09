/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "DnssdImpl.h"

#include <lib/dnssd/platform/Dnssd.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <cstddef>
#include <jni.h>
#include <string>

namespace chip {
namespace Dnssd {

using namespace chip::Platform;

namespace {
jobject sResolverObject         = nullptr;
jobject sMdnsCallbackObject     = nullptr;
jmethodID sResolveMethod        = nullptr;
jmethodID sPublishMethod        = nullptr;
jmethodID sRemoveServicesMethod = nullptr;
} // namespace

// Implementation of functions declared in lib/dnssd/platform/Dnssd.h

CHIP_ERROR ChipDnssdInit(DnssdAsyncReturnCallback initCallback, DnssdAsyncReturnCallback errorCallback, void * context)
{
    VerifyOrReturnError(initCallback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(errorCallback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    initCallback(context, CHIP_NO_ERROR);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdShutdown()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdRemoveServices()
{
    VerifyOrReturnError(sResolverObject != nullptr && sRemoveServicesMethod != nullptr, CHIP_ERROR_INCORRECT_STATE);
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    {
        DeviceLayer::StackUnlock unlock;
        env->CallVoidMethod(sResolverObject, sRemoveServicesMethod);
    }

    if (env->ExceptionCheck())
    {
        ChipLogError(Discovery, "Java exception in ChipDnssdRemoveServices");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return CHIP_JNI_ERROR_EXCEPTION_THROWN;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdPublishService(const DnssdService * service, DnssdPublishCallback callback, void * context)
{
    VerifyOrReturnError(service != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(sResolverObject != nullptr && sPublishMethod != nullptr, CHIP_ERROR_INCORRECT_STATE);

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    UtfString jniName(env, service->mName);
    UtfString jniHostName(env, service->mHostName);

    std::string serviceType = service->mType;
    serviceType += '.';
    serviceType += (service->mProtocol == DnssdServiceProtocol::kDnssdProtocolUdp ? "_udp" : "_tcp");
    UtfString jniServiceType(env, serviceType.c_str());

    jclass stringClass = env->FindClass("java/lang/String");
    jobjectArray keys  = env->NewObjectArray(service->mTextEntrySize, stringClass, nullptr);

    jclass arrayElemType = env->FindClass("[B");
    jobjectArray datas   = env->NewObjectArray(service->mTextEntrySize, arrayElemType, nullptr);

    for (size_t i = 0; i < service->mTextEntrySize; i++)
    {
        UtfString jniKey(env, service->mTextEntries[i].mKey);
        env->SetObjectArrayElement(keys, i, jniKey.jniValue());

        ByteArray jniData(env, (const jbyte *) service->mTextEntries[i].mData, service->mTextEntries[i].mDataSize);
        env->SetObjectArrayElement(datas, i, jniData.jniValue());
    }

    jobjectArray subTypes = env->NewObjectArray(service->mSubTypeSize, stringClass, nullptr);
    for (size_t i = 0; i < service->mSubTypeSize; i++)
    {
        UtfString jniSubType(env, service->mSubTypes[i]);
        env->SetObjectArrayElement(subTypes, i, jniSubType.jniValue());
    }

    {
        DeviceLayer::StackUnlock unlock;
        env->CallVoidMethod(sResolverObject, sPublishMethod, jniName.jniValue(), jniHostName.jniValue(), jniServiceType.jniValue(),
                            service->mPort, keys, datas, subTypes);
    }

    if (env->ExceptionCheck())
    {
        ChipLogError(Discovery, "Java exception in ChipDnssdPublishService");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return CHIP_JNI_ERROR_EXCEPTION_THROWN;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdFinalizeServiceUpdate()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdBrowse(const char * type, DnssdServiceProtocol protocol, Inet::IPAddressType addressType,
                           Inet::InterfaceId interface, DnssdBrowseCallback callback, void * context)
{
    // TODO: Implement DNS-SD browse for Android
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ChipDnssdResolve(DnssdService * service, Inet::InterfaceId interface, DnssdResolveCallback callback, void * context)
{
    VerifyOrReturnError(service != nullptr && callback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(sResolverObject != nullptr && sResolveMethod != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(sMdnsCallbackObject != nullptr, CHIP_ERROR_INCORRECT_STATE);

    std::string serviceType = service->mType;
    serviceType += '.';
    serviceType += (service->mProtocol == DnssdServiceProtocol::kDnssdProtocolUdp ? "_udp" : "_tcp");

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    UtfString jniInstanceName(env, service->mName);
    UtfString jniServiceType(env, serviceType.c_str());

    {
        DeviceLayer::StackUnlock unlock;
        env->CallVoidMethod(sResolverObject, sResolveMethod, jniInstanceName.jniValue(), jniServiceType.jniValue(),
                            reinterpret_cast<jlong>(callback), reinterpret_cast<jlong>(context), sMdnsCallbackObject);
    }

    if (env->ExceptionCheck())
    {
        ChipLogError(Discovery, "Java exception in ChipDnssdResolve");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return CHIP_JNI_ERROR_EXCEPTION_THROWN;
    }

    return CHIP_NO_ERROR;
}

// Implemention of Java-specific functions

void InitializeWithObjects(jobject resolverObject, jobject mdnsCallbackObject)
{
    JNIEnv * env         = JniReferences::GetInstance().GetEnvForCurrentThread();
    sResolverObject      = env->NewGlobalRef(resolverObject);
    sMdnsCallbackObject  = env->NewGlobalRef(mdnsCallbackObject);
    jclass resolverClass = env->GetObjectClass(sResolverObject);

    VerifyOrReturn(resolverClass != nullptr, ChipLogError(Discovery, "Failed to get Resolver Java class"));

    sResolveMethod =
        env->GetMethodID(resolverClass, "resolve", "(Ljava/lang/String;Ljava/lang/String;JJLchip/platform/ChipMdnsCallback;)V");
    if (sResolveMethod == nullptr)
    {
        ChipLogError(Discovery, "Failed to access Resolver 'resolve' method");
        env->ExceptionClear();
    }

    sPublishMethod =
        env->GetMethodID(resolverClass, "publish",
                         "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I[Ljava/lang/String;[[B[Ljava/lang/String;)V");
    if (sPublishMethod == nullptr)
    {
        ChipLogError(Discovery, "Failed to access Resolver 'publish' method");
        env->ExceptionClear();
    }

    sRemoveServicesMethod = env->GetMethodID(resolverClass, "removeServices", "()V");
    if (sRemoveServicesMethod == nullptr)
    {
        ChipLogError(Discovery, "Failed to access Resolver 'removeServices' method");
        env->ExceptionClear();
    }
}

void HandleResolve(jstring instanceName, jstring serviceType, jstring address, jint port, jlong callbackHandle, jlong contextHandle)
{
    VerifyOrReturn(callbackHandle != 0, ChipLogError(Discovery, "HandleResolve called with callback equal to nullptr"));

    const auto dispatch = [callbackHandle, contextHandle](CHIP_ERROR error, DnssdService * service = nullptr) {
        DeviceLayer::StackLock lock;
        DnssdResolveCallback callback = reinterpret_cast<DnssdResolveCallback>(callbackHandle);
        callback(reinterpret_cast<void *>(contextHandle), service, Span<Inet::IPAddress>(), error);
    };

    VerifyOrReturn(address != nullptr && port != 0, dispatch(CHIP_ERROR_UNKNOWN_RESOURCE_ID));

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    JniUtfString jniInstanceName(env, instanceName);
    JniUtfString jniServiceType(env, serviceType);
    JniUtfString jniAddress(env, address);
    Inet::IPAddress ipAddress;

    VerifyOrReturn(strlen(jniInstanceName.c_str()) <= Operational::kInstanceNameMaxLength, dispatch(CHIP_ERROR_INVALID_ARGUMENT));
    VerifyOrReturn(strlen(jniServiceType.c_str()) <= kDnssdTypeAndProtocolMaxSize, dispatch(CHIP_ERROR_INVALID_ARGUMENT));
    VerifyOrReturn(CanCastTo<uint16_t>(port), dispatch(CHIP_ERROR_INVALID_ARGUMENT));
    VerifyOrReturn(Inet::IPAddress::FromString(jniAddress.c_str(), ipAddress), dispatch(CHIP_ERROR_INVALID_ARGUMENT));

    DnssdService service = {};
    CopyString(service.mName, jniInstanceName.c_str());
    CopyString(service.mType, jniServiceType.c_str());
    service.mAddress.SetValue(ipAddress);
    service.mPort = static_cast<uint16_t>(port);

    dispatch(CHIP_NO_ERROR, &service);
}

} // namespace Dnssd
} // namespace chip
