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

#include <app/MessageDef.h>
#include <core/CHIPTLVDebug.hpp>
#include <support/UnitTestRegistration.h>
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
    aAttributePathBuilder.NodeId(1).EndpointId(2).NamespacedClusterId(3).FieldId(4).ListIndex(5).EndOfAttributePath();
    err = aAttributePathBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void ParseAttributePath(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    AttributePath::Parser attributePathParser;
    CHIP_ERROR err                      = CHIP_NO_ERROR;
    chip::NodeId nodeId                 = 1;
    chip::EndpointId endpointId         = 2;
    chip::ClusterId namespacedClusterId = 3;
    uint8_t fieldId                     = 4;
    uint16_t listIndex                  = 5;

    err = attributePathParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = attributePathParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = attributePathParser.GetNodeId(&nodeId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && nodeId == 1);

    err = attributePathParser.GetEndpointId(&endpointId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && endpointId == 2);

    err = attributePathParser.GetNamespacedClusterId(&namespacedClusterId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && namespacedClusterId == 3);

    err = attributePathParser.GetFieldId(&fieldId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && fieldId == 4);

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
    aEventPathBuilder.NodeId(1).EndpointId(2).NamespacedClusterId(3).EventId(4).EndOfEventPath();
    err = aEventPathBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void ParseEventPath(nlTestSuite * apSuite, EventPath::Parser & aEventPathParser)
{
    CHIP_ERROR err                      = CHIP_NO_ERROR;
    chip::NodeId nodeId                 = 1;
    chip::EndpointId endpointId         = 2;
    chip::ClusterId namespacedClusterId = 3;
    chip::EventId eventId               = 4;

    err = aEventPathParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = aEventPathParser.GetNodeId(&nodeId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && nodeId == 1);

    err = aEventPathParser.GetEndpointId(&endpointId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && endpointId == 2);

    err = aEventPathParser.GetNamespacedClusterId(&namespacedClusterId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && namespacedClusterId == 3);

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

void BuildCommandPath(nlTestSuite * apSuite, CommandPath::Builder & aCommandPathBuilder)
{
    aCommandPathBuilder.EndpointId(1).NamespacedClusterId(3).CommandId(4).EndOfCommandPath();
    NL_TEST_ASSERT(apSuite, aCommandPathBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseCommandPath(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandPath::Parser commandPathParser;
    chip::EndpointId endpointId         = 0;
    chip::ClusterId namespacedClusterId = 0;
    chip::CommandId commandId           = 0;

    err = commandPathParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = commandPathParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = commandPathParser.GetEndpointId(&endpointId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && endpointId == 1);

    err = commandPathParser.GetNamespacedClusterId(&namespacedClusterId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && namespacedClusterId == 3);

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

    err = eventListParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void BuildStatusElement(nlTestSuite * apSuite, StatusElement::Builder & aStatusElementBuilder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    aStatusElementBuilder.EncodeStatusElement(1, 2, 3, 4).EndOfStatusElement();
    err = aStatusElementBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void ParseStatusElement(nlTestSuite * apSuite, StatusElement::Parser & aStatusElementParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    StatusElement::Parser StatusElementParser;

    uint16_t generalCode                = 0;
    uint32_t protocolId                 = 0;
    uint16_t protocolCode               = 0;
    chip::ClusterId namespacedClusterId = 0;

    err = aStatusElementParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = aStatusElementParser.DecodeStatusElement(&generalCode, &protocolId, &protocolCode, &namespacedClusterId);
    NL_TEST_ASSERT(apSuite,
                   err == CHIP_NO_ERROR && generalCode == 1 && protocolId == 2 && protocolCode == 3 && namespacedClusterId == 4);
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

    err = aAttributeStatusElementParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

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

    err = attributeStatusParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
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

    err = aAttributeDataElementParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

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

    err = attributeDataListParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
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

    err = aCommandDataElementParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

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
    commandListParser.Init(aReader);
    err = commandListParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void BuildReportData(nlTestSuite * apSuite, chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ReportData::Builder reportDataBuilder;

    err = reportDataBuilder.Init(&aWriter);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    reportDataBuilder.RequestResponse(1).SubscriptionId(2);
    NL_TEST_ASSERT(apSuite, reportDataBuilder.GetError() == CHIP_NO_ERROR);

    AttributeStatusList::Builder attributeStatusList = reportDataBuilder.CreateAttributeStatusListBuilder();
    NL_TEST_ASSERT(apSuite, reportDataBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributeStatusList(apSuite, attributeStatusList);

    AttributeDataList::Builder attributeDataList = reportDataBuilder.CreateAttributeDataListBuilder();
    NL_TEST_ASSERT(apSuite, reportDataBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributeDataList(apSuite, attributeDataList);

    EventList::Builder eventList = reportDataBuilder.CreateEventDataListBuilder();
    NL_TEST_ASSERT(apSuite, reportDataBuilder.GetError() == CHIP_NO_ERROR);
    BuildEventList(apSuite, eventList);

    reportDataBuilder.IsLastReport(6);
    NL_TEST_ASSERT(apSuite, reportDataBuilder.GetError() == CHIP_NO_ERROR);

    reportDataBuilder.EndOfReportData();
    NL_TEST_ASSERT(apSuite, reportDataBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseReportData(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ReportData::Parser reportDataParser;

    bool requestResponse    = false;
    uint64_t subscriptionId = 0;
    AttributeStatusList::Parser attributeStatusListParser;
    AttributeDataList::Parser attributeDataListParser;
    EventList::Parser eventListParser;
    bool isLastReport = false;
    reportDataParser.Init(aReader);

    err = reportDataParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = reportDataParser.GetRequestResponse(&requestResponse);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && requestResponse);

    err = reportDataParser.GetSubscriptionId(&subscriptionId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && subscriptionId == 2);

    err = reportDataParser.GetAttributeStatusList(&attributeStatusListParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = reportDataParser.GetAttributeDataList(&attributeDataListParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = reportDataParser.GetEventDataList(&eventListParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = reportDataParser.GetIsLastReport(&isLastReport);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && isLastReport);
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

    err = invokeCommandParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = invokeCommandParser.GetCommandList(&commandListParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void AttributePathTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributePath::Builder attributePathBuilder;
    chip::TLV::TLVWriter writer;
    chip::TLV::TLVReader reader;
    chip::System::PacketBufferHandle bufHandle = chip::System::PacketBuffer::New();
    chip::System::PacketBuffer * buf           = bufHandle.Get_ForNow();
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

    bufHandle = nullptr;
}

void AttributePathListTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVWriter writer;
    chip::TLV::TLVReader reader;
    chip::System::PacketBufferHandle bufHandle = chip::System::PacketBuffer::New();
    chip::System::PacketBuffer * buf           = bufHandle.Get_ForNow();
    writer.Init(buf);
    BuildAttributePathList(apSuite, writer);
    err = writer.Finalize();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(buf);
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseAttributePathList(apSuite, reader);

    bufHandle = nullptr;
}

void EventPathTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventPath::Parser eventPathParser;
    EventPath::Builder eventPathBuilder;
    chip::TLV::TLVWriter writer;
    chip::TLV::TLVReader reader;
    chip::System::PacketBufferHandle bufHandle = chip::System::PacketBuffer::New();
    chip::System::PacketBuffer * buf           = bufHandle.Get_ForNow();
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

    bufHandle = nullptr;
}

void EventPathListTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVWriter writer;
    chip::TLV::TLVReader reader;
    chip::System::PacketBufferHandle bufHandle = chip::System::PacketBuffer::New();
    chip::System::PacketBuffer * buf           = bufHandle.Get_ForNow();
    writer.Init(buf);
    BuildEventPathList(apSuite, writer);
    err = writer.Finalize();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(buf);
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseEventPathList(apSuite, reader);

    bufHandle = nullptr;
}

void CommandPathTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVWriter writer;
    chip::TLV::TLVReader reader;
    CommandPath::Builder commandPathBuilder;
    chip::System::PacketBufferHandle bufHandle = chip::System::PacketBuffer::New();
    chip::System::PacketBuffer * buf           = bufHandle.Get_ForNow();
    writer.Init(buf);
    err = commandPathBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    BuildCommandPath(apSuite, commandPathBuilder);

    err = writer.Finalize();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(buf);
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ParseCommandPath(apSuite, reader);

    bufHandle = nullptr;
}

void EventDataElementTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventDataElement::Builder eventDataElementBuilder;
    EventDataElement::Parser eventDataElementParser;
    chip::TLV::TLVWriter writer;
    chip::TLV::TLVReader reader;
    chip::System::PacketBufferHandle bufHandle = chip::System::PacketBuffer::New();
    chip::System::PacketBuffer * buf           = bufHandle.Get_ForNow();
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

    bufHandle = nullptr;
}

void EventListTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVWriter writer;
    chip::TLV::TLVReader reader;
    EventList::Builder eventListBuilder;
    chip::System::PacketBufferHandle bufHandle = chip::System::PacketBuffer::New();
    chip::System::PacketBuffer * buf           = bufHandle.Get_ForNow();
    writer.Init(buf);
    eventListBuilder.Init(&writer);
    BuildEventList(apSuite, eventListBuilder);
    err = writer.Finalize();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(buf);
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseEventList(apSuite, reader);

    bufHandle = nullptr;
}

void StatusElementTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    StatusElement::Builder statusElementBuilder;
    StatusElement::Parser statusElementParser;
    chip::TLV::TLVWriter writer;
    chip::TLV::TLVReader reader;
    chip::System::PacketBufferHandle bufHandle = chip::System::PacketBuffer::New();
    chip::System::PacketBuffer * buf           = bufHandle.Get_ForNow();
    writer.Init(buf);
    statusElementBuilder.Init(&writer);
    BuildStatusElement(apSuite, statusElementBuilder);
    err = writer.Finalize();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(buf);
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    statusElementParser.Init(reader);
    ParseStatusElement(apSuite, statusElementParser);

    bufHandle = nullptr;
}

void AttributeStatusElementTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributeStatusElement::Builder attributeStatusElementBuilder;
    AttributeStatusElement::Parser attributeStatusElementParser;
    chip::TLV::TLVWriter writer;
    chip::TLV::TLVReader reader;
    chip::System::PacketBufferHandle bufHandle = chip::System::PacketBuffer::New();
    chip::System::PacketBuffer * buf           = bufHandle.Get_ForNow();
    writer.Init(buf);
    attributeStatusElementBuilder.Init(&writer);
    BuildAttributeStatusElement(apSuite, attributeStatusElementBuilder);
    err = writer.Finalize();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(buf);
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    attributeStatusElementParser.Init(reader);
    ParseAttributeStatusElement(apSuite, attributeStatusElementParser);

    bufHandle = nullptr;
}

void AttributeStatusListTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVWriter writer;
    chip::TLV::TLVReader reader;
    chip::System::PacketBufferHandle bufHandle = chip::System::PacketBuffer::New();
    chip::System::PacketBuffer * buf           = bufHandle.Get_ForNow();
    writer.Init(buf);
    AttributeStatusList::Builder attributeStatusListBuilder;
    err = attributeStatusListBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    BuildAttributeStatusList(apSuite, attributeStatusListBuilder);
    err = writer.Finalize();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(buf);
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseAttributeStatusList(apSuite, reader);

    bufHandle = nullptr;
}

void AttributeDataElementTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributeDataElement::Builder attributeDataElementBuilder;
    AttributeDataElement::Parser attributeDataElementParser;
    chip::TLV::TLVWriter writer;
    chip::TLV::TLVReader reader;
    chip::System::PacketBufferHandle bufHandle = chip::System::PacketBuffer::New();
    chip::System::PacketBuffer * buf           = bufHandle.Get_ForNow();
    writer.Init(buf);
    attributeDataElementBuilder.Init(&writer);
    BuildAttributeDataElement(apSuite, attributeDataElementBuilder);
    err = writer.Finalize();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(buf);
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    attributeDataElementParser.Init(reader);
    ParseAttributeDataElement(apSuite, attributeDataElementParser);

    bufHandle = nullptr;
}

void AttributeDataListTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVWriter writer;
    chip::TLV::TLVReader reader;
    chip::System::PacketBufferHandle bufHandle = chip::System::PacketBuffer::New();
    chip::System::PacketBuffer * buf           = bufHandle.Get_ForNow();
    writer.Init(buf);
    AttributeDataList::Builder attributeDataListBuilder;
    attributeDataListBuilder.Init(&writer);
    BuildAttributeDataList(apSuite, attributeDataListBuilder);
    err = writer.Finalize();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(buf);
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseAttributeDataList(apSuite, reader);

    bufHandle = nullptr;
}

void CommandDataElementTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandDataElement::Builder commandDataElementBuilder;
    CommandDataElement::Parser commandDataElementParser;
    chip::TLV::TLVWriter writer;
    chip::TLV::TLVReader reader;
    chip::System::PacketBufferHandle bufHandle = chip::System::PacketBuffer::New();
    chip::System::PacketBuffer * buf           = bufHandle.Get_ForNow();
    writer.Init(buf);
    commandDataElementBuilder.Init(&writer);
    BuildCommandDataElement(apSuite, commandDataElementBuilder);
    err = writer.Finalize();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(buf);
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    commandDataElementParser.Init(reader);
    ParseCommandDataElement(apSuite, commandDataElementParser);

    bufHandle = nullptr;
}

void CommandListTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVWriter writer;
    chip::TLV::TLVReader reader;
    CommandList::Builder commandListBuilder;
    chip::System::PacketBufferHandle bufHandle = chip::System::PacketBuffer::New();
    chip::System::PacketBuffer * buf           = bufHandle.Get_ForNow();
    writer.Init(buf);
    commandListBuilder.Init(&writer);
    BuildCommandList(apSuite, commandListBuilder);
    err = writer.Finalize();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(buf);
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseCommandList(apSuite, reader);

    bufHandle = nullptr;
}

void ReportDataTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVWriter writer;
    chip::TLV::TLVReader reader;
    chip::System::PacketBufferHandle bufHandle = chip::System::PacketBuffer::New();
    chip::System::PacketBuffer * buf           = bufHandle.Get_ForNow();
    writer.Init(buf);
    BuildReportData(apSuite, writer);
    err = writer.Finalize();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(buf);
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseReportData(apSuite, reader);

    bufHandle = nullptr;
}

void InvokeCommandTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVWriter writer;
    chip::TLV::TLVReader reader;
    chip::System::PacketBufferHandle bufHandle = chip::System::PacketBuffer::New();
    chip::System::PacketBuffer * buf           = bufHandle.Get_ForNow();
    writer.Init(buf);
    BuildInvokeCommand(apSuite, writer);
    err = writer.Finalize();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(buf);
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseInvokeCommand(apSuite, reader);

    bufHandle = nullptr;
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
                NL_TEST_DEF("CommandDataElementTest", CommandDataElementTest),
                NL_TEST_DEF("CommandListTest", CommandListTest),
                NL_TEST_DEF("ReportDataTest", ReportDataTest),
                NL_TEST_DEF("InvokeCommandTest", InvokeCommandTest),
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
