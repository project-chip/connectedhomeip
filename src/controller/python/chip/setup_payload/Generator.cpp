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

#include <lib/support/CodeUtils.h>
#include <setup_payload/ManualSetupPayloadGenerator.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>

#include <string>
#include <type_traits>

using namespace chip;

static_assert(std::is_same<uint32_t, ChipError::StorageType>::value, "python assumes CHIP_ERROR maps to c_uint32");

extern "C" ChipError::StorageType pychip_SetupPayload_PrintOnboardingCodes(uint32_t passcode, uint16_t vendorId, uint16_t productId,
                                                                           uint16_t discriminator, uint8_t customFlow,
                                                                           uint8_t capabilities, uint8_t version)
{
    std::string QRCode;
    std::string manualPairingCode;
    SetupPayload payload;
    RendezvousInformationFlags rendezvousFlags = RendezvousInformationFlag::kNone;

    payload.version      = version;
    payload.setUpPINCode = passcode;
    payload.vendorID     = vendorId;
    payload.productID    = productId;
    payload.discriminator.SetLongValue(discriminator);
    payload.rendezvousInformation.SetValue(rendezvousFlags.SetRaw(capabilities));

    switch (customFlow)
    {
    case 0:
        payload.commissioningFlow = CommissioningFlow::kStandard;
        break;
    case 1:
        payload.commissioningFlow = CommissioningFlow::kUserActionRequired;
        break;
    case 2:
        payload.commissioningFlow = CommissioningFlow::kCustom;
        break;
    default:
        ChipLogError(SetupPayload, "Invalid Custom Flow");
        return CHIP_ERROR_INVALID_ARGUMENT.AsInteger();
    }

    CHIP_ERROR err = ManualSetupPayloadGenerator(payload).payloadDecimalStringRepresentation(manualPairingCode);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err.AsInteger());
    ChipLogProgress(SetupPayload, "Manual pairing code: [%s]", manualPairingCode.c_str());

    err = QRCodeSetupPayloadGenerator(payload).payloadBase38Representation(QRCode);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err.AsInteger());
    ChipLogProgress(SetupPayload, "SetupQRCode: [%s]", QRCode.c_str());

    return CHIP_NO_ERROR.AsInteger();
}
