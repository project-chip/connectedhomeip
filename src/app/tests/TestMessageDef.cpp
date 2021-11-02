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
#include <app/MessageDef/EventFilters.h>
#include <app/MessageDef/EventStatusIB.h>
#include <app/MessageDef/InvokeRequestMessage.h>
#include <app/MessageDef/InvokeResponseMessage.h>
#include <app/MessageDef/ReadRequestMessage.h>
#include <app/MessageDef/ReportDataMessage.h>
#include <app/MessageDef/SubscribeRequestMessage.h>
#include <app/MessageDef/SubscribeResponseMessage.h>
#include <app/MessageDef/TimedRequestMessage.h>
#include <app/MessageDef/WriteRequestMessage.h>
#include <app/MessageDef/WriteResponseMessage.h>
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

void BuildEventFilterIB(nlTestSuite * apSuite, EventFilterIB::Builder & aEventFilterIBBuilder)
{
    aEventFilterIBBuilder.Node(1).EventMin(2).EndOfEventFilterIB();
    NL_TEST_ASSERT(apSuite, aEventFilterIBBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseEventFilterIB(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventFilterIB::Parser eventFilterIBParser;
    chip::NodeId node = 1;
    uint64_t eventMin = 2;

    err = eventFilterIBParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = eventFilterIBParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = eventFilterIBParser.GetNode(&node);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && node == 1);

    err = eventFilterIBParser.GetEventMin(&eventMin);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && eventMin == 2);
}

void BuildEventFilters(nlTestSuite * apSuite, EventFilters::Builder & aEventFiltersBuilder)
{
    EventFilterIB::Builder eventFilterBuilder = aEventFiltersBuilder.CreateEventFilter();
    NL_TEST_ASSERT(apSuite, aEventFiltersBuilder.GetError() == CHIP_NO_ERROR);
    BuildEventFilterIB(apSuite, eventFilterBuilder);
    aEventFiltersBuilder.EndOfEventFilters();
    NL_TEST_ASSERT(apSuite, aEventFiltersBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseEventFilters(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventFilters::Parser eventFiltersParser;
    AttributePath::Parser attributePathParser;

    err = eventFiltersParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = eventFiltersParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
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

void BuildEventPath(nlTestSuite * apSuite, EventPathIB::Builder & aEventPathBuilder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    aEventPathBuilder.Node(1).Endpoint(2).Cluster(3).Event(4).IsUrgent(true).EndOfEventPathIB();
    err = aEventPathBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void ParseEventPath(nlTestSuite * apSuite, EventPathIB::Parser & aEventPathParser)
{
    CHIP_ERROR err              = CHIP_NO_ERROR;
    chip::NodeId nodeId         = 1;
    chip::EndpointId endpointId = 2;
    chip::ClusterId clusterId   = 3;
    chip::EventId eventId       = 4;
    bool isUrgent               = false;

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = aEventPathParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = aEventPathParser.GetNode(&nodeId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && nodeId == 1);

    err = aEventPathParser.GetEndpoint(&endpointId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && endpointId == 2);

    err = aEventPathParser.GetCluster(&clusterId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && clusterId == 3);

    err = aEventPathParser.GetEvent(&eventId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && eventId == 4);

    err = aEventPathParser.GetIsUrgent(&isUrgent);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && isUrgent == true);
}

void BuildEventPaths(nlTestSuite * apSuite, EventPaths::Builder & aEventPathsBuilder)
{
    EventPathIB::Builder eventPathBuilder = aEventPathsBuilder.CreateEventPath();
    NL_TEST_ASSERT(apSuite, eventPathBuilder.GetError() == CHIP_NO_ERROR);
    BuildEventPath(apSuite, eventPathBuilder);

    aEventPathsBuilder.EndOfEventPaths();
    NL_TEST_ASSERT(apSuite, aEventPathsBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseEventPaths(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventPaths::Parser eventPathListParser;

    err = eventPathListParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = eventPathListParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
}

void BuildCommandPath(nlTestSuite * apSuite, CommandPathIB::Builder & aCommandPathBuilder)
{
    aCommandPathBuilder.EndpointId(1).ClusterId(3).CommandId(4).EndOfCommandPathIB();
    NL_TEST_ASSERT(apSuite, aCommandPathBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseCommandPath(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandPathIB::Parser commandPathParser;
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

    EventPathIB::Builder eventPathBuilder = aEventDataElementBuilder.CreateEventPath();
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
            EventPathIB::Parser eventPath;
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

void BuildStatusIB(nlTestSuite * apSuite, StatusIB::Builder & aStatusIBBuilder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    StatusIB statusIB;
    statusIB.mStatus = chip::Protocols::InteractionModel::Status::InvalidSubscription;
    aStatusIBBuilder.EncodeStatusIB(statusIB);
    err = aStatusIBBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void ParseStatusIB(nlTestSuite * apSuite, StatusIB::Parser & aStatusIBParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    StatusIB::Parser StatusIBParser;
    StatusIB statusIB;

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = aStatusIBParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = aStatusIBParser.DecodeStatusIB(statusIB);
    NL_TEST_ASSERT(apSuite,
                   err == CHIP_NO_ERROR && statusIB.mStatus == chip::Protocols::InteractionModel::Status::InvalidSubscription &&
                       !statusIB.mClusterStatus.HasValue());
}

void BuildEventStatusIB(nlTestSuite * apSuite, EventStatusIB::Builder & aEventStatusIBBuilder)
{
    EventPathIB::Builder eventPathBuilder = aEventStatusIBBuilder.CreatePath();
    NL_TEST_ASSERT(apSuite, aEventStatusIBBuilder.GetError() == CHIP_NO_ERROR);
    BuildEventPath(apSuite, eventPathBuilder);

    StatusIB::Builder statusIBBuilder = aEventStatusIBBuilder.CreateErrorStatus();
    NL_TEST_ASSERT(apSuite, statusIBBuilder.GetError() == CHIP_NO_ERROR);
    BuildStatusIB(apSuite, statusIBBuilder);

    aEventStatusIBBuilder.EndOfEventStatusIB();
    NL_TEST_ASSERT(apSuite, aEventStatusIBBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseEventStatusIB(nlTestSuite * apSuite, EventStatusIB::Parser & aEventStatusIBParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventPathIB::Parser eventPathParser;
    StatusIB::Parser statusParser;
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = aEventStatusIBParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = aEventStatusIBParser.GetPath(&eventPathParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = aEventStatusIBParser.GetErrorStatus(&statusParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void BuildAttributeStatusIB(nlTestSuite * apSuite, AttributeStatusIB::Builder & aAttributeStatusIBBuilder)
{
    AttributePath::Builder attributePathBuilder = aAttributeStatusIBBuilder.CreateAttributePathBuilder();
    NL_TEST_ASSERT(apSuite, attributePathBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributePath(apSuite, attributePathBuilder);

    StatusIB::Builder statusIBBuilder = aAttributeStatusIBBuilder.CreateStatusIBBuilder();
    NL_TEST_ASSERT(apSuite, statusIBBuilder.GetError() == CHIP_NO_ERROR);
    BuildStatusIB(apSuite, statusIBBuilder);

    aAttributeStatusIBBuilder.EndOfAttributeStatusIB();
    NL_TEST_ASSERT(apSuite, aAttributeStatusIBBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseAttributeStatusIB(nlTestSuite * apSuite, AttributeStatusIB::Parser & aAttributeStatusIBParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributePath::Parser attributePathParser;
    StatusIB::Parser StatusIBParser;

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = aAttributeStatusIBParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = aAttributeStatusIBParser.GetAttributePath(&attributePathParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = aAttributeStatusIBParser.GetStatusIB(&StatusIBParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void BuildAttributeStatusList(nlTestSuite * apSuite, AttributeStatusList::Builder & aAttributeStatusListBuilder)
{
    AttributeStatusIB::Builder aAttributeStatusIBBuilder = aAttributeStatusListBuilder.CreateAttributeStatusBuilder();
    NL_TEST_ASSERT(apSuite, aAttributeStatusListBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributeStatusIB(apSuite, aAttributeStatusIBBuilder);

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
    StatusIB::Parser StatusIBParser;
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

void BuildCommandDataIB(nlTestSuite * apSuite, CommandDataIB::Builder & aCommandDataIBBuilder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    CommandPathIB::Builder commandPathBuilder = aCommandDataIBBuilder.CreatePath();
    NL_TEST_ASSERT(apSuite, aCommandDataIBBuilder.GetError() == CHIP_NO_ERROR);
    BuildCommandPath(apSuite, commandPathBuilder);

    // Construct command data
    {
        chip::TLV::TLVWriter * pWriter = aCommandDataIBBuilder.GetWriter();
        chip::TLV::TLVType dummyType   = chip::TLV::kTLVType_NotSpecified;
        err = pWriter->StartContainer(chip::TLV::ContextTag(chip::to_underlying(CommandDataIB::Tag::kData)),
                                      chip::TLV::kTLVType_Structure, dummyType);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = pWriter->PutBoolean(chip::TLV::ContextTag(1), true);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = pWriter->EndContainer(dummyType);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    }

    aCommandDataIBBuilder.EndOfCommandDataIB();
    NL_TEST_ASSERT(apSuite, aCommandDataIBBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseCommandDataIB(nlTestSuite * apSuite, CommandDataIB::Parser & aCommandDataIBParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandPathIB::Parser commandPathParser;
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = aCommandDataIBParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = aCommandDataIBParser.GetPath(&commandPathParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    {
        chip::TLV::TLVReader reader;
        bool val = false;
        chip::TLV::TLVType container;
        aCommandDataIBParser.GetData(&reader);
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

void BuildCommandStatusIB(nlTestSuite * apSuite, CommandStatusIB::Builder & aCommandStatusIBBuilder)
{
    CommandPathIB::Builder commandPathBuilder = aCommandStatusIBBuilder.CreatePath();
    NL_TEST_ASSERT(apSuite, aCommandStatusIBBuilder.GetError() == CHIP_NO_ERROR);
    BuildCommandPath(apSuite, commandPathBuilder);

    StatusIB::Builder statusIBBuilder = aCommandStatusIBBuilder.CreateErrorStatus();
    NL_TEST_ASSERT(apSuite, statusIBBuilder.GetError() == CHIP_NO_ERROR);
    BuildStatusIB(apSuite, statusIBBuilder);

    aCommandStatusIBBuilder.EndOfCommandStatusIB();
    NL_TEST_ASSERT(apSuite, aCommandStatusIBBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseCommandStatusIB(nlTestSuite * apSuite, CommandStatusIB::Parser & aCommandStatusIBParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandPathIB::Parser commandPathParser;
    StatusIB::Parser statusParser;
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = aCommandStatusIBParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = aCommandStatusIBParser.GetPath(&commandPathParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = aCommandStatusIBParser.GetErrorStatus(&statusParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void BuildWrongInvokeResponseIB(nlTestSuite * apSuite, InvokeResponseIB::Builder & aInvokeResponseIBBuilder)
{
    CommandDataIB::Builder commandDataBuilder = aInvokeResponseIBBuilder.CreateCommand();
    NL_TEST_ASSERT(apSuite, aInvokeResponseIBBuilder.GetError() == CHIP_NO_ERROR);
    BuildCommandDataIB(apSuite, commandDataBuilder);
    CommandStatusIB::Builder commandStatusBuilder = aInvokeResponseIBBuilder.CreateStatus();
    NL_TEST_ASSERT(apSuite, aInvokeResponseIBBuilder.GetError() == CHIP_NO_ERROR);
    BuildCommandStatusIB(apSuite, commandStatusBuilder);
    aInvokeResponseIBBuilder.EndOfInvokeResponseIB();
    NL_TEST_ASSERT(apSuite, aInvokeResponseIBBuilder.GetError() == CHIP_NO_ERROR);
}

void BuildInvokeResponseIBWithCommandDataIB(nlTestSuite * apSuite, InvokeResponseIB::Builder & aInvokeResponseIBBuilder)
{
    CommandDataIB::Builder commandDataBuilder = aInvokeResponseIBBuilder.CreateCommand();
    NL_TEST_ASSERT(apSuite, aInvokeResponseIBBuilder.GetError() == CHIP_NO_ERROR);
    BuildCommandDataIB(apSuite, commandDataBuilder);
    aInvokeResponseIBBuilder.EndOfInvokeResponseIB();
    NL_TEST_ASSERT(apSuite, aInvokeResponseIBBuilder.GetError() == CHIP_NO_ERROR);
}

void BuildInvokeResponseIBWithCommandStatusIB(nlTestSuite * apSuite, InvokeResponseIB::Builder & aInvokeResponseIBBuilder)
{
    CommandStatusIB::Builder commandStatusBuilder = aInvokeResponseIBBuilder.CreateStatus();
    NL_TEST_ASSERT(apSuite, aInvokeResponseIBBuilder.GetError() == CHIP_NO_ERROR);
    BuildCommandStatusIB(apSuite, commandStatusBuilder);
    aInvokeResponseIBBuilder.EndOfInvokeResponseIB();
    NL_TEST_ASSERT(apSuite, aInvokeResponseIBBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseInvokeResponseIBWithCommandDataIB(nlTestSuite * apSuite, InvokeResponseIB::Parser & aInvokeResponseIBParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandDataIB::Parser commandDataParser;
    CommandStatusIB::Parser statusIBParser;
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = aInvokeResponseIBParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = aInvokeResponseIBParser.GetCommand(&commandDataParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void ParseInvokeResponseIBWithCommandStatusIB(nlTestSuite * apSuite, InvokeResponseIB::Parser & aInvokeResponseIBParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandDataIB::Parser commandDataParser;
    CommandStatusIB::Parser statusIBParser;
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = aInvokeResponseIBParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = aInvokeResponseIBParser.GetStatus(&statusIBParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void ParseWrongInvokeResponseIB(nlTestSuite * apSuite, InvokeResponseIB::Parser & aInvokeResponseIBParser)
{
    CommandDataIB::Parser commandDataParser;
    CommandStatusIB::Parser statusIBParser;
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    CHIP_ERROR err = aInvokeResponseIBParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err != CHIP_NO_ERROR);
#endif
}

void BuildInvokeRequests(nlTestSuite * apSuite, InvokeRequests::Builder & aInvokeRequestsBuilder)
{
    CommandDataIB::Builder aCommandDataIBBuilder = aInvokeRequestsBuilder.CreateCommandData();
    NL_TEST_ASSERT(apSuite, aInvokeRequestsBuilder.GetError() == CHIP_NO_ERROR);
    BuildCommandDataIB(apSuite, aCommandDataIBBuilder);

    aInvokeRequestsBuilder.EndOfInvokeRequests();
    NL_TEST_ASSERT(apSuite, aInvokeRequestsBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseInvokeRequests(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    InvokeRequests::Parser invokeRequestsParser;
    err = invokeRequestsParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = invokeRequestsParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
}

void BuildInvokeResponses(nlTestSuite * apSuite, InvokeResponses::Builder & aInvokeResponsesBuilder)
{
    InvokeResponseIB::Builder invokeResponseIBBuilder = aInvokeResponsesBuilder.CreateInvokeResponse();
    NL_TEST_ASSERT(apSuite, aInvokeResponsesBuilder.GetError() == CHIP_NO_ERROR);
    BuildInvokeResponseIBWithCommandDataIB(apSuite, invokeResponseIBBuilder);

    aInvokeResponsesBuilder.EndOfInvokeResponses();
    NL_TEST_ASSERT(apSuite, aInvokeResponsesBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseInvokeResponses(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    InvokeResponses::Parser invokeResponsesParser;
    err = invokeResponsesParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = invokeResponsesParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
}

void BuildInvokeRequestMessage(nlTestSuite * apSuite, chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    InvokeRequestMessage::Builder invokeRequestMessageBuilder;
    err = invokeRequestMessageBuilder.Init(&aWriter);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    invokeRequestMessageBuilder.SuppressResponse(true);
    invokeRequestMessageBuilder.TimedRequest(true);
    InvokeRequests::Builder invokeRequestsBuilder = invokeRequestMessageBuilder.CreateInvokeRequests();
    NL_TEST_ASSERT(apSuite, invokeRequestsBuilder.GetError() == CHIP_NO_ERROR);

    BuildInvokeRequests(apSuite, invokeRequestsBuilder);

    invokeRequestMessageBuilder.EndOfInvokeRequestMessage();
    NL_TEST_ASSERT(apSuite, invokeRequestMessageBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseInvokeRequestMessage(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    InvokeRequestMessage::Parser invokeRequestMessageParser;
    err = invokeRequestMessageParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    bool suppressResponse = false;
    bool timedRequest     = false;
    invokeRequestMessageParser.GetSuppressResponse(&suppressResponse);
    invokeRequestMessageParser.GetTimedRequest(&timedRequest);
    NL_TEST_ASSERT(apSuite, suppressResponse == true);
    NL_TEST_ASSERT(apSuite, timedRequest == true);
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = invokeRequestMessageParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
}

void BuildInvokeResponseMessage(nlTestSuite * apSuite, chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    InvokeResponseMessage::Builder invokeResponseMessageBuilder;
    err = invokeResponseMessageBuilder.Init(&aWriter);

    invokeResponseMessageBuilder.SuppressResponse(true);
    InvokeResponses::Builder invokeResponsesBuilder = invokeResponseMessageBuilder.CreateInvokeResponses();
    NL_TEST_ASSERT(apSuite, invokeResponseMessageBuilder.GetError() == CHIP_NO_ERROR);

    BuildInvokeResponses(apSuite, invokeResponsesBuilder);

    invokeResponseMessageBuilder.EndOfInvokeResponseMessage();
    NL_TEST_ASSERT(apSuite, invokeResponseMessageBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseInvokeResponseMessage(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    InvokeResponseMessage::Parser invokeResponseMessageParser;
    err = invokeResponseMessageParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    bool suppressResponse = false;
    invokeResponseMessageParser.GetSuppressResponse(&suppressResponse);
    NL_TEST_ASSERT(apSuite, suppressResponse == true);
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = invokeResponseMessageParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
}

void BuildReportDataMessage(nlTestSuite * apSuite, chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ReportDataMessage::Builder reportDataBuilder;

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

    reportDataBuilder.EndOfReportDataMessage();
    NL_TEST_ASSERT(apSuite, reportDataBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseReportDataMessage(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ReportDataMessage::Parser reportDataParser;

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

void BuildReadRequestMessage(nlTestSuite * apSuite, chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ReadRequestMessage::Builder readRequestBuilder;

    err = readRequestBuilder.Init(&aWriter);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    AttributePathList::Builder attributePathList = readRequestBuilder.CreateAttributePathListBuilder();
    NL_TEST_ASSERT(apSuite, readRequestBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributePathList(apSuite, attributePathList);

    EventPaths::Builder eventPathList = readRequestBuilder.CreateEventPathsBuilder();
    NL_TEST_ASSERT(apSuite, readRequestBuilder.GetError() == CHIP_NO_ERROR);
    BuildEventPaths(apSuite, eventPathList);

    AttributeDataVersionList::Builder attributeDataVersionList = readRequestBuilder.CreateAttributeDataVersionListBuilder();
    NL_TEST_ASSERT(apSuite, readRequestBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributeDataVersionList(apSuite, attributeDataVersionList);

    readRequestBuilder.EventNumber(1);
    NL_TEST_ASSERT(apSuite, readRequestBuilder.GetError() == CHIP_NO_ERROR);

    readRequestBuilder.EndOfReadRequestMessage();
    NL_TEST_ASSERT(apSuite, readRequestBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseReadRequestMessage(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ReadRequestMessage::Parser readRequestParser;
    AttributePathList::Parser attributePathListParser;
    EventPaths::Parser eventPathListParser;
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

    err = readRequestParser.GetEventPaths(&eventPathListParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = readRequestParser.GetAttributeDataVersionList(&attributeDataVersionListParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = readRequestParser.GetEventNumber(&eventNumber);
    NL_TEST_ASSERT(apSuite, eventNumber == 1 && err == CHIP_NO_ERROR);
}

void BuildWriteRequestMessage(nlTestSuite * apSuite, chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    WriteRequestMessage::Builder writeRequestBuilder;

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

    writeRequestBuilder.EndOfWriteRequestMessage();
    NL_TEST_ASSERT(apSuite, writeRequestBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseWriteRequestMessage(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    WriteRequestMessage::Parser writeRequestParser;
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

void BuildWriteResponseMessage(nlTestSuite * apSuite, chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    WriteResponseMessage::Builder writeResponseBuilder;

    err = writeResponseBuilder.Init(&aWriter);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    AttributeStatusList::Builder attributeStatusList = writeResponseBuilder.CreateAttributeStatusListBuilder();
    NL_TEST_ASSERT(apSuite, writeResponseBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributeStatusList(apSuite, attributeStatusList);

    writeResponseBuilder.EndOfWriteResponseMessage();
    NL_TEST_ASSERT(apSuite, writeResponseBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseWriteResponseMessage(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    WriteResponseMessage::Parser writeResponseParser;
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

void BuildSubscribeRequestMessage(nlTestSuite * apSuite, chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    SubscribeRequestMessage::Builder subscribeRequestBuilder;

    err = subscribeRequestBuilder.Init(&aWriter);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    AttributePathList::Builder attributePathList = subscribeRequestBuilder.CreateAttributePathListBuilder();
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributePathList(apSuite, attributePathList);

    EventPaths::Builder eventPathList = subscribeRequestBuilder.CreateEventPathsBuilder();
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);
    BuildEventPaths(apSuite, eventPathList);

    AttributeDataVersionList::Builder attributeDataVersionList = subscribeRequestBuilder.CreateAttributeDataVersionListBuilder();
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributeDataVersionList(apSuite, attributeDataVersionList);

    subscribeRequestBuilder.EventNumber(1);
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

    subscribeRequestBuilder.MinIntervalSeconds(2);
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

    subscribeRequestBuilder.MaxIntervalSeconds(3);
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

    subscribeRequestBuilder.KeepSubscriptions(true);
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

    subscribeRequestBuilder.IsProxy(true);
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

    subscribeRequestBuilder.EndOfSubscribeRequestMessage();
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseSubscribeRequestMessage(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    SubscribeRequestMessage::Parser subscribeRequestParser;
    AttributePathList::Parser attributePathListParser;
    EventPaths::Parser eventPathListParser;
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

    err = subscribeRequestParser.GetEventPaths(&eventPathListParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = subscribeRequestParser.GetAttributeDataVersionList(&attributeDataVersionListParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = subscribeRequestParser.GetEventNumber(&eventNumber);
    NL_TEST_ASSERT(apSuite, eventNumber == 1 && err == CHIP_NO_ERROR);

    err = subscribeRequestParser.GetMinIntervalSeconds(&minIntervalSeconds);
    NL_TEST_ASSERT(apSuite, minIntervalSeconds == 2 && err == CHIP_NO_ERROR);

    err = subscribeRequestParser.GetMaxIntervalSeconds(&maxIntervalSeconds);
    NL_TEST_ASSERT(apSuite, maxIntervalSeconds == 3 && err == CHIP_NO_ERROR);

    err = subscribeRequestParser.GetKeepSubscriptions(&keepExistingSubscription);
    NL_TEST_ASSERT(apSuite, keepExistingSubscription && err == CHIP_NO_ERROR);

    err = subscribeRequestParser.GetIsProxy(&isProxy);
    NL_TEST_ASSERT(apSuite, isProxy && err == CHIP_NO_ERROR);
}

void BuildSubscribeResponseMessage(nlTestSuite * apSuite, chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    SubscribeResponseMessage::Builder subscribeResponseBuilder;

    err = subscribeResponseBuilder.Init(&aWriter);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    subscribeResponseBuilder.SubscriptionId(1);
    NL_TEST_ASSERT(apSuite, subscribeResponseBuilder.GetError() == CHIP_NO_ERROR);

    subscribeResponseBuilder.MinIntervalFloorSeconds(1);
    NL_TEST_ASSERT(apSuite, subscribeResponseBuilder.GetError() == CHIP_NO_ERROR);

    subscribeResponseBuilder.MaxIntervalCeilingSeconds(2);
    NL_TEST_ASSERT(apSuite, subscribeResponseBuilder.GetError() == CHIP_NO_ERROR);

    subscribeResponseBuilder.EndOfSubscribeResponseMessage();
    NL_TEST_ASSERT(apSuite, subscribeResponseBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseSubscribeResponseMessage(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    SubscribeResponseMessage::Parser subscribeResponseParser;
    uint64_t subscriptionId            = 0;
    uint16_t minIntervalFloorSeconds   = 0;
    uint16_t maxIntervalCeilingSeconds = 0;
    err                                = subscribeResponseParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = subscribeResponseParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = subscribeResponseParser.GetSubscriptionId(&subscriptionId);
    NL_TEST_ASSERT(apSuite, subscriptionId == 1 && err == CHIP_NO_ERROR);

    err = subscribeResponseParser.GetMinIntervalFloorSeconds(&minIntervalFloorSeconds);
    NL_TEST_ASSERT(apSuite, minIntervalFloorSeconds == 1 && err == CHIP_NO_ERROR);

    err = subscribeResponseParser.GetMaxIntervalCeilingSeconds(&maxIntervalCeilingSeconds);
    NL_TEST_ASSERT(apSuite, maxIntervalCeilingSeconds == 2 && err == CHIP_NO_ERROR);
}

void BuildTimedRequestMessage(nlTestSuite * apSuite, chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TimedRequestMessage::Builder TimedRequestMessageBuilder;

    err = TimedRequestMessageBuilder.Init(&aWriter);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    TimedRequestMessageBuilder.TimeoutMs(1);
    NL_TEST_ASSERT(apSuite, TimedRequestMessageBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseTimedRequestMessage(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    TimedRequestMessage::Parser TimedRequestMessagearser;
    uint16_t timeout = 0;

    err = TimedRequestMessagearser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = TimedRequestMessagearser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = TimedRequestMessagearser.GetTimeoutMs(&timeout);
    NL_TEST_ASSERT(apSuite, timeout == 1 && err == CHIP_NO_ERROR);
}

void EventFilterTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventFilterIB::Builder eventFilterBuilder;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    eventFilterBuilder.Init(&writer);
    BuildEventFilterIB(apSuite, eventFilterBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ParseEventFilterIB(apSuite, reader);
}

void EventFiltersTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    EventFilters::Builder eventFiltersBuilder;

    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));

    err = eventFiltersBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    BuildEventFilters(apSuite, eventFiltersBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseEventFilters(apSuite, reader);
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
    EventPathIB::Parser eventPathParser;
    EventPathIB::Builder eventPathBuilder;
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

void EventPathsTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    EventPaths::Builder eventPathListBuilder;

    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));

    err = eventPathListBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    BuildEventPaths(apSuite, eventPathListBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseEventPaths(apSuite, reader);
}

void CommandPathIBTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    CommandPathIB::Builder commandPathBuilder;
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

void StatusIBTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    StatusIB::Builder statusIBBuilder;
    StatusIB::Parser StatusIBParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    statusIBBuilder.Init(&writer);
    BuildStatusIB(apSuite, statusIBBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    StatusIBParser.Init(reader);
    ParseStatusIB(apSuite, StatusIBParser);
}

void EventStatusIBTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventStatusIB::Builder eventStatusIBBuilder;
    EventStatusIB::Parser eventStatusIBParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    eventStatusIBBuilder.Init(&writer);
    BuildEventStatusIB(apSuite, eventStatusIBBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    eventStatusIBParser.Init(reader);
    ParseEventStatusIB(apSuite, eventStatusIBParser);
}

void AttributeStatusIBTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributeStatusIB::Builder attributeStatusIBBuilder;
    AttributeStatusIB::Parser attributeStatusIBParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    attributeStatusIBBuilder.Init(&writer);
    BuildAttributeStatusIB(apSuite, attributeStatusIBBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    attributeStatusIBParser.Init(reader);
    ParseAttributeStatusIB(apSuite, attributeStatusIBParser);
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

void CommandDataIBTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandDataIB::Builder commandDataIBBuilder;
    CommandDataIB::Parser commandDataIBParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    commandDataIBBuilder.Init(&writer);
    BuildCommandDataIB(apSuite, commandDataIBBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    commandDataIBParser.Init(reader);
    ParseCommandDataIB(apSuite, commandDataIBParser);
}

void CommandStatusIBTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandStatusIB::Builder commandStatusIBBuilder;
    CommandStatusIB::Parser commandStatusIBParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    commandStatusIBBuilder.Init(&writer);
    BuildCommandStatusIB(apSuite, commandStatusIBBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    commandStatusIBParser.Init(reader);
    ParseCommandStatusIB(apSuite, commandStatusIBParser);
}

void InvokeResponseIBWithCommandDataIBTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    InvokeResponseIB::Builder invokeResponseIBBuilder;
    InvokeResponseIB::Parser invokeResponseIBParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    invokeResponseIBBuilder.Init(&writer);
    BuildInvokeResponseIBWithCommandDataIB(apSuite, invokeResponseIBBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    invokeResponseIBParser.Init(reader);
    ParseInvokeResponseIBWithCommandDataIB(apSuite, invokeResponseIBParser);
}

void InvokeResponseIBWithCommandStatusIBTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    InvokeResponseIB::Builder invokeResponseIBBuilder;
    InvokeResponseIB::Parser invokeResponseIBParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    invokeResponseIBBuilder.Init(&writer);
    BuildInvokeResponseIBWithCommandStatusIB(apSuite, invokeResponseIBBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    invokeResponseIBParser.Init(reader);
    ParseInvokeResponseIBWithCommandStatusIB(apSuite, invokeResponseIBParser);
}

void InvokeResponseIBWithMalformDataTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    InvokeResponseIB::Builder invokeResponseIBBuilder;
    InvokeResponseIB::Parser invokeResponseIBParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    invokeResponseIBBuilder.Init(&writer);
    BuildWrongInvokeResponseIB(apSuite, invokeResponseIBBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    invokeResponseIBParser.Init(reader);
    ParseWrongInvokeResponseIB(apSuite, invokeResponseIBParser);
}

void InvokeRequestsTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    InvokeRequests::Builder invokeRequestsBuilder;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    invokeRequestsBuilder.Init(&writer);
    BuildInvokeRequests(apSuite, invokeRequestsBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseInvokeRequests(apSuite, reader);
}

void InvokeResponsesTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    InvokeResponses::Builder invokeResponsesBuilder;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    invokeResponsesBuilder.Init(&writer);
    BuildInvokeResponses(apSuite, invokeResponsesBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseInvokeResponses(apSuite, reader);
}

void InvokeInvokeRequestMessageTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildInvokeRequestMessage(apSuite, writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseInvokeRequestMessage(apSuite, reader);
}

void InvokeInvokeResponseMessageTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildInvokeResponseMessage(apSuite, writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseInvokeResponseMessage(apSuite, reader);
}

void ReportDataMessageTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildReportDataMessage(apSuite, writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseReportDataMessage(apSuite, reader);
}

void ReadRequestMessageTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildReadRequestMessage(apSuite, writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseReadRequestMessage(apSuite, reader);
}

void WriteRequestMessageTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildWriteRequestMessage(apSuite, writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseWriteRequestMessage(apSuite, reader);
}

void WriteResponseMessageTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildWriteResponseMessage(apSuite, writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseWriteResponseMessage(apSuite, reader);
}

void SubscribeRequestMessageTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildSubscribeRequestMessage(apSuite, writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseSubscribeRequestMessage(apSuite, reader);
}

void SubscribeResponseMessageTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildSubscribeResponseMessage(apSuite, writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseSubscribeResponseMessage(apSuite, reader);
}

void TimedRequestMessageTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildTimedRequestMessage(apSuite, writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseTimedRequestMessage(apSuite, reader);
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
                NL_TEST_DEF("EventFilterTest", EventFilterTest),
                NL_TEST_DEF("EventFiltersTest", EventFiltersTest),
                NL_TEST_DEF("AttributePathTest", AttributePathTest),
                NL_TEST_DEF("AttributePathListTest", AttributePathListTest),
                NL_TEST_DEF("EventPathTest", EventPathTest),
                NL_TEST_DEF("EventPathsTest", EventPathsTest),
                NL_TEST_DEF("EventDataElementTest", EventDataElementTest),
                NL_TEST_DEF("EventListTest", EventListTest),
                NL_TEST_DEF("StatusIBTest", StatusIBTest),
                NL_TEST_DEF("EventStatusIBTest", EventStatusIBTest),
                NL_TEST_DEF("AttributeStatusIBTest", AttributeStatusIBTest),
                NL_TEST_DEF("AttributeStatusListTest", AttributeStatusListTest),
                NL_TEST_DEF("AttributeDataElementTest", AttributeDataElementTest),
                NL_TEST_DEF("AttributeDataListTest", AttributeDataListTest),
                NL_TEST_DEF("AttributeDataVersionListTest", AttributeDataVersionListTest),
                NL_TEST_DEF("CommandPathIBTest", CommandPathIBTest),
                NL_TEST_DEF("CommandDataIBTest", CommandDataIBTest),
                NL_TEST_DEF("CommandStatusIBTest", CommandStatusIBTest),
                NL_TEST_DEF("InvokeResponseIBWithCommandDataIBTest", InvokeResponseIBWithCommandDataIBTest),
                NL_TEST_DEF("InvokeResponseIBWithCommandStatusIBTest", InvokeResponseIBWithCommandStatusIBTest),
                NL_TEST_DEF("InvokeResponseIBWithMalformDataTest", InvokeResponseIBWithMalformDataTest),
                NL_TEST_DEF("InvokeRequestsTest", InvokeRequestsTest),
                NL_TEST_DEF("InvokeResponsesTest", InvokeResponsesTest),
                NL_TEST_DEF("InvokeInvokeRequestMessageTest", InvokeInvokeRequestMessageTest),
                NL_TEST_DEF("InvokeInvokeResponseMessageTest", InvokeInvokeResponseMessageTest),
                NL_TEST_DEF("ReportDataMessageTest", ReportDataMessageTest),
                NL_TEST_DEF("ReadRequestMessageTest", ReadRequestMessageTest),
                NL_TEST_DEF("WriteRequestMessageTest", WriteRequestMessageTest),
                NL_TEST_DEF("WriteResponseMessageTest", WriteResponseMessageTest),
                NL_TEST_DEF("SubscribeRequestMessageTest", SubscribeRequestMessageTest),
                NL_TEST_DEF("SubscribeResponseMessageTest", SubscribeResponseMessageTest),
                NL_TEST_DEF("TimedRequestMessageTest", TimedRequestMessageTest),
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
