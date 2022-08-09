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

#pragma once

#include <bitset>

#include <setup_payload/Base38Decode.h>
#include <setup_payload/Base38Encode.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/QRCodeSetupPayloadParser.h>
#include <setup_payload/SetupPayload.h>

namespace chip {

const uint16_t kSmallBufferSizeInBytes   = 1;
const uint16_t kDefaultBufferSizeInBytes = 512;

const uint8_t kOptionalDefaultStringTag      = 0x82; // Vendor "test" tag
constexpr char kOptionalDefaultStringValue[] = "myData";

const uint8_t kOptionalDefaultIntTag    = 0x83; // Vendor "test" tag
const uint32_t kOptionalDefaultIntValue = 12;

constexpr char kSerialNumberDefaultStringValue[] = "123456789";
const uint32_t kSerialNumberDefaultUInt32Value   = 123456789;

constexpr const char * kDefaultPayloadQRCode = "MT:M5L90MP500K64J00000";

inline SetupPayload GetDefaultPayload()
{
    SetupPayload payload;

    payload.version           = 0;
    payload.vendorID          = 12;
    payload.productID         = 1;
    payload.commissioningFlow = CommissioningFlow::kStandard;
    payload.rendezvousInformation.SetValue(RendezvousInformationFlag::kSoftAP);
    payload.discriminator.SetLongValue(128);
    payload.setUpPINCode = 2048;

    return payload;
}

inline SetupPayload GetDefaultPayloadWithSerialNumber()
{
    SetupPayload payload = GetDefaultPayload();
    payload.addSerialNumber(kSerialNumberDefaultStringValue);

    return payload;
}

inline SetupPayload GetDefaultPayloadWithOptionalDefaults()
{
    SetupPayload payload = GetDefaultPayloadWithSerialNumber();

    payload.addOptionalVendorData(kOptionalDefaultStringTag, kOptionalDefaultStringValue);
    payload.addOptionalVendorData(kOptionalDefaultIntTag, kOptionalDefaultIntValue);

    return payload;
}

inline std::string toBinaryRepresentation(std::string base38Result)
{
    // Remove the kQRCodePrefix
    base38Result.erase(0, strlen(kQRCodePrefix));

    // Decode the base38 encoded string
    std::vector<uint8_t> buffer;
    base38Decode(base38Result, buffer);

    // Convert it to binary
    std::string binaryResult;
    for (size_t i = buffer.size(); i > 0; i--)
    {
        binaryResult += std::bitset<8>(buffer[i - 1]).to_string();
    }

    // Insert spaces after each block
    size_t pos = binaryResult.size();

    pos -= kVersionFieldLengthInBits;
    binaryResult.insert(pos, " ");

    pos -= kVendorIDFieldLengthInBits;
    binaryResult.insert(pos, " ");

    pos -= kProductIDFieldLengthInBits;
    binaryResult.insert(pos, " ");

    pos -= kCommissioningFlowFieldLengthInBits;
    binaryResult.insert(pos, " ");

    pos -= kRendezvousInfoFieldLengthInBits;
    binaryResult.insert(pos, " ");

    pos -= kPayloadDiscriminatorFieldLengthInBits;
    binaryResult.insert(pos, " ");

    pos -= kSetupPINCodeFieldLengthInBits;
    binaryResult.insert(pos, " ");

    pos -= kPaddingFieldLengthInBits;
    binaryResult.insert(pos, " ");

    return binaryResult;
}

inline bool CompareBinary(SetupPayload & payload, std::string & expectedBinary)
{
    QRCodeSetupPayloadGenerator generator(payload);

    std::string result;
    uint8_t optionalInfo[kDefaultBufferSizeInBytes] = {};
    generator.payloadBase38Representation(result, optionalInfo, sizeof(optionalInfo));

    std::string resultBinary = toBinaryRepresentation(result);
    return (expectedBinary == resultBinary);
}

inline bool CheckWriteRead(SetupPayload & inPayload, bool allowInvalidPayload = false)
{
    SetupPayload outPayload;
    std::string result;

    uint8_t optionalInfo[kDefaultBufferSizeInBytes];
    memset(optionalInfo, 0xFF, sizeof(optionalInfo));
    auto generator = QRCodeSetupPayloadGenerator(inPayload);
    generator.SetAllowInvalidPayload(allowInvalidPayload);
    generator.payloadBase38Representation(result, optionalInfo, sizeof(optionalInfo));

    outPayload = {};
    QRCodeSetupPayloadParser(result).populatePayload(outPayload);

    return inPayload == outPayload;
}

} // namespace chip
