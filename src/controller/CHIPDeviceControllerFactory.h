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
#include <credentials/DeviceAttestationVerifier.h>

namespace chip {

namespace Controller {

struct SetupParams
{
#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
    DeviceAddressUpdateDelegate * deviceAddressUpdateDelegate = nullptr;
#endif
    OperationalCredentialsDelegate * operationalCredentialsDelegate = nullptr;

    PersistentStorageDelegate * storageDelegate = nullptr;

    /* The following keypair must correspond to the public key used for generating
    controllerNOC. It's used by controller to establish CASE sessions with devices */
    Crypto::P256Keypair * operationalKeypair = nullptr;

    /* The following certificates must be in x509 DER format */
    ByteSpan controllerNOC;
    ByteSpan controllerICAC;
    ByteSpan controllerRCAC;

    uint16_t controllerVendorId;

    // The Device Pairing Delegated used to initialize a Commissioner
    DevicePairingDelegate * pairingDelegate = nullptr;

    Credentials::DeviceAttestationVerifier * deviceAttestationVerifier = nullptr;
    CommissioningDelegate * defaultCommissioner                        = nullptr;
};

// TODO everything other than the fabric storage here should be removed.
// We're blocked because of the need to support !CHIP_DEVICE_LAYER
struct FactoryInitParams
{
    FabricStorage * fabricStorage                                 = nullptr;
    System::Layer * systemLayer                                   = nullptr;
    Inet::EndPointManager<Inet::TCPEndPoint> * tcpEndPointManager = nullptr;
    Inet::EndPointManager<Inet::UDPEndPoint> * udpEndPointManager = nullptr;
#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * bleLayer = nullptr;
#endif

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
    // created to permit retention of the underlying system state to avoid that.
    //
    void RetainSystemState() { (void) mSystemState->Retain(); }

    //
    // To initiate shutdown of the stack upon termination of all resident controllers in the
    // system, invoke this method to decrement the refcount on the system state and consequently,
    // shut-down the stack.
    //
    // This should only be invoked if a matching call to RetainSystemState() was called prior.
    //
    void ReleaseSystemState() { mSystemState->Release(); }

private:
    DeviceControllerFactory(){};
    void PopulateInitParams(ControllerInitParams & controllerParams, const SetupParams & params);
    CHIP_ERROR InitSystemState(FactoryInitParams params);
    CHIP_ERROR InitSystemState();

    uint16_t mListenPort;
    FabricStorage * mFabricStorage             = nullptr;
    DeviceControllerSystemState * mSystemState = nullptr;
};

} // namespace Controller
} // namespace chip
