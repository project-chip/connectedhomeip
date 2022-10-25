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
#include "ConversionUtils.h"

#include <controller/CHIPCluster.h>
#include <jni.h>
#include <lib/core/CHIPError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

CHIP_ERROR convertJAppParametersToCppAppParams(jobject appParameters, AppParams & outAppParams)
{
    ChipLogProgress(AppServer, "convertJContentAppToTargetEndpointInfo called");
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnError(appParameters != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    jclass jAppParametersClass;
    ReturnErrorOnFailure(
        chip::JniReferences::GetInstance().GetClassRef(env, "com/chip/casting/AppParameters", jAppParametersClass));

    jfieldID jRotatingDeviceIdUniqueIdField = env->GetFieldID(jAppParametersClass, "rotatingDeviceIdUniqueId", "[B");
    jobject jRotatingDeviceIdUniqueId       = env->GetObjectField(appParameters, jRotatingDeviceIdUniqueIdField);
    if (jRotatingDeviceIdUniqueId != nullptr)
    {
        chip::JniByteArray jniRotatingDeviceIdUniqueIdByteArray(env, static_cast<jbyteArray>(jRotatingDeviceIdUniqueId));
        outAppParams.SetRotatingDeviceIdUniqueId(MakeOptional(jniRotatingDeviceIdUniqueIdByteArray.byteSpan()));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR convertJContentAppToTargetEndpointInfo(jobject contentApp, TargetEndpointInfo & outTargetEndpointInfo)
{
    ChipLogProgress(AppServer, "convertJContentAppToTargetEndpointInfo called");
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();

    jclass jContentAppClass;
    ReturnErrorOnFailure(chip::JniReferences::GetInstance().GetClassRef(env, "com/chip/casting/ContentApp", jContentAppClass));

    jfieldID jEndpointIdField = env->GetFieldID(jContentAppClass, "endpointId", "S");
    jshort jEndpointId        = env->GetShortField(contentApp, jEndpointIdField);
    outTargetEndpointInfo.Initialize(static_cast<chip::EndpointId>(jEndpointId));

    jfieldID jclusterIdsField = env->GetFieldID(jContentAppClass, "clusterIds", "Ljava/util/List;");
    jobject jClusterIds       = env->GetObjectField(contentApp, jclusterIdsField);
    if (jClusterIds == nullptr)
    {
        return CHIP_NO_ERROR;
    }

    jobject jIterator = env->CallObjectMethod(
        jClusterIds, env->GetMethodID(env->GetObjectClass(jClusterIds), "iterator", "()Ljava/util/Iterator;"));
    jmethodID jNextMid    = env->GetMethodID(env->GetObjectClass(jIterator), "next", "()Ljava/lang/Object;");
    jmethodID jHasNextMid = env->GetMethodID(env->GetObjectClass(jIterator), "hasNext", "()Z");

    while (env->CallBooleanMethod(jIterator, jHasNextMid))
    {
        jobject jClusterId     = env->CallObjectMethod(jIterator, jNextMid);
        jclass jIntegerClass   = env->FindClass("java/lang/Integer");
        jmethodID jIntValueMid = env->GetMethodID(jIntegerClass, "intValue", "()I");
        outTargetEndpointInfo.AddCluster(static_cast<chip::ClusterId>(env->CallIntMethod(jClusterId, jIntValueMid)));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR convertTargetEndpointInfoToJContentApp(TargetEndpointInfo * targetEndpointInfo, jobject & outContentApp)
{
    ChipLogProgress(AppServer, "convertTargetEndpointInfoToJContentApp called");
    if (targetEndpointInfo != nullptr && targetEndpointInfo->IsInitialized())
    {
        JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();

        jclass jContentAppClass;
        ReturnErrorOnFailure(chip::JniReferences::GetInstance().GetClassRef(env, "com/chip/casting/ContentApp", jContentAppClass));
        jmethodID jContentAppConstructor = env->GetMethodID(jContentAppClass, "<init>", "(SLjava/util/List;)V");
        chip::ClusterId * clusters       = targetEndpointInfo->GetClusters();
        jobject jClustersArrayList       = nullptr;
        if (clusters != nullptr)
        {
            chip::JniReferences::GetInstance().CreateArrayList(jClustersArrayList);
            for (size_t i = 0; i < kMaxNumberOfClustersPerEndpoint && clusters[i] != chip::kInvalidClusterId; i++)
            {
                jobject jCluster = nullptr;
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>("java/lang/Integer", "(I)V", clusters[i], jCluster);
                chip::JniReferences::GetInstance().AddToList(jClustersArrayList, jCluster);
            }
        }
        outContentApp =
            env->NewObject(jContentAppClass, jContentAppConstructor, targetEndpointInfo->GetEndpointId(), jClustersArrayList);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR convertJVideoPlayerToTargetVideoPlayerInfo(jobject videoPlayer, TargetVideoPlayerInfo & outTargetVideoPlayerInfo)
{
    ChipLogProgress(AppServer, "convertJVideoPlayerToTargetVideoPlayerInfo called");
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();

    jclass jVideoPlayerClass;
    ReturnErrorOnFailure(chip::JniReferences::GetInstance().GetClassRef(env, "com/chip/casting/VideoPlayer", jVideoPlayerClass));

    jfieldID jNodeIdField = env->GetFieldID(jVideoPlayerClass, "nodeId", "J");
    chip::NodeId nodeId   = static_cast<chip::NodeId>(env->GetLongField(videoPlayer, jNodeIdField));

    jfieldID jFabricIndexField    = env->GetFieldID(jVideoPlayerClass, "fabricIndex", "B");
    chip::FabricIndex fabricIndex = static_cast<chip::FabricIndex>(env->GetByteField(videoPlayer, jFabricIndexField));

    jfieldID jVendorIdField = env->GetFieldID(jVideoPlayerClass, "vendorId", "I");
    uint16_t vendorId       = static_cast<uint16_t>(env->GetIntField(videoPlayer, jVendorIdField));

    jfieldID jProductIdField = env->GetFieldID(jVideoPlayerClass, "productId", "I");
    uint16_t productId       = static_cast<uint16_t>(env->GetIntField(videoPlayer, jProductIdField));

    jfieldID jDeviceType = env->GetFieldID(jVideoPlayerClass, "deviceType", "I");
    uint16_t deviceType  = static_cast<uint16_t>(env->GetIntField(videoPlayer, jDeviceType));

    jfieldID getDeviceNameField = env->GetFieldID(jVideoPlayerClass, "deviceName", "Ljava/lang/String;");
    jstring jDeviceName         = static_cast<jstring>(env->GetObjectField(videoPlayer, getDeviceNameField));
    const char * deviceName     = env->GetStringUTFChars(jDeviceName, 0);
    outTargetVideoPlayerInfo.Initialize(nodeId, fabricIndex, nullptr, nullptr, vendorId, productId, deviceType, deviceName);

    jfieldID jContentAppsField = env->GetFieldID(jVideoPlayerClass, "contentApps", "Ljava/util/List;");
    jobject jContentApps       = env->GetObjectField(videoPlayer, jContentAppsField);
    if (jContentApps == nullptr)
    {
        return CHIP_NO_ERROR;
    }

    jobject jIterator = env->CallObjectMethod(
        jContentApps, env->GetMethodID(env->GetObjectClass(jContentApps), "iterator", "()Ljava/util/Iterator;"));
    jmethodID jNextMid    = env->GetMethodID(env->GetObjectClass(jIterator), "next", "()Ljava/lang/Object;");
    jmethodID jHasNextMid = env->GetMethodID(env->GetObjectClass(jIterator), "hasNext", "()Z");

    while (env->CallBooleanMethod(jIterator, jHasNextMid))
    {
        jobject jContentApp = env->CallObjectMethod(jIterator, jNextMid);

        jclass jContentAppClass;
        ReturnErrorOnFailure(chip::JniReferences::GetInstance().GetClassRef(env, "com/chip/casting/ContentApp", jContentAppClass));
        jfieldID jEndpointIdField     = env->GetFieldID(jContentAppClass, "endpointId", "S");
        chip::EndpointId endpointId   = static_cast<chip::EndpointId>(env->GetShortField(jContentApp, jEndpointIdField));
        TargetEndpointInfo * endpoint = outTargetVideoPlayerInfo.GetOrAddEndpoint(endpointId);

        ReturnErrorOnFailure(convertJContentAppToTargetEndpointInfo(jContentApp, *endpoint));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR convertTargetVideoPlayerInfoToJVideoPlayer(TargetVideoPlayerInfo * targetVideoPlayerInfo, jobject & outVideoPlayer)
{
    ChipLogProgress(AppServer, "convertTargetVideoPlayerInfoToJVideoPlayer called");
    if (targetVideoPlayerInfo != nullptr && targetVideoPlayerInfo->IsInitialized())
    {
        JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();

        jclass jVideoPlayerClass;
        ReturnErrorOnFailure(
            chip::JniReferences::GetInstance().GetClassRef(env, "com/chip/casting/VideoPlayer", jVideoPlayerClass));
        jmethodID jVideoPlayerConstructor =
            env->GetMethodID(jVideoPlayerClass, "<init>", "(JBLjava/lang/String;IIILjava/util/List;ILjava/util/List;Z)V");

        jobject jContentAppList        = nullptr;
        TargetEndpointInfo * endpoints = targetVideoPlayerInfo->GetEndpoints();
        if (endpoints != nullptr)
        {
            chip::JniReferences::GetInstance().CreateArrayList(jContentAppList);
            for (size_t i = 0; i < kMaxNumberOfEndpoints && endpoints[i].IsInitialized(); i++)
            {
                jobject contentApp = nullptr;
                ReturnErrorOnFailure(convertTargetEndpointInfoToJContentApp(&endpoints[i], contentApp));
                chip::JniReferences::GetInstance().AddToList(jContentAppList, contentApp);
            }
        }

        jstring deviceName =
            targetVideoPlayerInfo->GetDeviceName() == nullptr ? nullptr : env->NewStringUTF(targetVideoPlayerInfo->GetDeviceName());

        jobject jIPAddressList                    = nullptr;
        const chip::Inet::IPAddress * ipAddresses = targetVideoPlayerInfo->GetIpAddresses();
        if (ipAddresses != nullptr)
        {
            chip::JniReferences::GetInstance().CreateArrayList(jIPAddressList);
            for (size_t i = 0; i < targetVideoPlayerInfo->GetNumIPs() && i < chip::Dnssd::CommonResolutionData::kMaxIPAddresses;
                 i++)
            {
                char addrCString[chip::Inet::IPAddress::kMaxStringLength];
                ipAddresses[i].ToString(addrCString, chip::Inet::IPAddress::kMaxStringLength);
                jstring jIPAddressStr = env->NewStringUTF(addrCString);

                jclass jIPAddressClass;
                ReturnErrorOnFailure(chip::JniReferences::GetInstance().GetClassRef(env, "java/net/InetAddress", jIPAddressClass));
                jmethodID jGetByNameMid =
                    env->GetStaticMethodID(jIPAddressClass, "getByName", "(Ljava/lang/String;)Ljava/net/InetAddress;");
                jobject jIPAddress = env->CallStaticObjectMethod(jIPAddressClass, jGetByNameMid, jIPAddressStr);

                chip::JniReferences::GetInstance().AddToList(jIPAddressList, jIPAddress);
            }
        }

        outVideoPlayer = env->NewObject(jVideoPlayerClass, jVideoPlayerConstructor, targetVideoPlayerInfo->GetNodeId(),
                                        targetVideoPlayerInfo->GetFabricIndex(), deviceName, targetVideoPlayerInfo->GetVendorId(),
                                        targetVideoPlayerInfo->GetProductId(), targetVideoPlayerInfo->GetDeviceType(),
                                        jContentAppList, targetVideoPlayerInfo->GetNumIPs(), jIPAddressList,
                                        targetVideoPlayerInfo->GetOperationalDeviceProxy() != nullptr);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR convertJDiscoveredNodeDataToCppDiscoveredNodeData(jobject jDiscoveredNodeData,
                                                             chip::Dnssd::DiscoveredNodeData & outCppDiscoveredNodeData)
{
    ChipLogProgress(AppServer, "convertJDiscoveredNodeDataToCppDiscoveredNodeData called");

    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();

    jclass jDiscoveredNodeDataClass;
    ReturnErrorOnFailure(
        chip::JniReferences::GetInstance().GetClassRef(env, "com/chip/casting/DiscoveredNodeData", jDiscoveredNodeDataClass));

    jfieldID getHostNameField = env->GetFieldID(jDiscoveredNodeDataClass, "hostName", "Ljava/lang/String;");
    jstring jHostName         = static_cast<jstring>(env->GetObjectField(jDiscoveredNodeData, getHostNameField));
    if (jHostName != nullptr)
    {
        chip::Platform::CopyString(outCppDiscoveredNodeData.resolutionData.hostName, chip::Dnssd::kHostNameMaxLength + 1,
                                   env->GetStringUTFChars(jHostName, 0));
    }

    jfieldID getInstanceNameField = env->GetFieldID(jDiscoveredNodeDataClass, "deviceName", "Ljava/lang/String;");
    jstring jInstanceName         = static_cast<jstring>(env->GetObjectField(jDiscoveredNodeData, getInstanceNameField));
    if (jInstanceName != nullptr)
    {
        chip::Platform::CopyString(outCppDiscoveredNodeData.commissionData.instanceName,
                                   chip::Dnssd::Commission::kInstanceNameMaxLength + 1, env->GetStringUTFChars(jInstanceName, 0));
    }

    jfieldID jLongDiscriminatorField = env->GetFieldID(jDiscoveredNodeDataClass, "longDiscriminator", "J");
    outCppDiscoveredNodeData.commissionData.vendorId =
        static_cast<uint16_t>(env->GetLongField(jDiscoveredNodeData, jLongDiscriminatorField));

    jfieldID jVendorIdField = env->GetFieldID(jDiscoveredNodeDataClass, "vendorId", "J");
    outCppDiscoveredNodeData.commissionData.vendorId =
        static_cast<uint16_t>(env->GetLongField(jDiscoveredNodeData, jVendorIdField));

    jfieldID jProductIdField = env->GetFieldID(jDiscoveredNodeDataClass, "productId", "J");
    outCppDiscoveredNodeData.commissionData.productId =
        static_cast<uint16_t>(env->GetLongField(jDiscoveredNodeData, jProductIdField));

    jfieldID jCommissioningModeField = env->GetFieldID(jDiscoveredNodeDataClass, "commissioningMode", "B");
    outCppDiscoveredNodeData.commissionData.commissioningMode =
        static_cast<uint8_t>(env->GetByteField(jDiscoveredNodeData, jCommissioningModeField));

    jfieldID jDeviceTypeField = env->GetFieldID(jDiscoveredNodeDataClass, "deviceType", "J");
    outCppDiscoveredNodeData.commissionData.deviceType =
        static_cast<uint16_t>(env->GetLongField(jDiscoveredNodeData, jDeviceTypeField));

    jfieldID getDeviceNameField = env->GetFieldID(jDiscoveredNodeDataClass, "deviceName", "Ljava/lang/String;");
    jstring jDeviceName         = static_cast<jstring>(env->GetObjectField(jDiscoveredNodeData, getDeviceNameField));
    chip::Platform::CopyString(outCppDiscoveredNodeData.commissionData.deviceName, chip::Dnssd::kMaxDeviceNameLen + 1,
                               env->GetStringUTFChars(jDeviceName, 0));

    // TODO: map rotating ID
    jfieldID jRotatingIdLenField = env->GetFieldID(jDiscoveredNodeDataClass, "rotatingIdLen", "I");
    outCppDiscoveredNodeData.commissionData.rotatingIdLen =
        static_cast<size_t>(env->GetIntField(jDiscoveredNodeData, jRotatingIdLenField));

    jfieldID jPairingHintField = env->GetFieldID(jDiscoveredNodeDataClass, "pairingHint", "S");
    outCppDiscoveredNodeData.commissionData.pairingHint =
        static_cast<uint16_t>(env->GetShortField(jDiscoveredNodeData, jPairingHintField));

    jfieldID getPairingInstructionField = env->GetFieldID(jDiscoveredNodeDataClass, "pairingInstruction", "Ljava/lang/String;");
    jstring jPairingInstruction = static_cast<jstring>(env->GetObjectField(jDiscoveredNodeData, getPairingInstructionField));
    if (jPairingInstruction != nullptr)
    {
        chip::Platform::CopyString(outCppDiscoveredNodeData.commissionData.pairingInstruction,
                                   chip::Dnssd::kMaxPairingInstructionLen + 1, env->GetStringUTFChars(jPairingInstruction, 0));
    }

    jfieldID jPortField                          = env->GetFieldID(jDiscoveredNodeDataClass, "port", "I");
    outCppDiscoveredNodeData.resolutionData.port = static_cast<uint16_t>(env->GetIntField(jDiscoveredNodeData, jPortField));

    jfieldID jNumIpsField                          = env->GetFieldID(jDiscoveredNodeDataClass, "numIPs", "I");
    outCppDiscoveredNodeData.resolutionData.numIPs = static_cast<size_t>(env->GetIntField(jDiscoveredNodeData, jNumIpsField));

    jfieldID jIPAddressesField = env->GetFieldID(jDiscoveredNodeDataClass, "ipAddresses", "Ljava/util/List;");
    jobject jIPAddresses       = env->GetObjectField(jDiscoveredNodeData, jIPAddressesField);
    if (jIPAddresses == nullptr && outCppDiscoveredNodeData.resolutionData.numIPs > 0)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    jobject jIterator = env->CallObjectMethod(
        jIPAddresses, env->GetMethodID(env->GetObjectClass(jIPAddresses), "iterator", "()Ljava/util/Iterator;"));
    jmethodID jNextMid    = env->GetMethodID(env->GetObjectClass(jIterator), "next", "()Ljava/lang/Object;");
    jmethodID jHasNextMid = env->GetMethodID(env->GetObjectClass(jIterator), "hasNext", "()Z");

    size_t ipAddressCount = 0;
    while (env->CallBooleanMethod(jIterator, jHasNextMid))
    {
        jobject jIPAddress = env->CallObjectMethod(jIterator, jNextMid);
        jclass jIPAddressClass;
        ReturnErrorOnFailure(chip::JniReferences::GetInstance().GetClassRef(env, "java/net/InetAddress", jIPAddressClass));
        jmethodID jGetHostAddressMid = env->GetMethodID(jIPAddressClass, "getHostAddress", "()Ljava/lang/String;");
        jstring jIPAddressStr        = static_cast<jstring>(env->CallObjectMethod(jIPAddress, jGetHostAddressMid));

        chip::Inet::IPAddress addressInet;
        chip::JniUtfString addressJniString(env, jIPAddressStr);
        VerifyOrReturnError(chip::Inet::IPAddress::FromString(addressJniString.c_str(), addressInet), CHIP_ERROR_INVALID_ARGUMENT);
        outCppDiscoveredNodeData.resolutionData.ipAddress[ipAddressCount] = addressInet;

        if (ipAddressCount == 0)
        {
            outCppDiscoveredNodeData.resolutionData.interfaceId = chip::Inet::InterfaceId::FromIPAddress(addressInet);
        }
        ipAddressCount++;
    }

    return CHIP_NO_ERROR;
}
