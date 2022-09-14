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

#include "CommissionerCommands.h"

constexpr uint16_t kPayloadMaxSize = 64;

CHIP_ERROR
CommissionerCommands::PairWithCode(const char * identity,
                                   const chip::app::Clusters::CommissionerCommands::Commands::PairWithCode::Type & value)
{
    VerifyOrReturnError(value.payload.size() > 0 && value.payload.size() < kPayloadMaxSize, CHIP_ERROR_INVALID_ARGUMENT);

    GetCommissioner(identity).RegisterPairingDelegate(this);

    char code[kPayloadMaxSize];
    memset(code, '\0', sizeof(code));
    memcpy(code, value.payload.data(), value.payload.size());
    ChipLogError(chipTool, "Pairing Code is %s", code);

    // To reduce the scanning latency in some setups, and since the primary use for PairWithCode is to commission a device to
    // another commissioner, assume that the commissionable device is available on the network.
    chip::Controller::DiscoveryType discoveryType = chip::Controller::DiscoveryType::kDiscoveryNetworkOnly;
    return GetCommissioner(identity).PairDevice(value.nodeId, code, discoveryType);
}

CHIP_ERROR CommissionerCommands::Unpair(const char * identity,
                                        const chip::app::Clusters::CommissionerCommands::Commands::Unpair::Type & value)
{
    return GetCommissioner(identity).UnpairDevice(value.nodeId);
}

chip::app::StatusIB ConvertToStatusIB(CHIP_ERROR err)
{
    using chip::app::StatusIB;
    using namespace chip;
    using namespace chip::Protocols::InteractionModel;
    using namespace chip::app::Clusters::OperationalCredentials;

    if (CHIP_ERROR_INVALID_PUBLIC_KEY == err)
    {
        return StatusIB(Status::Failure, to_underlying(OperationalCertStatus::kInvalidPublicKey));
    }
    if (CHIP_ERROR_WRONG_NODE_ID == err)
    {
        return StatusIB(Status::Failure, to_underlying(OperationalCertStatus::kInvalidNodeOpId));
    }
    if (CHIP_ERROR_UNSUPPORTED_CERT_FORMAT == err)
    {
        return StatusIB(Status::Failure, to_underlying(OperationalCertStatus::kInvalidNOC));
    }
    if (CHIP_ERROR_FABRIC_EXISTS == err)
    {
        return StatusIB(Status::Failure, to_underlying(OperationalCertStatus::kFabricConflict));
    }
    if (CHIP_ERROR_INVALID_FABRIC_INDEX == err)
    {
        return StatusIB(Status::Failure, to_underlying(OperationalCertStatus::kInvalidFabricIndex));
    }

    return StatusIB(err);
}

void CommissionerCommands::OnStatusUpdate(DevicePairingDelegate::Status status)
{
    switch (status)
    {
    case DevicePairingDelegate::Status::SecurePairingSuccess:
        ChipLogProgress(chipTool, "Secure Pairing Success");
        break;
    case DevicePairingDelegate::Status::SecurePairingFailed:
        ChipLogError(chipTool, "Secure Pairing Failed");
        OnResponse(ConvertToStatusIB(CHIP_ERROR_INCORRECT_STATE), nullptr);
        break;
    case DevicePairingDelegate::Status::SecurePairingDiscoveringMoreDevices:
        break;
    }
}

void CommissionerCommands::OnPairingComplete(CHIP_ERROR err)
{
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(chipTool, "Pairing Complete Failure: %s", ErrorStr(err));
        OnResponse(ConvertToStatusIB(err), nullptr);
    }
}

void CommissionerCommands::OnPairingDeleted(CHIP_ERROR err)
{
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(chipTool, "Pairing Delete Failure: %s", ErrorStr(err));
    }

    OnResponse(ConvertToStatusIB(err), nullptr);
}

void CommissionerCommands::OnCommissioningComplete(chip::NodeId nodeId, CHIP_ERROR err)
{
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(chipTool, "Commissioning Complete Failure: %s", ErrorStr(err));
    }

    OnResponse(ConvertToStatusIB(err), nullptr);
}
