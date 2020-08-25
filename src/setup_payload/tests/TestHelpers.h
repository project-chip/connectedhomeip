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

#include <bitset>

#include "Base41.cpp"
#include "QRCodeSetupPayloadGenerator.cpp"
#include "QRCodeSetupPayloadParser.cpp"
#include "SetupPayload.cpp"

const uint16_t kSmallBufferSizeInBytes   = 1;
const uint16_t kDefaultBufferSizeInBytes = 512;

const uint8_t kOptionalDefaultStringTag  = 2;
const string kOptionalDefaultStringValue = "myData";

const uint8_t kOptionalDefaultIntTag    = 3;
const uint32_t kOptionalDefaultIntValue = 12;

const char * kSerialNumberDefaultStringValue   = "123456789";
const uint32_t kSerialNumberDefaultUInt32Value = 123456789;

SetupPayload GetDefaultPayload()
{
    SetupPayload payload;

    payload.version               = 5;
    payload.vendorID              = 12;
    payload.productID             = 1;
    payload.requiresCustomFlow    = 0;
    payload.rendezvousInformation = RendezvousInformationFlags::kWiFi;
    payload.discriminator         = 128;
    payload.setUpPINCode          = 2048;

    return payload;
}

SetupPayload GetDefaultPayloadWithSerialNumber()
{
    SetupPayload payload = GetDefaultPayload();
    payload.addSerialNumber(kSerialNumberDefaultStringValue);

    return payload;
}

SetupPayload GetDefaultPayloadWithOptionalDefaults()
{
    SetupPayload payload = GetDefaultPayloadWithSerialNumber();

    payload.addOptionalVendorData(kOptionalDefaultStringTag, kOptionalDefaultStringValue);
    payload.addOptionalVendorData(kOptionalDefaultIntTag, kOptionalDefaultIntValue);

    return payload;
}

string toBinaryRepresentation(string base41Result)
{
    // Remove the kQRCodePrefix
    base41Result.erase(0, strlen(kQRCodePrefix));

    // Decode the base41 encoded string
    vector<uint8_t> buffer;
    base41Decode(base41Result, buffer);

    // Convert it to binary
    string binaryResult;
    for (int i = buffer.size() - 1; i >= 0; i--)
    {
        binaryResult += bitset<8>(buffer[i]).to_string();
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

bool CompareBinary(SetupPayload & payload, string & expectedBinary)
{
    QRCodeSetupPayloadGenerator generator(payload);

    string result;
    uint8_t optionalInfo[kDefaultBufferSizeInBytes];
    generator.payloadBase41Representation(result, optionalInfo, sizeof(optionalInfo));

    string resultBinary = toBinaryRepresentation(result);
    return (expectedBinary == resultBinary);
}

bool CompareBinaryLength(SetupPayload & payload, size_t expectedTLVLengthInBytes)
{
    string result;
    uint8_t optionalInfo[kDefaultBufferSizeInBytes];

    SetupPayload basePayload = GetDefaultPayload();
    QRCodeSetupPayloadGenerator baseGenerator(basePayload);
    baseGenerator.payloadBase41Representation(result, optionalInfo, sizeof(optionalInfo));
    string baseBinary = toBinaryRepresentation(result);

    QRCodeSetupPayloadGenerator generator(payload);
    generator.payloadBase41Representation(result, optionalInfo, sizeof(optionalInfo));

    string resultBinary           = toBinaryRepresentation(result);
    size_t resultTLVLengthInBytes = (resultBinary.size() - baseBinary.size()) / 8;
    return (expectedTLVLengthInBytes == resultTLVLengthInBytes);
}

bool CheckWriteRead(SetupPayload & inPayload)
{
    SetupPayload outPayload;
    string result;

    QRCodeSetupPayloadGenerator generator(inPayload);
    uint8_t optionalInfo[kDefaultBufferSizeInBytes];
    generator.payloadBase41Representation(result, optionalInfo, sizeof(optionalInfo));

    QRCodeSetupPayloadParser parser = QRCodeSetupPayloadParser(result);
    parser.populatePayload(outPayload);

    return inPayload == outPayload;
}
