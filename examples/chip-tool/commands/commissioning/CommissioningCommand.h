/*
 *   Copyright (c) 2020 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#pragma once

#include "../../config/PersistentStorage.h"
#include "../common/Command.h"
#include <setup_payload/SetupPayload.h>

enum class CommissioningType
{
    DeviceAttestation,
    OpenPairingWindowReq,
};

class CommissioningCommand : public Command, public chip::Controller::DevicePairingDelegate
{
public:
    CommissioningCommand(const char * commandName, CommissioningType commissioningType) :
        Command(commandName), mCommissioningType(commissioningType)
    {
        switch (commissioningType)
        {
        case CommissioningType::DeviceAttestation:
            break;
        case CommissioningType::OpenPairingWindowReq:
            AddArgument("discriminator", 0, 4096, &mDiscriminator);
            break;
        }
    }

    /////////// Command Interface /////////
    CHIP_ERROR Run(PersistentStorage & storage, NodeId localId, NodeId remoteId) override;

    /////////// DevicePairingDelegate Interface /////////
    void OnStatusUpdate(chip::RendezvousSessionDelegate::Status status) override;
    void OnNetworkCredentialsRequested(chip::RendezvousDeviceCredentialsDelegate * callback) override {}
    void OnOperationalCredentialsRequested(const char * csr, size_t csr_length,
                                           chip::RendezvousDeviceCredentialsDelegate * callback) override
    {}
    void OnPairingComplete(CHIP_ERROR error) override;
    void OnPairingDeleted(CHIP_ERROR error) override;

private:
    CHIP_ERROR RunInternal();

    const CommissioningType mCommissioningType;
    ChipDeviceCommissioner mCommissioner;
    ChipDevice * mDevice;
    uint16_t mDiscriminator;
    chip::SetupPayload mSetupPayload;
};
