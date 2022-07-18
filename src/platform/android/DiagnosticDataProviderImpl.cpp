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

/**
 *    @file
 *          Provides an implementation of the DiagnosticDataProvider object
 *          for android platform.
 */

#include <cstddef>
#include <cstring>
#include <jni.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include "DiagnosticDataProviderImpl.h"
#include <lib/support/CHIPJNIError.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/JniTypeWrappers.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/DiagnosticDataProvider.h>
#include <unistd.h>

using namespace ::chip::app::Clusters::GeneralDiagnostics;

namespace chip {
namespace DeviceLayer {

DiagnosticDataProviderImpl & DiagnosticDataProviderImpl::GetDefaultInstance()
{
    static DiagnosticDataProviderImpl sInstance;
    return sInstance;
}

void DiagnosticDataProviderImpl::InitializeWithObject(jobject manager)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr,
                   ChipLogError(DeviceLayer, "Failed to GetEnvForCurrentThread for DiagnosticDataProviderManagerImpl"));

    mDiagnosticDataProviderManagerObject = env->NewGlobalRef(manager);
    VerifyOrReturn(mDiagnosticDataProviderManagerObject != nullptr,
                   ChipLogError(DeviceLayer, "Failed to NewGlobalRef DiagnosticDataProviderManager"));

    jclass DiagnosticDataProviderManagerClass = env->GetObjectClass(manager);
    VerifyOrReturn(DiagnosticDataProviderManagerClass != nullptr,
                   ChipLogError(DeviceLayer, "Failed to get DiagnosticDataProviderManager Java class"));

    mGetRebootCountMethod = env->GetMethodID(DiagnosticDataProviderManagerClass, "getRebootCount", "()I");
    if (mGetRebootCountMethod == nullptr)
    {
        ChipLogError(DeviceLayer, "Failed to access DiagnosticDataProviderManager 'getRebootCount' method");
        env->ExceptionClear();
    }

    mGetNifMethod =
        env->GetMethodID(DiagnosticDataProviderManagerClass, "getNetworkInterfaces", "()[Lchip/platform/NetworkInterface;");
    if (mGetNifMethod == nullptr)
    {
        ChipLogError(DeviceLayer, "Failed to access DiagnosticDataProviderManager 'getNetworkInterfaces' method");
        env->ExceptionClear();
    }
}

CHIP_ERROR DiagnosticDataProviderImpl::GetRebootCount(uint16_t & rebootCount)
{
    chip::DeviceLayer::StackUnlock unlock;
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnLogError(mDiagnosticDataProviderManagerObject != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnLogError(mGetRebootCountMethod != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnLogError(env != nullptr, CHIP_JNI_ERROR_NO_ENV);
    ChipLogProgress(DeviceLayer, "Received GetRebootCount");

    jint count = env->CallIntMethod(mDiagnosticDataProviderManagerObject, mGetRebootCountMethod);
    VerifyOrReturnLogError(count < UINT16_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
    rebootCount = static_cast<uint16_t>(count);

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetNetworkInterfaces(NetworkInterface ** netifpp)
{
    chip::DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrExit(mDiagnosticDataProviderManagerObject != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mGetNifMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(env != nullptr, err = CHIP_JNI_ERROR_NO_ENV);
    {
        ChipLogProgress(DeviceLayer, "Received GetNetworkInterfaces");
        jobjectArray nifList = (jobjectArray) env->CallObjectMethod(mDiagnosticDataProviderManagerObject, mGetNifMethod);
        if (env->ExceptionCheck())
        {
            ChipLogError(DeviceLayer, "Java exception in DiagnosticDataProviderImpl::GetNetworkInterfaces");
            env->ExceptionDescribe();
            env->ExceptionClear();
            return CHIP_ERROR_INCORRECT_STATE;
        }
        jint length = env->GetArrayLength(nifList);

        NetworkInterface * head = nullptr;
        for (jint i = 0; i < length; i++)
        {
            NetworkInterface * ifp = new NetworkInterface();

            jobject nifObject = env->GetObjectArrayElement(nifList, i);
            jclass nifClass   = env->GetObjectClass(nifObject);

            jfieldID getNameField = env->GetFieldID(nifClass, "name", "Ljava/lang/String;");
            jstring jname         = static_cast<jstring>(env->GetObjectField(nifObject, getNameField));
            if (jname != nullptr)
            {
                JniUtfString name(env, jname);
                Platform::CopyString(ifp->Name, name.c_str());
                ifp->Name[Inet::InterfaceId::kMaxIfNameLength - 1] = '\0';
                ifp->name                                          = CharSpan(ifp->Name, strlen(ifp->Name));
            }

            jfieldID isOperationalField = env->GetFieldID(nifClass, "isOperational", "Z");
            ifp->isOperational          = static_cast<bool>(env->GetBooleanField(nifObject, isOperationalField));

            jfieldID getOpsrIPV4Field = env->GetFieldID(nifClass, "offPremiseServicesReachableIPv4", "Ljava/lang/Boolean;");
            jobject opsrIPV4Obj       = env->GetObjectField(nifObject, getOpsrIPV4Field);
            if (opsrIPV4Obj == nullptr)
            {
                ifp->offPremiseServicesReachableIPv4.SetNull();
            }
            else
            {
                jboolean opsrIPV4 = JniReferences::GetInstance().BooleanToPrimitive(opsrIPV4Obj);
                ifp->offPremiseServicesReachableIPv4.SetNonNull(static_cast<bool>(opsrIPV4));
            }

            jfieldID getOpsrIPV6Field = env->GetFieldID(nifClass, "offPremiseServicesReachableIPv6", "Ljava/lang/Boolean;");
            jobject opsrIPV6Obj       = env->GetObjectField(nifObject, getOpsrIPV6Field);
            if (opsrIPV6Obj == nullptr)
            {
                ifp->offPremiseServicesReachableIPv6.SetNull();
            }
            else
            {
                jboolean opsrIPV6 = JniReferences::GetInstance().BooleanToPrimitive(opsrIPV6Obj);
                ifp->offPremiseServicesReachableIPv6.SetNonNull(static_cast<bool>(opsrIPV6));
            }

            jfieldID gethardwareAddressField = env->GetFieldID(nifClass, "hardwareAddress", "[B");
            jbyteArray jHardwareAddressObj   = static_cast<jbyteArray>(env->GetObjectField(nifObject, gethardwareAddressField));
            if (jHardwareAddressObj != nullptr)
            {
                size_t len = env->GetArrayLength(jHardwareAddressObj);
                len        = (len > kMaxHardwareAddrSize) ? kMaxHardwareAddrSize : len;
                env->GetByteArrayRegion(jHardwareAddressObj, 0, len, reinterpret_cast<jbyte *>(ifp->MacAddress));
                ifp->hardwareAddress = ByteSpan(ifp->MacAddress, 6);
            }

            jfieldID getTypeField = env->GetFieldID(nifClass, "type", "I");
            ifp->type             = static_cast<InterfaceType>(env->GetIntField(nifObject, getTypeField));

            jfieldID ipv4AddressField  = env->GetFieldID(nifClass, "ipv4Address", "[B");
            jbyteArray jIpv4AddressObj = static_cast<jbyteArray>(env->GetObjectField(nifObject, ipv4AddressField));
            if (jIpv4AddressObj != nullptr)
            {
                JniByteArray Ipv4ByteArray(env, jIpv4AddressObj);

                if (Ipv4ByteArray.size() == kMaxIPv4AddrSize)
                {
                    memcpy(ifp->Ipv4AddressesBuffer[0], reinterpret_cast<const uint8_t *>(Ipv4ByteArray.data()), kMaxIPv4AddrSize);
                    ifp->Ipv4AddressSpans[0] = ByteSpan(ifp->Ipv4AddressesBuffer[0], kMaxIPv4AddrSize);
                    ifp->IPv4Addresses       = chip::app::DataModel::List<chip::ByteSpan>(ifp->Ipv4AddressSpans, 1);
                }
                else
                {
                    ChipLogError(DeviceLayer, "ipv4Address size (%d) not equal to kMaxIPv4AddrSize", Ipv4ByteArray.size());
                }
            }

            jfieldID ipv6AddressField  = env->GetFieldID(nifClass, "ipv6Address", "[B");
            jbyteArray jIpv6AddressObj = static_cast<jbyteArray>(env->GetObjectField(nifObject, ipv6AddressField));
            if (jIpv6AddressObj != nullptr)
            {
                JniByteArray Ipv6ByteArray(env, jIpv6AddressObj);

                if (Ipv6ByteArray.size() == kMaxIPv6AddrSize)
                {
                    memcpy(ifp->Ipv6AddressesBuffer[0], reinterpret_cast<const uint8_t *>(Ipv6ByteArray.data()), kMaxIPv6AddrSize);
                    ifp->Ipv6AddressSpans[0] = ByteSpan(ifp->Ipv6AddressesBuffer[0], kMaxIPv6AddrSize);
                    ifp->IPv6Addresses       = chip::app::DataModel::List<chip::ByteSpan>(ifp->Ipv6AddressSpans, 1);
                }
                else
                {
                    ChipLogError(DeviceLayer, "ipv6Address size (%d) not equal to kMaxIPv6AddrSize", Ipv6ByteArray.size());
                }
            }

            ifp->Next = head;
            head      = ifp;
        }
        *netifpp = head;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "ChannelManager::getChannelList status error: %s", err.AsString());
    }
    return err;
}

void DiagnosticDataProviderImpl::ReleaseNetworkInterfaces(NetworkInterface * netifp)
{
    while (netifp)
    {
        NetworkInterface * del = netifp;
        netifp                 = netifp->Next;
        delete del;
    }
}

DiagnosticDataProvider & GetDiagnosticDataProviderImpl()
{
    return DiagnosticDataProviderImpl::GetDefaultInstance();
}

} // namespace DeviceLayer
} // namespace chip
