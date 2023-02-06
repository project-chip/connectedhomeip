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
#include <lib/core/TLV.h>
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
AdditionalDataPayloadGenerator::generateAdditionalDataPayload(AdditionalDataPayloadGeneratorParams & params,
                                                              PacketBufferHandle & bufferHandle,
                                                              BitFlags<AdditionalDataFields> additionalDataFields)
{
    System::PacketBufferTLVWriter writer;
    TLVWriter innerWriter;

    // Initialize TLVWriter
    auto tempBuffer = chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize);
    VerifyOrReturnError(!tempBuffer.IsNull(), CHIP_ERROR_NO_MEMORY);
    writer.Init(std::move(tempBuffer));

    ReturnErrorOnFailure(writer.OpenContainer(AnonymousTag(), kTLVType_Structure, innerWriter));

#if CHIP_ENABLE_ROTATING_DEVICE_ID
    if (additionalDataFields.Has(AdditionalDataFields::RotatingDeviceId))
    {
        uint8_t rotatingDeviceIdInternalBuffer[RotatingDeviceId::kMaxLength];
        MutableByteSpan rotatingDeviceIdBuffer(rotatingDeviceIdInternalBuffer);

        // Generating Device Rotating Id
        ReturnErrorOnFailure(generateRotatingDeviceIdAsBinary(params, rotatingDeviceIdBuffer));
        // Adding the rotating device id to the TLV data
        ReturnErrorOnFailure(innerWriter.Put(ContextTag(kRotatingDeviceIdTag), rotatingDeviceIdBuffer));
    }
#endif

    ReturnErrorOnFailure(writer.CloseContainer(innerWriter));

    return writer.Finalize(&bufferHandle);
}

#if CHIP_ENABLE_ROTATING_DEVICE_ID
CHIP_ERROR AdditionalDataPayloadGenerator::generateRotatingDeviceIdAsBinary(AdditionalDataPayloadGeneratorParams & params,
                                                                            MutableByteSpan & rotatingDeviceIdBuffer)
{
    uint8_t hashOutputBuffer[kSHA256_Hash_Length];
    BufferWriter outputBufferWriter(rotatingDeviceIdBuffer);
    uint8_t lifetimeCounterBuffer[2];

    if (params.rotatingDeviceIdUniqueId.data() == nullptr)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    Put16(lifetimeCounterBuffer, params.rotatingDeviceIdLifetimeCounter);

    // Computing the Rotating Device Id
    // RDI = Lifetime_Counter + SuffixBytes(SHA256(Unique_Id + Lifetime_Counter), 16)

    Hash_SHA256_stream hash;
    MutableByteSpan hashOutputSpan(hashOutputBuffer);
    ReturnErrorOnFailure(hash.Begin());
    ReturnErrorOnFailure(hash.AddData(params.rotatingDeviceIdUniqueId));
    ReturnErrorOnFailure(hash.AddData(ByteSpan{ lifetimeCounterBuffer, sizeof(params.rotatingDeviceIdLifetimeCounter) }));
    ReturnErrorOnFailure(hash.Finish(hashOutputSpan));

    outputBufferWriter.Put16(params.rotatingDeviceIdLifetimeCounter);
    outputBufferWriter.Put(&hashOutputBuffer[kSHA256_Hash_Length - RotatingDeviceId::kHashSuffixLength],
                           RotatingDeviceId::kHashSuffixLength);
    VerifyOrReturnError(outputBufferWriter.Fit(), CHIP_ERROR_BUFFER_TOO_SMALL);
    rotatingDeviceIdBuffer.reduce_size(outputBufferWriter.Needed());
    return CHIP_NO_ERROR;
}

CHIP_ERROR AdditionalDataPayloadGenerator::generateRotatingDeviceIdAsHexString(AdditionalDataPayloadGeneratorParams & params,
                                                                               char * rotatingDeviceIdBuffer,
                                                                               size_t rotatingDeviceIdBufferSize,
                                                                               size_t & rotatingDeviceIdValueOutputSize)
{
    uint8_t rotatingDeviceIdInternalBuffer[RotatingDeviceId::kMaxLength];
    MutableByteSpan rotatingDeviceIdBufferTemp(rotatingDeviceIdInternalBuffer);
    ReturnErrorOnFailure(generateRotatingDeviceIdAsBinary(params, rotatingDeviceIdBufferTemp));

    VerifyOrReturnError(rotatingDeviceIdBufferSize >= RotatingDeviceId::kHexMaxLength, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorOnFailure(BytesToUppercaseHexString(rotatingDeviceIdBufferTemp.data(), rotatingDeviceIdBufferTemp.size(),
                                                   rotatingDeviceIdBuffer, rotatingDeviceIdBufferSize));
    rotatingDeviceIdValueOutputSize = rotatingDeviceIdBufferTemp.size() * 2;
    return CHIP_NO_ERROR;
}
#endif
