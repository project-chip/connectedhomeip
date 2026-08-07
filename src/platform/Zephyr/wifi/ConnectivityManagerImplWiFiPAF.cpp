/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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
#include <platform/ConnectivityManager.h>

#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/Zephyr/InetUtils.h>

#include <zephyr/kernel.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/wifi_mgmt.h>

#include "ConnectivityManagerImplWiFiPAF.h"

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;

namespace chip {
namespace DeviceLayer {

// Service name for Matter NAN-USD
static constexpr char kMatterNanServiceName[] = "_matterc._udp";
// NAN Service Protocol Type for Matter (CSA Matter = 3)
static constexpr uint8_t kNanServiceProtocolTypeMatter = 3;
// Default TTL for NAN publish (seconds)
static constexpr uint32_t kDefaultPublishTtl = CHIP_DEVICE_CONFIG_WIFIPAF_MAX_ADVERTISING_TIMEOUT_SECS;
// Default frequency for 2.4GHz
static constexpr uint16_t kDefault24GHzFreq = CHIP_DEVICE_CONFIG_WIFIPAF_24G_DEFAUTL_CHNL;

// PAF Publish SSI structure
struct PAFPublishSSI
{
    uint8_t DevOpCode;
    uint16_t DevInfo; // Discriminator
    uint16_t ProductId;
    uint16_t VendorId;
} __packed;

CHIP_ERROR ConnectivityManagerImplWiFiPAF::InitWiFiPAF()
{
    ChipLogProgress(DeviceLayer, "WiFi-PAF: Initializing NAN transport");

    k_mutex_init(&mNanMutex);

    // Get and cache the WiFi interface
    mWiFiIface = InetUtils::GetWiFiInterface();
    if (mWiFiIface == nullptr)
    {
        ChipLogError(DeviceLayer, "WiFi-PAF: Failed to get WiFi interface");
        return CHIP_ERROR_INTERNAL;
    }

    // Initialize state
    mApFreq           = 0;
    mCurrentPublishId = 0;
    mPublishActive    = false;
    mPeerRegistered   = false;

    // Register NAN event callbacks
    net_mgmt_init_event_callback(&mNanMgmtClbk, NanMgmtEventHandler, kNanManagementEvents);
    net_mgmt_add_event_callback(&mNanMgmtClbk);

    ChipLogProgress(DeviceLayer, "WiFi-PAF: NAN event callbacks registered");

    // Initialize PAF layer
    CHIP_ERROR err = WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().Init(&DeviceLayer::SystemLayer());

    return err;
}

CHIP_ERROR ConnectivityManagerImplWiFiPAF::_WiFiPAFPublish(ConnectivityManager::WiFiPAFAdvertiseParam & args)
{
    ChipLogProgress(DeviceLayer, "WiFi-PAF: Starting NAN publish");

    // Prevent double publish
    k_mutex_lock(&mNanMutex, K_FOREVER);
    if (mPublishActive)
    {
        k_mutex_unlock(&mNanMutex);
        ChipLogError(DeviceLayer, "WiFi-PAF: Publish already active");
        return CHIP_ERROR_INCORRECT_STATE;
    }
    k_mutex_unlock(&mNanMutex);

    // Prepare commissioning data
    struct PAFPublishSSI ssi;
    ssi.DevOpCode = 0;

    // Get discriminator
    uint16_t discriminator;
    CHIP_ERROR err = DeviceLayer::GetCommissionableDataProvider()->GetSetupDiscriminator(discriminator);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err, ChipLogError(DeviceLayer, "WiFi-PAF: Failed to get discriminator"));
    ssi.DevInfo = discriminator;

    // Get Product ID (optional)
    uint16_t pid = 0;
    TEMPORARY_RETURN_IGNORED DeviceLayer::GetDeviceInstanceInfoProvider()->GetProductId(pid);
    ssi.ProductId = pid;

    // Get Vendor ID (optional)
    uint16_t vid = 0;
    TEMPORARY_RETURN_IGNORED DeviceLayer::GetDeviceInstanceInfoProvider()->GetVendorId(vid);
    ssi.VendorId = vid;

    ChipLogProgress(DeviceLayer, "WiFi-PAF: Publishing with discriminator=%u, VID=0x%04X, PID=0x%04X", ssi.DevInfo, ssi.VendorId,
                    ssi.ProductId);

    // Prepare NAN publish parameters
    wifi_nan_params params = {};
    params.op              = WIFI_NAN_OP_PUBLISH;

    // Service name
    static_assert(sizeof(kMatterNanServiceName) <= sizeof(params.publish.service_name), "Service name too long");
    Platform::CopyString(params.publish.service_name, kMatterNanServiceName);
    params.publish.service_name[sizeof(params.publish.service_name) - 1] = '\0';

    // Service protocol type
    params.publish.srv_proto_type = static_cast<wifi_nan_service_protocol_type>(kNanServiceProtocolTypeMatter);

    // TTL
    params.publish.ttl = kDefaultPublishTtl;

    // Frequency (use configured or default)
    params.publish.freq = (mApFreq != 0) ? mApFreq : kDefault24GHzFreq;

    // Frequency list (if provided)
    params.publish.freq_list[0] = '\0';
    if (args.freq_list_len > 0 && args.freq_list != nullptr)
    {
        constexpr size_t kMaxFreqEntries = 10;
        size_t copyLen                   = std::min(static_cast<size_t>(args.freq_list_len), kMaxFreqEntries);

        char * dst       = params.publish.freq_list;
        size_t remaining = sizeof(params.publish.freq_list);
        for (size_t i = 0; i < copyLen && remaining > 1; i++)
        {
            int written = snprintf(dst, remaining, "%u%s", args.freq_list[i], (i < copyLen - 1) ? "," : "");
            if (written < 0 || static_cast<size_t>(written) >= remaining)
                break;
            dst += written;
            remaining -= static_cast<size_t>(written);
        }
    }

    // Service Specific Information (SSI) - pointer, net_mgmt is synchronous so lifetime is safe
    params.publish.ssi     = reinterpret_cast<const uint8_t *>(&ssi);
    params.publish.ssi_len = sizeof(ssi);

    // Publish flags
    params.publish.unsolicited = true;
    params.publish.solicited   = true;
    params.publish.fsd         = true;

    // Call Zephyr NAN API
    int ret = net_mgmt(NET_REQUEST_WIFI_NAN, mWiFiIface, &params, sizeof(params));
    if (ret != 0)
    {
        ChipLogError(DeviceLayer, "WiFi-PAF: NAN publish failed: %d", ret);
        return CHIP_ERROR_INTERNAL;
    }

    // Parse publish ID from response
    params.resp[sizeof(params.resp) - 1] = '\0';
    uint32_t publishId                   = 0;
    if (sscanf(params.resp, "%u", &publishId) == 1)
    {
        k_mutex_lock(&mNanMutex, K_FOREVER);
        mCurrentPublishId = publishId;
        mPublishActive    = true;
        mPeerRegistered   = false; // Reset for new publish
        k_mutex_unlock(&mNanMutex);

        args.publish_id = publishId;

        ChipLogProgress(DeviceLayer, "WiFi-PAF: Publish started successfully, ID=%u", publishId);

        // Register session with PAF layer
        WiFiPAF::WiFiPAFSession sessionInfo = {};
        sessionInfo.role                    = WiFiPAF::WiFiPafRole::kWiFiPafRole_Publisher;
        sessionInfo.id                      = publishId;

        WiFiPAF::WiFiPAFLayer & pafLayer = WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer();
        err                              = pafLayer.AddPafSession(WiFiPAF::PafInfoAccess::kAccSessionId, sessionInfo);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "WiFi-PAF: Failed to register PAF session: %" CHIP_ERROR_FORMAT, err.Format());
            TEMPORARY_RETURN_IGNORED _WiFiPAFCancelPublish(publishId);
            return err;
        }

        return CHIP_NO_ERROR;
    }
    else
    {
        ChipLogError(DeviceLayer, "WiFi-PAF: Failed to parse publish ID from response");
        return CHIP_ERROR_INTERNAL;
    }
}

CHIP_ERROR ConnectivityManagerImplWiFiPAF::_WiFiPAFCancelPublish(uint32_t PublishId)
{
    ChipLogProgress(DeviceLayer, "WiFi-PAF: Canceling publish ID=%u", PublishId);

    k_mutex_lock(&mNanMutex, K_FOREVER);
    if (!mPublishActive)
    {
        k_mutex_unlock(&mNanMutex);
        ChipLogProgress(DeviceLayer, "WiFi-PAF: No active publish to cancel");
        return CHIP_NO_ERROR;
    }

    k_mutex_unlock(&mNanMutex);

    wifi_nan_params params = {};
    params.op              = WIFI_NAN_OP_CANCEL_PUBLISH;
    params.cancel_id       = static_cast<uint8_t>(PublishId);

    int ret = net_mgmt(NET_REQUEST_WIFI_NAN, mWiFiIface, &params, sizeof(params));
    if (ret != 0)
    {
        ChipLogError(DeviceLayer, "WiFi-PAF: Cancel publish failed: %d", ret);
        return CHIP_ERROR_INTERNAL;
    }

    // Clean up state
    k_mutex_lock(&mNanMutex, K_FOREVER);
    mPublishActive    = false;
    mCurrentPublishId = 0;
    mPeerRegistered   = false;
    k_mutex_unlock(&mNanMutex);

    // Remove session from PAF layer
    WiFiPAF::WiFiPAFSession sessionInfo = {};
    sessionInfo.id                      = PublishId;
    WiFiPAF::WiFiPAFLayer & pafLayer    = WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer();
    CHIP_ERROR err                      = pafLayer.RmPafSession(WiFiPAF::PafInfoAccess::kAccSessionId, sessionInfo);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "WiFi-PAF: Failed to remove PAF session: %" CHIP_ERROR_FORMAT, err.Format());
    }

    ChipLogProgress(DeviceLayer, "WiFi-PAF: Publish canceled successfully");

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImplWiFiPAF::_WiFiPAFSubscribe(const uint16_t & connDiscriminator, void * appState,
                                                             ConnectivityManager::OnConnectionCompleteFunct onSuccess,
                                                             ConnectivityManager::OnConnectionErrorFunct onError)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ConnectivityManagerImplWiFiPAF::_WiFiPAFCancelSubscribe(uint32_t SubscribeId)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ConnectivityManagerImplWiFiPAF::_WiFiPAFCancelIncompleteSubscribe()
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ConnectivityManagerImplWiFiPAF::_WiFiPAFSend(const WiFiPAF::WiFiPAFSession & TxInfo,
                                                        System::PacketBufferHandle && msgBuf)
{
    VerifyOrReturnError(!msgBuf.IsNull(), CHIP_ERROR_INVALID_ARGUMENT,
                        ChipLogError(DeviceLayer, "WiFi-PAF: Invalid packet buffer"));
    ChipLogProgress(DeviceLayer, "WiFi-PAF: Sending PAF frame, length=%u", static_cast<unsigned>(msgBuf->DataLength()));

    // Ensure outgoing message fits in a single contiguous packet buffer, as currently required by the
    // message fragmentation and reassembly engine.
    if (msgBuf->HasChainedBuffer())
    {
        msgBuf->CompactHead();
        VerifyOrReturnError(!msgBuf->HasChainedBuffer(), CHIP_ERROR_BUFFER_TOO_SMALL,
                            ChipLogError(DeviceLayer, "WiFi-PAF: Message too large to compact"));
    }

    wifi_nan_params params = {};
    params.op              = WIFI_NAN_OP_TRANSMIT;
    // Set handle (publish/subscribe ID)
    params.transmit.handle = static_cast<uint8_t>(TxInfo.id);
    // Set peer instance ID
    params.transmit.req_instance_id = static_cast<uint8_t>(TxInfo.peer_id);

    // Set peer MAC address
    static_assert(sizeof(params.transmit.peer_addr) >= sizeof(TxInfo.peer_addr), "Peer address buffer too small");
    memcpy(params.transmit.peer_addr, TxInfo.peer_addr, sizeof(TxInfo.peer_addr));

    // Set SSI payload (pointer to PacketBuffer data; net_mgmt is synchronous so lifetime is safe)
    size_t dataLen          = msgBuf->DataLength();
    params.transmit.ssi     = msgBuf->Start();
    params.transmit.ssi_len = dataLen;

    int ret = net_mgmt(NET_REQUEST_WIFI_NAN, mWiFiIface, &params, sizeof(params));
    if (ret != 0)
    {
        ChipLogError(DeviceLayer, "WiFi-PAF: Transmit failed: %d", ret);
        return CHIP_ERROR_INTERNAL;
    }

    // Post write done event
    ChipDeviceEvent event = {};
    event.Type            = DeviceEventType::kCHIPoWiFiPAFWriteDone;
    memcpy(&event.CHIPoWiFiPAFReceived.SessionInfo, &TxInfo, sizeof(WiFiPAF::WiFiPAFSession));
    event.CHIPoWiFiPAFReceived.result = true; // Indicate success
    PlatformMgr().PostEventOrDie(&event);

    ChipLogProgress(DeviceLayer, "WiFi-PAF: Frame sent successfully");
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImplWiFiPAF::_WiFiPAFShutdown(uint32_t id, WiFiPAF::WiFiPafRole role)
{
    ChipLogProgress(DeviceLayer, "WiFi-PAF: Shutting down, role=%d, id=%u", static_cast<int>(role), id);

    CHIP_ERROR err = CHIP_NO_ERROR;

    switch (role)
    {
    case WiFiPAF::WiFiPafRole::kWiFiPafRole_Publisher:
        err = _WiFiPAFCancelPublish(id);
        break;

    case WiFiPAF::WiFiPafRole::kWiFiPafRole_Subscriber:
        ChipLogProgress(DeviceLayer, "WiFi-PAF: Subscriber role is not supported, no action taken");
        break;

    default:
        ChipLogError(DeviceLayer, "WiFi-PAF: Unknown role: %d", static_cast<int>(role));
        err = CHIP_ERROR_INVALID_ARGUMENT;
        break;
    }

    return err;
}

void ConnectivityManagerImplWiFiPAF::OnNanReplied(const wifi_nan_replied_event * event)
{
    VerifyOrReturn(event != nullptr, ChipLogError(DeviceLayer, "WiFi-PAF: OnNanReplied: null event"));

    k_mutex_lock(&mNanMutex, K_FOREVER);

    // Verify this is for our active publish
    if (event->publish_id != mCurrentPublishId)
    {
        k_mutex_unlock(&mNanMutex);
        ChipLogError(DeviceLayer, "WiFi-PAF: Publish ID mismatch! Expected %u, got %u", mCurrentPublishId, event->publish_id);
        return;
    }

    // Early check for duplicate
    if (mPeerRegistered)
    {
        k_mutex_unlock(&mNanMutex);
        ChipLogProgress(DeviceLayer, "WiFi-PAF: Peer already registered, skipping duplicate");
        return;
    }

    k_mutex_unlock(&mNanMutex);

    ChipLogProgress(DeviceLayer, "WiFi-PAF: NAN replied - publish_id=%u, subscribe_id=%u", event->publish_id, event->subscribe_id);

    // Free dynamically allocated ssi (not used by Matter layer)
    if (event->ssi)
    {
        k_free(event->ssi);
    }

    // Populate session info
    WiFiPAF::WiFiPAFSession sessionInfo = {};
    sessionInfo.role                    = WiFiPAF::WiFiPafRole::kWiFiPafRole_Publisher;
    sessionInfo.id                      = event->publish_id;
    sessionInfo.peer_id                 = event->subscribe_id;
    static_assert(sizeof(sessionInfo.peer_addr) >= sizeof(event->peer_addr), "Peer address buffer mismatch");
    memcpy(sessionInfo.peer_addr, event->peer_addr, sizeof(event->peer_addr));

    auto * sess = chip::Platform::New<WiFiPAF::WiFiPAFSession>(sessionInfo);
    VerifyOrReturn(sess != nullptr, ChipLogError(DeviceLayer, "WiFi-PAF: Failed to allocate session"));

    CHIP_ERROR schedErr = DeviceLayer::SystemLayer().ScheduleLambda([this, sess]() {
        // Update PAF layer's persistent session
        WiFiPAF::WiFiPAFLayer & pafLayer   = WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer();
        WiFiPAF::WiFiPAFSession * pPafInfo = pafLayer.GetPAFInfo(WiFiPAF::PafInfoAccess::kAccSessionId, *sess);

        if (pPafInfo == nullptr)
        {
            ChipLogError(DeviceLayer, "WiFi-PAF: Session not found in PAF layer (id=%u)", sess->id);
            Platform::Delete(sess);
            return;
        }

        // Double-check on CHIP thread to avoid race conditions
        k_mutex_lock(&mNanMutex, K_FOREVER);
        if (mPeerRegistered)
        {
            k_mutex_unlock(&mNanMutex);
            ChipLogProgress(DeviceLayer, "WiFi-PAF: Race condition detected, peer already registered");
            Platform::Delete(sess);
            return;
        }

        // Update PAF layer session
        pPafInfo->peer_id = sess->peer_id;
        memcpy(pPafInfo->peer_addr, sess->peer_addr, sizeof(sess->peer_addr));

        // Mark as peer registered
        mPeerRegistered = true;
        k_mutex_unlock(&mNanMutex);

        ChipLogProgress(DeviceLayer, "WiFi-PAF: Updated PAF layer session with peer info");

        CHIP_ERROR initErr = pafLayer.HandleTransportConnectionInitiated(*sess);
        if (initErr != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "WiFi-PAF: HandleTransportConnectionInitiated failed: %" CHIP_ERROR_FORMAT, initErr.Format());
            // Rollback on failure
            k_mutex_lock(&mNanMutex, K_FOREVER);
            mPeerRegistered = false;
            k_mutex_unlock(&mNanMutex);
        }
        else
        {
            ChipLogProgress(DeviceLayer, "WiFi-PAF: Endpoint created, waiting for subscriber to connect");
        }

        // Free after handling
        Platform::Delete(sess);
    });

    if (schedErr != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "WiFi-PAF: Failed to schedule Lambda: %" CHIP_ERROR_FORMAT, schedErr.Format());
        Platform::Delete(sess);
        return;
    }
}

void ConnectivityManagerImplWiFiPAF::OnNanReceive(const wifi_nan_receive_event * event)
{
    VerifyOrReturn(event != nullptr, ChipLogError(DeviceLayer, "WiFi-PAF: OnNanReceive: null event"));

    // Verify that the event belongs to our active session
    k_mutex_lock(&mNanMutex, K_FOREVER);
    bool isActiveSession = (mPublishActive && event->id == mCurrentPublishId);
    k_mutex_unlock(&mNanMutex);
    VerifyOrReturn(isActiveSession, ChipLogProgress(DeviceLayer, "WiFi-PAF: Ignoring event for inactive session %u", event->id));

    VerifyOrReturn(event->ssi != nullptr && event->ssi_len > 0,
                   ChipLogError(DeviceLayer, "WiFi-PAF: OnNanReceive: null or empty SSI"));

    ChipLogProgress(DeviceLayer, "WiFi-PAF: NAN receive - id=%u, peer_id=%u, len=%u", event->id, event->peer_instance_id,
                    static_cast<unsigned>(event->ssi_len));

    // Populate session info
    WiFiPAF::WiFiPAFSession sessionInfo = {};
    sessionInfo.id                      = event->id;
    sessionInfo.peer_id                 = event->peer_instance_id;
    static_assert(sizeof(sessionInfo.peer_addr) >= sizeof(event->peer_addr), "Peer address buffer mismatch");
    memcpy(sessionInfo.peer_addr, event->peer_addr, sizeof(event->peer_addr));

    // Create packet buffer (copies data), then free the dynamically allocated ssi
    uint8_t * ssi_to_free          = event->ssi;
    System::PacketBufferHandle buf = System::PacketBufferHandle::NewWithData(event->ssi, event->ssi_len);
    k_free(ssi_to_free);
    VerifyOrReturn(!buf.IsNull(), ChipLogError(DeviceLayer, "WiFi-PAF: Failed to allocate packet buffer"));

    // Post Matter event
    ChipDeviceEvent matterEvent = {};
    matterEvent.Type            = DeviceEventType::kCHIPoWiFiPAFReceived;
    memcpy(&matterEvent.CHIPoWiFiPAFReceived.SessionInfo, &sessionInfo, sizeof(WiFiPAF::WiFiPAFSession));
    matterEvent.CHIPoWiFiPAFReceived.Data = std::move(buf).UnsafeRelease();
    PlatformMgr().PostEventOrDie(&matterEvent);
}

void ConnectivityManagerImplWiFiPAF::OnNanPublishTerminated(const wifi_nan_terminated_event * event)
{
    VerifyOrReturn(event != nullptr, ChipLogError(DeviceLayer, "WiFi-PAF: OnNanPublishTerminated: null event"));
    ChipLogProgress(DeviceLayer, "WiFi-PAF: NAN publish terminated - publish_id=%u, reason=%s", event->id, event->reason);

    // Clean up state
    if (event->id == mCurrentPublishId)
    {
        k_mutex_lock(&mNanMutex, K_FOREVER);
        mCurrentPublishId = 0;
        mPublishActive    = false;
        mPeerRegistered   = false;
        k_mutex_unlock(&mNanMutex);
    }

    // Remove session from PAF layer
    WiFiPAF::WiFiPAFSession sessionInfo = {};
    sessionInfo.id                      = event->id;
    WiFiPAF::WiFiPAFLayer & pafLayer    = WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer();
    CHIP_ERROR err                      = pafLayer.RmPafSession(WiFiPAF::PafInfoAccess::kAccSessionId, sessionInfo);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "WiFi-PAF: Failed to remove PAF session: %" CHIP_ERROR_FORMAT, err.Format());
    }

    // Post Matter event
    ChipDeviceEvent matterEvent = {};
    matterEvent.Type            = DeviceEventType::kCHIPoWiFiPAFCancelConnect;
    PlatformMgr().PostEventOrDie(&matterEvent);
}

ConnectivityManagerImplWiFiPAF & ConnectivityManagerImplWiFiPAF::GetInstance()
{
    return static_cast<ConnectivityManagerImplWiFiPAF &>(ConnectivityMgrImpl());
}

#if KERNEL_VERSION_MAJOR >= 4 && KERNEL_VERSION_MINOR >= 2
void ConnectivityManagerImplWiFiPAF::NanMgmtEventHandler(net_mgmt_event_callback * cb, uint64_t mgmtEvent, net_if * iface)
#else
void ConnectivityManagerImplWiFiPAF::NanMgmtEventHandler(net_mgmt_event_callback * cb, uint32_t mgmtEvent, net_if * iface)
#endif
{
    // Get instance reference
    ConnectivityManagerImplWiFiPAF & self = GetInstance();

    // Validate interface
    if (iface != self.mWiFiIface)
    {
        return;
    }

    switch (mgmtEvent)
    {
    case NET_EVENT_WIFI_NAN_REPLIED: {
        const wifi_nan_replied_event * event = static_cast<const wifi_nan_replied_event *>(cb->info);
        if (event != nullptr)
        {
            self.OnNanReplied(event);
        }
        break;
    }

    case NET_EVENT_WIFI_NAN_RECEIVE: {
        const wifi_nan_receive_event * event = static_cast<const wifi_nan_receive_event *>(cb->info);
        if (event != nullptr)
        {
            self.OnNanReceive(event);
        }
        break;
    }

    case NET_EVENT_WIFI_NAN_PUBLISH_TERMINATED: {
        const wifi_nan_terminated_event * event = static_cast<const wifi_nan_terminated_event *>(cb->info);
        if (event != nullptr)
        {
            self.OnNanPublishTerminated(event);
        }
        break;
    }

    default:
        ChipLogError(DeviceLayer, "WiFi-PAF: Unexpected NAN event: 0x" ChipLogFormatX64, ChipLogValueX64(mgmtEvent));
        break;
    }
}

void ConnectivityManagerImplWiFiPAF::OnWiFiPAFPlatformEvent(const ChipDeviceEvent * event)
{
    WiFiPAF::WiFiPAFLayer & pafLayer = WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer();

    switch (event->Type)
    {
    case DeviceEventType::kCHIPoWiFiPAFReceived: {
        ChipLogProgress(DeviceLayer, "WiFi-PAF: event: kCHIPoWiFiPAFReceived");
        WiFiPAF::WiFiPAFSession rxInfo;
        memcpy(&rxInfo, &event->CHIPoWiFiPAFReceived.SessionInfo, sizeof(WiFiPAF::WiFiPAFSession));
        pafLayer.OnWiFiPAFMessageReceived(rxInfo, System::PacketBufferHandle::Adopt(event->CHIPoWiFiPAFReceived.Data));
        break;
    }

    case DeviceEventType::kCHIPoWiFiPAFWriteDone: {
        ChipLogProgress(DeviceLayer, "WiFi-PAF: event: kCHIPoWiFiPAFWriteDone");
        WiFiPAF::WiFiPAFSession txInfo;
        memcpy(&txInfo, &event->CHIPoWiFiPAFReceived.SessionInfo, sizeof(WiFiPAF::WiFiPAFSession));
        TEMPORARY_RETURN_IGNORED pafLayer.HandleWriteConfirmed(txInfo, event->CHIPoWiFiPAFReceived.result);
        break;
    }

    case DeviceEventType::kCHIPoWiFiPAFCancelConnect: {
        ChipLogProgress(DeviceLayer, "WiFi-PAF: event: kCHIPoWiFiPAFCancelConnect");
        // Connection was canceled or terminated
        // The PAF layer and NAN callbacks will handle cleanup
        break;
    }

    default:
        // Not a WiFi-PAF event, ignore
        break;
    }
}

void ConnectivityManagerImplWiFiPAF::_WiFiPAFSetParam(const ConnectivityManager::WiFiPAFAdvertiseParam & pafAdvParam)
{
    mPafAdvParam.freq_list_len = pafAdvParam.freq_list_len;
    if (mPafAdvParam.freq_list_len > 0)
    {
        mPafAdvParam.freq_list = std::make_unique<uint16_t[]>(mPafAdvParam.freq_list_len);
        for (size_t i = 0; i < mPafAdvParam.freq_list_len; i++)
        {
            mPafAdvParam.freq_list[i] = pafAdvParam.freq_list[i];
        }
    }
    else
    {
        mPafAdvParam.freq_list.reset();
    }
}

CHIP_ERROR ConnectivityManagerImplWiFiPAF::_SetWiFiPAFAdvertisingEnabled(bool enabled, uint32_t & publishId)
{
    if (enabled)
    {
        auto res = _WiFiPAFPublish(mPafAdvParam);
        if ((res == CHIP_NO_ERROR) && (mPafAdvParam.publish_id != WiFiPAF::kUndefinedWiFiPafSessionId))
        {
            publishId = mPafAdvParam.publish_id;
        }
        return res;
    }
    // Cancel publish; publishId must be valid
    VerifyOrReturnError((publishId != 0) && (publishId != WiFiPAF::kUndefinedWiFiPafSessionId), CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(DeviceLayer, "WiFi-PAF: _SetWiFiPAFAdvertisingEnabled: invalid publishId=%u", publishId));
    CHIP_ERROR err = _WiFiPAFCancelPublish(publishId);
    if (err == CHIP_NO_ERROR)
    {
        publishId = WiFiPAF::kUndefinedWiFiPafSessionId;
    }
    return err;
}

} // namespace DeviceLayer
} // namespace chip
