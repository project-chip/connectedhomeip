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
#include <stdlib.h>
#include <support/BufferWriter.h>
#include <support/CHIPMem.h>

using namespace chip;
using namespace chip::System;
using namespace chip::TLV;
using namespace chip::Crypto;
using namespace chip::SetupPayload;
using namespace chip::Encoding::LittleEndian;

static uint8_t * sHashInput;

AdditionalDataPayloadGenerator::AdditionalDataPayloadGenerator(uint16_t lifetimeCounter, char * serialNumberBuffer,
                                                               size_t serialNumberBufferSize) :
    mLifetimeCounter(lifetimeCounter),
    mSerialNumberBuffer(serialNumberBuffer), mSerialNumberBufferSize(serialNumberBufferSize)
{
    sHashInput =
        reinterpret_cast<uint8_t *>(chip::Platform::MemoryAlloc(ROTATING_DEVICE_ID_COUNTER_STR_LENGTH + serialNumberBufferSize));
}

AdditionalDataPayloadGenerator::~AdditionalDataPayloadGenerator()
{
    if (sHashInput != nullptr)
    {
        chip::Platform::MemoryFree(sHashInput);
        sHashInput = nullptr;
    }
}

CHIP_ERROR AdditionalDataPayloadGenerator::generateAdditionalDataPayload(PacketBufferHandle & bufferHandle,
                                                                         bool enableRotatingDeviceId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVWriter writer;
    TLVWriter innerWriter;
    char rotatingDeviceIdBuffer[ROTATING_DEVICE_ID_LENGTH * 2 + 1];
    size_t rotatingDeviceIdBufferSize = 0;

    // Initialize TLVWriter
    writer.Init(PacketBuffer::New());

    err = writer.OpenContainer(AnonymousTag, kTLVType_Structure, innerWriter);
    SuccessOrExit(err);

    if (enableRotatingDeviceId)
    {
        // Generating Device Rotating Id
        err = generateRotatingDeviceId(rotatingDeviceIdBuffer, ROTATING_DEVICE_ID_LENGTH * 2 + 1, rotatingDeviceIdBufferSize);
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

CHIP_ERROR AdditionalDataPayloadGenerator::generateRotatingDeviceId(char rotatingDeviceIdBuffer[],
                                                                    size_t rotatingDeviceIdBufferSize,
                                                                    size_t & rotatingDeviceIdBufferOutputSize)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    char lifetimeCounterBuf[ROTATING_DEVICE_ID_COUNTER_STR_LENGTH];

    uint8_t outputBuffer[ROTATING_DEVICE_ID_LENGTH];
    uint8_t hashOutputBuffer[kSHA256_Hash_Length];
    BufferWriter hashInputBufferWriter(sHashInput, ROTATING_DEVICE_ID_COUNTER_STR_LENGTH + mSerialNumberBufferSize);
    BufferWriter outputBuferWriter(&outputBuffer[0], ArraySize(outputBuffer));

    VerifyOrExit(rotatingDeviceIdBufferSize >= ROTATING_DEVICE_ID_LENGTH * 2 + 1, err = CHIP_ERROR_BUFFER_TOO_SMALL);

    snprintf(lifetimeCounterBuf, sizeof(lifetimeCounterBuf), "%09u", mLifetimeCounter);
    hashInputBufferWriter.Put(lifetimeCounterBuf, ROTATING_DEVICE_ID_COUNTER_STR_LENGTH);
    hashInputBufferWriter.Put(mSerialNumberBuffer, mSerialNumberBufferSize);

    err = Hash_SHA256(hashInputBufferWriter.Buffer(), ROTATING_DEVICE_ID_COUNTER_STR_LENGTH + mSerialNumberBufferSize,
                      hashOutputBuffer);
    SuccessOrExit(err);

    // Computing the Rotating Device Id
    // RDI = Rotation_Counter + SuffixBytes(HMAC_SHA256(Serial_Number + Rotation_Counter), 16)
    outputBuferWriter.Put(lifetimeCounterBuf);
    outputBuferWriter.Put(
        reinterpret_cast<const char *>(&hashOutputBuffer[kSHA256_Hash_Length - ROTATING_DEVICE_ID_HASH_SUFFIX_LENGTH]));

    for (size_t i = 0; i < ROTATING_DEVICE_ID_LENGTH; i++)
    {
        snprintf(&rotatingDeviceIdBuffer[i * 2], 3, "%02X", outputBuferWriter.Buffer()[i]);
    }

    rotatingDeviceIdBuffer[ROTATING_DEVICE_ID_LENGTH * 2] = 0;
    rotatingDeviceIdBufferOutputSize                      = ROTATING_DEVICE_ID_LENGTH * 2 + 1;
    ChipLogDetail(DeviceLayer, "rotatingDeviceId: %s", rotatingDeviceIdBuffer);

exit:
    return err;
}
