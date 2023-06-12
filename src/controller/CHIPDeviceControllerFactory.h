/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/**
 *    @file
 *      DeviceControllerFactory is a singleton utility class that manages the
 *      runtime DeviceControllerSystemState and provides APIs to setup DeviceControllers
 *      and DeviceCommissioners.
 *
 *      Together with the SystemState this class implicitly manages the lifecycle of the underlying
 *      CHIP stack. It lazily initializes the CHIPStack when setting up Controllers if the SystemState
 *      was previously shutdown.
 */

#pragma once

#include <controller/CHIPDeviceController.h>
#include <controller/CHIPDeviceControllerSystemState.h>
#include <credentials/GroupDataProvider.h>
#include <credentials/OperationalCertificateStore.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>

namespace chip {

namespace Controller {

struct SetupParams
{
    OperationalCredentialsDelegate * operationalCredentialsDelegate = nullptr;

    /* The following keypair must correspond to the public key used for generating
    controllerNOC. It's used by controller to establish CASE sessions with devices */
    Crypto::P256Keypair * operationalKeypair = nullptr;

    /**
     * Controls whether or not the operationalKeypair should be owned by the
     * caller.  By default, this is false, but if the keypair cannot be
     * serialized, then setting this to true will allow the caller to manage
     * this keypair's lifecycle.
     */
    bool hasExternallyOwnedOperationalKeypair = false;

    /* The following certificates must be in x509 DER format */
    ByteSpan controllerNOC;
    ByteSpan controllerICAC;
    ByteSpan controllerRCAC;

    //
    // This must be set to a valid, operational VendorId value associated with
    // the controller/commissioner.
    //
    chip::VendorId controllerVendorId = VendorId::Unspecified;

    // The Device Pairing Delegated used to initialize a Commissioner
    DevicePairingDelegate * pairingDelegate = nullptr;

    /**
     * Controls whether we permit multiple DeviceController instances to exist
     * on the same logical fabric (identified by the tuple of the fabric's
     * root public key + fabric id).
     *
     * Each controller instance will be associated with its own FabricIndex.
     * This pivots the FabricTable to tracking identities instead of fabrics,
     * represented by FabricInfo instances that can have colliding logical fabrics.
     *
     */
    bool permitMultiControllerFabrics = false;

    //
    // Controls enabling server cluster interactions on a controller. This in turn
    // causes the following to get enabled:
    //
    //  - CASEServer to listen for unsolicited Sigma1 messages.
    //  - Advertisement of active controller operational identities.
    //
    bool enableServerInteractions = false;

    /**
     * Controls whether shutdown of the controller removes the corresponding
     * entry from the fabric table.  For now the removal is just from the
     * in-memory table, not from storage, which means that after controller
     * shutdown the storage and the in-memory fabric table will be out of sync.
     * This is acceptable for implementations that don't actually store any of
     * the fabric table information, but if someone wants a true removal at some
     * point another option will need to be added here.
     */
    bool removeFromFabricTableOnShutdown = true;

    Credentials::DeviceAttestationVerifier * deviceAttestationVerifier = nullptr;
    CommissioningDelegate * defaultCommissioner                        = nullptr;
};

// TODO everything other than the fabric storage, group data provider, OperationalKeystore,
// OperationalCertificateStore and SessionKeystore here should be removed. We're blocked
// because of the need to support !CHIP_DEVICE_LAYER
struct FactoryInitParams
{
    System::Layer * systemLayer                                        = nullptr;
    PersistentStorageDelegate * fabricIndependentStorage               = nullptr;
    Credentials::CertificateValidityPolicy * certificateValidityPolicy = nullptr;
    Credentials::GroupDataProvider * groupDataProvider                 = nullptr;
    Crypto::SessionKeystore * sessionKeystore                          = nullptr;
    Inet::EndPointManager<Inet::TCPEndPoint> * tcpEndPointManager      = nullptr;
    Inet::EndPointManager<Inet::UDPEndPoint> * udpEndPointManager      = nullptr;
    FabricTable * fabricTable                                          = nullptr;
    OperationalKeystore * operationalKeystore                          = nullptr;
    Credentials::OperationalCertificateStore * opCertStore             = nullptr;
#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * bleLayer = nullptr;
#endif

    //
    // Controls enabling server cluster interactions on a controller. This in turn
    // causes the following to get enabled:
    //
    //  - Advertisement of active controller operational identities.
    //
    bool enableServerInteractions = false;

    /* The port used for operational communication to listen for and send messages over UDP/TCP.
     * The default value of `0` will pick any available port. */
    uint16_t listenPort = 0;
};

class DeviceControllerFactory
{
public:
    static DeviceControllerFactory & GetInstance()
    {
        static DeviceControllerFactory instance;
        return instance;
    }

    CHIP_ERROR Init(FactoryInitParams params);

    // Shuts down matter and frees the system state.
    //
    // Must not be called while any controllers are alive.
    void Shutdown();

    CHIP_ERROR SetupController(SetupParams params, DeviceController & controller);
    CHIP_ERROR SetupCommissioner(SetupParams params, DeviceCommissioner & commissioner);

    // ----- IO -----
    /**
     * @brief
     * Start the event loop task within the CHIP stack
     * @return CHIP_ERROR   The return status
     */
    CHIP_ERROR ServiceEvents();

    ~DeviceControllerFactory();
    DeviceControllerFactory(DeviceControllerFactory const &) = delete;
    void operator=(DeviceControllerFactory const &) = delete;

    //
    // Some clients do not prefer a complete shutdown of the stack being initiated if
    // all device controllers have ceased to exist. To avoid that, this method has been
    // created to permit retention of the underlying system state.
    //
    // NB: The system state will still be freed in Shutdown() regardless of this call.
    void RetainSystemState();

    //
    // To initiate shutdown of the stack upon termination of all resident controllers in the
    // system, invoke this method to decrement the refcount on the system state and consequently,
    // shut-down the stack.
    //
    // This should only be invoked if a matching call to RetainSystemState() was called prior.
    //
    void ReleaseSystemState();

    //
    // Retrieve a read-only pointer to the system state object that contains pointers to key stack
    // singletons. If the pointer is null, it indicates that the DeviceControllerFactory has yet to
    // be initialized properly, or has already been shut-down.
    //
    // This pointer ceases to be valid after a call to Shutdown has been made, or if all active
    // DeviceController instances have gone to 0. Consequently, care has to be taken to correctly
    // sequence the shutting down of active controllers with any entity that interacts with objects
    // present in the system state object. If de-coupling is desired, RetainSystemState and
    // ReleaseSystemState can be used to avoid this.
    //
    const DeviceControllerSystemState * GetSystemState() const { return mSystemState; }

    class ControllerFabricDelegate final : public chip::FabricTable::Delegate
    {
    public:
        CHIP_ERROR Init(SessionResumptionStorage * sessionResumptionStorage, Credentials::GroupDataProvider * groupDataProvider)
        {
            VerifyOrReturnError(sessionResumptionStorage != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
            VerifyOrReturnError(groupDataProvider != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

            mSessionResumptionStorage = sessionResumptionStorage;
            mGroupDataProvider        = groupDataProvider;
            return CHIP_NO_ERROR;
        };

        void OnFabricRemoved(const chip::FabricTable & fabricTable, FabricIndex fabricIndex) override
        {
            (void) fabricTable;
            if (mGroupDataProvider != nullptr)
            {
                mGroupDataProvider->RemoveFabric(fabricIndex);
            }
            ClearCASEResumptionStateOnFabricChange(fabricIndex);
        };

        void OnFabricUpdated(const chip::FabricTable & fabricTable, chip::FabricIndex fabricIndex) override
        {
            (void) fabricTable;
            ClearCASEResumptionStateOnFabricChange(fabricIndex);
        }

    private:
        void ClearCASEResumptionStateOnFabricChange(chip::FabricIndex fabricIndex)
        {
            VerifyOrReturn(mSessionResumptionStorage != nullptr);
            CHIP_ERROR err = mSessionResumptionStorage->DeleteAll(fabricIndex);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Controller,
                             "Warning, failed to delete session resumption state for fabric index 0x%x: %" CHIP_ERROR_FORMAT,
                             static_cast<unsigned>(fabricIndex), err.Format());
            }
        }

        Credentials::GroupDataProvider * mGroupDataProvider  = nullptr;
        SessionResumptionStorage * mSessionResumptionStorage = nullptr;
    };

private:
    DeviceControllerFactory() {}
    void PopulateInitParams(ControllerInitParams & controllerParams, const SetupParams & params);
    CHIP_ERROR InitSystemState(FactoryInitParams params);
    CHIP_ERROR InitSystemState();

    uint16_t mListenPort;
    DeviceControllerSystemState * mSystemState                          = nullptr;
    PersistentStorageDelegate * mFabricIndependentStorage               = nullptr;
    Crypto::OperationalKeystore * mOperationalKeystore                  = nullptr;
    Credentials::OperationalCertificateStore * mOpCertStore             = nullptr;
    Credentials::CertificateValidityPolicy * mCertificateValidityPolicy = nullptr;
    bool mEnableServerInteractions                                      = false;
};

} // namespace Controller
} // namespace chip
