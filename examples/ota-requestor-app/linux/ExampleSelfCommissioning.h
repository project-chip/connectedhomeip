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

#pragma once

#include <controller/CHIPDeviceControllerFactory.h>
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/ScopedBuffer.h>

using chip::PersistentStorageDelegate;
using chip::Controller::DeviceController;
using chip::Controller::ExampleOperationalCredentialsIssuer;

CHIP_ERROR DoExampleSelfCommissioning(DeviceController & controller, ExampleOperationalCredentialsIssuer * opCredsIssuer,
                                      PersistentStorageDelegate * storage, chip::NodeId localNodeId, uint16_t listenPort)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::Platform::ScopedMemoryBuffer<uint8_t> noc;
    chip::Platform::ScopedMemoryBuffer<uint8_t> icac;
    chip::Platform::ScopedMemoryBuffer<uint8_t> rcac;
    chip::Controller::FactoryInitParams initParams;
    chip::Controller::SetupParams setupParams;

    VerifyOrExit(storage != nullptr && opCredsIssuer != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    err = opCredsIssuer->Initialize(*storage);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init failure! Operational Cred Issuer: %s", chip::ErrorStr(err)));

    VerifyOrExit(rcac.Alloc(chip::Controller::kMaxCHIPDERCertLength), err = CHIP_ERROR_NO_MEMORY);
    VerifyOrExit(noc.Alloc(chip::Controller::kMaxCHIPDERCertLength), err = CHIP_ERROR_NO_MEMORY);
    VerifyOrExit(icac.Alloc(chip::Controller::kMaxCHIPDERCertLength), err = CHIP_ERROR_NO_MEMORY);

    {
        chip::MutableByteSpan nocSpan(noc.Get(), chip::Controller::kMaxCHIPDERCertLength);
        chip::MutableByteSpan icacSpan(icac.Get(), chip::Controller::kMaxCHIPDERCertLength);
        chip::MutableByteSpan rcacSpan(rcac.Get(), chip::Controller::kMaxCHIPDERCertLength);

        chip::Crypto::P256Keypair ephemeralKey;
        SuccessOrExit(err = ephemeralKey.Initialize());

        // TODO - OpCreds should only be generated for pairing command
        //        store the credentials in persistent storage, and
        //        generate when not available in the storage.
        err = opCredsIssuer->GenerateNOCChainAfterValidation(localNodeId, 0, ephemeralKey.Pubkey(), rcacSpan, icacSpan, nocSpan);
        SuccessOrExit(err);

        setupParams.ephemeralKeypair               = &ephemeralKey;
        setupParams.controllerRCAC                 = rcacSpan;
        setupParams.controllerICAC                 = icacSpan;
        setupParams.controllerNOC                  = nocSpan;
        setupParams.operationalCredentialsDelegate = opCredsIssuer;

        initParams.storageDelegate = storage;
        initParams.listenPort      = listenPort;

        auto & factory = chip::Controller::DeviceControllerFactory::GetInstance();

        err = factory.Init(initParams);
        VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Controller Factory init failure! %s", chip::ErrorStr(err)));

        err = factory.SetupController(setupParams, controller);
        VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Controller init failure! %s", chip::ErrorStr(err)));
    }

exit:
    return err;
}
