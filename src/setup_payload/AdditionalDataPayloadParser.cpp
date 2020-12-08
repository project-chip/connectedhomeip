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
        ChipLogProgress(AdditionalDataPayload,
            "AdditonalData - Parsing: octetStringDecode => str:%s, x:%d", str.c_str(), x);
        result.push_back(x);
    }
    return CHIP_NO_ERROR;
}

static CHIP_ERROR openTLVContainer(TLVReader & reader, TLVType type, uint64_t tag, TLVReader & containerReader)
{
    ChipLogProgress(AdditionalDataPayload,
        "AdditonalData - Parsing: openTLVContainer,type=%d, tag=%d, getType()=%d, getTag()=%d, getLength()=%d",
        type, tag, reader.GetType(), reader.GetTag(), reader.GetLength());
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(reader.GetType() == type, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(reader.GetTag() == tag, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(reader.GetLength() == 0, err = CHIP_ERROR_INVALID_ARGUMENT);

    ChipLogProgress(AdditionalDataPayload, "AdditonalData - Parsing: open");
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

    // Generate Dummy payload
    GenerateSamplePayload(payload, payloadLength);
    ChipLogProgress(AdditionalDataPayload, "AdditonalData - Parsing: Generated Dummy Payload, size=%d", payloadLength);
    // Dump the payload TLV structure
    // DebugPrettyPrint(payload, payloadLength);

    // Parse TLV fields
    err = parseTLVFields2(outPayload, payload, payloadLength);
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR AdditionalDataPayloadParser::DecodeInput(uint8_t * tlvDataStart, size_t tlvDataLengthInBytes)
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    vector<uint8_t> buf;
    // TLV parsing
    size_t index = 0;
    size_t bitsLeftToRead;
    size_t tlvBytesLength;
    chip::Platform::ScopedMemoryBuffer<uint8_t> tlvArray;

    ChipLogProgress(AdditionalDataPayload, "AdditonalData - Parsing: Started");
    VerifyOrExit(mPayload.length() != 0, err = CHIP_ERROR_INVALID_ARGUMENT);

    // convert the octet string to buf
    err = octetStringDecode(mPayload, buf);
    SuccessOrExit(err);
    ChipLogProgress(AdditionalDataPayload, "AdditonalData - Parsing: OctetString is decoded, Buf.size:%d", buf.size());

    // convert the buff to TLV buffer
    bitsLeftToRead = (buf.size() * 8) - index;
    tlvBytesLength = (bitsLeftToRead + 7) / 8; // ceil(bitsLeftToRead/8)
    SuccessOrExit(tlvBytesLength == 0);
    tlvArray.Alloc(tlvBytesLength);
    VerifyOrExit(tlvArray, err = CHIP_ERROR_NO_MEMORY);

    for (size_t i = 0; i < tlvBytesLength; i++)
    {
        uint64_t dest = buf[i];
        ChipLogProgress(AdditionalDataPayload, "AdditonalData - Parsing: readBits => Buf:%d, Dest:%d", buf[index], dest);
        tlvArray[i] = static_cast<uint8_t>(dest);
    }
    ChipLogProgress(AdditionalDataPayload, "AdditonalData - Parsing: TLV buffer is created");

    // returning data
    tlvDataStart = tlvArray.Get();
    tlvDataLengthInBytes = tlvBytesLength;

exit:
    return err;
}

CHIP_ERROR AdditionalDataPayloadParser::parseTLVFields2(chip::AdditionalDataPayload & outPayload, uint8_t * tlvDataStart, size_t tlvDataLengthInBytes)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;
    err = reader.Skip();
    SuccessOrExit(err);
    char value[256];
    err = reader.GetString(value, sizeof(value));
    ChipLogProgress(AdditionalDataPayload, "AdditonalData - Parsing: parseTLVFields2, value:%s", value);
exit:
    return err;
}

CHIP_ERROR AdditionalDataPayloadParser::parseTLVFields(chip::AdditionalDataPayload & outPayload, uint8_t * tlvDataStart, size_t tlvDataLengthInBytes)
{
    ChipLogProgress(AdditionalDataPayload, "AdditonalData - Parsing: Start Parsing TLV Fields, Length:%d", tlvDataLengthInBytes);
    CHIP_ERROR err = CHIP_NO_ERROR;
    if (!CanCastTo<uint32_t>(tlvDataLengthInBytes))
    {
        ChipLogProgress(AdditionalDataPayload, "AdditonalData - Parsing: Invalid TLV data length");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ChipLogProgress(AdditionalDataPayload, "AdditonalData - Parsing: Init TLVReader");
    TLVReader rootReader;
    rootReader.Init(tlvDataStart, static_cast<uint32_t>(tlvDataLengthInBytes));
    rootReader.ImplicitProfileId = chip::Protocols::kProtocol_ServiceProvisioning;
    ChipLogProgress(AdditionalDataPayload, "AdditonalData - Parsing: Start Reading TLV");
    err                          = rootReader.Next();
    SuccessOrExit(err);

    ChipLogProgress(AdditionalDataPayload, "AdditonalData - Parsing: Checking TLV structure");
    if (rootReader.GetType() == kTLVType_Structure)
    {
        ChipLogProgress(AdditionalDataPayload, "AdditonalData - Parsing: Stucture is TLV");
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
        ChipLogProgress(AdditionalDataPayload, "AdditonalData - Parsing: Stucture is NOT TLV");
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

CHIP_ERROR AdditionalDataPayloadParser::GenerateSamplePayload(uint8_t * output, size_t & tlvDataLengthInBytes)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLVWriter rootWriter;
    chip::System::PacketBufferHandle bufferHandle = chip::System::PacketBuffer::New();
    chip::System::PacketBuffer * buffer = bufferHandle.Get_ForNow();

    char testRotatingDeviceId[] = "1122334455667788";
    rootWriter.Init(buffer);
    rootWriter.ImplicitProfileId = chip::Protocols::kProtocol_ServiceProvisioning;

    TLVWriter innerStructureWriter;
    err = rootWriter.OpenContainer(ProfileTag(rootWriter.ImplicitProfileId, kTag_AdditionalDataExensionDescriptor), kTLVType_Structure,
                                       innerStructureWriter);
    SuccessOrExit(err);
    err = innerStructureWriter.PutString(CommonTag(kRotatingDeviceIdTag), testRotatingDeviceId);
    SuccessOrExit(err);

    err = rootWriter.CloseContainer(innerStructureWriter);
    SuccessOrExit(err);

    err = rootWriter.Finalize();
    SuccessOrExit(err);

    output = buffer->Start();
    tlvDataLengthInBytes = rootWriter.GetLengthWritten();
    for(size_t i=0; i<tlvDataLengthInBytes; i++)
    {
        ChipLogProgress(DataManagement, "GenerateSamplePayload[%d] = %d", i, output[i]);
    }
exit:
    return err;
}
