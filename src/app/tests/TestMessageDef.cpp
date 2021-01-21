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

#include <app/MessageDef/CommandDataElement.h>
#include <app/MessageDef/CommandList.h>
#include <app/MessageDef/InvokeCommand.h>
#include <app/MessageDef/MessageDef.h>
#include <app/MessageDef/ReadRequest.h>
#include <app/MessageDef/ReportData.h>
#include <core/CHIPTLVDebug.hpp>
#include <support/UnitTestRegistration.h>
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

    err = attributeDataVersionListParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

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

    err = readRequestParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = readRequestParser.GetAttributePathList(&attributePathListParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = readRequestParser.GetEventPathList(&eventPathListParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = readRequestParser.GetAttributeDataVersionList(&attributeDataVersionListParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = readRequestParser.GetEventNumber(&eventNumber);
    NL_TEST_ASSERT(apSuite, eventNumber == 1 && err == CHIP_NO_ERROR);
}

void AttributePathTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributePath::Builder attributePathBuilder;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBuffer::New());
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

    writer.Init(chip::System::PacketBuffer::New());

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
    writer.Init(chip::System::PacketBuffer::New());
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

    writer.Init(chip::System::PacketBuffer::New());

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
    writer.Init(chip::System::PacketBuffer::New());
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
    writer.Init(chip::System::PacketBuffer::New());
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
    writer.Init(chip::System::PacketBuffer::New());
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
    writer.Init(chip::System::PacketBuffer::New());
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
    writer.Init(chip::System::PacketBuffer::New());
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
    writer.Init(chip::System::PacketBuffer::New());
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
    writer.Init(chip::System::PacketBuffer::New());
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
    writer.Init(chip::System::PacketBuffer::New());
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
    writer.Init(chip::System::PacketBuffer::New());
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
    writer.Init(chip::System::PacketBuffer::New());
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

void CommandListTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    CommandList::Builder commandListBuilder;
    writer.Init(chip::System::PacketBuffer::New());
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
    writer.Init(chip::System::PacketBuffer::New());
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
    writer.Init(chip::System::PacketBuffer::New());
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
    writer.Init(chip::System::PacketBuffer::New());
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
                NL_TEST_DEF("CommandListTest", CommandListTest),
                NL_TEST_DEF("ReportDataTest", ReportDataTest),
                NL_TEST_DEF("InvokeCommandTest", InvokeCommandTest),
                NL_TEST_DEF("ReadRequestTest", ReadRequestTest),
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
