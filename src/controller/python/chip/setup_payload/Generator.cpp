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

#include <string>
#include <type_traits>

#include <controller/python/chip/native/PyChipError.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <setup_payload/ManualSetupPayloadGenerator.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>

using namespace chip;

namespace {

CHIP_ERROR InitSetupPayload(SetupPayload & payload, uint32_t passcode, uint16_t vendorId, uint16_t productId,
                            uint16_t discriminator, uint8_t customFlow, uint8_t capabilities, uint8_t version)
{

    payload.version      = version;
    payload.setUpPINCode = passcode;
    payload.vendorID     = vendorId;
    payload.productID    = productId;
    payload.discriminator.SetLongValue(discriminator);

    RendezvousInformationFlags rendezvousFlags = RendezvousInformationFlag::kNone;
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
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}

} // namespace

extern "C" PyChipError pychip_SetupPayload_PrintOnboardingCodes(uint32_t passcode, uint16_t vendorId, uint16_t productId,
                                                                uint16_t discriminator, uint8_t customFlow, uint8_t capabilities,
                                                                uint8_t version)
{
    SetupPayload payload;
    CHIP_ERROR err = InitSetupPayload(payload, passcode, vendorId, productId, discriminator, customFlow, capabilities, version);
    VerifyOrReturnError(err == CHIP_NO_ERROR, ToPyChipError(err));

    std::string manualPairingCode;
    err = ManualSetupPayloadGenerator(payload).payloadDecimalStringRepresentation(manualPairingCode);
    VerifyOrReturnError(err == CHIP_NO_ERROR, ToPyChipError(err));
    ChipLogProgress(SetupPayload, "Manual pairing code: [%s]", manualPairingCode.c_str());

    std::string QRCode;
    err = QRCodeSetupPayloadGenerator(payload).payloadBase38Representation(QRCode);
    VerifyOrReturnError(err == CHIP_NO_ERROR, ToPyChipError(err));
    ChipLogProgress(SetupPayload, "SetupQRCode: [%s]", QRCode.c_str());

    return ToPyChipError(CHIP_NO_ERROR);
}

extern "C" PyChipError pychip_SetupPayload_GenerateQrCode(char * output, size_t size, uint32_t passcode, uint16_t vendorId,
                                                          uint16_t productId, uint16_t discriminator, uint8_t customFlow,
                                                          uint8_t capabilities, uint8_t version)
{
    SetupPayload payload;
    CHIP_ERROR err = InitSetupPayload(payload, passcode, vendorId, productId, discriminator, customFlow, capabilities, version);
    VerifyOrReturnError(err == CHIP_NO_ERROR, ToPyChipError(err));

    std::string QRCode;
    err = QRCodeSetupPayloadGenerator(payload).payloadBase38Representation(QRCode);
    VerifyOrReturnError(err == CHIP_NO_ERROR, ToPyChipError(err));

    Platform::CopyString(output, size, QRCode.c_str());
    return ToPyChipError(CHIP_NO_ERROR);
}

extern "C" PyChipError pychip_SetupPayload_GenerateManualPairingCode(char * output, size_t size, uint32_t passcode,
                                                                     uint16_t vendorId, uint16_t productId, uint16_t discriminator,
                                                                     uint8_t customFlow, uint8_t capabilities, uint8_t version)
{
    SetupPayload payload;
    CHIP_ERROR err = InitSetupPayload(payload, passcode, vendorId, productId, discriminator, customFlow, capabilities, version);
    VerifyOrReturnError(err == CHIP_NO_ERROR, ToPyChipError(err));

    std::string manualPairingCode;
    err = ManualSetupPayloadGenerator(payload).payloadDecimalStringRepresentation(manualPairingCode);
    VerifyOrReturnError(err == CHIP_NO_ERROR, ToPyChipError(err));

    Platform::CopyString(output, size, manualPairingCode.c_str());
    return ToPyChipError(CHIP_NO_ERROR);
}
