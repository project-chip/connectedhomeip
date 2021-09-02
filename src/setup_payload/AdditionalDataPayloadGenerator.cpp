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

#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CHIPMem.h>
#include <stdlib.h>

using namespace chip;
using namespace chip::System;
using namespace chip::TLV;
using namespace chip::Crypto;
using namespace chip::SetupPayloadData;
using namespace chip::Encoding::LittleEndian;

using chip::Encoding::BytesToUppercaseHexString;

CHIP_ERROR
AdditionalDataPayloadGenerator::generateAdditionalDataPayload(uint16_t lifetimeCounter, const char * serialNumberBuffer,
                                                              size_t serialNumberBufferSize, PacketBufferHandle & bufferHandle,
                                                              BitFlags<AdditionalDataFields> additionalDataFields)
{
    System::PacketBufferTLVWriter writer;
    TLVWriter innerWriter;
    char rotatingDeviceIdBuffer[RotatingDeviceId::kHexMaxLength];
    size_t rotatingDeviceIdBufferSize = 0;

    // Initialize TLVWriter
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));

    ReturnErrorOnFailure(writer.OpenContainer(AnonymousTag, kTLVType_Structure, innerWriter));

    if (additionalDataFields.Has(AdditionalDataFields::RotatingDeviceId))
    {
        // Generating Device Rotating Id
        ReturnErrorOnFailure(generateRotatingDeviceId(lifetimeCounter, serialNumberBuffer, serialNumberBufferSize,
                                                      rotatingDeviceIdBuffer, ArraySize(rotatingDeviceIdBuffer),
                                                      rotatingDeviceIdBufferSize));

        // Adding the rotating device id to the TLV data
        ReturnErrorOnFailure(innerWriter.PutString(ContextTag(kRotatingDeviceIdTag), rotatingDeviceIdBuffer,
                                                   static_cast<uint32_t>(rotatingDeviceIdBufferSize)));
    }

    ReturnErrorOnFailure(writer.CloseContainer(innerWriter));

    return writer.Finalize(&bufferHandle);
}

CHIP_ERROR AdditionalDataPayloadGenerator::generateRotatingDeviceId(uint16_t lifetimeCounter, const char * serialNumberBuffer,
                                                                    size_t serialNumberBufferSize, char rotatingDeviceIdBuffer[],
                                                                    size_t rotatingDeviceIdBufferSize,
                                                                    size_t & rotatingDeviceIdValueOutputSize)
{
    uint8_t outputBuffer[RotatingDeviceId::kMaxLength];
    uint8_t hashOutputBuffer[kSHA256_Hash_Length];
    BufferWriter outputBufferWriter(outputBuffer, sizeof(outputBuffer));
    uint8_t lifetimeCounterBuffer[2];

    Put16(lifetimeCounterBuffer, lifetimeCounter);

    VerifyOrReturnError(rotatingDeviceIdBufferSize >= RotatingDeviceId::kHexMaxLength, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Computing the Rotating Device Id
    // RDI = Lifetime_Counter + SuffixBytes(SHA256(Serial_Number + Lifetime_Counter), 16)

    Hash_SHA256_stream hash;
    MutableByteSpan hashOutputSpan(hashOutputBuffer);
    ReturnErrorOnFailure(hash.Begin());
    ReturnErrorOnFailure(hash.AddData(ByteSpan{ Uint8::from_const_char(serialNumberBuffer), serialNumberBufferSize }));
    ReturnErrorOnFailure(hash.AddData(ByteSpan{ lifetimeCounterBuffer, sizeof(lifetimeCounter) }));
    ReturnErrorOnFailure(hash.Finish(hashOutputSpan));

    outputBufferWriter.Put16(lifetimeCounter);
    outputBufferWriter.Put(&hashOutputBuffer[kSHA256_Hash_Length - RotatingDeviceId::kHashSuffixLength],
                           RotatingDeviceId::kHashSuffixLength);

    ReturnErrorOnFailure(
        BytesToUppercaseHexString(outputBuffer, outputBufferWriter.Needed(), rotatingDeviceIdBuffer, rotatingDeviceIdBufferSize));
    rotatingDeviceIdValueOutputSize = outputBufferWriter.Needed() * 2;
    ChipLogDetail(DeviceLayer, "rotatingDeviceId: %s", rotatingDeviceIdBuffer);

    return CHIP_NO_ERROR;
}
