/*
 *   Copyright (c) 2022 Project CHIP Authors
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

#include <controller/CHIPDeviceController.h>
#include <lib/support/CodeUtils.h>

#include <app-common/zap-generated/tests/simulated-cluster-objects.h>

class CommissionerCommands : public chip::Controller::DevicePairingDelegate
{
public:
    CommissionerCommands(){};
    ~CommissionerCommands() override{};

    virtual void OnResponse(const chip::app::StatusIB & status, chip::TLV::TLVReader * data) = 0;
    virtual CHIP_ERROR ContinueOnChipMainThread(CHIP_ERROR err)                              = 0;
    virtual chip::Controller::DeviceCommissioner & GetCommissioner(const char * identity)    = 0;

    CHIP_ERROR PairWithCode(const char * identity,
                            const chip::app::Clusters::CommissionerCommands::Commands::PairWithCode::Type & value);
    CHIP_ERROR Unpair(const char * identity, const chip::app::Clusters::CommissionerCommands::Commands::Unpair::Type & value);

    /////////// DevicePairingDelegate Interface /////////
    void OnStatusUpdate(chip::Controller::DevicePairingDelegate::Status status) override;
    void OnPairingComplete(CHIP_ERROR error) override;
    void OnPairingDeleted(CHIP_ERROR error) override;
    void OnCommissioningComplete(chip::NodeId deviceId, CHIP_ERROR error) override;
};
