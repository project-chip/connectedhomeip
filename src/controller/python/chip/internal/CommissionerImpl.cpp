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
#include <memory>

#include <controller/CHIPDeviceController.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/KeyValueStoreManager.h>
#include <support/logging/CHIPLogging.h>

#include "ChipThreadWork.h"

namespace {

class ServerStorageDelegate : public chip::PersistentStorageDelegate
{
public:
    void SetStorageDelegate(chip::PersistentStorageResultDelegate * delegate) override { mAsyncDelegate = delegate; }

    void AsyncSetKeyValue(const char * key, const char * value) override
    {

        CHIP_ERROR err = SyncSetKeyValue(key, value, strlen(value));

        if (err != CHIP_NO_ERROR)
        {
            mAsyncDelegate->OnPersistentStorageStatus(key, chip::PersistentStorageResultDelegate::Operation::kSET, err);
        }
    }

    CHIP_ERROR
    SyncGetKeyValue(const char * key, void * buffer, uint16_t & size) override
    {
        return chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr().Get(key, buffer, size);
    }

    CHIP_ERROR SyncSetKeyValue(const char * key, const void * value, uint16_t size) override
    {
        return chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr().Put(key, value, size);
    }

    void AsyncDeleteKeyValue(const char * key) override { chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr().Delete(key); }

private:
    chip::PersistentStorageResultDelegate * mAsyncDelegate = nullptr;
};

// FIXME: implement this class
class ScriptDevicePairingDelegate final : public chip::Controller::DevicePairingDelegate
{
public:
    using OnNetworkCredentialsRequestedCallback     = void (*)();
    using OnOperationalCredentialsRequestedCallback = void (*)(const char * csr, size_t length);
    using OnPairingCompleteCallback                 = void (*)(CHIP_ERROR err);

    ~ScriptDevicePairingDelegate() = default;

    void OnNetworkCredentialsRequested(chip::RendezvousDeviceCredentialsDelegate * callback) override
    {
        mCredentialsDelegate = callback;
        if (mNetworkCredentialsRequested == nullptr)
        {
            ChipLogError(Controller, "Callback for network credentials is not defined.");
            return;
        }
        mNetworkCredentialsRequested();
    }

    void OnOperationalCredentialsRequested(const char * csr, size_t csr_length,
                                           chip::RendezvousDeviceCredentialsDelegate * callback) override
    {
        mCredentialsDelegate = callback;
        if (mOperationalCredentialsRequested == nullptr)
        {
            ChipLogError(Controller, "Callback for operational credentials is not defined.");
            return;
        }

        mOperationalCredentialsRequested(csr, csr_length);
    }

    void OnPairingComplete(CHIP_ERROR error) override
    {
        if (mPairingComplete == nullptr)
        {
            ChipLogError(Controller, "Callback for pairing coomplete is not defined.");
            return;
        }
        mPairingComplete(error);
    }

    void SetNetworkCredentialsRequestedCallback(OnNetworkCredentialsRequestedCallback callback)
    {
        mNetworkCredentialsRequested = callback;
    }

    void SetOperatioonalCredentialsRequestedCallback(OnOperationalCredentialsRequestedCallback callback)
    {
        mOperationalCredentialsRequested = callback;
    }

    void SetPairingCompleteCallback(OnPairingCompleteCallback callback) { mPairingComplete = callback; }

    void SetWifiCredentials(const char * ssid, const char * password)
    {
        if (mCredentialsDelegate == nullptr)
        {
            ChipLogError(Controller, "Wifi credentials received before delegate available.");
            return;
        }

        mCredentialsDelegate->SendNetworkCredentials(ssid, password);
    }

    void SetThreadCredentials(const chip::DeviceLayer::Internal::DeviceNetworkInfo & threadData)
    {
        if (mCredentialsDelegate == nullptr)
        {
            ChipLogError(Controller, "Thread credentials received before delegate available.");
            return;
        }

        mCredentialsDelegate->SendThreadCredentials(threadData);
    }

private:
    OnNetworkCredentialsRequestedCallback mNetworkCredentialsRequested         = nullptr;
    OnOperationalCredentialsRequestedCallback mOperationalCredentialsRequested = nullptr;
    OnPairingCompleteCallback mPairingComplete                                 = nullptr;

    /// Delegate is set during request callbacks
    chip::RendezvousDeviceCredentialsDelegate * mCredentialsDelegate = nullptr;
};

ServerStorageDelegate gServerStorage;
ScriptDevicePairingDelegate gPairingDelegate;

} // namespace

extern "C" void pychip_internal_PairingDelegate_SetWifiCredentials(const char * ssid, const char * password)
{
    chip::python::ChipMainThreadScheduleAndWait([&]() { gPairingDelegate.SetWifiCredentials(ssid, password); });
}

extern "C" CHIP_ERROR pychip_internal_PairingDelegate_SetThreadCredentials(const void * data, uint32_t length)
{

    // Openthread is OPAQUE by the spec, however current CHIP stack does not have any
    // validation/support for opaque blobs. As a result, we try to do some
    // pre-validation here

    // TODO: there should be uniform 'BLOBL' support within the thread stack
    if (length != sizeof(chip::DeviceLayer::Internal::DeviceNetworkInfo))
    {
        ChipLogError(Controller, "Received invalid thread credential blob. Expected size %u and got %u bytes instead",
                     sizeof(chip::DeviceLayer::Internal::DeviceNetworkInfo), length);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // unsure about alignment so copy into a properly aligned item
    chip::DeviceLayer::Internal::DeviceNetworkInfo threadInfo;
    memcpy(&threadInfo, data, sizeof(threadInfo));

    // TODO: figure out a proper way to validate this or remove validation once
    // thread credentials are assumed opaque throughout
    if ((threadInfo.ThreadChannel != chip::DeviceLayer::Internal::kThreadChannel_NotSpecified) &&
        ((threadInfo.ThreadChannel < 11) || (threadInfo.ThreadChannel > 26)))
    {
        ChipLogError(Controller, "Failed to validate thread info: channel %d is not valid", threadInfo.ThreadChannel);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    chip::python::ChipMainThreadScheduleAndWait([&]() { gPairingDelegate.SetThreadCredentials(threadInfo); });

    return CHIP_NO_ERROR;
}

extern "C" void pychip_internal_PairingDelegate_SetNetworkCredentialsRequestedCallback(
    ScriptDevicePairingDelegate::OnNetworkCredentialsRequestedCallback callback)
{
    gPairingDelegate.SetNetworkCredentialsRequestedCallback(callback);
}

extern "C" void pychip_internal_PairingDelegate_SetOperationalCredentialsRequestedCallback(
    ScriptDevicePairingDelegate::OnOperationalCredentialsRequestedCallback callback)
{
    gPairingDelegate.SetOperatioonalCredentialsRequestedCallback(callback);
}

extern "C" void
pychip_internal_PairingDelegate_SetPairingCompleteCallback(ScriptDevicePairingDelegate::OnPairingCompleteCallback callback)
{
    gPairingDelegate.SetPairingCompleteCallback(callback);
}

extern "C" chip::Controller::DeviceCommissioner * pychip_internal_Commissioner_New(uint64_t localDeviceId)
{
    std::unique_ptr<chip::Controller::DeviceCommissioner> result;
    CHIP_ERROR err;

    chip::python::ChipMainThreadScheduleAndWait([&]() {
        result = std::make_unique<chip::Controller::DeviceCommissioner>();

        // System and Inet layers explicitly passed to indicate that the CHIP stack is
        // already assumed initialized
        err = result->Init(localDeviceId, &gServerStorage, &gPairingDelegate, &chip::DeviceLayer::SystemLayer,
                           &chip::DeviceLayer::InetLayer);
    });

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Commissioner initialization failed: %s", chip::ErrorStr(err));
        return nullptr;
    }

    return result.release();
}

/// Returns CHIP_ERROR corresponding to an UnpairDevice call
extern "C" uint32_t pychip_internal_Commissioner_Unpair(chip::Controller::DeviceCommissioner * commissioner,
                                                        uint64_t remoteDeviceId)
{
    CHIP_ERROR err;

    chip::python::ChipMainThreadScheduleAndWait([&]() { err = commissioner->UnpairDevice(remoteDeviceId); });

    return err;
}

extern "C" uint32_t pychip_internal_Commissioner_BleConnectForPairing(chip::Controller::DeviceCommissioner * commissioner,
                                                                      uint64_t remoteNodeId, uint32_t pinCode,
                                                                      uint16_t discriminator)
{

    CHIP_ERROR err;

    chip::python::ChipMainThreadScheduleAndWait([&]() {
        chip::RendezvousParameters params;

        params.SetDiscriminator(discriminator).SetSetupPINCode(pinCode).SetRemoteNodeId(remoteNodeId);
#if CONFIG_NETWORK_LAYER_BLE
        params.SetBleLayer(chip::DeviceLayer::ConnectivityMgr().GetBleLayer()).SetPeerAddress(chip::Transport::PeerAddress::BLE());
#endif

        err = commissioner->PairDevice(remoteNodeId, params);
    });

    return err;
}
