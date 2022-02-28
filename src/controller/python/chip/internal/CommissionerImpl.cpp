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
#include <controller/CHIPDeviceControllerFactory.h>
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/KeyValueStoreManager.h>

#include "ChipThreadWork.h"

using DeviceControllerFactory = chip::Controller::DeviceControllerFactory;

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
chip::SimpleFabricStorage gFabricStorage;
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
        chip::Controller::SetupParams commissionerParams;
        chip::Controller::FactoryInitParams factoryParams;
        chip::Platform::ScopedMemoryBuffer<uint8_t> noc;
        chip::Platform::ScopedMemoryBuffer<uint8_t> icac;
        chip::Platform::ScopedMemoryBuffer<uint8_t> rcac;
        chip::Crypto::P256Keypair ephemeralKey;

        // Initialize device attestation verifier
        // TODO: Replace testingRootStore with a AttestationTrustStore that has the necessary official PAA roots available
        const chip::Credentials::AttestationTrustStore * testingRootStore = chip::Credentials::GetTestAttestationTrustStore();
        chip::Credentials::SetDeviceAttestationVerifier(chip::Credentials::GetDefaultDACVerifier(testingRootStore));

        err = gFabricStorage.Initialize(&gServerStorage);
        SuccessOrExit(err);

        factoryParams.fabricStorage            = &gFabricStorage;
        factoryParams.fabricIndependentStorage = &gServerStorage;

        commissionerParams.pairingDelegate = &gPairingDelegate;
        commissionerParams.storageDelegate = &gServerStorage;

        err = ephemeralKey.Initialize();
        SuccessOrExit(err);

        err = gOperationalCredentialsIssuer.Initialize(gServerStorage);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "Operational credentials issuer initialization failed: %s", chip::ErrorStr(err));
            ExitNow();
        }

        VerifyOrExit(noc.Alloc(chip::Controller::kMaxCHIPDERCertLength), err = CHIP_ERROR_NO_MEMORY);
        VerifyOrExit(icac.Alloc(chip::Controller::kMaxCHIPDERCertLength), err = CHIP_ERROR_NO_MEMORY);
        VerifyOrExit(rcac.Alloc(chip::Controller::kMaxCHIPDERCertLength), err = CHIP_ERROR_NO_MEMORY);

        {
            chip::MutableByteSpan nocSpan(noc.Get(), chip::Controller::kMaxCHIPDERCertLength);
            chip::MutableByteSpan icacSpan(icac.Get(), chip::Controller::kMaxCHIPDERCertLength);
            chip::MutableByteSpan rcacSpan(rcac.Get(), chip::Controller::kMaxCHIPDERCertLength);
            err = gOperationalCredentialsIssuer.GenerateNOCChainAfterValidation(localDeviceId, /* fabricId = */ 1,
                                                                                ephemeralKey.Pubkey(), rcacSpan, icacSpan, nocSpan);
            SuccessOrExit(err);

            commissionerParams.operationalCredentialsDelegate = &gOperationalCredentialsIssuer;
            commissionerParams.operationalKeypair             = &ephemeralKey;
            commissionerParams.controllerRCAC                 = rcacSpan;
            commissionerParams.controllerICAC                 = icacSpan;
            commissionerParams.controllerNOC                  = nocSpan;

            SuccessOrExit(DeviceControllerFactory::GetInstance().Init(factoryParams));
            err = DeviceControllerFactory::GetInstance().SetupCommissioner(commissionerParams, *result);
        }
    exit:
        ChipLogProgress(Controller, "Commissioner initialization status: %s", chip::ErrorStr(err));
    });

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Commissioner initialization failed: %s", chip::ErrorStr(err));
        return nullptr;
    }

    return result.release();
}

static_assert(std::is_same<uint32_t, chip::ChipError::StorageType>::value, "python assumes CHIP_ERROR maps to c_uint32");

/// Returns CHIP_ERROR corresponding to an UnpairDevice call
extern "C" chip::ChipError::StorageType pychip_internal_Commissioner_Unpair(chip::Controller::DeviceCommissioner * commissioner,
                                                                            uint64_t remoteDeviceId)
{
    CHIP_ERROR err;

    chip::python::ChipMainThreadScheduleAndWait([&]() { err = commissioner->UnpairDevice(remoteDeviceId); });

    return err.AsInteger();
}

extern "C" chip::ChipError::StorageType
pychip_internal_Commissioner_BleConnectForPairing(chip::Controller::DeviceCommissioner * commissioner, uint64_t remoteNodeId,
                                                  uint32_t pinCode, uint16_t discriminator)
{

    CHIP_ERROR err;

    chip::python::ChipMainThreadScheduleAndWait([&]() {
        chip::RendezvousParameters params;

        params.SetDiscriminator(discriminator).SetSetupPINCode(pinCode);
#if CONFIG_NETWORK_LAYER_BLE
        params.SetBleLayer(chip::DeviceLayer::ConnectivityMgr().GetBleLayer()).SetPeerAddress(chip::Transport::PeerAddress::BLE());
#endif

        err = commissioner->PairDevice(remoteNodeId, params);
    });

    return err.AsInteger();
}
