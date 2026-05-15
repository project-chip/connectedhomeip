/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

#include <platform/ConnectivityManager.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include "ConnectivityManagerImpl.h"

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF

#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>
#include <lib/support/logging/CHIPLogging.h>
#include <wifipaf/WiFiPAFLayer.h>

using namespace ::chip::WiFiPAF;

namespace chip {
namespace DeviceLayer {

void ConnectivityManagerImpl::InitializeWifiPafManager(jobject manager)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(DeviceLayer, "InitializeWifiPafManager: JNIEnv is null"));

    mWifiPafManagerObject = env->NewGlobalRef(manager);
    VerifyOrReturn(mWifiPafManagerObject != nullptr, ChipLogError(DeviceLayer, "Failed to create global ref for WifiPafManager"));

    jclass managerClass = env->GetObjectClass(mWifiPafManagerObject);
    VerifyOrReturn(managerClass != nullptr, ChipLogError(DeviceLayer, "Failed to get WifiPafManager class"));

    mWifiPafSubscribeMethod       = env->GetMethodID(managerClass, "wifiPafSubscribe", "(II)I");
    mWifiPafCancelSubscribeMethod = env->GetMethodID(managerClass, "wifiPafCancelSubscribe", "(I)I");
    mWifiPafSendMethod            = env->GetMethodID(managerClass, "wifiPafSend", "(IILjava/lang/String;[B)Z");

    env->DeleteLocalRef(managerClass);

    if (mWifiPafSubscribeMethod == nullptr || mWifiPafCancelSubscribeMethod == nullptr || mWifiPafSendMethod == nullptr)
    {
        ChipLogError(DeviceLayer, "Failed to get WifiPafManager method IDs");
        env->DeleteGlobalRef(mWifiPafManagerObject);
        mWifiPafManagerObject = nullptr;
    }
}

CHIP_ERROR ConnectivityManagerImpl::_WiFiPAFSubscribe(const uint16_t & connDiscriminator, void * appState,
                                                      OnConnectionCompleteFunct onSuccess, OnConnectionErrorFunct onError)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnError(env != nullptr, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(mWifiPafManagerObject != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mWifiPafSubscribeMethod != nullptr, CHIP_ERROR_INCORRECT_STATE);

    mAppState               = appState;
    mOnPafSubscribeComplete = onSuccess;
    mOnPafSubscribeError    = onError;

    // Default freq to 0 (let Java choose)
    jint subscribeId = env->CallIntMethod(mWifiPafManagerObject, mWifiPafSubscribeMethod, static_cast<jint>(connDiscriminator),
                                          static_cast<jint>(0));
    if (env->ExceptionCheck())
    {
        ChipLogError(DeviceLayer, "Exception in WifiPafManager.wifiPafSubscribe");
        env->ExceptionClear();
        return CHIP_ERROR_INTERNAL;
    }

    ChipLogProgress(DeviceLayer, "WiFi-PAF: Java subscribe returned ID: %d", subscribeId);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::_WiFiPAFCancelSubscribe(uint32_t SubscribeId)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnError(env != nullptr, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(mWifiPafManagerObject != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mWifiPafCancelSubscribeMethod != nullptr, CHIP_ERROR_INCORRECT_STATE);

    env->CallIntMethod(mWifiPafManagerObject, mWifiPafCancelSubscribeMethod, static_cast<jint>(SubscribeId));
    if (env->ExceptionCheck())
    {
        ChipLogError(DeviceLayer, "Exception in WifiPafManager.wifiPafCancelSubscribe");
        env->ExceptionClear();
        return CHIP_ERROR_INTERNAL;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::_WiFiPAFCancelIncompleteSubscribe()
{
    mOnPafSubscribeComplete = nullptr;
    mOnPafSubscribeError    = nullptr;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::_WiFiPAFSend(const WiFiPAF::WiFiPAFSession & TxInfo, System::PacketBufferHandle && msgBuf)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnError(env != nullptr, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(mWifiPafManagerObject != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mWifiPafSendMethod != nullptr, CHIP_ERROR_INCORRECT_STATE);

    // Convert peer_addr to string
    char addrStr[18];
    snprintf(addrStr, sizeof(addrStr), "%02x:%02x:%02x:%02x:%02x:%02x", TxInfo.peer_addr[0], TxInfo.peer_addr[1],
             TxInfo.peer_addr[2], TxInfo.peer_addr[3], TxInfo.peer_addr[4], TxInfo.peer_addr[5]);

    jstring jPeerAddr = env->NewStringUTF(addrStr);
    VerifyOrReturnError(jPeerAddr != nullptr, CHIP_ERROR_NO_MEMORY);

    // Convert msgBuf to jbyteArray
    jbyteArray jData;
    CHIP_ERROR err = JniReferences::GetInstance().N2J_ByteArray(env, msgBuf->Start(), static_cast<jsize>(msgBuf->DataLength()), jData);
    if (err != CHIP_NO_ERROR)
    {
        env->DeleteLocalRef(jPeerAddr);
        return err;
    }

    jboolean result = env->CallBooleanMethod(mWifiPafManagerObject, mWifiPafSendMethod, static_cast<jint>(TxInfo.id),
                                             static_cast<jint>(TxInfo.peer_id), jPeerAddr, jData);

    env->DeleteLocalRef(jPeerAddr);
    env->DeleteLocalRef(jData);

    if (env->ExceptionCheck())
    {
        ChipLogError(DeviceLayer, "Exception in WifiPafManager.wifiPafSend");
        env->ExceptionClear();
        return CHIP_ERROR_INTERNAL;
    }

    return result ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
}

void ConnectivityManagerImpl::HandleDiscoveryResult(int subscribeId, int peerPublishId, const char * peerAddr, int discriminator,
                                                   int productId, int vendorId)
{
    ChipLogProgress(DeviceLayer, "WiFi-PAF: HandleDiscoveryResult: subscribeId=%d, peerAddr=%s, discriminator=%d", subscribeId,
                    peerAddr, discriminator);

    WiFiPAFSession sessionInfo;
    sessionInfo.role          = WiFiPafRole::kWiFiPafRole_Subscriber;
    sessionInfo.id            = static_cast<uint32_t>(subscribeId);
    sessionInfo.peer_id       = static_cast<uint32_t>(peerPublishId);
    sessionInfo.nodeId        = kUndefinedNodeId;
    sessionInfo.discriminator = static_cast<uint16_t>(discriminator);

    // Parse MAC address string to bytes
    sscanf(peerAddr, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", &sessionInfo.peer_addr[0], &sessionInfo.peer_addr[1],
           &sessionInfo.peer_addr[2], &sessionInfo.peer_addr[3], &sessionInfo.peer_addr[4], &sessionInfo.peer_addr[5]);

    WiFiPAFLayer & WiFiPafLayer = WiFiPAFLayer::GetWiFiPAFLayer();
    CHIP_ERROR err              = WiFiPafLayer.AddPafSession(PafInfoAccess::kAccDisc, sessionInfo);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to AddPafSession: %" CHIP_ERROR_FORMAT, err.Format());
        return;
    }

    WiFiPAFSession * pPafInfo = WiFiPafLayer.GetPAFInfo(PafInfoAccess::kAccDisc, sessionInfo);
    if (pPafInfo == nullptr)
    {
        ChipLogError(DeviceLayer, "Failed to get PAF info after add");
        return;
    }

    pPafInfo->id      = static_cast<uint32_t>(subscribeId);
    pPafInfo->peer_id = static_cast<uint32_t>(peerPublishId);
    memcpy(pPafInfo->peer_addr, sessionInfo.peer_addr, 6);

    // Post event to Chip stack
    ChipDeviceEvent event{ .Type = DeviceEventType::kCHIPoWiFiPAFConnected };
    memcpy(&event.CHIPoWiFiPAFReceived.SessionInfo, pPafInfo, sizeof(WiFiPAFSession));
    PlatformMgr().PostEventOrDie(&event);
}

void ConnectivityManagerImpl::HandleReceive(int id, int peerId, const char * peerAddr, const uint8_t * data, size_t len)
{
    ChipLogProgress(DeviceLayer, "WiFi-PAF: HandleReceive: id=%d, peerId=%d, peerAddr=%s, len=%zu", id, peerId, peerAddr, len);

    WiFiPAFSession rxInfo;
    rxInfo.id      = static_cast<uint32_t>(id);
    rxInfo.peer_id = static_cast<uint32_t>(peerId);
    sscanf(peerAddr, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", &rxInfo.peer_addr[0], &rxInfo.peer_addr[1], &rxInfo.peer_addr[2],
           &rxInfo.peer_addr[3], &rxInfo.peer_addr[4], &rxInfo.peer_addr[5]);

    System::PacketBufferHandle buf = System::PacketBufferHandle::NewWithData(data, len);
    if (buf.IsNull())
    {
        ChipLogError(DeviceLayer, "Failed to allocate packet buffer for received data");
        return;
    }

    ChipDeviceEvent event{ .Type                 = DeviceEventType::kCHIPoWiFiPAFReceived,
                           .CHIPoWiFiPAFReceived = { .Data = std::move(buf).UnsafeRelease() } };
    memcpy(&event.CHIPoWiFiPAFReceived.SessionInfo, &rxInfo, sizeof(WiFiPAFSession));
    PlatformMgr().PostEventOrDie(&event);
}

void ConnectivityManagerImpl::HandleSubscribeTerminated(int subscribeId, const char * reason)
{
    ChipLogProgress(DeviceLayer, "WiFi-PAF: HandleSubscribeTerminated: id=%d, reason=%s", subscribeId, reason);
    WiFiPAFSession sessionInfo;
    sessionInfo.id              = static_cast<uint32_t>(subscribeId);
    WiFiPAFLayer & WiFiPafLayer = WiFiPAFLayer::GetWiFiPAFLayer();
    TEMPORARY_RETURN_IGNORED WiFiPafLayer.RmPafSession(PafInfoAccess::kAccSessionId, sessionInfo);

    ChipDeviceEvent event{ .Type = DeviceEventType::kCHIPoWiFiPAFCancelConnect };
    PlatformMgr().PostEventOrDie(&event);
}

void ConnectivityManagerImpl::HandleSendWriteDone(int id, int peerId, const char * peerAddr, bool success)
{
    ChipLogProgress(DeviceLayer, "WiFi-PAF: HandleSendWriteDone: id=%d, success=%d", id, success);

    WiFiPAFSession txInfo;
    txInfo.id      = static_cast<uint32_t>(id);
    txInfo.peer_id = static_cast<uint32_t>(peerId);
    sscanf(peerAddr, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", &txInfo.peer_addr[0], &txInfo.peer_addr[1], &txInfo.peer_addr[2],
           &txInfo.peer_addr[3], &txInfo.peer_addr[4], &txInfo.peer_addr[5]);

    ChipDeviceEvent event{ .Type = DeviceEventType::kCHIPoWiFiPAFWriteDone };
    memcpy(&event.CHIPoWiFiPAFReceived.SessionInfo, &txInfo, sizeof(WiFiPAFSession));
    event.CHIPoWiFiPAFReceived.result = success;
    PlatformMgr().PostEventOrDie(&event);
}

CHIP_ERROR ConnectivityManagerImpl::_WiFiPAFShutdown(uint32_t id, WiFiPAF::WiFiPafRole role)
{
    VerifyOrReturnError(((id != kUndefinedWiFiPafSessionId) && (id != 0)), CHIP_ERROR_INTERNAL);
    if (role == WiFiPAF::WiFiPafRole::kWiFiPafRole_Subscriber)
    {
        return _WiFiPAFCancelSubscribe(id);
    }
    return CHIP_ERROR_INTERNAL;
}

// Publisher methods stubbed out as they are not needed for chip-tool (Subscriber only)

void ConnectivityManagerImpl::_WiFiPAFSetParam(const WiFiPAFAdvertiseParam & pafAdvParam)
{
    ChipLogError(DeviceLayer, "WiFi-PAF: Publisher role not supported on Android");
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiPAFAdvertisingEnabled(bool enabled, uint32_t & publishId)
{
    ChipLogError(DeviceLayer, "WiFi-PAF: Publisher role not supported on Android");
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR ConnectivityManagerImpl::_WiFiPAFPublish(WiFiPAFAdvertiseParam & args)
{
    ChipLogError(DeviceLayer, "WiFi-PAF: Publisher role not supported on Android");
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR ConnectivityManagerImpl::_WiFiPAFCancelPublish(uint32_t PublishId)
{
    ChipLogError(DeviceLayer, "WiFi-PAF: Publisher role not supported on Android");
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

void ConnectivityManagerImpl::_WiFiPafSetApFreq(const uint16_t freq) {}

bool ConnectivityManagerImpl::_WiFiPAFResourceAvailable()
{
    return true;
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF

} // namespace DeviceLayer
} // namespace chip
