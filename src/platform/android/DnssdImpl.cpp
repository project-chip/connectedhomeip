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

#include <cstddef>
#include <jni.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

#include <lib/dnssd/platform/Dnssd.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>

#include <memory>
#include <string>

namespace chip {
namespace Dnssd {

using namespace chip::Platform;

namespace {

jobject sResolverObject           = nullptr;
jobject sDiscoverObject           = nullptr;
jobject sMdnsCallbackObject       = nullptr;
jmethodID sResolveMethod          = nullptr;
jmethodID sDiscoveryMethod        = nullptr;
jmethodID sGetAttributeKeysMethod = nullptr;
jmethodID sGetAttributeDataMethod = nullptr;
jclass sMdnsCallbackClass         = nullptr;
jmethodID sPublishMethod        = nullptr;
jmethodID sRemoveServicesMethod = nullptr;

} // namespace

// Implemention of functions declared in lib/dnssd/platform/Dnssd.h

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

    env->CallVoidMethod(sResolverObject, sRemoveServicesMethod);

    if (env->ExceptionCheck())
    {
        ChipLogError(Discovery, "Java exception in ChipDnssdRemoveServices");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return CHIP_JNI_ERROR_EXCEPTION_THROWN;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdPublishService(const DnssdService * service)
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

    env->CallVoidMethod(sResolverObject, sPublishMethod, jniName.jniValue(), jniHostName.jniValue(), jniServiceType.jniValue(),
                        service->mPort, keys, datas, subTypes);

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
    VerifyOrReturnError(type != nullptr && callback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(sDiscoverObject != nullptr && sDiscoveryMethod != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(sMdnsCallbackObject != nullptr, CHIP_ERROR_INCORRECT_STATE);

    std::string serviceType = type;
    serviceType += '.';
    serviceType += (protocol == DnssdServiceProtocol::kDnssdProtocolUdp ? "_udp" : "_tcp");

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    UtfString jniServiceType(env, serviceType.c_str());

    env->CallVoidMethod(sDiscoverObject, sDiscoveryMethod, jniServiceType.jniValue(), reinterpret_cast<jlong>(callback),
                        reinterpret_cast<jlong>(context), sMdnsCallbackObject);

    if (env->ExceptionCheck())
    {
        ChipLogError(Discovery, "Java exception in ChipDnssdBrowse");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return CHIP_JNI_ERROR_EXCEPTION_THROWN;
    }

    return CHIP_NO_ERROR;
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

    env->CallVoidMethod(sResolverObject, sResolveMethod, jniInstanceName.jniValue(), jniServiceType.jniValue(),
                        reinterpret_cast<jlong>(callback), reinterpret_cast<jlong>(context), sMdnsCallbackObject);

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

void InitializeWithObjects(jobject resolverObject, jobject browseObject, jobject mdnsCallbackObject)
{
    JNIEnv * env         = JniReferences::GetInstance().GetEnvForCurrentThread();
    sResolverObject      = env->NewGlobalRef(resolverObject);
    sDiscoverObject      = env->NewGlobalRef(browseObject);
    sMdnsCallbackObject  = env->NewGlobalRef(mdnsCallbackObject);
    jclass resolverClass = env->GetObjectClass(sResolverObject);
    jclass browseClass   = env->GetObjectClass(browseObject);
    sMdnsCallbackClass   = env->GetObjectClass(sMdnsCallbackObject);

    VerifyOrReturn(browseClass != nullptr, ChipLogError(Discovery, "Failed to get Browse Java class"));
    VerifyOrReturn(resolverClass != nullptr, ChipLogError(Discovery, "Failed to get Resolver Java class"));

    sResolveMethod =
        env->GetMethodID(resolverClass, "resolve", "(Ljava/lang/String;Ljava/lang/String;JJLchip/platform/ChipMdnsCallback;)V");

    sDiscoveryMethod = env->GetMethodID(browseClass, "browse", "(Ljava/lang/String;JJLchip/platform/ChipMdnsCallback;)V");

    sGetAttributeKeysMethod = env->GetMethodID(sMdnsCallbackClass, "getAttributeKeys", "(Ljava/util/Map;)[Ljava/lang/String;");

    sGetAttributeDataMethod = env->GetMethodID(sMdnsCallbackClass, "getAttributeData", "(Ljava/util/Map;Ljava/lang/String;)[B");

    if (sResolveMethod == nullptr)
    {
        ChipLogError(Discovery, "Failed to access Resolver 'resolve' method");
        env->ExceptionClear();
    }

    if (sDiscoveryMethod == nullptr)
    {
        ChipLogError(Discovery, "Failed to access Discover 'browse' method");
        env->ExceptionClear();
    }

    if (sGetAttributeKeysMethod == nullptr)
    {
        ChipLogError(Discovery, "Failed to access MdnsCallback 'getAttributeKeys' method");
        env->ExceptionClear();
    }

    if (sGetAttributeDataMethod == nullptr)
    {
        ChipLogError(Discovery, "Failed to access MdnsCallback 'getAttributeData' method");
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

void HandleResolve(jstring instanceName, jstring serviceType, jstring hostName, jstring address, jint port, jobject attributes,
                   jlong callbackHandle, jlong contextHandle)
{
    VerifyOrReturn(callbackHandle != 0, ChipLogError(Discovery, "HandleResolve called with callback equal to nullptr"));

    const auto dispatch = [callbackHandle, contextHandle](CHIP_ERROR error, DnssdService * service = nullptr) {
        DnssdResolveCallback callback = reinterpret_cast<DnssdResolveCallback>(callbackHandle);
        callback(reinterpret_cast<void *>(contextHandle), service, error);
    };

    VerifyOrReturn(address != nullptr && port != 0, dispatch(CHIP_ERROR_UNKNOWN_RESOURCE_ID));

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    JniUtfString jniInstanceName(env, instanceName);
    JniUtfString jniServiceType(env, serviceType);
    JniUtfString jnihostName(env, hostName);
    JniUtfString jniAddress(env, address);
    Inet::IPAddress ipAddress;

    VerifyOrReturn(strlen(jniInstanceName.c_str()) <= Operational::kInstanceNameMaxLength, dispatch(CHIP_ERROR_INVALID_ARGUMENT));
    VerifyOrReturn(strlen(jniServiceType.c_str()) <= kDnssdTypeAndProtocolMaxSize, dispatch(CHIP_ERROR_INVALID_ARGUMENT));
    VerifyOrReturn(CanCastTo<uint16_t>(port), dispatch(CHIP_ERROR_INVALID_ARGUMENT));
    VerifyOrReturn(Inet::IPAddress::FromString(jniAddress.c_str(), ipAddress), dispatch(CHIP_ERROR_INVALID_ARGUMENT));

    DnssdService service = {};
    CopyString(service.mName, jniInstanceName.c_str());
    CopyString(service.mHostName, jnihostName.c_str());
    CopyString(service.mType, jniServiceType.c_str());
    service.mAddress.SetValue(ipAddress);
    service.mPort = static_cast<uint16_t>(port);

    if (attributes != nullptr)
    {
        jobjectArray keys   = (jobjectArray) env->CallObjectMethod(sMdnsCallbackObject, sGetAttributeKeysMethod, attributes);
        size_t size         = env->GetArrayLength(keys);
        TextEntry * entries = new TextEntry[size];
        for (size_t i = 0; i < size; i++)
        {
            jstring jniKeyObject = (jstring) env->GetObjectArrayElement(keys, i);
            entries[i].mKey      = getTxtInfoKey(env, jniKeyObject);

            jbyteArray datas =
                (jbyteArray) env->CallObjectMethod(sMdnsCallbackObject, sGetAttributeDataMethod, attributes, jniKeyObject);
            if (datas != nullptr)
            {
                size_t dataSize = env->GetArrayLength(datas);
                uint8_t * data  = new uint8_t[dataSize + 1];
                jbyte * jnidata = env->GetByteArrayElements(datas, nullptr);
                for (size_t j = 0; j < dataSize; j++)
                {
                    data[j] = (uint8_t) jnidata[j];
                }
                entries[i].mDataSize = dataSize;
                entries[i].mData     = data;
            }
            else
            {
                entries[i].mDataSize = 0;
                entries[i].mData     = nullptr;
            }
        }
        service.mTextEntrySize = size;
        service.mTextEntries   = entries;
    }
    else
    {
        ChipLogError(Discovery, "attributes is null");
        service.mTextEntrySize = 0;
        service.mTextEntries   = nullptr;
    }
    dispatch(CHIP_NO_ERROR, &service);

    if (service.mTextEntries != nullptr)
    {
        size_t size = service.mTextEntrySize;
        for (size_t i = 0; i < size; i++)
        {
            // if (service.mTextEntries[i].mKey != nullptr) {
            //    delete[] service.mTextEntries[i].mKey;
            //}
            if (service.mTextEntries[i].mData != nullptr)
            {
                delete[] service.mTextEntries[i].mData;
            }
        }
        delete[] service.mTextEntries;
    }
}

void HandleBrowse(jobjectArray instanceName, jstring serviceType, jlong callbackHandle, jlong contextHandle)
{
    VerifyOrReturn(callbackHandle != 0, ChipLogError(Discovery, "HandleDiscover called with callback equal to nullptr"));

    const auto dispatch = [callbackHandle, contextHandle](CHIP_ERROR error, DnssdService * service = nullptr, size_t size = 0) {
        DnssdBrowseCallback callback = reinterpret_cast<DnssdBrowseCallback>(callbackHandle);
        callback(reinterpret_cast<void *>(contextHandle), service, size, error);
    };

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    JniUtfString jniServiceType(env, serviceType);

    VerifyOrReturn(strlen(jniServiceType.c_str()) <= kDnssdTypeAndProtocolMaxSize, dispatch(CHIP_ERROR_INVALID_ARGUMENT));

    size_t size            = env->GetArrayLength(instanceName);
    DnssdService * service = new DnssdService[size];
    for (size_t i = 0; i < size; i++)
    {
        JniUtfString jniInstanceName(env, (jstring) env->GetObjectArrayElement(instanceName, i));
        VerifyOrReturn(strlen(jniInstanceName.c_str()) <= Operational::kInstanceNameMaxLength,
                       dispatch(CHIP_ERROR_INVALID_ARGUMENT));

        CopyString(service[i].mName, jniInstanceName.c_str());
        CopyString(service[i].mType, jniServiceType.c_str());
    }

    dispatch(CHIP_NO_ERROR, service, size);
    delete[] service;
}

const char * getTxtInfoKey(JNIEnv * env, jstring key)
{
    const char * keyList[] = { "D", "VP", "AP", "CM", "DT", "DN", "RI", "PI", "PH", "CRI", "CRA", "T" };
    JniUtfString jniKey(env, key);
    for (auto & info : keyList)
    {
        if (strcmp(info, jniKey.c_str()) == 0)
        {
            return info;
        }
    }
    return "";
}

} // namespace Dnssd
} // namespace chip
