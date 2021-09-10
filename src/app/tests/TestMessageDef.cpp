/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    All rights reserved.
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
 *      This file implements a test for  CHIP Interaction Model Message Def
 *
 */

#include <app/AppBuildConfig.h>
#include <app/MessageDef/CommandDataElement.h>
#include <app/MessageDef/CommandList.h>
#include <app/MessageDef/InvokeCommand.h>
#include <app/MessageDef/ReadRequest.h>
#include <app/MessageDef/ReportData.h>
#include <app/MessageDef/SubscribeRequest.h>
#include <app/MessageDef/SubscribeResponse.h>
#include <app/MessageDef/TimedRequest.h>
#include <app/MessageDef/WriteRequest.h>
#include <app/MessageDef/WriteResponse.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPTLVDebug.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/UnitTestRegistration.h>
#include <system/TLVPacketBufferBackingStore.h>

#include <nlunit-test.h>

namespace {

using namespace chip::app;

void TLVPrettyPrinter(const char * aFormat, ...)
{
    va_list args;

    va_start(args, aFormat);

    vprintf(aFormat, args);

    va_end(args);
}

CHIP_ERROR DebugPrettyPrint(const chip::System::PacketBufferHandle & aMsgBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVReader reader;
    reader.Init(aMsgBuf.Retain());
    err = reader.Next();
    chip::TLV::Debug::Dump(reader, TLVPrettyPrinter);

    if (CHIP_NO_ERROR != err)
    {
        ChipLogProgress(DataManagement, "DebugPrettyPrint fails with err %" CHIP_ERROR_FORMAT, err.Format());
    }

    return err;
}

void BuildAttributePath(nlTestSuite * apSuite, AttributePath::Builder & aAttributePathBuilder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    aAttributePathBuilder.NodeId(1).EndpointId(2).ClusterId(3).FieldId(4).ListIndex(5).EndOfAttributePath();
    err = aAttributePathBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void ParseAttributePath(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    AttributePath::Parser attributePathParser;
    CHIP_ERROR err              = CHIP_NO_ERROR;
    chip::NodeId nodeId         = 1;
    chip::EndpointId endpointId = 2;
    chip::ClusterId clusterId   = 3;
    chip::AttributeId fieldId   = 4;
    chip::ListIndex listIndex   = 5;

    err = attributePathParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = attributePathParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = attributePathParser.GetNodeId(&nodeId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && nodeId == 1);

    err = attributePathParser.GetEndpointId(&endpointId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && endpointId == 2);

    err = attributePathParser.GetClusterId(&clusterId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && clusterId == 3);

    err = attributePathParser.GetFieldId(&fieldId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && fieldId == 4);

    err = attributePathParser.GetListIndex(&listIndex);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && listIndex == 5);
}

void BuildAttributePathList(nlTestSuite * apSuite, AttributePathList::Builder & aAttributePathListBuilder)
{
    AttributePath::Builder attributePathBuilder = aAttributePathListBuilder.CreateAttributePathBuilder();
    NL_TEST_ASSERT(apSuite, attributePathBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributePath(apSuite, attributePathBuilder);

    aAttributePathListBuilder.EndOfAttributePathList();
    NL_TEST_ASSERT(apSuite, aAttributePathListBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseAttributePathList(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributePathList::Parser attributePathListParser;
    AttributePath::Parser attributePathParser;

    err = attributePathListParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = attributePathListParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
}

void BuildEventPath(nlTestSuite * apSuite, EventPath::Builder & aEventPathBuilder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    aEventPathBuilder.NodeId(1).EndpointId(2).ClusterId(3).EventId(4).EndOfEventPath();
    err = aEventPathBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void ParseEventPath(nlTestSuite * apSuite, EventPath::Parser & aEventPathParser)
{
    CHIP_ERROR err              = CHIP_NO_ERROR;
    chip::NodeId nodeId         = 1;
    chip::EndpointId endpointId = 2;
    chip::ClusterId clusterId   = 3;
    chip::EventId eventId       = 4;

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = aEventPathParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = aEventPathParser.GetNodeId(&nodeId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && nodeId == 1);

    err = aEventPathParser.GetEndpointId(&endpointId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && endpointId == 2);

    err = aEventPathParser.GetClusterId(&clusterId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && clusterId == 3);

    err = aEventPathParser.GetEventId(&eventId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && eventId == 4);
}

void BuildEventPathList(nlTestSuite * apSuite, EventPathList::Builder & aEventPathListBuilder)
{
    EventPath::Builder eventPathBuilder = aEventPathListBuilder.CreateEventPathBuilder();
    NL_TEST_ASSERT(apSuite, eventPathBuilder.GetError() == CHIP_NO_ERROR);
    BuildEventPath(apSuite, eventPathBuilder);

    aEventPathListBuilder.EndOfEventPathList();
    NL_TEST_ASSERT(apSuite, aEventPathListBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseEventPathList(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventPathList::Parser eventPathListParser;

    err = eventPathListParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = eventPathListParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
}

void BuildCommandPath(nlTestSuite * apSuite, CommandPath::Builder & aCommandPathBuilder)
{
    aCommandPathBuilder.EndpointId(1).ClusterId(3).CommandId(4).EndOfCommandPath();
    NL_TEST_ASSERT(apSuite, aCommandPathBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseCommandPath(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandPath::Parser commandPathParser;
    chip::EndpointId endpointId = 0;
    chip::ClusterId clusterId   = 0;
    chip::CommandId commandId   = 0;

    err = commandPathParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = commandPathParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = commandPathParser.GetEndpointId(&endpointId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && endpointId == 1);

    err = commandPathParser.GetClusterId(&clusterId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && clusterId == 3);

    err = commandPathParser.GetCommandId(&commandId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && commandId == 4);
}

void BuildEventDataElement(nlTestSuite * apSuite, EventDataElement::Builder & aEventDataElementBuilder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    EventPath::Builder eventPathBuilder = aEventDataElementBuilder.CreateEventPathBuilder();
    NL_TEST_ASSERT(apSuite, eventPathBuilder.GetError() == CHIP_NO_ERROR);
    BuildEventPath(apSuite, eventPathBuilder);

    aEventDataElementBuilder.PriorityLevel(2)
        .Number(3)
        .UTCTimestamp(4)
        .SystemTimestamp(5)
        .DeltaUTCTimestamp(6)
        .DeltaSystemTimestamp(7);
    err = aEventDataElementBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    // Construct test event data
    {
        chip::TLV::TLVWriter * pWriter = aEventDataElementBuilder.GetWriter();
        chip::TLV::TLVType dummyType   = chip::TLV::kTLVType_NotSpecified;
        err =
            pWriter->StartContainer(chip::TLV::ContextTag(EventDataElement::kCsTag_Data), chip::TLV::kTLVType_Structure, dummyType);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = pWriter->PutBoolean(chip::TLV::ContextTag(1), true);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = pWriter->EndContainer(dummyType);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    }

    aEventDataElementBuilder.EndOfEventDataElement();
}

void ParseEventDataElement(nlTestSuite * apSuite, EventDataElement::Parser & aEventDataElementParser)
{
    CHIP_ERROR err                = CHIP_NO_ERROR;
    uint8_t importanceLevel       = 0;
    uint64_t number               = 0;
    uint64_t uTCTimestamp         = 0;
    uint64_t systemTimestamp      = 0;
    uint64_t deltaUTCTimestamp    = 0;
    uint64_t deltaSystemTimestamp = 0;

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = aEventDataElementParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    {
        {
            EventPath::Parser eventPath;
            err = aEventDataElementParser.GetEventPath(&eventPath);
            NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        }
        err = aEventDataElementParser.GetPriorityLevel(&importanceLevel);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && importanceLevel == 2);
        err = aEventDataElementParser.GetNumber(&number);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && number == 3);
        err = aEventDataElementParser.GetUTCTimestamp(&uTCTimestamp);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && uTCTimestamp == 4);
        err = aEventDataElementParser.GetSystemTimestamp(&systemTimestamp);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && systemTimestamp == 5);
        err = aEventDataElementParser.GetDeltaUTCTimestamp(&deltaUTCTimestamp);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && deltaUTCTimestamp == 6);
        err = aEventDataElementParser.GetDeltaSystemTimestamp(&deltaSystemTimestamp);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && deltaSystemTimestamp == 7);

        {
            chip::TLV::TLVReader reader;
            bool val = false;
            chip::TLV::TLVType container;
            aEventDataElementParser.GetData(&reader);
            err = reader.EnterContainer(container);
            NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

            err = reader.Next();
            NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

            err = reader.Get(val);
            NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && val);

            err = reader.ExitContainer(container);
            NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        }
    }
}

void BuildEventList(nlTestSuite * apSuite, EventList::Builder & aEventListBuilder)
{
    EventDataElement::Builder eventDataElementBuilder = aEventListBuilder.CreateEventBuilder();
    NL_TEST_ASSERT(apSuite, eventDataElementBuilder.GetError() == CHIP_NO_ERROR);
    BuildEventDataElement(apSuite, eventDataElementBuilder);

    aEventListBuilder.EndOfEventList();
    NL_TEST_ASSERT(apSuite, aEventListBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseEventList(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventList::Parser eventListParser;

    err = eventListParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = eventListParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
}

void BuildStatusElement(nlTestSuite * apSuite, StatusElement::Builder & aStatusElementBuilder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    aStatusElementBuilder.EncodeStatusElement(chip::Protocols::SecureChannel::GeneralStatusCode::kFailure, 2, 3)
        .EndOfStatusElement();
    err = aStatusElementBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void ParseStatusElement(nlTestSuite * apSuite, StatusElement::Parser & aStatusElementParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    StatusElement::Parser StatusElementParser;

    chip::Protocols::SecureChannel::GeneralStatusCode generalCode = chip::Protocols::SecureChannel::GeneralStatusCode::kFailure;
    uint32_t protocolId                                           = 0;
    uint16_t protocolCode                                         = 0;

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = aStatusElementParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = aStatusElementParser.DecodeStatusElement(&generalCode, &protocolId, &protocolCode);
    NL_TEST_ASSERT(apSuite,
                   err == CHIP_NO_ERROR &&
                       static_cast<uint16_t>(generalCode) ==
                           static_cast<uint16_t>(chip::Protocols::SecureChannel::GeneralStatusCode::kFailure) &&
                       protocolId == 2 && protocolCode == 3);
}

void BuildAttributeStatusElement(nlTestSuite * apSuite, AttributeStatusElement::Builder & aAttributeStatusElementBuilder)
{
    AttributePath::Builder attributePathBuilder = aAttributeStatusElementBuilder.CreateAttributePathBuilder();
    NL_TEST_ASSERT(apSuite, attributePathBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributePath(apSuite, attributePathBuilder);

    StatusElement::Builder statusElementBuilder = aAttributeStatusElementBuilder.CreateStatusElementBuilder();
    NL_TEST_ASSERT(apSuite, statusElementBuilder.GetError() == CHIP_NO_ERROR);
    BuildStatusElement(apSuite, statusElementBuilder);

    aAttributeStatusElementBuilder.EndOfAttributeStatusElement();
    NL_TEST_ASSERT(apSuite, aAttributeStatusElementBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseAttributeStatusElement(nlTestSuite * apSuite, AttributeStatusElement::Parser & aAttributeStatusElementParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributePath::Parser attributePathParser;
    StatusElement::Parser statusElementParser;

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = aAttributeStatusElementParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = aAttributeStatusElementParser.GetAttributePath(&attributePathParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = aAttributeStatusElementParser.GetStatusElement(&statusElementParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void BuildAttributeStatusList(nlTestSuite * apSuite, AttributeStatusList::Builder & aAttributeStatusListBuilder)
{
    AttributeStatusElement::Builder aAttributeStatusElementBuilder = aAttributeStatusListBuilder.CreateAttributeStatusBuilder();
    NL_TEST_ASSERT(apSuite, aAttributeStatusListBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributeStatusElement(apSuite, aAttributeStatusElementBuilder);

    aAttributeStatusListBuilder.EndOfAttributeStatusList();
    NL_TEST_ASSERT(apSuite, aAttributeStatusListBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseAttributeStatusList(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributeStatusList::Parser attributeStatusParser;

    err = attributeStatusParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = attributeStatusParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
}

void BuildAttributeDataElement(nlTestSuite * apSuite, AttributeDataElement::Builder & aAttributeDataElementBuilder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    AttributePath::Builder attributePathBuilder = aAttributeDataElementBuilder.CreateAttributePathBuilder();
    NL_TEST_ASSERT(apSuite, aAttributeDataElementBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributePath(apSuite, attributePathBuilder);

    aAttributeDataElementBuilder.DataVersion(2);
    err = aAttributeDataElementBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    // Construct attribute data
    {
        chip::TLV::TLVWriter * pWriter = aAttributeDataElementBuilder.GetWriter();
        chip::TLV::TLVType dummyType   = chip::TLV::kTLVType_NotSpecified;
        err = pWriter->StartContainer(chip::TLV::ContextTag(AttributeDataElement::kCsTag_Data), chip::TLV::kTLVType_Structure,
                                      dummyType);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = pWriter->PutBoolean(chip::TLV::ContextTag(1), true);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = pWriter->EndContainer(dummyType);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    }

    aAttributeDataElementBuilder.MoreClusterData(true);
    err = aAttributeDataElementBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    aAttributeDataElementBuilder.EndOfAttributeDataElement();
    NL_TEST_ASSERT(apSuite, aAttributeDataElementBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseAttributeDataElement(nlTestSuite * apSuite, AttributeDataElement::Parser & aAttributeDataElementParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributePath::Parser attributePathParser;
    StatusElement::Parser statusElementParser;
    chip::DataVersion version = 0;
    bool moreClusterDataFlag  = false;
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = aAttributeDataElementParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = aAttributeDataElementParser.GetAttributePath(&attributePathParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = aAttributeDataElementParser.GetDataVersion(&version);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && version == 2);

    {
        chip::TLV::TLVReader reader;
        bool val = false;
        chip::TLV::TLVType container;
        aAttributeDataElementParser.GetData(&reader);
        err = reader.EnterContainer(container);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = reader.Next();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = reader.Get(val);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && val);

        err = reader.ExitContainer(container);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    }

    err = aAttributeDataElementParser.GetMoreClusterDataFlag(&moreClusterDataFlag);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && moreClusterDataFlag);
}

void BuildAttributeDataList(nlTestSuite * apSuite, AttributeDataList::Builder & aAttributeDataListBuilder)
{
    AttributeDataElement::Builder attributeDataElementBuilder = aAttributeDataListBuilder.CreateAttributeDataElementBuilder();
    NL_TEST_ASSERT(apSuite, aAttributeDataListBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributeDataElement(apSuite, attributeDataElementBuilder);

    aAttributeDataListBuilder.EndOfAttributeDataList();
    NL_TEST_ASSERT(apSuite, aAttributeDataListBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseAttributeDataList(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributeDataList::Parser attributeDataListParser;

    err = attributeDataListParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = attributeDataListParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
}

void BuildAttributeDataVersionList(nlTestSuite * apSuite, AttributeDataVersionList::Builder & aAttributeDataVersionListBuilder)
{
    aAttributeDataVersionListBuilder.AddVersion(1);

    aAttributeDataVersionListBuilder.EndOfAttributeDataVersionList();
    NL_TEST_ASSERT(apSuite, aAttributeDataVersionListBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseAttributeDataVersionList(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::DataVersion version;
    AttributeDataVersionList::Parser attributeDataVersionListParser;

    err = attributeDataVersionListParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = attributeDataVersionListParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    attributeDataVersionListParser.GetVersion(&version);
}

void BuildCommandDataElement(nlTestSuite * apSuite, CommandDataElement::Builder & aCommandDataElementBuilder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    CommandPath::Builder commandPathBuilder = aCommandDataElementBuilder.CreateCommandPathBuilder();
    NL_TEST_ASSERT(apSuite, aCommandDataElementBuilder.GetError() == CHIP_NO_ERROR);
    BuildCommandPath(apSuite, commandPathBuilder);

    // Construct command data
    {
        chip::TLV::TLVWriter * pWriter = aCommandDataElementBuilder.GetWriter();
        chip::TLV::TLVType dummyType   = chip::TLV::kTLVType_NotSpecified;
        err = pWriter->StartContainer(chip::TLV::ContextTag(CommandDataElement::kCsTag_Data), chip::TLV::kTLVType_Structure,
                                      dummyType);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = pWriter->PutBoolean(chip::TLV::ContextTag(1), true);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = pWriter->EndContainer(dummyType);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    }

    aCommandDataElementBuilder.EndOfCommandDataElement();
    NL_TEST_ASSERT(apSuite, aCommandDataElementBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseCommandDataElement(nlTestSuite * apSuite, CommandDataElement::Parser & aCommandDataElementParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandPath::Parser commandPathParser;
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = aCommandDataElementParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = aCommandDataElementParser.GetCommandPath(&commandPathParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    {
        chip::TLV::TLVReader reader;
        bool val = false;
        chip::TLV::TLVType container;
        aCommandDataElementParser.GetData(&reader);
        err = reader.EnterContainer(container);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = reader.Next();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = reader.Get(val);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && val);

        err = reader.ExitContainer(container);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    }
}

void BuildCommandDataElementWithStatusCode(nlTestSuite * apSuite, CommandDataElement::Builder & aCommandDataElementBuilder)
{
    CommandPath::Builder commandPathBuilder = aCommandDataElementBuilder.CreateCommandPathBuilder();
    NL_TEST_ASSERT(apSuite, aCommandDataElementBuilder.GetError() == CHIP_NO_ERROR);
    BuildCommandPath(apSuite, commandPathBuilder);

    StatusElement::Builder statusElementBuilder = aCommandDataElementBuilder.CreateStatusElementBuilder();
    NL_TEST_ASSERT(apSuite, statusElementBuilder.GetError() == CHIP_NO_ERROR);
    BuildStatusElement(apSuite, statusElementBuilder);

    aCommandDataElementBuilder.EndOfCommandDataElement();
    NL_TEST_ASSERT(apSuite, aCommandDataElementBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseCommandDataElementWithStatusCode(nlTestSuite * apSuite, CommandDataElement::Parser & aCommandDataElementParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandPath::Parser commandPathParser;
    StatusElement::Parser statusElementParser;
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = aCommandDataElementParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = aCommandDataElementParser.GetCommandPath(&commandPathParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = aCommandDataElementParser.GetStatusElement(&statusElementParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void BuildCommandList(nlTestSuite * apSuite, CommandList::Builder & aCommandListBuilder)
{
    CommandDataElement::Builder commandDataElementBuilder = aCommandListBuilder.CreateCommandDataElementBuilder();
    NL_TEST_ASSERT(apSuite, aCommandListBuilder.GetError() == CHIP_NO_ERROR);
    BuildCommandDataElement(apSuite, commandDataElementBuilder);

    aCommandListBuilder.EndOfCommandList();
    NL_TEST_ASSERT(apSuite, aCommandListBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseCommandList(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandList::Parser commandListParser;
    err = commandListParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = commandListParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
}

void BuildReportData(nlTestSuite * apSuite, chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ReportData::Builder reportDataBuilder;

    err = reportDataBuilder.Init(&aWriter);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    reportDataBuilder.SuppressResponse(true).SubscriptionId(2);
    NL_TEST_ASSERT(apSuite, reportDataBuilder.GetError() == CHIP_NO_ERROR);

    AttributeDataList::Builder attributeDataList = reportDataBuilder.CreateAttributeDataListBuilder();
    NL_TEST_ASSERT(apSuite, reportDataBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributeDataList(apSuite, attributeDataList);

    EventList::Builder eventList = reportDataBuilder.CreateEventDataListBuilder();
    NL_TEST_ASSERT(apSuite, reportDataBuilder.GetError() == CHIP_NO_ERROR);
    BuildEventList(apSuite, eventList);

    reportDataBuilder.MoreChunkedMessages(true);
    NL_TEST_ASSERT(apSuite, reportDataBuilder.GetError() == CHIP_NO_ERROR);

    reportDataBuilder.EndOfReportData();
    NL_TEST_ASSERT(apSuite, reportDataBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseReportData(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ReportData::Parser reportDataParser;

    bool suppressResponse   = false;
    uint64_t subscriptionId = 0;
    AttributeDataList::Parser attributeDataListParser;
    EventList::Parser eventListParser;
    bool moreChunkedMessages = false;
    reportDataParser.Init(aReader);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = reportDataParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = reportDataParser.GetSuppressResponse(&suppressResponse);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && suppressResponse);

    err = reportDataParser.GetSubscriptionId(&subscriptionId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && subscriptionId == 2);

    err = reportDataParser.GetAttributeDataList(&attributeDataListParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = reportDataParser.GetEventDataList(&eventListParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = reportDataParser.GetMoreChunkedMessages(&moreChunkedMessages);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && moreChunkedMessages);
}

void BuildInvokeCommand(nlTestSuite * apSuite, chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    InvokeCommand::Builder invokeCommandBuilder;

    err = invokeCommandBuilder.Init(&aWriter);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    CommandList::Builder commandList = invokeCommandBuilder.CreateCommandListBuilder();
    NL_TEST_ASSERT(apSuite, invokeCommandBuilder.GetError() == CHIP_NO_ERROR);
    BuildCommandList(apSuite, commandList);

    invokeCommandBuilder.EndOfInvokeCommand();
    NL_TEST_ASSERT(apSuite, invokeCommandBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseInvokeCommand(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    InvokeCommand::Parser invokeCommandParser;
    CommandList::Parser commandListParser;

    err = invokeCommandParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = invokeCommandParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = invokeCommandParser.GetCommandList(&commandListParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void BuildReadRequest(nlTestSuite * apSuite, chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ReadRequest::Builder readRequestBuilder;

    err = readRequestBuilder.Init(&aWriter);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    AttributePathList::Builder attributePathList = readRequestBuilder.CreateAttributePathListBuilder();
    NL_TEST_ASSERT(apSuite, readRequestBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributePathList(apSuite, attributePathList);

    EventPathList::Builder eventPathList = readRequestBuilder.CreateEventPathListBuilder();
    NL_TEST_ASSERT(apSuite, readRequestBuilder.GetError() == CHIP_NO_ERROR);
    BuildEventPathList(apSuite, eventPathList);

    AttributeDataVersionList::Builder attributeDataVersionList = readRequestBuilder.CreateAttributeDataVersionListBuilder();
    NL_TEST_ASSERT(apSuite, readRequestBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributeDataVersionList(apSuite, attributeDataVersionList);

    readRequestBuilder.EventNumber(1);
    NL_TEST_ASSERT(apSuite, readRequestBuilder.GetError() == CHIP_NO_ERROR);

    readRequestBuilder.EndOfReadRequest();
    NL_TEST_ASSERT(apSuite, readRequestBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseReadRequest(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ReadRequest::Parser readRequestParser;
    AttributePathList::Parser attributePathListParser;
    EventPathList::Parser eventPathListParser;
    AttributeDataVersionList::Parser attributeDataVersionListParser;
    uint64_t eventNumber;

    err = readRequestParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = readRequestParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = readRequestParser.GetAttributePathList(&attributePathListParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = readRequestParser.GetEventPathList(&eventPathListParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = readRequestParser.GetAttributeDataVersionList(&attributeDataVersionListParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = readRequestParser.GetEventNumber(&eventNumber);
    NL_TEST_ASSERT(apSuite, eventNumber == 1 && err == CHIP_NO_ERROR);
}

void BuildWriteRequest(nlTestSuite * apSuite, chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    WriteRequest::Builder writeRequestBuilder;

    err = writeRequestBuilder.Init(&aWriter);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    writeRequestBuilder.SuppressResponse(true);
    NL_TEST_ASSERT(apSuite, writeRequestBuilder.GetError() == CHIP_NO_ERROR);

    AttributeDataList::Builder attributeDataList = writeRequestBuilder.CreateAttributeDataListBuilder();
    NL_TEST_ASSERT(apSuite, writeRequestBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributeDataList(apSuite, attributeDataList);

    AttributeDataVersionList::Builder attributeDataVersionList = writeRequestBuilder.CreateAttributeDataVersionListBuilder();
    NL_TEST_ASSERT(apSuite, writeRequestBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributeDataVersionList(apSuite, attributeDataVersionList);

    writeRequestBuilder.MoreChunkedMessages(true);
    NL_TEST_ASSERT(apSuite, writeRequestBuilder.GetError() == CHIP_NO_ERROR);

    writeRequestBuilder.EndOfWriteRequest();
    NL_TEST_ASSERT(apSuite, writeRequestBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseWriteRequest(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    WriteRequest::Parser writeRequestParser;
    bool suppressResponse = false;
    AttributeDataList::Parser attributeDataList;
    AttributeDataVersionList::Parser attributeDataVersionList;
    bool moreChunkedMessages = false;

    err = writeRequestParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = writeRequestParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = writeRequestParser.GetSuppressResponse(&suppressResponse);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && suppressResponse);

    err = writeRequestParser.GetAttributeDataList(&attributeDataList);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = writeRequestParser.GetAttributeDataVersionList(&attributeDataVersionList);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = writeRequestParser.GetMoreChunkedMessages(&moreChunkedMessages);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && moreChunkedMessages);
}

void BuildWriteResponse(nlTestSuite * apSuite, chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    WriteResponse::Builder writeResponseBuilder;

    err = writeResponseBuilder.Init(&aWriter);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    AttributeStatusList::Builder attributeStatusList = writeResponseBuilder.CreateAttributeStatusListBuilder();
    NL_TEST_ASSERT(apSuite, writeResponseBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributeStatusList(apSuite, attributeStatusList);

    writeResponseBuilder.EndOfWriteResponse();
    NL_TEST_ASSERT(apSuite, writeResponseBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseWriteResponse(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    WriteResponse::Parser writeResponseParser;
    AttributeStatusList::Parser attributeStatusListParser;
    err = writeResponseParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = writeResponseParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = writeResponseParser.GetAttributeStatusList(&attributeStatusListParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void BuildSubscribeRequest(nlTestSuite * apSuite, chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    SubscribeRequest::Builder subscribeRequestBuilder;

    err = subscribeRequestBuilder.Init(&aWriter);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    AttributePathList::Builder attributePathList = subscribeRequestBuilder.CreateAttributePathListBuilder();
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributePathList(apSuite, attributePathList);

    EventPathList::Builder eventPathList = subscribeRequestBuilder.CreateEventPathListBuilder();
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);
    BuildEventPathList(apSuite, eventPathList);

    AttributeDataVersionList::Builder attributeDataVersionList = subscribeRequestBuilder.CreateAttributeDataVersionListBuilder();
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributeDataVersionList(apSuite, attributeDataVersionList);

    subscribeRequestBuilder.EventNumber(1);
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

    subscribeRequestBuilder.MinIntervalSeconds(2);
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

    subscribeRequestBuilder.MaxIntervalSeconds(3);
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

    subscribeRequestBuilder.KeepExistingSubscriptions(true);
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

    subscribeRequestBuilder.IsProxy(true);
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

    subscribeRequestBuilder.EndOfSubscribeRequest();
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseSubscribeRequest(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    SubscribeRequest::Parser subscribeRequestParser;
    AttributePathList::Parser attributePathListParser;
    EventPathList::Parser eventPathListParser;
    AttributeDataVersionList::Parser attributeDataVersionListParser;
    uint64_t eventNumber          = 0;
    uint16_t minIntervalSeconds   = 0;
    uint16_t maxIntervalSeconds   = 0;
    bool keepExistingSubscription = false;
    bool isProxy                  = false;

    err = subscribeRequestParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = subscribeRequestParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = subscribeRequestParser.GetAttributePathList(&attributePathListParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = subscribeRequestParser.GetEventPathList(&eventPathListParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = subscribeRequestParser.GetAttributeDataVersionList(&attributeDataVersionListParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = subscribeRequestParser.GetEventNumber(&eventNumber);
    NL_TEST_ASSERT(apSuite, eventNumber == 1 && err == CHIP_NO_ERROR);

    err = subscribeRequestParser.GetMinIntervalSeconds(&minIntervalSeconds);
    NL_TEST_ASSERT(apSuite, minIntervalSeconds == 2 && err == CHIP_NO_ERROR);

    err = subscribeRequestParser.GetMaxIntervalSeconds(&maxIntervalSeconds);
    NL_TEST_ASSERT(apSuite, maxIntervalSeconds == 3 && err == CHIP_NO_ERROR);

    err = subscribeRequestParser.GetKeepExistingSubscriptions(&keepExistingSubscription);
    NL_TEST_ASSERT(apSuite, keepExistingSubscription && err == CHIP_NO_ERROR);

    err = subscribeRequestParser.GetIsProxy(&isProxy);
    NL_TEST_ASSERT(apSuite, isProxy && err == CHIP_NO_ERROR);
}

void BuildSubscribeResponse(nlTestSuite * apSuite, chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    SubscribeResponse::Builder subscribeResponseBuilder;

    err = subscribeResponseBuilder.Init(&aWriter);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    subscribeResponseBuilder.SubscriptionId(1);
    NL_TEST_ASSERT(apSuite, subscribeResponseBuilder.GetError() == CHIP_NO_ERROR);

    subscribeResponseBuilder.FinalSyncIntervalSeconds(1);
    NL_TEST_ASSERT(apSuite, subscribeResponseBuilder.GetError() == CHIP_NO_ERROR);

    subscribeResponseBuilder.EndOfSubscribeResponse();
    NL_TEST_ASSERT(apSuite, subscribeResponseBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseSubscribeResponse(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    SubscribeResponse::Parser subscribeResponseParser;
    uint64_t subscriptionId           = 0;
    uint16_t finalSyncIntervalSeconds = 0;

    err = subscribeResponseParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = subscribeResponseParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = subscribeResponseParser.GetSubscriptionId(&subscriptionId);
    NL_TEST_ASSERT(apSuite, subscriptionId == 1 && err == CHIP_NO_ERROR);

    err = subscribeResponseParser.GetFinalSyncIntervalSeconds(&finalSyncIntervalSeconds);
    NL_TEST_ASSERT(apSuite, finalSyncIntervalSeconds == 1 && err == CHIP_NO_ERROR);
}

void BuildTimedRequest(nlTestSuite * apSuite, chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TimedRequest::Builder timedRequestBuilder;

    err = timedRequestBuilder.Init(&aWriter);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    timedRequestBuilder.TimeoutMs(1);
    NL_TEST_ASSERT(apSuite, timedRequestBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseTimedRequest(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    TimedRequest::Parser timedRequestarser;
    uint16_t timeout = 0;

    err = timedRequestarser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = timedRequestarser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = timedRequestarser.GetTimeoutMs(&timeout);
    NL_TEST_ASSERT(apSuite, timeout == 1 && err == CHIP_NO_ERROR);
}

void AttributePathTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributePath::Builder attributePathBuilder;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    attributePathBuilder.Init(&writer);
    BuildAttributePath(apSuite, attributePathBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ParseAttributePath(apSuite, reader);
}

void AttributePathListTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    AttributePathList::Builder attributePathListBuilder;

    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));

    err = attributePathListBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    BuildAttributePathList(apSuite, attributePathListBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseAttributePathList(apSuite, reader);
}

void EventPathTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventPath::Parser eventPathParser;
    EventPath::Builder eventPathBuilder;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    eventPathBuilder.Init(&writer);
    BuildEventPath(apSuite, eventPathBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    eventPathParser.Init(reader);
    ParseEventPath(apSuite, eventPathParser);
}

void EventPathListTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    EventPathList::Builder eventPathListBuilder;

    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));

    err = eventPathListBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    BuildEventPathList(apSuite, eventPathListBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseEventPathList(apSuite, reader);
}

void CommandPathTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    CommandPath::Builder commandPathBuilder;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    err = commandPathBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    BuildCommandPath(apSuite, commandPathBuilder);

    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ParseCommandPath(apSuite, reader);
}

void EventDataElementTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventDataElement::Builder eventDataElementBuilder;
    EventDataElement::Parser eventDataElementParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    eventDataElementBuilder.Init(&writer);
    BuildEventDataElement(apSuite, eventDataElementBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    eventDataElementParser.Init(reader);
    ParseEventDataElement(apSuite, eventDataElementParser);
}

void EventListTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    EventList::Builder eventListBuilder;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    eventListBuilder.Init(&writer);
    BuildEventList(apSuite, eventListBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseEventList(apSuite, reader);
}

void StatusElementTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    StatusElement::Builder statusElementBuilder;
    StatusElement::Parser statusElementParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    statusElementBuilder.Init(&writer);
    BuildStatusElement(apSuite, statusElementBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    statusElementParser.Init(reader);
    ParseStatusElement(apSuite, statusElementParser);
}

void AttributeStatusElementTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributeStatusElement::Builder attributeStatusElementBuilder;
    AttributeStatusElement::Parser attributeStatusElementParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    attributeStatusElementBuilder.Init(&writer);
    BuildAttributeStatusElement(apSuite, attributeStatusElementBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    attributeStatusElementParser.Init(reader);
    ParseAttributeStatusElement(apSuite, attributeStatusElementParser);
}

void AttributeStatusListTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    AttributeStatusList::Builder attributeStatusListBuilder;
    err = attributeStatusListBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    BuildAttributeStatusList(apSuite, attributeStatusListBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseAttributeStatusList(apSuite, reader);
}

void AttributeDataElementTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributeDataElement::Builder attributeDataElementBuilder;
    AttributeDataElement::Parser attributeDataElementParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    attributeDataElementBuilder.Init(&writer);
    BuildAttributeDataElement(apSuite, attributeDataElementBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    attributeDataElementParser.Init(reader);
    ParseAttributeDataElement(apSuite, attributeDataElementParser);
}

void AttributeDataListTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    AttributeDataList::Builder attributeDataListBuilder;
    attributeDataListBuilder.Init(&writer);
    BuildAttributeDataList(apSuite, attributeDataListBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseAttributeDataList(apSuite, reader);
}

void AttributeDataVersionListTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    AttributeDataVersionList::Builder attributeDataVersionListBuilder;
    attributeDataVersionListBuilder.Init(&writer);
    BuildAttributeDataVersionList(apSuite, attributeDataVersionListBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseAttributeDataVersionList(apSuite, reader);
}

void CommandDataElementTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandDataElement::Builder commandDataElementBuilder;
    CommandDataElement::Parser commandDataElementParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    commandDataElementBuilder.Init(&writer);
    BuildCommandDataElement(apSuite, commandDataElementBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    commandDataElementParser.Init(reader);
    ParseCommandDataElement(apSuite, commandDataElementParser);
}

void CommandDataElementWithStatusCodeTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandDataElement::Builder commandDataElementBuilder;
    CommandDataElement::Parser commandDataElementParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    commandDataElementBuilder.Init(&writer);
    BuildCommandDataElementWithStatusCode(apSuite, commandDataElementBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    commandDataElementParser.Init(reader);
    ParseCommandDataElementWithStatusCode(apSuite, commandDataElementParser);
}

void CommandListTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    CommandList::Builder commandListBuilder;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    commandListBuilder.Init(&writer);
    BuildCommandList(apSuite, commandListBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseCommandList(apSuite, reader);
}

void ReportDataTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildReportData(apSuite, writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseReportData(apSuite, reader);
}

void InvokeCommandTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildInvokeCommand(apSuite, writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseInvokeCommand(apSuite, reader);
}

void ReadRequestTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildReadRequest(apSuite, writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseReadRequest(apSuite, reader);
}

void WriteRequestTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildWriteRequest(apSuite, writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseWriteRequest(apSuite, reader);
}

void WriteResponseTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildWriteResponse(apSuite, writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseWriteResponse(apSuite, reader);
}

void SubscribeRequestTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildSubscribeRequest(apSuite, writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseSubscribeRequest(apSuite, reader);
}

void SubscribeResponseTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildSubscribeResponse(apSuite, writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseSubscribeResponse(apSuite, reader);
}

void TimedRequestTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildTimedRequest(apSuite, writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseTimedRequest(apSuite, reader);
}

void CheckPointRollbackTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err        = CHIP_NO_ERROR;
    size_t NumDataElement = 0;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    AttributeDataList::Parser attributeDataListParser;
    chip::TLV::TLVWriter checkpoint;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    AttributeDataList::Builder attributeDataListBuilder;
    attributeDataListBuilder.Init(&writer);

    // encode one attribute element
    AttributeDataElement::Builder attributeDataElementBuilder1 = attributeDataListBuilder.CreateAttributeDataElementBuilder();
    NL_TEST_ASSERT(apSuite, attributeDataListBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributeDataElement(apSuite, attributeDataElementBuilder1);
    // checkpoint
    attributeDataListBuilder.Checkpoint(checkpoint);
    // encode another attribute element
    AttributeDataElement::Builder attributeDataElementBuilder2 = attributeDataListBuilder.CreateAttributeDataElementBuilder();
    NL_TEST_ASSERT(apSuite, attributeDataListBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributeDataElement(apSuite, attributeDataElementBuilder2);
    // rollback to previous checkpoint
    attributeDataListBuilder.Rollback(checkpoint);

    attributeDataListBuilder.EndOfAttributeDataList();
    NL_TEST_ASSERT(apSuite, attributeDataListBuilder.GetError() == CHIP_NO_ERROR);

    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = attributeDataListParser.Init(reader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = attributeDataListParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    while (CHIP_NO_ERROR == (err = attributeDataListParser.Next()))
    {
        ++NumDataElement;
    }

    NL_TEST_ASSERT(apSuite, NumDataElement == 1);
}

/**
 *   Test Suite. It lists all the test functions.
 */

// clang-format off
const nlTest sTests[] =
        {
                NL_TEST_DEF("AttributePathTest", AttributePathTest),
                NL_TEST_DEF("AttributePathListTest", AttributePathListTest),
                NL_TEST_DEF("EventPathTest", EventPathTest),
                NL_TEST_DEF("EventPathListTest", EventPathListTest),
                NL_TEST_DEF("CommandPathTest", CommandPathTest),
                NL_TEST_DEF("EventDataElementTest", EventDataElementTest),
                NL_TEST_DEF("EventListTest", EventListTest),
                NL_TEST_DEF("StatusElementTest", StatusElementTest),
                NL_TEST_DEF("AttributeStatusElementTest", AttributeStatusElementTest),
                NL_TEST_DEF("AttributeStatusListTest", AttributeStatusListTest),
                NL_TEST_DEF("AttributeDataElementTest", AttributeDataElementTest),
                NL_TEST_DEF("AttributeDataListTest", AttributeDataListTest),
                NL_TEST_DEF("AttributeDataVersionListTest", AttributeDataVersionListTest),
                NL_TEST_DEF("CommandDataElementTest", CommandDataElementTest),
                NL_TEST_DEF("CommandDataElementWithStatusCodeTest", CommandDataElementWithStatusCodeTest),
                NL_TEST_DEF("CommandListTest", CommandListTest),
                NL_TEST_DEF("ReportDataTest", ReportDataTest),
                NL_TEST_DEF("InvokeCommandTest", InvokeCommandTest),
                NL_TEST_DEF("ReadRequestTest", ReadRequestTest),
                NL_TEST_DEF("WriteRequestTest", WriteRequestTest),
                NL_TEST_DEF("WriteResponseTest", WriteResponseTest),
                NL_TEST_DEF("SubscribeRequestTest", SubscribeRequestTest),
                NL_TEST_DEF("SubscribeResponseTest", SubscribeResponseTest),
                NL_TEST_DEF("TimedRequestTest", TimedRequestTest),
                NL_TEST_DEF("CheckPointRollbackTest", CheckPointRollbackTest),
                NL_TEST_SENTINEL()
        };
// clang-format on
} // namespace

/**
 *  Set up the test suite.
 */
static int TestSetup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    if (error != CHIP_NO_ERROR)
        return FAILURE;
    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
static int TestTeardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

int TestMessageDef()
{
    // clang-format off
    nlTestSuite theSuite =
	{
        "MessageDef",
        &sTests[0],
        TestSetup,
        TestTeardown,
    };
    // clang-format on

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestMessageDef)
