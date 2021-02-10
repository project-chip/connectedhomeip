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
#include <core/CHIPEncoding.h>
#include <core/CHIPSafeCasts.h>
#include <core/CHIPTLV.h>
#include <crypto/CHIPCryptoPAL.h>
#include <stdlib.h>
#include <support/BufferWriter.h>
#include <support/CHIPMem.h>

using namespace chip;
using namespace chip::System;
using namespace chip::TLV;
using namespace chip::Crypto;
using namespace chip::SetupPayloadData;
using namespace chip::Encoding::LittleEndian;

CHIP_ERROR AdditionalDataPayloadGenerator::generateAdditionalDataPayload(uint16_t lifetimeCounter, const char * serialNumberBuffer,
                                                                         size_t serialNumberBufferSize,
                                                                         PacketBufferHandle & bufferHandle,
                                                                         AdditionalDataFields additionalDataFields)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVWriter writer;
    TLVWriter innerWriter;
    char rotatingDeviceIdBuffer[RotatingDeviceId::kRotatingDeviceIdHexMaxLength];
    size_t rotatingDeviceIdBufferSize = 0;

    // Initialize TLVWriter
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::kMaxPacketBufferSize));

    err = writer.OpenContainer(AnonymousTag, kTLVType_Structure, innerWriter);
    SuccessOrExit(err);

    if ((additionalDataFields & AdditionalDataFields::RotatingDeviceId) == AdditionalDataFields::RotatingDeviceId)
    {
        // Generating Device Rotating Id
        err = generateRotatingDeviceId(lifetimeCounter, serialNumberBuffer, serialNumberBufferSize, rotatingDeviceIdBuffer,
                                       ArraySize(rotatingDeviceIdBuffer), rotatingDeviceIdBufferSize);
        SuccessOrExit(err);

        // Adding the rotating device id to the TLV data
        err = innerWriter.PutString(ContextTag(kRotatingDeviceIdTag), rotatingDeviceIdBuffer);
        SuccessOrExit(err);
    }

    err = writer.CloseContainer(innerWriter);
    SuccessOrExit(err);

    err = writer.Finalize(&bufferHandle);
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR AdditionalDataPayloadGenerator::generateRotatingDeviceId(uint16_t lifetimeCounter, const char * serialNumberBuffer,
                                                                    size_t serialNumberBufferSize, char rotatingDeviceIdBuffer[],
                                                                    size_t rotatingDeviceIdBufferSize,
                                                                    size_t & rotatingDeviceIdBufferOutputSize)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    Hash_SHA256_stream hash;
    uint8_t outputBuffer[RotatingDeviceId::kRotatingDeviceIdMaxLength];
    uint8_t hashOutputBuffer[kSHA256_Hash_Length];
    BufferWriter outputBufferWriter(&outputBuffer[0], ArraySize(outputBuffer));
    size_t rotatingDeviceIdBufferIndex = 0;
    const PacketBufferHandle & lifetimeCounterBufferHandle =
        chip::System::PacketBufferHandle::New(chip::System::kMaxPacketBufferSize);
    uint8_t * lifetimeCounterBuffer = lifetimeCounterBufferHandle->Start();

    VerifyOrExit(rotatingDeviceIdBufferSize >= RotatingDeviceId::kRotatingDeviceIdHexMaxLength, err = CHIP_ERROR_BUFFER_TOO_SMALL);

    chip::Encoding::LittleEndian::Write16(lifetimeCounterBuffer, lifetimeCounter);

    // Computing the Rotating Device Id
    // RDI = Lifetime_Counter + SuffixBytes(SHA256(Serial_Number + Lifetime_Counter), 16)

    err = hash.Begin();
    SuccessOrExit(err);

    err = hash.AddData(Uint8::from_const_char(serialNumberBuffer), serialNumberBufferSize);
    SuccessOrExit(err);

    err = hash.AddData(lifetimeCounterBuffer, sizeof(lifetimeCounter));
    SuccessOrExit(err);
    err = hash.Finish(hashOutputBuffer);
    SuccessOrExit(err);

    outputBufferWriter.Put16(lifetimeCounter);
    outputBufferWriter.Put(&hashOutputBuffer[kSHA256_Hash_Length - RotatingDeviceId::kRotatingDeviceIdHashSuffixLength],
                           RotatingDeviceId::kRotatingDeviceIdHashSuffixLength);

    for (; rotatingDeviceIdBufferIndex < outputBufferWriter.Needed(); rotatingDeviceIdBufferIndex++)
    {
        snprintf(&rotatingDeviceIdBuffer[rotatingDeviceIdBufferIndex * 2],
                 rotatingDeviceIdBufferSize - rotatingDeviceIdBufferIndex * 2, "%02X",
                 outputBufferWriter.Buffer()[rotatingDeviceIdBufferIndex]);
    }

    rotatingDeviceIdBuffer[rotatingDeviceIdBufferIndex * 2 + 1] = 0;
    rotatingDeviceIdBufferOutputSize                            = rotatingDeviceIdBufferIndex;
    ChipLogDetail(DeviceLayer, "rotatingDeviceId: %s", rotatingDeviceIdBuffer);

exit:
    return err;
}
