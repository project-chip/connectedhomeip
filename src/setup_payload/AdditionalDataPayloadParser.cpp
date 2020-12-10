/**
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
 *      This file describes a AdditionalData Payload parser based on the
 *      CHIP specification.
 */

#include "AdditionalDataPayloadParser.h"
#include "Base41.h"

#include <math.h>
#include <memory>
#include <string.h>
#include <vector>
#include <sstream>
#include <iomanip>


#include <core/CHIPCore.h>
#include <core/CHIPError.h>
#include <core/CHIPTLVData.hpp>
#include <core/CHIPTLVUtilities.hpp>
#include <protocols/Protocols.h>
#include <support/CodeUtils.h>
#include <support/RandUtils.h>
#include <support/SafeInt.h>
#include <support/ScopedBuffer.h>
#include <core/CHIPTLVDebug.hpp>
#include <iomanip>
#include <cstdlib>
#include <iostream>

using namespace chip;
using namespace std;
using namespace chip::TLV;
using namespace chip::TLV::Utilities;

static CHIP_ERROR octetStringDecode(string octetString, vector<uint8_t> & result) {
    result.clear();
    size_t len = octetString.length();

    for(size_t i = 0; i < len; i += 2) {
        auto str = octetString.substr(i, 2);
        uint8_t x = (uint8_t)stoi(str, 0, 16);
        result.push_back(x);
    }
    return CHIP_NO_ERROR;
}

static CHIP_ERROR openTLVContainer(TLVReader & reader, TLVType type, uint64_t tag, TLVReader & containerReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(reader.GetType() == type, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(reader.GetTag() == tag, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(reader.GetLength() == 0, err = CHIP_ERROR_INVALID_ARGUMENT);

    err = reader.OpenContainer(containerReader);
    SuccessOrExit(err);

    VerifyOrExit(containerReader.GetContainerType() == type, err = CHIP_ERROR_INVALID_ARGUMENT);
exit:
    return err;
}

CHIP_ERROR AdditionalDataPayloadParser::populatePayload(AdditionalDataPayload & outPayload)
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    uint8_t * payload = nullptr;
    size_t payloadLength = 0;

    // Decoding input
    err = DecodeInput(&payload, payloadLength);

    // Generate Dummy payload
    //err = GenerateSamplePayload(&payload, payloadLength);
    // Dump the payload TLV structure
    // DebugPrettyPrint(payload, payloadLength);

    //std::cout <<"Dummy:" << payloadLength << ":";
    for(uint8_t i=0; i<payloadLength; i++)
    {
        //std::cout << std::setfill('0') << std::setw(2) << std::hex << (0xff & (unsigned int)payload[i]);
    }
    //std::cout << std::endl;

    SuccessOrExit(err);
    // Parse TLV fields
    err = parseTLVFields2(outPayload, payload, payloadLength);
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR AdditionalDataPayloadParser::DecodeInput(uint8_t ** output, size_t & tlvDataLengthInBytes)
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    vector<uint8_t> buf;
    // TLV parsing
    size_t index = 0;
    size_t bitsLeftToRead;
    size_t tlvBytesLength;
    chip::Platform::ScopedMemoryBuffer<uint8_t> tlvArray;

    VerifyOrExit(mPayload.length() != 0, err = CHIP_ERROR_INVALID_ARGUMENT);

    // convert the octet string to buf
    err = octetStringDecode(mPayload, buf);
    SuccessOrExit(err);

    // convert the buff to TLV buffer
    bitsLeftToRead = (buf.size() * 8) - index;
    tlvBytesLength = (bitsLeftToRead + 7) / 8; // ceil(bitsLeftToRead/8)
    SuccessOrExit(tlvBytesLength == 0);
    tlvArray.Alloc(tlvBytesLength);
    VerifyOrExit(tlvArray, err = CHIP_ERROR_NO_MEMORY);

    *output = new uint8_t[tlvBytesLength];
    for (size_t i = 0; i < tlvBytesLength; i++)
    {
        uint64_t dest = buf[i];
        (*output)[i] = static_cast<uint8_t>(dest);
        tlvArray[i] = static_cast<uint8_t>(dest);
    }

    // returning data
    //*output = tlvArray.Get();
    tlvDataLengthInBytes = tlvBytesLength;

exit:
    return err;
}

CHIP_ERROR AdditionalDataPayloadParser::parseTLVFields2(chip::AdditionalDataPayload & outPayload, uint8_t * tlvDataStart, size_t tlvDataLengthInBytes)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;
    chip::TLV::TLVReader innerReader;
    // Outter Reader
    for(size_t i=0; i< tlvDataLengthInBytes; i++)
    {
    }
    reader.Init(tlvDataStart, (uint32_t)tlvDataLengthInBytes);
    reader.ImplicitProfileId = chip::Protocols::kProtocol_ServiceProvisioning;
    err = reader.Next();
    SuccessOrExit(err);

    // Open the container
    err = reader.OpenContainer(innerReader);
    SuccessOrExit(err);

    err = innerReader.Next();
    SuccessOrExit(err);

    // Get the value of the rotating device id
    char rotatingDeviceId[256];
    err = innerReader.GetString(rotatingDeviceId, sizeof(rotatingDeviceId)+1);
    ChipLogProgress(AdditionalDataPayload, "AdditonalData - Parsing: parseTLVFields, rotatingDeviceId:%s", rotatingDeviceId);
exit:
    return err;
}

CHIP_ERROR AdditionalDataPayloadParser::parseTLVFields(chip::AdditionalDataPayload & outPayload, uint8_t * tlvDataStart, size_t tlvDataLengthInBytes)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    if (!CanCastTo<uint32_t>(tlvDataLengthInBytes))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    TLVReader rootReader;
    rootReader.Init(tlvDataStart, static_cast<uint32_t>(tlvDataLengthInBytes));
    rootReader.ImplicitProfileId = chip::Protocols::kProtocol_ServiceProvisioning;
    err                          = rootReader.Next();
    SuccessOrExit(err);

    if (rootReader.GetType() == kTLVType_Structure)
    {
        TLVReader innerStructureReader;
        err = openTLVContainer(rootReader, kTLVType_Structure,
                               ProfileTag(rootReader.ImplicitProfileId, kTag_AdditionalDataExensionDescriptor),
                               innerStructureReader);
        SuccessOrExit(err);
        err = innerStructureReader.Next();
        SuccessOrExit(err);
        //err = retrieveOptionalInfos(outPayload, innerStructureReader);
    }
    else
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
exit:
    return err;
}

static void TLVPrettyPrinter(const char * aFormat, ...)
{
    va_list args;

    va_start(args, aFormat);

    vprintf(aFormat, args);

    va_end(args);
}

CHIP_ERROR AdditionalDataPayloadParser::DebugPrettyPrint(uint8_t * input, size_t & tlvDataLengthInBytes)
{
    chip::System::PacketBufferHandle bufferHandle = chip::System::PacketBuffer::New();
    chip::System::PacketBuffer * buffer = bufferHandle.Get_ForNow();

    buffer->SetStart(input);
    buffer->SetDataLength((uint16_t)tlvDataLengthInBytes);
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;
    reader.Init(buffer);
    err = reader.Next();
     ChipLogProgress(DataManagement, "DebugPrettyPrint Start:");
    chip::TLV::Debug::Dump(reader, TLVPrettyPrinter);
    ChipLogProgress(DataManagement, "DebugPrettyPrint End");

    if (CHIP_NO_ERROR != err)
    {
        ChipLogProgress(DataManagement, "DebugPrettyPrint fails with err %d", err);
    }

    return err;
}

CHIP_ERROR AdditionalDataPayloadParser::GenerateSamplePayload(uint8_t ** output, size_t & tlvDataLengthInBytes)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLVWriter rootWriter;
    chip::System::PacketBufferHandle bufferHandle = chip::System::PacketBuffer::New();
    chip::System::PacketBuffer * buffer = bufferHandle.Get_ForNow();

    char testRotatingDeviceId[] = "1122334455667788";
    rootWriter.Init(buffer);
    rootWriter.ImplicitProfileId = chip::Protocols::kProtocol_ServiceProvisioning;

    TLVWriter innerStructureWriter;
    err = rootWriter.OpenContainer(ProfileTag(rootWriter.ImplicitProfileId, kTag_AdditionalDataExensionDescriptor),
        kTLVType_Structure,
        innerStructureWriter);

    SuccessOrExit(err);
    err = innerStructureWriter.PutString(ProfileTag(rootWriter.ImplicitProfileId, kRotatingDeviceIdTag),
        testRotatingDeviceId);
    SuccessOrExit(err);

    err = rootWriter.CloseContainer(innerStructureWriter);
    SuccessOrExit(err);

    err = rootWriter.Finalize();
    SuccessOrExit(err);

    *output = buffer->Start();
    tlvDataLengthInBytes = rootWriter.GetLengthWritten();
    //std::cout <<"Dummy:" << tlvDataLengthInBytes << ":";
    for(size_t i=0; i<tlvDataLengthInBytes; i++)
    {
        //ChipLogProgress(DataManagement, "GenerateSamplePayload[%d] = %d", i, output[i]);
        //std::cout << std::hex << buffer->Start()[i];
    }
    //std::cout << std::endl;
exit:
    return err;
}
