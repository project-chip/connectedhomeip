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

/**
 *    @file
 *      This file provides a utility to generate Additional Data payload and its members
 *      (e.g. rotating device id)
 *
 */

#include "AdditionalDataPayloadGenerator.h"
#include "AdditionalDataPayload.h"

#include <core/CHIPCore.h>
#include <core/CHIPTLV.h>
#include <crypto/CHIPCryptoPAL.h>
#include <sstream>
#include <stdlib.h>

using namespace chip;
using namespace chip::TLV;
using namespace chip::Crypto;
using namespace chip::SetupPayload;

CHIP_ERROR AdditionalDataPayloadGenerator::generateAdditionalDataPayload(uint16_t rotationCounter, std::string serialNumber,
                                                                         chip::System::PacketBuffer * buffer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLVWriter writer;
    TLVWriter innerWriter;
    std::string rotatingDeviceId;

    // Initialize TLVWriter
    writer.Init(buffer);

    err = writer.OpenContainer(AnonymousTag, kTLVType_Structure, innerWriter);
    SuccessOrExit(err);

    // Generating Device Rotating Id
    err = generateRotatingDeviceId(rotationCounter, serialNumber, rotatingDeviceId);
    SuccessOrExit(err);

    // Adding the rotating device id to the TLV data
    err = innerWriter.PutString(ContextTag(kRotatingDeviceIdTag), rotatingDeviceId.c_str());
    SuccessOrExit(err);

    err = writer.CloseContainer(innerWriter);
    SuccessOrExit(err);

    writer.Finalize();

exit:
    return err;
}

CHIP_ERROR AdditionalDataPayloadGenerator::generateRotatingDeviceId(uint16_t rotationCounter, std::string serialNumber,
                                                                    std::string & rotatingDeviceId)
{
    unsigned char * hashInput;
    uint8_t hashOutputBuffer[kSHA256_Hash_Length];
    std::ostringstream hashOutputStream;
    std::string hashOutput;
    std::string rotationCounterStr = std::to_string(rotationCounter);

    hashInput = reinterpret_cast<unsigned char *>(const_cast<char *>((serialNumber + rotationCounterStr).c_str()));
    Hash_SHA256(hashInput, serialNumber.size() + rotationCounterStr.size(), hashOutputBuffer);

    for (size_t i = 0; i < kSHA256_Hash_Length; i++)
    {
        hashOutputStream << std::uppercase << std::hex << (int) hashOutputBuffer[i];
    }

    // Computing the Rotating Device Id
    // RDI = Rotation_Counter + SuffixBytes(HMAC_SHA256(Serial_Number + Rotation_Counter), 16)
    hashOutput = hashOutputStream.str();
    if (hashOutput.size() > 16)
    {
        hashOutput = hashOutput.substr(hashOutput.size() - 16, 16);
    }
    rotatingDeviceId = rotationCounterStr + hashOutput;
    ChipLogDetail(DeviceLayer, "rotatingDeviceId: %s", rotatingDeviceId.c_str());

    return CHIP_NO_ERROR;
}
