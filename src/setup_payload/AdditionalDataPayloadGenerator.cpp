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

using namespace chip;
using namespace chip::System;
using namespace chip::TLV;
using namespace chip::Crypto;
using namespace chip::SetupPayload;

CHIP_ERROR AdditionalDataPayloadGenerator::generateAdditionalDataPayload(PacketBufferHandle & bufferHandle)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVWriter writer;
    TLVWriter innerWriter;
    char rotatingDeviceIdBuffer[ROTATING_DEVICE_ID_LENGTH + 1];
    uint8_t rotatingDeviceIdBufferSize;

    // Initialize TLVWriter
    writer.Init(PacketBuffer::New());

    err = writer.OpenContainer(AnonymousTag, kTLVType_Structure, innerWriter);
    SuccessOrExit(err);

    // Generating Device Rotating Id
    err = generateRotatingDeviceId(rotatingDeviceIdBuffer, rotatingDeviceIdBufferSize);
    SuccessOrExit(err);

    // Adding the rotating device id to the TLV data
    err = innerWriter.PutString(ContextTag(kRotatingDeviceIdTag), rotatingDeviceIdBuffer);
    SuccessOrExit(err);

    err = writer.CloseContainer(innerWriter);
    SuccessOrExit(err);

    err = writer.Finalize(&bufferHandle);
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR AdditionalDataPayloadGenerator::generateRotatingDeviceId(char * rotatingDeviceIdBuffer,
                                                                    uint8_t & rotatingDeviceIdBufferSize)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    char mRotationCounterStr[ROTATING_DEVICE_ID_COUNTER_STR_LENGTH];
    char hashInput[ROTATING_DEVICE_ID_COUNTER_STR_LENGTH + mSerialNumberBufferSize];
    char output[ROTATING_DEVICE_ID_COUNTER_STR_LENGTH + ROTATING_DEVICE_ID_HASH_SUFFIX_LENGTH];
    uint8_t hashOutputBuffer[kSHA256_Hash_Length];
    uint8_t outputIndex     = 0;
    uint8_t hashOutputIndex = 0;

    sprintf(mRotationCounterStr, "%d", mRotationCounter);
    strcpy(hashInput, mRotationCounterStr);
    strcat(hashInput, mSerialNumberBuffer);

    err = Hash_SHA256(reinterpret_cast<unsigned char *>(const_cast<char *>(hashInput)),
                      ROTATING_DEVICE_ID_COUNTER_STR_LENGTH + mSerialNumberBufferSize, hashOutputBuffer);
    SuccessOrExit(err);

    // Computing the Rotating Device Id
    // RDI = Rotation_Counter + SuffixBytes(HMAC_SHA256(Serial_Number + Rotation_Counter), 16)
    while (outputIndex < ROTATING_DEVICE_ID_COUNTER_STR_LENGTH && mRotationCounterStr[outputIndex] != '\0')
    {
        output[outputIndex] = mRotationCounterStr[outputIndex];
        outputIndex++;
    }

    while (hashOutputIndex < ROTATING_DEVICE_ID_HASH_SUFFIX_LENGTH)
    {
        output[outputIndex++] =
            static_cast<char>(hashOutputBuffer[kSHA256_Hash_Length - ROTATING_DEVICE_ID_HASH_SUFFIX_LENGTH - 1 + hashOutputIndex]);
        hashOutputIndex++;
    }

    memcpy(rotatingDeviceIdBuffer, output, ROTATING_DEVICE_ID_COUNTER_STR_LENGTH + ROTATING_DEVICE_ID_HASH_SUFFIX_LENGTH);
    rotatingDeviceIdBufferSize = ROTATING_DEVICE_ID_COUNTER_STR_LENGTH + ROTATING_DEVICE_ID_HASH_SUFFIX_LENGTH;
    ChipLogDetail(DeviceLayer, "rotatingDeviceId: %s", rotatingDeviceIdBuffer);

exit:
    return err;
}
