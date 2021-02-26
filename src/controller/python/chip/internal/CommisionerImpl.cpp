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
    void SetDelegate(chip::PersistentStorageResultDelegate * delegate) override { mAsyncDelegate = delegate; }

    void GetKeyValue(const char * key) override
    {
        uint8_t buffer[kMaxKeyValueSize];
        uint16_t bufferSize = sizeof(buffer) + 1;
        CHIP_ERROR err      = GetKeyValue(key, buffer, bufferSize);

        if (err == CHIP_NO_ERROR)
        {
            buffer[bufferSize] = 0;
            mAsyncDelegate->OnValue(key, reinterpret_cast<const char *>(buffer));
        }
        else
        {
            mAsyncDelegate->OnStatus(key, chip::PersistentStorageResultDelegate::Operation::kGET, err);
        }
    }

    void SetKeyValue(const char * key, const char * value) override
    {

        CHIP_ERROR err = SetKeyValue(key, value, strlen(value));

        if (err != CHIP_NO_ERROR)
        {
            mAsyncDelegate->OnStatus(key, chip::PersistentStorageResultDelegate::Operation::kSET, err);
        }
    }

    CHIP_ERROR
    GetKeyValue(const char * key, void * buffer, uint16_t & size) override
    {
        return chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr().Get(key, buffer, size);
    }

    CHIP_ERROR SetKeyValue(const char * key, const void * value, uint16_t size) override
    {
        return chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr().Put(key, value, size);
    }

    void DeleteKeyValue(const char * key) override { chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr().Delete(key); }

private:
    static constexpr size_t kMaxKeyValueSize = 1024;

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

        mWifiSsid     = ssid;
        mWifiPassword = password;
        mCredentialsDelegate->SendNetworkCredentials(mWifiSsid.c_str(), mWifiPassword.c_str());
    }

private:
    OnNetworkCredentialsRequestedCallback mNetworkCredentialsRequested         = nullptr;
    OnOperationalCredentialsRequestedCallback mOperationalCredentialsRequested = nullptr;
    OnPairingCompleteCallback mPairingComplete                                 = nullptr;

    /// Delegate is set during request callbacks
    chip::RendezvousDeviceCredentialsDelegate * mCredentialsDelegate = nullptr;

    // Copy of wifi credentials, to allow them to be used by callbacks
    std::string mWifiSsid;
    std::string mWifiPassword;
};

ServerStorageDelegate gServerStorage;
ScriptDevicePairingDelegate gPairingDelegate;

} // namespace

extern "C" void pychip_internal_PairingDelegate_SetNetworkCredentials(const char * ssid, const char * password)
{
    chip::python::ChipMainThreadScheduleAndWait([&]() { gPairingDelegate.SetWifiCredentials(ssid, password); });
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

extern "C" chip::Controller::DeviceCommissioner * pychip_internal_Commisioner_New(uint64_t localDeviceId)
{
    std::unique_ptr<chip::Controller::DeviceCommissioner> result;
    CHIP_ERROR err;

    chip::python::ChipMainThreadScheduleAndWait([&]() {
        result = std::make_unique<chip::Controller::DeviceCommissioner>();

        // System and Inet layers exlicitly passed to indicate that the CHIP stack is
        // already assumed initialized
        err = result->Init(localDeviceId, &gServerStorage, &gPairingDelegate, &chip::DeviceLayer::SystemLayer,
                           &chip::DeviceLayer::InetLayer);
    });

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Commisioner initialization failed: %s", chip::ErrorStr(err));
        return nullptr;
    }

    return result.release();
}

/// Returns CHIP_ERROR corresponding to an UnpairDevice call
extern "C" uint32_t pychip_internal_Commisioner_Unpair(chip::Controller::DeviceCommissioner * commisioner, uint64_t remoteDeviceId)
{
    CHIP_ERROR err;

    chip::python::ChipMainThreadScheduleAndWait([&]() { err = commisioner->UnpairDevice(remoteDeviceId); });

    return err;
}

extern "C" uint32_t pychip_internal_Commisioner_BleConnectForPairing(chip::Controller::DeviceCommissioner * commisioner,
                                                                     uint64_t remoteNodeId, uint32_t pinCode,
                                                                     uint16_t discriminator)
{

    CHIP_ERROR err;

    chip::python::ChipMainThreadScheduleAndWait([&]() {
        chip::RendezvousParameters params;

        params.SetDiscriminator(discriminator)
            .SetSetupPINCode(pinCode)
            .SetRemoteNodeId(remoteNodeId)
            .SetBleLayer(chip::DeviceLayer::ConnectivityMgr().GetBleLayer())
            .SetPeerAddress(chip::Transport::PeerAddress::BLE());

        err = commisioner->PairDevice(remoteNodeId, params);
    });

    return err;
}
