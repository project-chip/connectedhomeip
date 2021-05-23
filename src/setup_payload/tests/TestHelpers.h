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

#include "Base41.h"
#include "QRCodeSetupPayloadGenerator.h"
#include "QRCodeSetupPayloadParser.h"
#include "SetupPayload.h"

namespace chip {

static constexpr uint16_t kSmallBufferSizeInBytes   = 1;
static constexpr uint16_t kDefaultBufferSizeInBytes = 512;

static constexpr uint8_t kOptionalDefaultStringTag  = 2;
static constexpr char kOptionalDefaultStringValue[] = "myData";

static constexpr uint8_t kOptionalDefaultIntTag    = 3;
static constexpr uint32_t kOptionalDefaultIntValue = 12;

static constexpr char kSerialNumberDefaultStringValue[]   = "123456789";
static constexpr uint32_t kSerialNumberDefaultUInt32Value = 123456789;

inline SetupPayload GetDefaultPayload()
{
    SetupPayload payload;

    payload.version               = 5;
    payload.vendorID              = 12;
    payload.productID             = 1;
    payload.requiresCustomFlow    = 0;
    payload.rendezvousInformation = RendezvousInformationFlags(RendezvousInformationFlag::kSoftAP);
    payload.discriminator         = 128;
    payload.setUpPINCode          = 2048;

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

inline std::string toBinaryRepresentation(std::string base41Result)
{
    // Remove the kQRCodePrefix
    base41Result.erase(0, strlen(kQRCodePrefix));

    // Decode the base41 encoded string
    std::vector<uint8_t> buffer;
    base41Decode(base41Result, buffer);

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

    pos -= kCustomFlowRequiredFieldLengthInBits;
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
    uint8_t optionalInfo[kDefaultBufferSizeInBytes];
    generator.payloadBase41Representation(result, optionalInfo, sizeof(optionalInfo));

    std::string resultBinary = toBinaryRepresentation(result);
    return (expectedBinary == resultBinary);
}

inline bool CheckWriteRead(SetupPayload & inPayload)
{
    SetupPayload outPayload;
    std::string result;

    QRCodeSetupPayloadGenerator generator(inPayload);
    uint8_t optionalInfo[kDefaultBufferSizeInBytes];
    generator.payloadBase41Representation(result, optionalInfo, sizeof(optionalInfo));

    QRCodeSetupPayloadParser parser = QRCodeSetupPayloadParser(result);
    parser.populatePayload(outPayload);

    return inPayload == outPayload;
}

} // namespace chip
