/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
