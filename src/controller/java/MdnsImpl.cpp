/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "MdnsImpl.h"

#include "CHIPJNIError.h"
#include "JniReferences.h"
#include "JniTypeWrappers.h"

#include <lib/mdns/platform/Mdns.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>

#include <string>

namespace chip {
namespace Mdns {

using namespace chip::Controller;
using namespace chip::Platform;

namespace {
jobject sResolverObject     = nullptr;
jobject sMdnsCallbackObject = nullptr;
jmethodID sResolveMethod    = nullptr;
} // namespace

// Implemention of functions declared in lib/mdns/platform/Mdns.h

CHIP_ERROR ChipMdnsInit(MdnsAsyncReturnCallback initCallback, MdnsAsyncReturnCallback errorCallback, void * context)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipMdnsShutdown()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipMdnsPublishService(const MdnsService * service)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ChipMdnsStopPublish()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipMdnsStopPublishService(const MdnsService * service)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ChipMdnsBrowse(const char * type, MdnsServiceProtocol protocol, Inet::IPAddressType addressType,
                          Inet::InterfaceId interface, MdnsBrowseCallback callback, void * context)
{
    // TODO: Implement DNS-SD browse for Android
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ChipMdnsResolve(MdnsService * service, Inet::InterfaceId interface, MdnsResolveCallback callback, void * context)
{
    VerifyOrReturnError(service != nullptr && callback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(sResolverObject != nullptr && sResolveMethod != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(sMdnsCallbackObject != nullptr, CHIP_ERROR_INCORRECT_STATE);

    std::string serviceType = service->mType;
    serviceType += '.';
    serviceType += (service->mProtocol == MdnsServiceProtocol::kMdnsProtocolUdp ? "_udp" : "_tcp");

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    UtfString jniInstanceName(env, service->mName);
    UtfString jniServiceType(env, serviceType.c_str());

    env->CallVoidMethod(sResolverObject, sResolveMethod, jniInstanceName.jniValue(), jniServiceType.jniValue(),
                        reinterpret_cast<jlong>(callback), reinterpret_cast<jlong>(context), sMdnsCallbackObject);

    if (env->ExceptionCheck())
    {
        ChipLogError(Discovery, "Java exception in ChipMdnsResolve");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return CHIP_JNI_ERROR_EXCEPTION_THROWN;
    }

    return CHIP_NO_ERROR;
}

// Implementation of other methods required by the CHIP stack

void GetMdnsTimeout(timeval & timeout) {}
void HandleMdnsTimeout() {}

// Implemention of Java-specific functions

void InitializeWithObjects(jobject resolverObject, jobject mdnsCallbackObject)
{
    JNIEnv * env         = JniReferences::GetInstance().GetEnvForCurrentThread();
    sResolverObject      = env->NewGlobalRef(resolverObject);
    sMdnsCallbackObject  = env->NewGlobalRef(mdnsCallbackObject);
    jclass resolverClass = env->GetObjectClass(sResolverObject);

    VerifyOrReturn(resolverClass != nullptr, ChipLogError(Discovery, "Failed to get Resolver Java class"));

    sResolveMethod = env->GetMethodID(resolverClass, "resolve",
                                      "(Ljava/lang/String;Ljava/lang/String;JJLchip/devicecontroller/mdns/ChipMdnsCallback;)V");

    if (sResolveMethod == nullptr)
    {
        ChipLogError(Discovery, "Failed to access Resolver 'resolve' method");
        env->ExceptionClear();
    }
}

void HandleResolve(jstring instanceName, jstring serviceType, jstring address, jint port, jlong callbackHandle, jlong contextHandle)
{
    VerifyOrReturn(callbackHandle != 0, ChipLogError(Discovery, "HandleResolve called with callback equal to nullptr"));

    const auto dispatch = [callbackHandle, contextHandle](CHIP_ERROR error, MdnsService * service = nullptr) {
        MdnsResolveCallback callback = reinterpret_cast<MdnsResolveCallback>(callbackHandle);
        callback(reinterpret_cast<void *>(contextHandle), service, error);
    };

    VerifyOrReturn(address != nullptr && port != 0, dispatch(CHIP_ERROR_UNKNOWN_RESOURCE_ID));

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    JniUtfString jniInstanceName(env, instanceName);
    JniUtfString jniServiceType(env, serviceType);
    JniUtfString jniAddress(env, address);
    Inet::IPAddress ipAddress;

    VerifyOrReturn(strlen(jniInstanceName.c_str()) <= kMdnsInstanceNameMaxSize, dispatch(CHIP_ERROR_INVALID_ARGUMENT));
    VerifyOrReturn(strlen(jniServiceType.c_str()) <= kMdnsTypeAndProtocolMaxSize, dispatch(CHIP_ERROR_INVALID_ARGUMENT));
    VerifyOrReturn(CanCastTo<uint16_t>(port), dispatch(CHIP_ERROR_INVALID_ARGUMENT));
    VerifyOrReturn(Inet::IPAddress::FromString(jniAddress.c_str(), ipAddress), dispatch(CHIP_ERROR_INVALID_ARGUMENT));

    MdnsService service = {};
    CopyString(service.mName, jniInstanceName.c_str());
    CopyString(service.mType, jniServiceType.c_str());
    service.mAddress.SetValue(ipAddress);
    service.mPort = static_cast<uint16_t>(port);

    dispatch(CHIP_NO_ERROR, &service);
}

} // namespace Mdns
} // namespace chip
