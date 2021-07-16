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
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/KeyValueStoreManager.h>
#include <support/CodeUtils.h>
#include <support/ThreadOperationalDataset.h>
#include <support/logging/CHIPLogging.h>

#include "ChipThreadWork.h"

namespace {

class ServerStorageDelegate : public chip::PersistentStorageDelegate
{
public:
    CHIP_ERROR
    SyncGetKeyValue(const char * key, void * buffer, uint16_t & size) override
    {
        return chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr().Get(key, buffer, size);
    }

    CHIP_ERROR SyncSetKeyValue(const char * key, const void * value, uint16_t size) override
    {
        return chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr().Put(key, value, size);
    }

    CHIP_ERROR SyncDeleteKeyValue(const char * key) override
    {
        return chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr().Delete(key);
    }
};

// FIXME: implement this class
class ScriptDevicePairingDelegate final : public chip::Controller::DevicePairingDelegate
{
public:
    using OnPairingCompleteCallback = void (*)(CHIP_ERROR err);

    ~ScriptDevicePairingDelegate() = default;

    void OnPairingComplete(CHIP_ERROR error) override
    {
        if (mPairingComplete == nullptr)
        {
            ChipLogError(Controller, "Callback for pairing coomplete is not defined.");
            return;
        }
        mPairingComplete(error);
    }

    void SetPairingCompleteCallback(OnPairingCompleteCallback callback) { mPairingComplete = callback; }

private:
    OnPairingCompleteCallback mPairingComplete = nullptr;
};

ServerStorageDelegate gServerStorage;
ScriptDevicePairingDelegate gPairingDelegate;
chip::Controller::ExampleOperationalCredentialsIssuer gOperationalCredentialsIssuer;

} // namespace

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
        chip::Controller::CommissionerInitParams params;

        params.storageDelegate = &gServerStorage;
        params.systemLayer     = &chip::DeviceLayer::SystemLayer;
        params.inetLayer       = &chip::DeviceLayer::InetLayer;
        params.pairingDelegate = &gPairingDelegate;

        err = gOperationalCredentialsIssuer.Initialize(gServerStorage);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "Operational credentials issuer initialization failed: %s", chip::ErrorStr(err));
        }
        else
        {
            params.operationalCredentialsDelegate = &gOperationalCredentialsIssuer;

            err = result->Init(localDeviceId, params);
        }
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

    return chip::ChipError::AsInteger(err);
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

    return chip::ChipError::AsInteger(err);
}
