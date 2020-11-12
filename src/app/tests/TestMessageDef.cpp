/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "TestApp.h"

#include <app/MessageDef.h>
#include <core/CHIPTLVDebug.hpp>
#include <support/TestUtils.h>
#include <system/SystemPacketBuffer.h>

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

CHIP_ERROR DebugPrettyPrint(chip::System::PacketBuffer * apMsgBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;
    reader.Init(apMsgBuf);
    err = reader.Next();
    chip::TLV::Debug::Dump(reader, TLVPrettyPrinter);

    if (CHIP_NO_ERROR != err)
    {
        ChipLogProgress(DataManagement, "DebugPrettyPrint fails with err %d", err);
    }

    return err;
}

void BuildAttributePath(nlTestSuite * apSuite, AttributePath::Builder & aAttributePathBuilder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    aAttributePathBuilder.NodeId(1).EndpointId(2).ClusterId(3).FieldTag(4).ListIndex(5).EndOfAttributePath();
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
    uint8_t fieldTag            = 4;
    uint16_t listIndex          = 5;

    err = attributePathParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = attributePathParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = attributePathParser.GetNodeId(&nodeId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && nodeId == 1);

    err = attributePathParser.GetEndpointId(&endpointId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && endpointId == 2);

    err = attributePathParser.GetClusterId(&clusterId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && clusterId == 3);

    err = attributePathParser.GetFieldTag(&fieldTag);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && fieldTag == 4);

    err = attributePathParser.GetListIndex(&listIndex);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && listIndex == 5);
}

void BuildAttributePathList(nlTestSuite * apSuite, chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributePathList::Builder attributePathListBuilder;

    err = attributePathListBuilder.Init(&aWriter);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    {
        AttributePath::Builder attributePathBuilder = attributePathListBuilder.CreateAttributePathBuilder();
        NL_TEST_ASSERT(apSuite, attributePathBuilder.GetError() == CHIP_NO_ERROR);
        BuildAttributePath(apSuite, attributePathBuilder);
    }

    attributePathListBuilder.EndOfAttributePathList();
    NL_TEST_ASSERT(apSuite, attributePathListBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseAttributePathList(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributePathList::Parser attributePathListParser;
    AttributePath::Parser attributePathParser;

    err = attributePathListParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = attributePathListParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
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

    err = aEventPathParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = aEventPathParser.GetNodeId(&nodeId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && nodeId == 1);

    err = aEventPathParser.GetEndpointId(&endpointId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && endpointId == 2);

    err = aEventPathParser.GetClusterId(&clusterId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && clusterId == 3);

    err = aEventPathParser.GetEventId(&eventId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && eventId == 4);
}

void BuildEventPathList(nlTestSuite * apSuite, chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventPathList::Builder eventPathListBuilder;

    err = eventPathListBuilder.Init(&aWriter);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    {
        EventPath::Builder eventPathBuilder = eventPathListBuilder.CreateEventPathBuilder();
        NL_TEST_ASSERT(apSuite, eventPathBuilder.GetError() == CHIP_NO_ERROR);
        BuildEventPath(apSuite, eventPathBuilder);
    }

    eventPathListBuilder.EndOfEventPathList();
    NL_TEST_ASSERT(apSuite, eventPathListBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseEventPathList(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventPathList::Parser eventPathListParser;

    err = eventPathListParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = eventPathListParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void BuildCommandPath(nlTestSuite * apSuite, chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandPath::Builder commandPathBuilder;
    commandPathBuilder.Init(&aWriter);
    commandPathBuilder.EndpointId(1).GroupId(2).ClusterId(3).CommandId(4).EndOfCommandPath();
    err = commandPathBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void ParseCommandPath(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandPath::Parser commandPathParser;
    chip::EndpointId endpointId = 0;
    chip::GroupId groupId       = 0;
    chip::ClusterId clusterId   = 0;
    chip::CommandId commandId   = 0;

    err = commandPathParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = commandPathParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = commandPathParser.GetEndpointId(&endpointId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && endpointId == 1);

    err = commandPathParser.GetGroupId(&groupId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && groupId == 2);

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

    aEventDataElementBuilder.ImportanceLevel(2).Number(3).UTCTimestamp(4).SystemTimestamp(5).DeltaUTCTime(6).DeltaSystemTime(7);
    err = aEventDataElementBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    // Contruct test event data
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
    CHIP_ERROR err           = CHIP_NO_ERROR;
    uint8_t importanceLevel  = 0;
    uint64_t number          = 0;
    uint64_t uTCTimestamp    = 0;
    uint64_t systemTimestamp = 0;
    uint64_t deltaUTCTime    = 0;
    uint64_t deltaSystemTime = 0;

    err = aEventDataElementParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    {
        {
            EventPath::Parser eventPath;
            err = aEventDataElementParser.GetEventPath(&eventPath);
            NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        }
        err = aEventDataElementParser.GetImportanceLevel(&importanceLevel);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && importanceLevel == 2);
        err = aEventDataElementParser.GetNumber(&number);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && number == 3);
        err = aEventDataElementParser.GetUTCTimestamp(&uTCTimestamp);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && uTCTimestamp == 4);
        err = aEventDataElementParser.GetSystemTimestamp(&systemTimestamp);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && systemTimestamp == 5);
        err = aEventDataElementParser.GetDeltaUTCTime(&deltaUTCTime);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && deltaUTCTime == 6);
        err = aEventDataElementParser.GetDeltaSystemTime(&deltaSystemTime);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && deltaSystemTime == 7);

        {
            chip::TLV::TLVReader reader;
            bool val = false;
            chip::TLV::TLVType container;
            aEventDataElementParser.GetReaderOnEvent(&reader);
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

void BuildEventList(nlTestSuite * apSuite, chip::TLV::TLVWriter & aTLVWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventList::Builder eventListBuilder;

    err = eventListBuilder.Init(&aTLVWriter);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    {
        EventDataElement::Builder eventDataElementBuilder = eventListBuilder.CreateEventBuilder();
        NL_TEST_ASSERT(apSuite, eventDataElementBuilder.GetError() == CHIP_NO_ERROR);
        BuildEventDataElement(apSuite, eventDataElementBuilder);
    }

    eventListBuilder.EndOfEventList();
    NL_TEST_ASSERT(apSuite, eventListBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseEventList(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventList::Parser eventListParser;

    err = eventListParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = eventListParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void AttributePathTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributePath::Builder attributePathBuilder;
    chip::TLV::TLVWriter writer;
    chip::TLV::TLVReader reader;
    chip::System::PacketBuffer * buf = chip::System::PacketBuffer::New();

    writer.Init(buf);
    attributePathBuilder.Init(&writer);
    BuildAttributePath(apSuite, attributePathBuilder);
    err = writer.Finalize();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(buf);
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ParseAttributePath(apSuite, reader);

    chip::System::PacketBuffer::Free(buf);
}

void AttributePathListTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVWriter writer;
    chip::TLV::TLVReader reader;
    chip::System::PacketBuffer * buf = chip::System::PacketBuffer::New();

    writer.Init(buf);
    BuildAttributePathList(apSuite, writer);
    err = writer.Finalize();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(buf);
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseAttributePathList(apSuite, reader);

    chip::System::PacketBuffer::Free(buf);
}

void EventPathTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventPath::Parser eventPathParser;
    EventPath::Builder eventPathBuilder;
    chip::TLV::TLVWriter writer;
    chip::TLV::TLVReader reader;
    chip::System::PacketBuffer * buf = chip::System::PacketBuffer::New();

    writer.Init(buf);
    eventPathBuilder.Init(&writer);
    BuildEventPath(apSuite, eventPathBuilder);
    err = writer.Finalize();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(buf);
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    eventPathParser.Init(reader);
    ParseEventPath(apSuite, eventPathParser);

    chip::System::PacketBuffer::Free(buf);
}

void EventPathListTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVWriter writer;
    chip::TLV::TLVReader reader;
    chip::System::PacketBuffer * buf = chip::System::PacketBuffer::New();

    writer.Init(buf);
    BuildEventPathList(apSuite, writer);
    err = writer.Finalize();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(buf);
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseEventPathList(apSuite, reader);

    chip::System::PacketBuffer::Free(buf);
}

void CommandPathTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err;
    chip::TLV::TLVWriter writer;
    chip::TLV::TLVReader reader;
    chip::System::PacketBuffer * buf = chip::System::PacketBuffer::New();

    writer.Init(buf);
    BuildCommandPath(apSuite, writer);

    err = writer.Finalize();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(buf);
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ParseCommandPath(apSuite, reader);

    chip::System::PacketBuffer::Free(buf);
}

void EventDataElementTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventDataElement::Builder eventDataElementBuilder;
    EventDataElement::Parser eventDataElementParser;
    chip::TLV::TLVWriter writer;
    chip::TLV::TLVReader reader;
    chip::System::PacketBuffer * buf = chip::System::PacketBuffer::New();

    writer.Init(buf);
    eventDataElementBuilder.Init(&writer);
    BuildEventDataElement(apSuite, eventDataElementBuilder);
    err = writer.Finalize();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(buf);
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    eventDataElementParser.Init(reader);
    ParseEventDataElement(apSuite, eventDataElementParser);

    chip::System::PacketBuffer::Free(buf);
}

void EventListTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVWriter writer;
    chip::TLV::TLVReader reader;
    chip::System::PacketBuffer * buf = chip::System::PacketBuffer::New();

    writer.Init(buf);
    BuildEventList(apSuite, writer);
    err = writer.Finalize();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(buf);
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseEventList(apSuite, reader);

    chip::System::PacketBuffer::Free(buf);
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
                NL_TEST_SENTINEL()
        };
// clang-format on
} // namespace

int TestMessageDef()
{
    // clang-format off
    nlTestSuite theSuite =
	{
        "MessageDef",
        &sTests[0],
        nullptr,
        nullptr
    };
    // clang-format on

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestMessageDef)
