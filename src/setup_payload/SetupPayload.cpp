/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      The implementation of the Setup Payload. Currently only needed to
 *      verify the validity of a Setup Payload
 */

#include "SetupPayload.h"

#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <lib/core/TLV.h>
#include <lib/core/TLVData.h>
#include <lib/core/TLVUtilities.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <setup_payload/ManualSetupPayloadParser.h>
#include <setup_payload/QRCodeSetupPayloadParser.h>
#include <utility>

namespace chip {

// Check the Setup Payload for validity
//
// `vendor_id` and `product_id` are allowed all of uint16_t
bool PayloadContents::isValidQRCodePayload(ValidationMode mode) const
{
    // 3-bit value specifying the QR code payload version.
    VerifyOrReturnValue(version < (1 << kVersionFieldLengthInBits), false);

    VerifyOrReturnValue(static_cast<uint8_t>(commissioningFlow) < (1 << kCommissioningFlowFieldLengthInBits), false);

    // Device Commissioning Flow
    // Even in ValidationMode::kConsume we can only handle modes that we understand.
    // 0: Standard commissioning flow: such a device, when uncommissioned, always enters commissioning mode upon power-up, subject
    // to the rules in [ref_Announcement_Commencement]. 1: User-intent commissioning flow: user action required to enter
    // commissioning mode. 2: Custom commissioning flow: interaction with a vendor-specified means is needed before commissioning.
    // 3: Reserved
    VerifyOrReturnValue(commissioningFlow == CommissioningFlow::kStandard ||
                            commissioningFlow == CommissioningFlow::kUserActionRequired ||
                            commissioningFlow == CommissioningFlow::kCustom,
                        false);

    // General discriminator validity is enforced by the SetupDiscriminator class, but it can't be short for QR a code.
    VerifyOrReturnValue(!discriminator.IsShortDiscriminator(), false);

    // RendevouzInformation must be present for a QR code.
    VerifyOrReturnValue(rendezvousInformation.HasValue(), false);
    if (mode == ValidationMode::kProduce)
    {
        chip::RendezvousInformationFlags valid(RendezvousInformationFlag::kBLE, RendezvousInformationFlag::kOnNetwork,
                                               RendezvousInformationFlag::kSoftAP, RendezvousInformationFlag::kWiFiPAF,
                                               RendezvousInformationFlag::kNFC, RendezvousInformationFlag::kThread);
        VerifyOrReturnValue(rendezvousInformation.Value().HasOnly(valid), false);
    }

    return CheckPayloadCommonConstraints();
}

bool PayloadContents::isValidManualCode(ValidationMode mode) const
{
    // No additional constraints apply to Manual Pairing Codes.
    // (If the payload has a long discriminator it will be converted automatically.)
    return CheckPayloadCommonConstraints();
}

bool PayloadContents::IsValidSetupPIN(uint32_t setupPIN)
{
    // SHALL be restricted to the values 0x0000001 to 0x5F5E0FE (00000001 to 99999998 in decimal), excluding the invalid Passcode
    // values.
    if (setupPIN == kSetupPINCodeUndefinedValue || setupPIN > kSetupPINCodeMaximumValue || setupPIN == 11111111 ||
        setupPIN == 22222222 || setupPIN == 33333333 || setupPIN == 44444444 || setupPIN == 55555555 || setupPIN == 66666666 ||
        setupPIN == 77777777 || setupPIN == 88888888 || setupPIN == 12345678 || setupPIN == 87654321)
    {
        return false;
    }

    return true;
}

bool PayloadContents::CheckPayloadCommonConstraints() const
{
    // Validation rules in this method apply to all validation modes.

    // Even in ValidationMode::kConsume we don't understand how to handle any payload version other than 0.
    VerifyOrReturnValue(version == 0, false);

    VerifyOrReturnValue(IsValidSetupPIN(setUpPINCode), false);

    // VendorID must be unspecified (0) or in valid range expected.
    VerifyOrReturnValue((vendorID == VendorId::Unspecified) || IsVendorIdValidOperationally(vendorID), false);

    // A value of 0x0000 SHALL NOT be assigned to a product since Product ID = 0x0000 is used for these specific cases:
    //  * To announce an anonymized Product ID as part of device discovery
    //  * To indicate an OTA software update file applies to multiple Product IDs equally.
    //  * To avoid confusion when presenting the Onboarding Payload for ECM with multiple nodes
    // In these special cases the vendorID must be 0 (Unspecified)
    VerifyOrReturnValue(productID != 0 || vendorID == VendorId::Unspecified, false);

    return true;
}

bool PayloadContents::operator==(const PayloadContents & input) const
{
    return (this->version == input.version && this->vendorID == input.vendorID && this->productID == input.productID &&
            this->commissioningFlow == input.commissioningFlow && this->rendezvousInformation == input.rendezvousInformation &&
            this->discriminator == input.discriminator && this->setUpPINCode == input.setUpPINCode);
}

CHIP_ERROR SetupPayload::addOptionalVendorData(uint8_t tag, std::string data)
{
    return addOptionalVendorData(OptionalQRCodeInfo{ tag, std::move(data) });
}

CHIP_ERROR SetupPayload::addOptionalVendorData(uint8_t tag, int64_t data)
{
    return addOptionalVendorData(OptionalQRCodeInfo{ tag, data });
}

CHIP_ERROR SetupPayload::addOptionalVendorData(uint8_t tag, uint64_t data)
{
    return addOptionalVendorData(OptionalQRCodeInfo{ tag, data });
}

std::vector<OptionalQRCodeInfo> SetupPayload::getAllOptionalVendorData() const
{
    std::vector<OptionalQRCodeInfo> returnedOptionalInfo;
    for (auto & entry : optionalVendorData)
    {
        returnedOptionalInfo.push_back(entry.second);
    }
    return returnedOptionalInfo;
}

CHIP_ERROR SetupPayload::removeOptionalVendorData(uint8_t tag)
{
    VerifyOrReturnError(optionalVendorData.find(tag) != optionalVendorData.end(), CHIP_ERROR_KEY_NOT_FOUND);
    optionalVendorData.erase(tag);

    return CHIP_NO_ERROR;
}

CHIP_ERROR SetupPayload::addSerialNumber(std::string serialNumber)
{
    return addOptionalExtensionData(OptionalQRCodeInfo{ kSerialNumberTag, std::move(serialNumber) });
}

CHIP_ERROR SetupPayload::addSerialNumber(uint32_t serialNumber)
{
    return addOptionalExtensionData(OptionalQRCodeInfo{ kSerialNumberTag, uint64_t(serialNumber) });
}

CHIP_ERROR SetupPayload::getSerialNumber(std::string & outSerialNumber) const
{
    std::optional<OptionalQRCodeInfo> info = getOptionalExtensionData(kSerialNumberTag);
    VerifyOrReturnError(info.has_value(), CHIP_ERROR_KEY_NOT_FOUND);

    return info->visitValue(
        [&](const std::string & v) {
            outSerialNumber = v;
            return CHIP_NO_ERROR;
        },
        [](int64_t) { return CHIP_ERROR_INVALID_ARGUMENT; },
        [&](uint64_t v) {
            outSerialNumber = std::to_string(v);
            return CHIP_NO_ERROR;
        });
}

CHIP_ERROR SetupPayload::removeSerialNumber()
{
    VerifyOrReturnError(optionalExtensionData.find(kSerialNumberTag) != optionalExtensionData.end(), CHIP_ERROR_KEY_NOT_FOUND);
    optionalExtensionData.erase(kSerialNumberTag);

    return CHIP_NO_ERROR;
}

CHIP_ERROR SetupPayload::generateRandomSetupPin(uint32_t & setupPINCode)
{
    uint8_t retries          = 0;
    const uint8_t maxRetries = 10;

    do
    {
        ReturnErrorOnFailure(Crypto::DRBG_get_bytes(reinterpret_cast<uint8_t *>(&setupPINCode), sizeof(setupPINCode)));

        // Passcodes shall be restricted to the values 00000001 to 99999998 in decimal, see 5.1.1.6
        // TODO: Consider revising this method to ensure uniform distribution of setup PIN codes
        setupPINCode = (setupPINCode % kSetupPINCodeMaximumValue) + 1;

        // Make sure that the Generated Setup Pin code is not one of the invalid passcodes/pin codes defined in the
        // specification.
        if (IsValidSetupPIN(setupPINCode))
        {
            return CHIP_NO_ERROR;
        }

        retries++;
        // We got pretty unlucky with the random number generator, Just try again.
        // This shouldn't take many retries assuming DRBG_get_bytes is not broken.
    } while (retries < maxRetries);

    return CHIP_ERROR_INTERNAL;
}

CHIP_ERROR SetupPayload::addOptionalVendorData(const OptionalQRCodeInfo & info)
{
    VerifyOrReturnError(IsVendorTag(info.tag), CHIP_ERROR_INVALID_ARGUMENT);
    optionalVendorData.insert_or_assign(info.tag, info);

    return CHIP_NO_ERROR;
}

CHIP_ERROR SetupPayload::addOptionalExtensionData(const OptionalQRCodeInfo & info)
{
    VerifyOrReturnError(IsCommonTag(info.tag), CHIP_ERROR_INVALID_ARGUMENT);

    // We're lenient for tags that are reserved for future use, for forward-compatibility reasons.
    bool validValue = info.visitValue(
        [&](const std::string & v) {
            return !(info.tag == kPBKDFIterationsTag || info.tag == kNumberOFDevicesTag || info.tag == kCommissioningTimeoutTag);
        },
        [&](int64_t v) {
            return !(info.tag == kSerialNumberTag || info.tag == kPBKDFIterationsTag || info.tag == kPBKFSaltTag ||
                     info.tag == kNumberOFDevicesTag || info.tag == kCommissioningTimeoutTag);
        },
        [&](uint64_t v) {
            if (info.tag == kSerialNumberTag)
            {
                return CanCastTo<uint32_t>(v);
            }
            if (info.tag == kPBKDFIterationsTag)
            {
                return Crypto::kSpake2p_Min_PBKDF_Iterations <= v && v <= Crypto::kSpake2p_Max_PBKDF_Iterations;
            }
            if (info.tag == kNumberOFDevicesTag)
            {
                return 0 < v && v <= 255;
            }
            return info.tag != kPBKFSaltTag;
        });
    // It is not clearly specified whether this should return an error, or simply ignore.
    VerifyOrReturnError(validValue, CHIP_ERROR_INVALID_ARGUMENT);

    optionalExtensionData.insert_or_assign(info.tag, info);

    return CHIP_NO_ERROR;
}

std::optional<OptionalQRCodeInfo> SetupPayload::getOptionalVendorData(uint8_t tag) const
{
    const auto it = optionalVendorData.find(tag);
    VerifyOrReturnError(it != optionalVendorData.end(), std::nullopt);
    return it->second;
}

std::optional<OptionalQRCodeInfo> SetupPayload::getOptionalExtensionData(uint8_t tag) const
{
    const auto it = optionalExtensionData.find(tag);
    VerifyOrReturnError(it != optionalExtensionData.end(), std::nullopt);
    return it->second;
}

std::vector<OptionalQRCodeInfo> SetupPayload::getAllOptionalExtensionData() const
{
    std::vector<OptionalQRCodeInfo> returnedOptionalInfo;
    for (auto & entry : optionalExtensionData)
    {
        returnedOptionalInfo.push_back(entry.second);
    }
    return returnedOptionalInfo;
}

bool SetupPayload::operator==(const SetupPayload & input) const
{
    std::vector<OptionalQRCodeInfo> inputOptionalVendorData;
    std::vector<OptionalQRCodeInfo> inputOptionalExtensionData;

    VerifyOrReturnError(PayloadContents::operator==(input), false);

    inputOptionalVendorData = input.getAllOptionalVendorData();
    VerifyOrReturnError(optionalVendorData.size() == inputOptionalVendorData.size(), false);

    for (const OptionalQRCodeInfo & inputInfo : inputOptionalVendorData)
    {
        std::optional<OptionalQRCodeInfo> info = getOptionalVendorData(inputInfo.tag);
        VerifyOrReturnError(info.has_value(), false);
        VerifyOrReturnError(inputInfo.value == info->value, false);
    }

    inputOptionalExtensionData = input.getAllOptionalExtensionData();
    VerifyOrReturnError(optionalExtensionData.size() == inputOptionalExtensionData.size(), false);

    for (const OptionalQRCodeInfo & inputInfo : inputOptionalExtensionData)
    {
        std::optional<OptionalQRCodeInfo> info = getOptionalExtensionData(inputInfo.tag);
        VerifyOrReturnError(info.has_value(), false);
        VerifyOrReturnError(inputInfo.value == info->value, false);
    }

    return true;
}

CHIP_ERROR SetupPayload::FromStringRepresentation(std::string stringRepresentation, std::vector<SetupPayload> & outPayloads)
{
    // We're going to assume that in practice all these allocations are small
    // enough that allocation failure will not happen.  If that ever turns out
    // to not be the case, we may need to figure out how to handle that.

    // std::string::starts_with is C++20, sadly.
    bool isQRCode = (stringRepresentation.rfind(kQRCodePrefix, 0) == 0);
    if (!isQRCode)
    {
        outPayloads.clear();
        auto & payload = outPayloads.emplace_back();
        ReturnErrorOnFailure(ManualSetupPayloadParser(stringRepresentation).populatePayload(payload));
        VerifyOrReturnError(payload.isValidManualCode(), CHIP_ERROR_INVALID_ARGUMENT);
        return CHIP_NO_ERROR;
    }

    ReturnErrorOnFailure(QRCodeSetupPayloadParser(stringRepresentation).populatePayloads(outPayloads));

    for (auto & entry : outPayloads)
    {
        VerifyOrReturnError(entry.isValidQRCodePayload(), CHIP_ERROR_INVALID_ARGUMENT);
    }
    return CHIP_NO_ERROR;
}

} // namespace chip
