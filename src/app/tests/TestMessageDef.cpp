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

#include <app/AppConfig.h>
#include <app/MessageDef/EventFilterIBs.h>
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
#include <lib/core/TLVDebug.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/EnforceFormat.h>
#include <lib/support/logging/Constants.h>
#include <system/TLVPacketBufferBackingStore.h>

#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>

namespace {

using namespace chip::app;

class TestMessageDef : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

void ENFORCE_FORMAT(1, 2) TLVPrettyPrinter(const char * aFormat, ...)
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

void BuildStatusIB(StatusIB::Builder & aStatusIBBuilder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    StatusIB statusIB;
    statusIB.mStatus = chip::Protocols::InteractionModel::Status::InvalidSubscription;
    aStatusIBBuilder.EncodeStatusIB(statusIB);
    err = aStatusIBBuilder.GetError();
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

void ParseStatusIB(StatusIB::Parser & aStatusIBParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    StatusIB::Parser StatusIBParser;
    StatusIB statusIB;

#if CHIP_CONFIG_IM_PRETTY_PRINT
    aStatusIBParser.PrettyPrint();
#endif
    err = aStatusIBParser.DecodeStatusIB(statusIB);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(statusIB.mStatus, chip::Protocols::InteractionModel::Status::InvalidSubscription);
    EXPECT_FALSE(statusIB.mClusterStatus.HasValue());
}

void BuildClusterPathIB(ClusterPathIB::Builder & aClusterPathBuilder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    aClusterPathBuilder.Node(1).Endpoint(2).Cluster(3).EndOfClusterPathIB();
    err = aClusterPathBuilder.GetError();
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

void ParseClusterPathIB(chip::TLV::TLVReader & aReader)
{
    ClusterPathIB::Parser clusterPathParser;
    CHIP_ERROR err            = CHIP_NO_ERROR;
    chip::NodeId node         = 0;
    chip::EndpointId endpoint = 0;
    chip::ClusterId cluster   = 0;

    err = clusterPathParser.Init(aReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_PRETTY_PRINT
    clusterPathParser.PrettyPrint();
#endif

    err = clusterPathParser.GetNode(&node);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(node, 1u);

    err = clusterPathParser.GetEndpoint(&endpoint);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(endpoint, 2u);

    err = clusterPathParser.GetCluster(&cluster);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(cluster, 3u);
}

void BuildDataVersionFilterIB(DataVersionFilterIB::Builder & aDataVersionFilterIBBuilder)
{
    ClusterPathIB::Builder & clusterPathBuilder = aDataVersionFilterIBBuilder.CreatePath();
    EXPECT_EQ(clusterPathBuilder.GetError(), CHIP_NO_ERROR);
    BuildClusterPathIB(clusterPathBuilder);
    aDataVersionFilterIBBuilder.DataVersion(2).EndOfDataVersionFilterIB();
    EXPECT_EQ(aDataVersionFilterIBBuilder.GetError(), CHIP_NO_ERROR);
}

void ParseDataVersionFilterIB(chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    DataVersionFilterIB::Parser dataVersionFilterIBParser;
    ClusterPathIB::Parser clusterPath;
    chip::DataVersion dataVersion = 2;

    err = dataVersionFilterIBParser.Init(aReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_PRETTY_PRINT
    dataVersionFilterIBParser.PrettyPrint();
#endif

    err = dataVersionFilterIBParser.GetPath(&clusterPath);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = dataVersionFilterIBParser.GetDataVersion(&dataVersion);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(dataVersion, 2u);
}

void BuildDataVersionFilterIBs(DataVersionFilterIBs::Builder & aDataVersionFilterIBsBuilder)
{
    DataVersionFilterIB::Builder & dataVersionFilterIBBuilder = aDataVersionFilterIBsBuilder.CreateDataVersionFilter();
    EXPECT_EQ(aDataVersionFilterIBsBuilder.GetError(), CHIP_NO_ERROR);
    BuildDataVersionFilterIB(dataVersionFilterIBBuilder);
    aDataVersionFilterIBsBuilder.EndOfDataVersionFilterIBs();
    EXPECT_EQ(aDataVersionFilterIBsBuilder.GetError(), CHIP_NO_ERROR);
}

void ParseDataVersionFilterIBs(chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    DataVersionFilterIBs::Parser dataVersionFilterIBsParser;

    err = dataVersionFilterIBsParser.Init(aReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_PRETTY_PRINT
    dataVersionFilterIBsParser.PrettyPrint();
#endif
}

void BuildEventFilterIB(EventFilterIB::Builder & aEventFilterIBBuilder)
{
    aEventFilterIBBuilder.Node(1).EventMin(2).EndOfEventFilterIB();
    EXPECT_EQ(aEventFilterIBBuilder.GetError(), CHIP_NO_ERROR);
}

void ParseEventFilterIB(chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventFilterIB::Parser eventFilterIBParser;
    chip::NodeId node = 1;
    uint64_t eventMin = 2;

    err = eventFilterIBParser.Init(aReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_PRETTY_PRINT
    eventFilterIBParser.PrettyPrint();
#endif
    err = eventFilterIBParser.GetNode(&node);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_EQ(node, 1u);

    err = eventFilterIBParser.GetEventMin(&eventMin);
    EXPECT_EQ(eventMin, 2u);
}

void BuildEventFilters(EventFilterIBs::Builder & aEventFiltersBuilder)
{
    EventFilterIB::Builder & eventFilterBuilder = aEventFiltersBuilder.CreateEventFilter();
    EXPECT_EQ(aEventFiltersBuilder.GetError(), CHIP_NO_ERROR);
    BuildEventFilterIB(eventFilterBuilder);
    aEventFiltersBuilder.EndOfEventFilters();
    EXPECT_EQ(aEventFiltersBuilder.GetError(), CHIP_NO_ERROR);
}

void ParseEventFilters(chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventFilterIBs::Parser eventFiltersParser;

    err = eventFiltersParser.Init(aReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_PRETTY_PRINT
    eventFiltersParser.PrettyPrint();
#endif
}

void BuildAttributePathIB(AttributePathIB::Builder & aAttributePathBuilder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    err            = aAttributePathBuilder.EnableTagCompression(false)
              .Node(1)
              .Endpoint(2)
              .Cluster(3)
              .Attribute(4)
              .ListIndex(5)
              .EndOfAttributePathIB();
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

void ParseAttributePathIB(chip::TLV::TLVReader & aReader)
{
    AttributePathIB::Parser attributePathParser;
    CHIP_ERROR err              = CHIP_NO_ERROR;
    bool enableTagCompression   = true;
    chip::NodeId node           = 1;
    chip::EndpointId endpoint   = 2;
    chip::ClusterId cluster     = 3;
    chip::AttributeId attribute = 4;
    chip::ListIndex listIndex   = 5;

    err = attributePathParser.Init(aReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_PRETTY_PRINT
    attributePathParser.PrettyPrint();
#endif

    err = attributePathParser.GetEnableTagCompression(&enableTagCompression);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_FALSE(enableTagCompression);

    err = attributePathParser.GetNode(&node);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(node, 1u);

    err = attributePathParser.GetEndpoint(&endpoint);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(endpoint, 2u);

    err = attributePathParser.GetCluster(&cluster);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(cluster, 3u);

    err = attributePathParser.GetAttribute(&attribute);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(attribute, 4u);

    err = attributePathParser.GetListIndex(&listIndex);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(listIndex, 5u);
}

void BuildAttributePathList(AttributePathIBs::Builder & aAttributePathListBuilder)
{
    AttributePathIB::Builder & attributePathBuilder = aAttributePathListBuilder.CreatePath();
    EXPECT_EQ(attributePathBuilder.GetError(), CHIP_NO_ERROR);
    BuildAttributePathIB(attributePathBuilder);

    aAttributePathListBuilder.EndOfAttributePathIBs();
    EXPECT_EQ(aAttributePathListBuilder.GetError(), CHIP_NO_ERROR);
}

void ParseAttributePathList(chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributePathIBs::Parser attributePathListParser;
    AttributePathIB::Parser attributePathParser;

    err = attributePathListParser.Init(aReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_PRETTY_PRINT
    attributePathListParser.PrettyPrint();
#endif
}

void BuildEventPath(EventPathIB::Builder & aEventPathBuilder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    aEventPathBuilder.Node(1).Endpoint(2).Cluster(3).Event(4).IsUrgent(true).EndOfEventPathIB();
    err = aEventPathBuilder.GetError();
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

void ParseEventPath(EventPathIB::Parser & aEventPathParser)
{
    CHIP_ERROR err            = CHIP_NO_ERROR;
    chip::NodeId node         = 1;
    chip::EndpointId endpoint = 2;
    chip::ClusterId cluster   = 3;
    chip::EventId event       = 4;
    bool isUrgent             = false;

#if CHIP_CONFIG_IM_PRETTY_PRINT
    aEventPathParser.PrettyPrint();
#endif
    err = aEventPathParser.GetNode(&node);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(node, 1u);

    err = aEventPathParser.GetEndpoint(&endpoint);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(endpoint, 2u);

    err = aEventPathParser.GetCluster(&cluster);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(cluster, 3u);

    err = aEventPathParser.GetEvent(&event);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(event, 4u);

    err = aEventPathParser.GetIsUrgent(&isUrgent);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(isUrgent);
}

void BuildEventPaths(EventPathIBs::Builder & aEventPathsBuilder)
{
    EventPathIB::Builder & eventPathBuilder = aEventPathsBuilder.CreatePath();
    EXPECT_EQ(eventPathBuilder.GetError(), CHIP_NO_ERROR);
    BuildEventPath(eventPathBuilder);

    aEventPathsBuilder.EndOfEventPaths();
    EXPECT_EQ(aEventPathsBuilder.GetError(), CHIP_NO_ERROR);
}

void ParseEventPaths(chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventPathIBs::Parser eventPathListParser;

    err = eventPathListParser.Init(aReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_PRETTY_PRINT
    eventPathListParser.PrettyPrint();
#endif
}

void BuildCommandPath(CommandPathIB::Builder & aCommandPathBuilder)
{
    aCommandPathBuilder.EndpointId(1).ClusterId(3).CommandId(4).EndOfCommandPathIB();
    EXPECT_EQ(aCommandPathBuilder.GetError(), CHIP_NO_ERROR);
}

void ParseCommandPath(chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandPathIB::Parser commandPathParser;
    chip::EndpointId endpointId = 0;
    chip::ClusterId clusterId   = 0;
    chip::CommandId commandId   = 0;

    err = commandPathParser.Init(aReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_PRETTY_PRINT
    commandPathParser.PrettyPrint();
#endif
    err = commandPathParser.GetEndpointId(&endpointId);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(endpointId, 1u);

    err = commandPathParser.GetClusterId(&clusterId);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(clusterId, 3u);

    err = commandPathParser.GetCommandId(&commandId);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(commandId, 4u);
}

void BuildEventDataIB(EventDataIB::Builder & aEventDataIBBuilder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    EventPathIB::Builder & eventPathBuilder = aEventDataIBBuilder.CreatePath();
    EXPECT_EQ(eventPathBuilder.GetError(), CHIP_NO_ERROR);
    BuildEventPath(eventPathBuilder);

    aEventDataIBBuilder.EventNumber(2).Priority(3).EpochTimestamp(4).SystemTimestamp(5).DeltaEpochTimestamp(6).DeltaSystemTimestamp(
        7);
    err = aEventDataIBBuilder.GetError();
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Construct test event data
    {
        chip::TLV::TLVWriter * pWriter = aEventDataIBBuilder.GetWriter();
        chip::TLV::TLVType dummyType   = chip::TLV::kTLVType_NotSpecified;
        err = pWriter->StartContainer(chip::TLV::ContextTag(chip::to_underlying(EventDataIB::Tag::kData)),
                                      chip::TLV::kTLVType_Structure, dummyType);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = pWriter->PutBoolean(chip::TLV::ContextTag(1), true);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = pWriter->EndContainer(dummyType);
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    aEventDataIBBuilder.EndOfEventDataIB();
}

void ParseEventDataIB(EventDataIB::Parser & aEventDataIBParser)
{
    CHIP_ERROR err                = CHIP_NO_ERROR;
    uint8_t priorityLevel         = 0;
    chip::EventNumber number      = 0;
    uint64_t EpochTimestamp       = 0;
    uint64_t systemTimestamp      = 0;
    uint64_t deltaUTCTimestamp    = 0;
    uint64_t deltaSystemTimestamp = 0;

#if CHIP_CONFIG_IM_PRETTY_PRINT
    aEventDataIBParser.PrettyPrint();
#endif
    {
        {
            EventPathIB::Parser eventPath;
            err = aEventDataIBParser.GetPath(&eventPath);
            EXPECT_EQ(err, CHIP_NO_ERROR);
        }
        err = aEventDataIBParser.GetEventNumber(&number);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(number, 2u);

        err = aEventDataIBParser.GetPriority(&priorityLevel);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(priorityLevel, 3u);

        err = aEventDataIBParser.GetEpochTimestamp(&EpochTimestamp);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(EpochTimestamp, 4u);

        err = aEventDataIBParser.GetSystemTimestamp(&systemTimestamp);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(systemTimestamp, 5u);

        err = aEventDataIBParser.GetDeltaEpochTimestamp(&deltaUTCTimestamp);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(deltaUTCTimestamp, 6u);

        err = aEventDataIBParser.GetDeltaSystemTimestamp(&deltaSystemTimestamp);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(deltaSystemTimestamp, 7u);

        {
            chip::TLV::TLVReader reader;
            bool val = false;
            chip::TLV::TLVType container;
            aEventDataIBParser.GetData(&reader);
            err = reader.EnterContainer(container);
            EXPECT_EQ(err, CHIP_NO_ERROR);

            err = reader.Next();
            EXPECT_EQ(err, CHIP_NO_ERROR);

            err = reader.Get(val);
            EXPECT_EQ(err, CHIP_NO_ERROR);
            EXPECT_TRUE(val);

            err = reader.ExitContainer(container);
            EXPECT_EQ(err, CHIP_NO_ERROR);
        }
    }
}

void BuildEventStatusIB(EventStatusIB::Builder & aEventStatusIBBuilder)
{
    EventPathIB::Builder & eventPathBuilder = aEventStatusIBBuilder.CreatePath();
    EXPECT_EQ(aEventStatusIBBuilder.GetError(), CHIP_NO_ERROR);
    BuildEventPath(eventPathBuilder);

    StatusIB::Builder & statusIBBuilder = aEventStatusIBBuilder.CreateErrorStatus();
    EXPECT_EQ(statusIBBuilder.GetError(), CHIP_NO_ERROR);
    BuildStatusIB(statusIBBuilder);

    aEventStatusIBBuilder.EndOfEventStatusIB();
    EXPECT_EQ(aEventStatusIBBuilder.GetError(), CHIP_NO_ERROR);
}

void ParseEventStatusIB(EventStatusIB::Parser & aEventStatusIBParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventPathIB::Parser eventPathParser;
    StatusIB::Parser statusParser;
#if CHIP_CONFIG_IM_PRETTY_PRINT
    aEventStatusIBParser.PrettyPrint();
#endif
    err = aEventStatusIBParser.GetPath(&eventPathParser);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = aEventStatusIBParser.GetErrorStatus(&statusParser);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

void BuildEventReportIB(EventReportIB::Builder & aEventReportIBBuilder)
{
    EventDataIB::Builder & eventDataIBBuilder = aEventReportIBBuilder.CreateEventData();
    EXPECT_EQ(aEventReportIBBuilder.GetError(), CHIP_NO_ERROR);
    BuildEventDataIB(eventDataIBBuilder);

    aEventReportIBBuilder.EndOfEventReportIB();
    EXPECT_EQ(aEventReportIBBuilder.GetError(), CHIP_NO_ERROR);
}

void ParseEventReportIB(EventReportIB::Parser & aEventReportIBParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventStatusIB::Parser eventStatusParser;
    EventDataIB::Parser eventDataParser;

#if CHIP_CONFIG_IM_PRETTY_PRINT
    aEventReportIBParser.PrettyPrint();
#endif

    err = aEventReportIBParser.GetEventData(&eventDataParser);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

void BuildEventReports(EventReportIBs::Builder & aEventReportsBuilder)
{
    EventReportIB::Builder & eventReportIBBuilder = aEventReportsBuilder.CreateEventReport();
    EXPECT_EQ(aEventReportsBuilder.GetError(), CHIP_NO_ERROR);
    BuildEventReportIB(eventReportIBBuilder);
    aEventReportsBuilder.EndOfEventReports();
    EXPECT_EQ(aEventReportsBuilder.GetError(), CHIP_NO_ERROR);
}

void ParseEventReports(chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventReportIBs::Parser eventReportsParser;

    err = eventReportsParser.Init(aReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_PRETTY_PRINT
    eventReportsParser.PrettyPrint();
#endif
}

void BuildAttributeStatusIB(AttributeStatusIB::Builder & aAttributeStatusIBBuilder)
{
    AttributePathIB::Builder & attributePathBuilder = aAttributeStatusIBBuilder.CreatePath();
    EXPECT_EQ(attributePathBuilder.GetError(), CHIP_NO_ERROR);
    BuildAttributePathIB(attributePathBuilder);

    StatusIB::Builder & statusIBBuilder = aAttributeStatusIBBuilder.CreateErrorStatus();
    EXPECT_EQ(statusIBBuilder.GetError(), CHIP_NO_ERROR);
    BuildStatusIB(statusIBBuilder);

    aAttributeStatusIBBuilder.EndOfAttributeStatusIB();
    EXPECT_EQ(aAttributeStatusIBBuilder.GetError(), CHIP_NO_ERROR);
}

void ParseAttributeStatusIB(AttributeStatusIB::Parser & aAttributeStatusIBParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributePathIB::Parser attributePathParser;
    StatusIB::Parser StatusIBParser;

#if CHIP_CONFIG_IM_PRETTY_PRINT
    aAttributeStatusIBParser.PrettyPrint();
#endif
    err = aAttributeStatusIBParser.GetPath(&attributePathParser);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = aAttributeStatusIBParser.GetErrorStatus(&StatusIBParser);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

void BuildAttributeStatuses(AttributeStatusIBs::Builder & aAttributeStatusesBuilder)
{
    AttributeStatusIB::Builder & aAttributeStatusIBBuilder = aAttributeStatusesBuilder.CreateAttributeStatus();
    EXPECT_EQ(aAttributeStatusesBuilder.GetError(), CHIP_NO_ERROR);
    BuildAttributeStatusIB(aAttributeStatusIBBuilder);

    aAttributeStatusesBuilder.EndOfAttributeStatuses();
    EXPECT_EQ(aAttributeStatusesBuilder.GetError(), CHIP_NO_ERROR);
}

void ParseAttributeStatuses(chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributeStatusIBs::Parser attributeStatusParser;

    err = attributeStatusParser.Init(aReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_PRETTY_PRINT
    attributeStatusParser.PrettyPrint();
#endif
}

void BuildAttributeDataIB(AttributeDataIB::Builder & aAttributeDataIBBuilder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    aAttributeDataIBBuilder.DataVersion(2);
    AttributePathIB::Builder & attributePathBuilder = aAttributeDataIBBuilder.CreatePath();
    EXPECT_EQ(aAttributeDataIBBuilder.GetError(), CHIP_NO_ERROR);
    BuildAttributePathIB(attributePathBuilder);

    // Construct attribute data
    {
        chip::TLV::TLVWriter * pWriter = aAttributeDataIBBuilder.GetWriter();
        chip::TLV::TLVType dummyType   = chip::TLV::kTLVType_NotSpecified;
        err = pWriter->StartContainer(chip::TLV::ContextTag(chip::to_underlying(AttributeDataIB::Tag::kData)),
                                      chip::TLV::kTLVType_Structure, dummyType);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = pWriter->PutBoolean(chip::TLV::ContextTag(1), true);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = pWriter->EndContainer(dummyType);
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    err = aAttributeDataIBBuilder.GetError();
    EXPECT_EQ(err, CHIP_NO_ERROR);
    aAttributeDataIBBuilder.EndOfAttributeDataIB();
    EXPECT_EQ(aAttributeDataIBBuilder.GetError(), CHIP_NO_ERROR);
}

void ParseAttributeDataIB(AttributeDataIB::Parser & aAttributeDataIBParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributePathIB::Parser attributePathParser;
    chip::DataVersion version = 0;
#if CHIP_CONFIG_IM_PRETTY_PRINT
    aAttributeDataIBParser.PrettyPrint();
#endif
    err = aAttributeDataIBParser.GetPath(&attributePathParser);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = aAttributeDataIBParser.GetDataVersion(&version);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(version, 2u);

    {
        chip::TLV::TLVReader reader;
        bool val = false;
        chip::TLV::TLVType container;
        aAttributeDataIBParser.GetData(&reader);
        err = reader.EnterContainer(container);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = reader.Next();
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = reader.Get(val);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(val);

        err = reader.ExitContainer(container);
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }
}

void BuildAttributeDataIBs(AttributeDataIBs::Builder & aAttributeDataIBsBuilder)
{
    AttributeDataIB::Builder & attributeDataIBBuilder = aAttributeDataIBsBuilder.CreateAttributeDataIBBuilder();
    EXPECT_EQ(aAttributeDataIBsBuilder.GetError(), CHIP_NO_ERROR);
    BuildAttributeDataIB(attributeDataIBBuilder);

    aAttributeDataIBsBuilder.EndOfAttributeDataIBs();
    EXPECT_EQ(aAttributeDataIBsBuilder.GetError(), CHIP_NO_ERROR);
}

void ParseAttributeDataIBs(chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributeDataIBs::Parser AttributeDataIBsParser;

    err = AttributeDataIBsParser.Init(aReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_PRETTY_PRINT
    AttributeDataIBsParser.PrettyPrint();
#endif
}

void BuildAttributeReportIB(AttributeReportIB::Builder & aAttributeReportIBBuilder)
{
    AttributeDataIB::Builder & attributeDataIBBuilder = aAttributeReportIBBuilder.CreateAttributeData();
    EXPECT_EQ(aAttributeReportIBBuilder.GetError(), CHIP_NO_ERROR);
    BuildAttributeDataIB(attributeDataIBBuilder);

    aAttributeReportIBBuilder.EndOfAttributeReportIB();
    EXPECT_EQ(aAttributeReportIBBuilder.GetError(), CHIP_NO_ERROR);
}

void ParseAttributeReportIB(AttributeReportIB::Parser & aAttributeReportIBParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributeStatusIB::Parser attributeStatusParser;
    AttributeDataIB::Parser attributeDataParser;

#if CHIP_CONFIG_IM_PRETTY_PRINT
    aAttributeReportIBParser.PrettyPrint();
#endif
    err = aAttributeReportIBParser.GetAttributeData(&attributeDataParser);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

void BuildAttributeReportIBs(AttributeReportIBs::Builder & aAttributeReportIBsBuilder)
{
    AttributeReportIB::Builder & attributeReportIBBuilder = aAttributeReportIBsBuilder.CreateAttributeReport();
    EXPECT_EQ(aAttributeReportIBsBuilder.GetError(), CHIP_NO_ERROR);
    BuildAttributeReportIB(attributeReportIBBuilder);

    aAttributeReportIBsBuilder.EndOfAttributeReportIBs();
    EXPECT_EQ(aAttributeReportIBsBuilder.GetError(), CHIP_NO_ERROR);
}

void ParseAttributeReportIBs(chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributeReportIBs::Parser attributeReportIBsParser;

    err = attributeReportIBsParser.Init(aReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_PRETTY_PRINT
    attributeReportIBsParser.PrettyPrint();
#endif
}

void BuildCommandDataIB(CommandDataIB::Builder & aCommandDataIBBuilder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    CommandPathIB::Builder & commandPathBuilder = aCommandDataIBBuilder.CreatePath();
    EXPECT_EQ(aCommandDataIBBuilder.GetError(), CHIP_NO_ERROR);
    BuildCommandPath(commandPathBuilder);

    // Construct command data
    {
        chip::TLV::TLVWriter * pWriter = aCommandDataIBBuilder.GetWriter();
        chip::TLV::TLVType dummyType   = chip::TLV::kTLVType_NotSpecified;
        err = pWriter->StartContainer(chip::TLV::ContextTag(chip::to_underlying(CommandDataIB::Tag::kFields)),
                                      chip::TLV::kTLVType_Structure, dummyType);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = pWriter->PutBoolean(chip::TLV::ContextTag(1), true);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = pWriter->EndContainer(dummyType);
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    aCommandDataIBBuilder.EndOfCommandDataIB();
    EXPECT_EQ(aCommandDataIBBuilder.GetError(), CHIP_NO_ERROR);
}

void ParseCommandDataIB(CommandDataIB::Parser & aCommandDataIBParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandPathIB::Parser commandPathParser;
#if CHIP_CONFIG_IM_PRETTY_PRINT
    aCommandDataIBParser.PrettyPrint();
#endif
    err = aCommandDataIBParser.GetPath(&commandPathParser);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    {
        chip::TLV::TLVReader reader;
        bool val = false;
        chip::TLV::TLVType container;
        aCommandDataIBParser.GetFields(&reader);
        err = reader.EnterContainer(container);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = reader.Next();
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = reader.Get(val);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(val);

        err = reader.ExitContainer(container);
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }
}

void BuildCommandStatusIB(CommandStatusIB::Builder & aCommandStatusIBBuilder)
{
    CommandPathIB::Builder & commandPathBuilder = aCommandStatusIBBuilder.CreatePath();
    EXPECT_EQ(aCommandStatusIBBuilder.GetError(), CHIP_NO_ERROR);
    BuildCommandPath(commandPathBuilder);

    StatusIB::Builder & statusIBBuilder = aCommandStatusIBBuilder.CreateErrorStatus();
    EXPECT_EQ(statusIBBuilder.GetError(), CHIP_NO_ERROR);
    BuildStatusIB(statusIBBuilder);

    aCommandStatusIBBuilder.EndOfCommandStatusIB();
    EXPECT_EQ(aCommandStatusIBBuilder.GetError(), CHIP_NO_ERROR);
}

void ParseCommandStatusIB(CommandStatusIB::Parser & aCommandStatusIBParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandPathIB::Parser commandPathParser;
    StatusIB::Parser statusParser;
#if CHIP_CONFIG_IM_PRETTY_PRINT
    aCommandStatusIBParser.PrettyPrint();
#endif
    err = aCommandStatusIBParser.GetPath(&commandPathParser);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = aCommandStatusIBParser.GetErrorStatus(&statusParser);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

void BuildWrongInvokeResponseIB(InvokeResponseIB::Builder & aInvokeResponseIBBuilder)
{
    aInvokeResponseIBBuilder.CreateCommand();
    EXPECT_EQ(aInvokeResponseIBBuilder.GetError(), CHIP_NO_ERROR);
}

void BuildInvokeResponseIBWithCommandDataIB(InvokeResponseIB::Builder & aInvokeResponseIBBuilder)
{
    CommandDataIB::Builder & commandDataBuilder = aInvokeResponseIBBuilder.CreateCommand();
    EXPECT_EQ(aInvokeResponseIBBuilder.GetError(), CHIP_NO_ERROR);
    BuildCommandDataIB(commandDataBuilder);
    aInvokeResponseIBBuilder.EndOfInvokeResponseIB();
    EXPECT_EQ(aInvokeResponseIBBuilder.GetError(), CHIP_NO_ERROR);
}

void BuildInvokeResponseIBWithCommandStatusIB(InvokeResponseIB::Builder & aInvokeResponseIBBuilder)
{
    CommandStatusIB::Builder & commandStatusBuilder = aInvokeResponseIBBuilder.CreateStatus();
    EXPECT_EQ(aInvokeResponseIBBuilder.GetError(), CHIP_NO_ERROR);
    BuildCommandStatusIB(commandStatusBuilder);
    aInvokeResponseIBBuilder.EndOfInvokeResponseIB();
    EXPECT_EQ(aInvokeResponseIBBuilder.GetError(), CHIP_NO_ERROR);
}

void ParseInvokeResponseIBWithCommandDataIB(InvokeResponseIB::Parser & aInvokeResponseIBParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandDataIB::Parser commandDataParser;
    CommandStatusIB::Parser statusIBParser;
#if CHIP_CONFIG_IM_PRETTY_PRINT
    aInvokeResponseIBParser.PrettyPrint();
#endif
    err = aInvokeResponseIBParser.GetCommand(&commandDataParser);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

void ParseInvokeResponseIBWithCommandStatusIB(InvokeResponseIB::Parser & aInvokeResponseIBParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandDataIB::Parser commandDataParser;
    CommandStatusIB::Parser statusIBParser;
#if CHIP_CONFIG_IM_PRETTY_PRINT
    aInvokeResponseIBParser.PrettyPrint();
#endif
    err = aInvokeResponseIBParser.GetStatus(&statusIBParser);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

void BuildInvokeRequests(InvokeRequests::Builder & aInvokeRequestsBuilder)
{
    CommandDataIB::Builder & aCommandDataIBBuilder = aInvokeRequestsBuilder.CreateCommandData();
    EXPECT_EQ(aInvokeRequestsBuilder.GetError(), CHIP_NO_ERROR);
    BuildCommandDataIB(aCommandDataIBBuilder);

    aInvokeRequestsBuilder.EndOfInvokeRequests();
    EXPECT_EQ(aInvokeRequestsBuilder.GetError(), CHIP_NO_ERROR);
}

void ParseInvokeRequests(chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    InvokeRequests::Parser invokeRequestsParser;
    err = invokeRequestsParser.Init(aReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_PRETTY_PRINT
    invokeRequestsParser.PrettyPrint();
#endif
}

void BuildInvokeResponses(InvokeResponseIBs::Builder & aInvokeResponsesBuilder)
{
    InvokeResponseIB::Builder & invokeResponseIBBuilder = aInvokeResponsesBuilder.CreateInvokeResponse();
    EXPECT_EQ(aInvokeResponsesBuilder.GetError(), CHIP_NO_ERROR);
    BuildInvokeResponseIBWithCommandDataIB(invokeResponseIBBuilder);

    aInvokeResponsesBuilder.EndOfInvokeResponses();
    EXPECT_EQ(aInvokeResponsesBuilder.GetError(), CHIP_NO_ERROR);
}

void ParseInvokeResponses(chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    InvokeResponseIBs::Parser invokeResponsesParser;
    err = invokeResponsesParser.Init(aReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_PRETTY_PRINT
    invokeResponsesParser.PrettyPrint();
#endif
}

void BuildInvokeRequestMessage(chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    InvokeRequestMessage::Builder invokeRequestMessageBuilder;
    err = invokeRequestMessageBuilder.Init(&aWriter);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    invokeRequestMessageBuilder.SuppressResponse(true);
    invokeRequestMessageBuilder.TimedRequest(true);
    InvokeRequests::Builder & invokeRequestsBuilder = invokeRequestMessageBuilder.CreateInvokeRequests();
    EXPECT_EQ(invokeRequestsBuilder.GetError(), CHIP_NO_ERROR);

    BuildInvokeRequests(invokeRequestsBuilder);

    invokeRequestMessageBuilder.EndOfInvokeRequestMessage();
    EXPECT_EQ(invokeRequestMessageBuilder.GetError(), CHIP_NO_ERROR);
}

void ParseInvokeRequestMessage(chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    InvokeRequestMessage::Parser invokeRequestMessageParser;
    err = invokeRequestMessageParser.Init(aReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    bool suppressResponse = false;
    bool timedRequest     = false;
    invokeRequestMessageParser.GetSuppressResponse(&suppressResponse);
    invokeRequestMessageParser.GetTimedRequest(&timedRequest);
    EXPECT_TRUE(suppressResponse);
    EXPECT_TRUE(timedRequest);
#if CHIP_CONFIG_IM_PRETTY_PRINT
    invokeRequestMessageParser.PrettyPrint();
#endif
    EXPECT_EQ(invokeRequestMessageParser.ExitContainer(), CHIP_NO_ERROR);
}

void BuildInvokeResponseMessage(chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    InvokeResponseMessage::Builder invokeResponseMessageBuilder;
    err = invokeResponseMessageBuilder.Init(&aWriter);

    invokeResponseMessageBuilder.SuppressResponse(true);
    InvokeResponseIBs::Builder & invokeResponsesBuilder = invokeResponseMessageBuilder.CreateInvokeResponses();
    EXPECT_EQ(invokeResponseMessageBuilder.GetError(), CHIP_NO_ERROR);

    BuildInvokeResponses(invokeResponsesBuilder);

    invokeResponseMessageBuilder.MoreChunkedMessages(true);
    EXPECT_EQ(invokeResponseMessageBuilder.GetError(), CHIP_NO_ERROR);

    invokeResponseMessageBuilder.EndOfInvokeResponseMessage();
    EXPECT_EQ(invokeResponseMessageBuilder.GetError(), CHIP_NO_ERROR);
}

void ParseInvokeResponseMessage(chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    InvokeResponseMessage::Parser invokeResponseMessageParser;
    err = invokeResponseMessageParser.Init(aReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    bool suppressResponse = false;
    err                   = invokeResponseMessageParser.GetSuppressResponse(&suppressResponse);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(suppressResponse);

    bool moreChunkedMessages = true;
    err                      = invokeResponseMessageParser.GetMoreChunkedMessages(&suppressResponse);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(moreChunkedMessages);

#if CHIP_CONFIG_IM_PRETTY_PRINT
    invokeResponseMessageParser.PrettyPrint();
#endif
    EXPECT_EQ(invokeResponseMessageParser.ExitContainer(), CHIP_NO_ERROR);
}

void BuildReportDataMessage(chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ReportDataMessage::Builder reportDataMessageBuilder;

    err = reportDataMessageBuilder.Init(&aWriter);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    reportDataMessageBuilder.SubscriptionId(2);
    EXPECT_EQ(reportDataMessageBuilder.GetError(), CHIP_NO_ERROR);

    AttributeReportIBs::Builder & attributeReportIBs = reportDataMessageBuilder.CreateAttributeReportIBs();
    EXPECT_EQ(reportDataMessageBuilder.GetError(), CHIP_NO_ERROR);
    BuildAttributeReportIBs(attributeReportIBs);

    EventReportIBs::Builder & eventReportIBs = reportDataMessageBuilder.CreateEventReports();
    EXPECT_EQ(reportDataMessageBuilder.GetError(), CHIP_NO_ERROR);
    BuildEventReports(eventReportIBs);

    reportDataMessageBuilder.MoreChunkedMessages(true).SuppressResponse(true);
    EXPECT_EQ(reportDataMessageBuilder.GetError(), CHIP_NO_ERROR);

    reportDataMessageBuilder.EndOfReportDataMessage();
    EXPECT_EQ(reportDataMessageBuilder.GetError(), CHIP_NO_ERROR);
}

void ParseReportDataMessage(chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ReportDataMessage::Parser reportDataParser;

    bool suppressResponse               = false;
    chip::SubscriptionId subscriptionId = 0;
    AttributeReportIBs::Parser attributeReportIBsParser;
    EventReportIBs::Parser eventReportsParser;
    bool moreChunkedMessages = false;
    reportDataParser.Init(aReader);

#if CHIP_CONFIG_IM_PRETTY_PRINT
    reportDataParser.PrettyPrint();
#endif
    err = reportDataParser.GetSuppressResponse(&suppressResponse);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(suppressResponse);

    err = reportDataParser.GetSubscriptionId(&subscriptionId);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(subscriptionId, 2u);

    err = reportDataParser.GetAttributeReportIBs(&attributeReportIBsParser);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = reportDataParser.GetEventReports(&eventReportsParser);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = reportDataParser.GetMoreChunkedMessages(&moreChunkedMessages);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(moreChunkedMessages);
    EXPECT_EQ(reportDataParser.ExitContainer(), CHIP_NO_ERROR);
}

void BuildReadRequestMessage(chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ReadRequestMessage::Builder readRequestBuilder;

    err = readRequestBuilder.Init(&aWriter);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    AttributePathIBs::Builder & attributePathIBs = readRequestBuilder.CreateAttributeRequests();
    EXPECT_EQ(readRequestBuilder.GetError(), CHIP_NO_ERROR);
    BuildAttributePathList(attributePathIBs);

    EventPathIBs::Builder & eventPathList = readRequestBuilder.CreateEventRequests();

    EXPECT_EQ(readRequestBuilder.GetError(), CHIP_NO_ERROR);
    BuildEventPaths(eventPathList);

    EventFilterIBs::Builder & eventFilters = readRequestBuilder.CreateEventFilters();
    EXPECT_EQ(readRequestBuilder.GetError(), CHIP_NO_ERROR);
    BuildEventFilters(eventFilters);

    readRequestBuilder.IsFabricFiltered(true);
    EXPECT_EQ(readRequestBuilder.GetError(), CHIP_NO_ERROR);

    DataVersionFilterIBs::Builder & dataVersionFilters = readRequestBuilder.CreateDataVersionFilters();
    EXPECT_EQ(readRequestBuilder.GetError(), CHIP_NO_ERROR);
    BuildDataVersionFilterIBs(dataVersionFilters);

    readRequestBuilder.EndOfReadRequestMessage();
    EXPECT_EQ(readRequestBuilder.GetError(), CHIP_NO_ERROR);
}

void ParseReadRequestMessage(chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ReadRequestMessage::Parser readRequestParser;
    AttributePathIBs::Parser attributePathListParser;
    DataVersionFilterIBs::Parser dataVersionFilterIBsParser;
    EventPathIBs::Parser eventPathListParser;
    EventFilterIBs::Parser eventFiltersParser;
    bool isFabricFiltered = false;

    err = readRequestParser.Init(aReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_PRETTY_PRINT
    readRequestParser.PrettyPrint();
#endif
    err = readRequestParser.GetAttributeRequests(&attributePathListParser);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = readRequestParser.GetDataVersionFilters(&dataVersionFilterIBsParser);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = readRequestParser.GetEventRequests(&eventPathListParser);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = readRequestParser.GetEventFilters(&eventFiltersParser);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = readRequestParser.GetIsFabricFiltered(&isFabricFiltered);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(isFabricFiltered);
    EXPECT_EQ(readRequestParser.ExitContainer(), CHIP_NO_ERROR);
}

void BuildWriteRequestMessage(chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    WriteRequestMessage::Builder writeRequestBuilder;

    err = writeRequestBuilder.Init(&aWriter);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    writeRequestBuilder.SuppressResponse(true);
    EXPECT_EQ(writeRequestBuilder.GetError(), CHIP_NO_ERROR);

    writeRequestBuilder.TimedRequest(true);
    EXPECT_EQ(writeRequestBuilder.GetError(), CHIP_NO_ERROR);

    AttributeDataIBs::Builder & attributeDataIBs = writeRequestBuilder.CreateWriteRequests();
    EXPECT_EQ(writeRequestBuilder.GetError(), CHIP_NO_ERROR);
    BuildAttributeDataIBs(attributeDataIBs);

    writeRequestBuilder.MoreChunkedMessages(true);
    EXPECT_EQ(writeRequestBuilder.GetError(), CHIP_NO_ERROR);

    writeRequestBuilder.EndOfWriteRequestMessage();
    EXPECT_EQ(writeRequestBuilder.GetError(), CHIP_NO_ERROR);
}

void ParseWriteRequestMessage(chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    WriteRequestMessage::Parser writeRequestParser;
    bool suppressResponse = false;
    bool timeRequest      = false;
    AttributeDataIBs::Parser writeRequests;
    bool moreChunkedMessages = false;

    err = writeRequestParser.Init(aReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_PRETTY_PRINT
    writeRequestParser.PrettyPrint();
#endif
    err = writeRequestParser.GetSuppressResponse(&suppressResponse);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(suppressResponse);

    err = writeRequestParser.GetTimedRequest(&timeRequest);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(timeRequest);

    err = writeRequestParser.GetWriteRequests(&writeRequests);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writeRequestParser.GetMoreChunkedMessages(&moreChunkedMessages);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(moreChunkedMessages);
}

void BuildWriteResponseMessage(chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    WriteResponseMessage::Builder writeResponseBuilder;

    err = writeResponseBuilder.Init(&aWriter);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    AttributeStatusIBs::Builder & attributeStatuses = writeResponseBuilder.CreateWriteResponses();
    EXPECT_EQ(writeResponseBuilder.GetError(), CHIP_NO_ERROR);
    BuildAttributeStatuses(attributeStatuses);

    writeResponseBuilder.EndOfWriteResponseMessage();
    EXPECT_EQ(writeResponseBuilder.GetError(), CHIP_NO_ERROR);
}

void ParseWriteResponseMessage(chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    WriteResponseMessage::Parser writeResponseParser;
    AttributeStatusIBs::Parser attributeStatusesParser;
    err = writeResponseParser.Init(aReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_PRETTY_PRINT
    writeResponseParser.PrettyPrint();
#endif
    err = writeResponseParser.GetWriteResponses(&attributeStatusesParser);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(writeResponseParser.ExitContainer(), CHIP_NO_ERROR);
}

void BuildSubscribeRequestMessage(chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    SubscribeRequestMessage::Builder subscribeRequestBuilder;

    err = subscribeRequestBuilder.Init(&aWriter);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    subscribeRequestBuilder.KeepSubscriptions(true);
    EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

    subscribeRequestBuilder.MinIntervalFloorSeconds(2);
    EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

    subscribeRequestBuilder.MaxIntervalCeilingSeconds(3);
    EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

    AttributePathIBs::Builder & attributePathIBs = subscribeRequestBuilder.CreateAttributeRequests();
    EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);
    BuildAttributePathList(attributePathIBs);

    EventPathIBs::Builder & eventPathList = subscribeRequestBuilder.CreateEventRequests();

    EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);
    BuildEventPaths(eventPathList);

    EventFilterIBs::Builder & eventFilters = subscribeRequestBuilder.CreateEventFilters();
    EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);
    BuildEventFilters(eventFilters);

    subscribeRequestBuilder.IsFabricFiltered(true);
    EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

    DataVersionFilterIBs::Builder & dataVersionFilters = subscribeRequestBuilder.CreateDataVersionFilters();
    EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);
    BuildDataVersionFilterIBs(dataVersionFilters);

    subscribeRequestBuilder.EndOfSubscribeRequestMessage();
    EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);
}

void ParseSubscribeRequestMessage(chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    SubscribeRequestMessage::Parser subscribeRequestParser;
    AttributePathIBs::Parser attributePathListParser;
    DataVersionFilterIBs::Parser dataVersionFilterIBsParser;
    EventPathIBs::Parser eventPathListParser;
    EventFilterIBs::Parser eventFiltersParser;
    uint16_t minIntervalFloorSeconds   = 0;
    uint16_t maxIntervalCeilingSeconds = 0;
    bool keepExistingSubscription      = false;
    bool isFabricFiltered              = false;

    err = subscribeRequestParser.Init(aReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_PRETTY_PRINT
    subscribeRequestParser.PrettyPrint();
#endif
    err = subscribeRequestParser.GetAttributeRequests(&attributePathListParser);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = subscribeRequestParser.GetDataVersionFilters(&dataVersionFilterIBsParser);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = subscribeRequestParser.GetEventRequests(&eventPathListParser);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = subscribeRequestParser.GetEventFilters(&eventFiltersParser);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = subscribeRequestParser.GetMinIntervalFloorSeconds(&minIntervalFloorSeconds);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(minIntervalFloorSeconds, 2u);

    err = subscribeRequestParser.GetMaxIntervalCeilingSeconds(&maxIntervalCeilingSeconds);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(maxIntervalCeilingSeconds, 3u);

    err = subscribeRequestParser.GetKeepSubscriptions(&keepExistingSubscription);
    EXPECT_TRUE(keepExistingSubscription);

    err = subscribeRequestParser.GetIsFabricFiltered(&isFabricFiltered);
    EXPECT_TRUE(isFabricFiltered);
    EXPECT_EQ(subscribeRequestParser.ExitContainer(), CHIP_NO_ERROR);
}

void BuildSubscribeResponseMessage(chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    SubscribeResponseMessage::Builder subscribeResponseBuilder;

    err = subscribeResponseBuilder.Init(&aWriter);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    subscribeResponseBuilder.SubscriptionId(1);
    EXPECT_EQ(subscribeResponseBuilder.GetError(), CHIP_NO_ERROR);

    subscribeResponseBuilder.MaxInterval(2);
    EXPECT_EQ(subscribeResponseBuilder.GetError(), CHIP_NO_ERROR);

    subscribeResponseBuilder.EndOfSubscribeResponseMessage();
    EXPECT_EQ(subscribeResponseBuilder.GetError(), CHIP_NO_ERROR);
}

void ParseSubscribeResponseMessage(chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    SubscribeResponseMessage::Parser subscribeResponseParser;
    chip::SubscriptionId subscriptionId = 0;
    uint16_t maxInterval                = 0;
    err                                 = subscribeResponseParser.Init(aReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_PRETTY_PRINT
    subscribeResponseParser.PrettyPrint();
#endif
    err = subscribeResponseParser.GetSubscriptionId(&subscriptionId);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(subscriptionId, 1u);

    err = subscribeResponseParser.GetMaxInterval(&maxInterval);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(maxInterval, 2u);

    EXPECT_EQ(subscribeResponseParser.ExitContainer(), CHIP_NO_ERROR);
}

void BuildTimedRequestMessage(chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TimedRequestMessage::Builder TimedRequestMessageBuilder;

    err = TimedRequestMessageBuilder.Init(&aWriter);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    TimedRequestMessageBuilder.TimeoutMs(1);
    EXPECT_EQ(TimedRequestMessageBuilder.GetError(), CHIP_NO_ERROR);
}

void ParseTimedRequestMessage(chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    TimedRequestMessage::Parser timedRequestMessageParser;
    uint16_t timeout = 0;

    err = timedRequestMessageParser.Init(aReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_PRETTY_PRINT
    timedRequestMessageParser.PrettyPrint();
#endif
    err = timedRequestMessageParser.GetTimeoutMs(&timeout);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_EQ(timeout, 1u);

    EXPECT_EQ(timedRequestMessageParser.ExitContainer(), CHIP_NO_ERROR);
}

TEST_F(TestMessageDef, TestDataVersionFilterIB)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    DataVersionFilterIB::Builder dataVersionFilterIBBuilder;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    dataVersionFilterIBBuilder.Init(&writer);
    BuildDataVersionFilterIB(dataVersionFilterIBBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);
    ParseDataVersionFilterIB(reader);
}

TEST_F(TestMessageDef, TestDataVersionFilterIBs)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    DataVersionFilterIBs::Builder dataVersionFilterIBsBuilder;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));

    err = dataVersionFilterIBsBuilder.Init(&writer);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    BuildDataVersionFilterIBs(dataVersionFilterIBsBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);
    ParseDataVersionFilterIBs(reader);
}

TEST_F(TestMessageDef, TestEventFilter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventFilterIB::Builder eventFilterBuilder;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    eventFilterBuilder.Init(&writer);
    BuildEventFilterIB(eventFilterBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);
    ParseEventFilterIB(reader);
}

TEST_F(TestMessageDef, TestEventFilters)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    EventFilterIBs::Builder eventFiltersBuilder;
    EventFilterIBs::Parser eventFiltersParser;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));

    err = eventFiltersBuilder.Init(&writer);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    BuildEventFilters(eventFiltersBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);
    ParseEventFilters(reader);
}

TEST_F(TestMessageDef, TestClusterPathIB)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ClusterPathIB::Builder clusterPathBuilder;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    clusterPathBuilder.Init(&writer);
    BuildClusterPathIB(clusterPathBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    ParseClusterPathIB(reader);
}

TEST_F(TestMessageDef, TestAttributePath)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributePathIB::Builder attributePathBuilder;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    attributePathBuilder.Init(&writer);
    BuildAttributePathIB(attributePathBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    ParseAttributePathIB(reader);
}

TEST_F(TestMessageDef, TestAttributePathList)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    AttributePathIBs::Builder attributePathListBuilder;

    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));

    err = attributePathListBuilder.Init(&writer);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    BuildAttributePathList(attributePathListBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);
    ParseAttributePathList(reader);
}

TEST_F(TestMessageDef, TestEventPath)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventPathIB::Parser eventPathParser;
    EventPathIB::Builder eventPathBuilder;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    eventPathBuilder.Init(&writer);
    BuildEventPath(eventPathBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    eventPathParser.Init(reader);
    ParseEventPath(eventPathParser);
}

TEST_F(TestMessageDef, TestEventPaths)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    EventPathIBs::Builder eventPathListBuilder;

    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));

    err = eventPathListBuilder.Init(&writer);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    BuildEventPaths(eventPathListBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);
    ParseEventPaths(reader);
}

TEST_F(TestMessageDef, TestCommandPathIB)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    CommandPathIB::Builder commandPathBuilder;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    err = commandPathBuilder.Init(&writer);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    BuildCommandPath(commandPathBuilder);

    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    ParseCommandPath(reader);
}

TEST_F(TestMessageDef, TestEventDataIB)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventDataIB::Builder eventDataIBBuilder;
    EventDataIB::Parser eventDataIBParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    eventDataIBBuilder.Init(&writer);
    BuildEventDataIB(eventDataIBBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    eventDataIBParser.Init(reader);
    ParseEventDataIB(eventDataIBParser);
}

TEST_F(TestMessageDef, TestEventReportIB)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventReportIB::Builder eventReportIBBuilder;
    EventReportIB::Parser eventReportIBParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    eventReportIBBuilder.Init(&writer);
    BuildEventReportIB(eventReportIBBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    eventReportIBParser.Init(reader);
    ParseEventReportIB(eventReportIBParser);
}

TEST_F(TestMessageDef, TestEventReports)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    EventReportIBs::Builder eventReportsBuilder;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    eventReportsBuilder.Init(&writer);
    BuildEventReports(eventReportsBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);
    ParseEventReports(reader);
}

TEST_F(TestMessageDef, TestEmptyEventReports)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    EventReportIBs::Builder eventReportsBuilder;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    eventReportsBuilder.Init(&writer);
    eventReportsBuilder.EndOfEventReports();
    EXPECT_EQ(eventReportsBuilder.GetError(), CHIP_NO_ERROR);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);
    ParseEventReports(reader);
}

TEST_F(TestMessageDef, TestAttributeReportIB)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributeReportIB::Builder attributeReportIBBuilder;
    AttributeReportIB::Parser attributeReportIBParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    attributeReportIBBuilder.Init(&writer);
    BuildAttributeReportIB(attributeReportIBBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    attributeReportIBParser.Init(reader);
    ParseAttributeReportIB(attributeReportIBParser);
}

TEST_F(TestMessageDef, TestAttributeReportIBs)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    AttributeReportIBs::Builder attributeReportIBsBuilder;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    attributeReportIBsBuilder.Init(&writer);
    BuildAttributeReportIBs(attributeReportIBsBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);
    ParseAttributeReportIBs(reader);
}

TEST_F(TestMessageDef, TestEmptyAttributeReportIBs)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    AttributeReportIBs::Builder attributeReportIBsBuilder;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    attributeReportIBsBuilder.Init(&writer);
    attributeReportIBsBuilder.EndOfAttributeReportIBs();
    EXPECT_EQ(attributeReportIBsBuilder.GetError(), CHIP_NO_ERROR);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);
    ParseAttributeReportIBs(reader);
}

TEST_F(TestMessageDef, TestStatusIB)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    StatusIB::Builder statusIBBuilder;
    StatusIB::Parser StatusIBParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    statusIBBuilder.Init(&writer);
    BuildStatusIB(statusIBBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    StatusIBParser.Init(reader);
    ParseStatusIB(StatusIBParser);
}

TEST_F(TestMessageDef, TestEventStatusIB)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventStatusIB::Builder eventStatusIBBuilder;
    EventStatusIB::Parser eventStatusIBParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    eventStatusIBBuilder.Init(&writer);
    BuildEventStatusIB(eventStatusIBBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    eventStatusIBParser.Init(reader);
    ParseEventStatusIB(eventStatusIBParser);
}

TEST_F(TestMessageDef, TestAttributeStatusIB)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributeStatusIB::Builder attributeStatusIBBuilder;
    AttributeStatusIB::Parser attributeStatusIBParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    attributeStatusIBBuilder.Init(&writer);
    BuildAttributeStatusIB(attributeStatusIBBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    attributeStatusIBParser.Init(reader);
    ParseAttributeStatusIB(attributeStatusIBParser);
}

TEST_F(TestMessageDef, TestAttributeStatuses)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    AttributeStatusIBs::Builder attributeStatusesBuilder;
    err = attributeStatusesBuilder.Init(&writer);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    BuildAttributeStatuses(attributeStatusesBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);
    ParseAttributeStatuses(reader);
}

TEST_F(TestMessageDef, TestAttributeDataIB)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributeDataIB::Builder AttributeDataIBBuilder;
    AttributeDataIB::Parser AttributeDataIBParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    AttributeDataIBBuilder.Init(&writer);
    BuildAttributeDataIB(AttributeDataIBBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    AttributeDataIBParser.Init(reader);
    ParseAttributeDataIB(AttributeDataIBParser);
}

TEST_F(TestMessageDef, TestAttributeDataIBs)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    AttributeDataIBs::Builder AttributeDataIBsBuilder;
    AttributeDataIBsBuilder.Init(&writer);
    BuildAttributeDataIBs(AttributeDataIBsBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);
    ParseAttributeDataIBs(reader);
}

TEST_F(TestMessageDef, TestCommandDataIB)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandDataIB::Builder commandDataIBBuilder;
    CommandDataIB::Parser commandDataIBParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    commandDataIBBuilder.Init(&writer);
    BuildCommandDataIB(commandDataIBBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    commandDataIBParser.Init(reader);
    ParseCommandDataIB(commandDataIBParser);
}

TEST_F(TestMessageDef, TestCommandStatusIB)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandStatusIB::Builder commandStatusIBBuilder;
    CommandStatusIB::Parser commandStatusIBParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    commandStatusIBBuilder.Init(&writer);
    BuildCommandStatusIB(commandStatusIBBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    commandStatusIBParser.Init(reader);
    ParseCommandStatusIB(commandStatusIBParser);
}

TEST_F(TestMessageDef, TestInvokeResponseIBWithCommandDataIB)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    InvokeResponseIB::Builder invokeResponseIBBuilder;
    InvokeResponseIB::Parser invokeResponseIBParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    invokeResponseIBBuilder.Init(&writer);
    BuildInvokeResponseIBWithCommandDataIB(invokeResponseIBBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    invokeResponseIBParser.Init(reader);
    ParseInvokeResponseIBWithCommandDataIB(invokeResponseIBParser);
}

TEST_F(TestMessageDef, TestInvokeResponseIBWithCommandStatusIB)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    InvokeResponseIB::Builder invokeResponseIBBuilder;
    InvokeResponseIB::Parser invokeResponseIBParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    invokeResponseIBBuilder.Init(&writer);
    BuildInvokeResponseIBWithCommandStatusIB(invokeResponseIBBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    invokeResponseIBParser.Init(reader);
    ParseInvokeResponseIBWithCommandStatusIB(invokeResponseIBParser);
}

TEST_F(TestMessageDef, TestInvokeResponseIBWithMalformData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    InvokeResponseIB::Builder invokeResponseIBBuilder;
    InvokeResponseIB::Parser invokeResponseIBParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    invokeResponseIBBuilder.Init(&writer);
    BuildWrongInvokeResponseIB(invokeResponseIBBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = invokeResponseIBParser.Init(reader);
    EXPECT_NE(err, CHIP_NO_ERROR);
}

TEST_F(TestMessageDef, TestInvokeRequests)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    InvokeRequests::Builder invokeRequestsBuilder;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    invokeRequestsBuilder.Init(&writer);
    BuildInvokeRequests(invokeRequestsBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);
    ParseInvokeRequests(reader);
}

TEST_F(TestMessageDef, TestInvokeResponses)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    InvokeResponseIBs::Builder invokeResponsesBuilder;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    invokeResponsesBuilder.Init(&writer);
    BuildInvokeResponses(invokeResponsesBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);
    ParseInvokeResponses(reader);
}

TEST_F(TestMessageDef, TestInvokeInvokeRequestMessage)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildInvokeRequestMessage(writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    ParseInvokeRequestMessage(reader);
}

TEST_F(TestMessageDef, TestInvokeRequestMessageEndOfMessageReservation)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    InvokeRequestMessage::Builder invokeRequestMessageBuilder;
    const uint32_t kSmallBufferSize = 100;
    writer.Init(chip::System::PacketBufferHandle::New(kSmallBufferSize, /* aReservedSize = */ 0), /* useChainedBuffers = */ false);
    err = invokeRequestMessageBuilder.InitWithEndBufferReserved(&writer);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    uint32_t remainingLengthAfterInitWithReservation = writer.GetRemainingFreeLength();

    err = invokeRequestMessageBuilder.EndOfInvokeRequestMessage();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    uint32_t remainingLengthAfterEndingInvokeRequestMessage = writer.GetRemainingFreeLength();
    EXPECT_EQ(remainingLengthAfterInitWithReservation, remainingLengthAfterEndingInvokeRequestMessage);
}

TEST_F(TestMessageDef, TestInvokeRequestsEndOfRequestReservation)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    InvokeRequestMessage::Builder invokeRequestMessageBuilder;
    const uint32_t kSmallBufferSize = 100;
    writer.Init(chip::System::PacketBufferHandle::New(kSmallBufferSize, /* aReservedSize = */ 0), /* useChainedBuffers = */ false);
    err = invokeRequestMessageBuilder.InitWithEndBufferReserved(&writer);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    invokeRequestMessageBuilder.CreateInvokeRequests(/* aReserveEndBuffer = */ true);
    InvokeRequests::Builder & invokeRequestsBuilder = invokeRequestMessageBuilder.GetInvokeRequests();
    err                                             = invokeRequestsBuilder.GetError();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    auto * invokeRequestsWriter                      = invokeRequestsBuilder.GetWriter();
    uint32_t remainingLengthAfterInitWithReservation = invokeRequestsWriter->GetRemainingFreeLength();

    err = invokeRequestsBuilder.EndOfInvokeRequests();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    uint32_t remainingLengthAfterEndingInvokeRequests = invokeRequestsWriter->GetRemainingFreeLength();
    EXPECT_EQ(remainingLengthAfterInitWithReservation, remainingLengthAfterEndingInvokeRequests);
}

TEST_F(TestMessageDef, TestInvokeInvokeResponseMessage)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildInvokeResponseMessage(writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    ParseInvokeResponseMessage(reader);
}

TEST_F(TestMessageDef, TestInvokeResponseMessageEndOfMessageReservation)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    InvokeResponseMessage::Builder invokeResponseMessageBuilder;
    const uint32_t kSmallBufferSize = 100;
    writer.Init(chip::System::PacketBufferHandle::New(kSmallBufferSize, /* aReservedSize = */ 0), /* useChainedBuffers = */ false);
    err = invokeResponseMessageBuilder.InitWithEndBufferReserved(&writer);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    uint32_t remainingLengthAfterInitWithReservation = writer.GetRemainingFreeLength();
    err                                              = invokeResponseMessageBuilder.EndOfInvokeResponseMessage();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    uint32_t remainingLengthAfterEndingInvokeResponseMessage = writer.GetRemainingFreeLength();
    EXPECT_EQ(remainingLengthAfterInitWithReservation, remainingLengthAfterEndingInvokeResponseMessage);
}

TEST_F(TestMessageDef, TestInvokeResponseMessageReservationForEndandMoreChunk)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    InvokeResponseMessage::Builder invokeResponseMessageBuilder;
    const uint32_t kSmallBufferSize = 100;
    writer.Init(chip::System::PacketBufferHandle::New(kSmallBufferSize, /* aReservedSize = */ 0), /* useChainedBuffers = */ false);
    err = invokeResponseMessageBuilder.InitWithEndBufferReserved(&writer);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    err = invokeResponseMessageBuilder.ReserveSpaceForMoreChunkedMessages();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    uint32_t remainingLengthAllReservations = writer.GetRemainingFreeLength();

    invokeResponseMessageBuilder.MoreChunkedMessages(/* aMoreChunkedMessages = */ true);
    EXPECT_EQ(invokeResponseMessageBuilder.GetError(), CHIP_NO_ERROR);
    err = invokeResponseMessageBuilder.EndOfInvokeResponseMessage();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    uint32_t remainingLengthAfterEndingInvokeResponseMessage = writer.GetRemainingFreeLength();
    EXPECT_EQ(remainingLengthAllReservations, remainingLengthAfterEndingInvokeResponseMessage);
}

TEST_F(TestMessageDef, TestInvokeResponsesEndOfResponseReservation)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    InvokeResponseMessage::Builder invokeResponseMessageBuilder;
    const uint32_t kSmallBufferSize = 100;
    writer.Init(chip::System::PacketBufferHandle::New(kSmallBufferSize, /* aReservedSize = */ 0), /* useChainedBuffers = */ false);
    err = invokeResponseMessageBuilder.InitWithEndBufferReserved(&writer);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    invokeResponseMessageBuilder.CreateInvokeResponses(/* aReserveEndBuffer = */ true);
    InvokeResponseIBs::Builder & invokeResponsesBuilder = invokeResponseMessageBuilder.GetInvokeResponses();
    err                                                 = invokeResponsesBuilder.GetError();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    auto * invokeResponsesWriter                     = invokeResponsesBuilder.GetWriter();
    uint32_t remainingLengthAfterInitWithReservation = invokeResponsesWriter->GetRemainingFreeLength();
    err                                              = invokeResponsesBuilder.EndOfInvokeResponses();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    uint32_t remainingLengthAfterEndingInvokeResponses = invokeResponsesWriter->GetRemainingFreeLength();
    EXPECT_EQ(remainingLengthAfterInitWithReservation, remainingLengthAfterEndingInvokeResponses);
}

TEST_F(TestMessageDef, TestReportDataMessage)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildReportDataMessage(writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    ParseReportDataMessage(reader);
}

TEST_F(TestMessageDef, TestReadRequestMessage)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildReadRequestMessage(writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    ParseReadRequestMessage(reader);
}

TEST_F(TestMessageDef, TestWriteRequestMessage)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildWriteRequestMessage(writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    ParseWriteRequestMessage(reader);
}

TEST_F(TestMessageDef, TestWriteResponseMessage)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildWriteResponseMessage(writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    ParseWriteResponseMessage(reader);
}

TEST_F(TestMessageDef, TestSubscribeRequestMessage)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildSubscribeRequestMessage(writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    ParseSubscribeRequestMessage(reader);
}

TEST_F(TestMessageDef, TestSubscribeResponseMessage)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildSubscribeResponseMessage(writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    ParseSubscribeResponseMessage(reader);
}

TEST_F(TestMessageDef, TestTimedRequestMessage)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildTimedRequestMessage(writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    ParseTimedRequestMessage(reader);
}

TEST_F(TestMessageDef, TestCheckPointRollback)
{
    CHIP_ERROR err        = CHIP_NO_ERROR;
    size_t NumDataElement = 0;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    AttributeDataIBs::Parser AttributeDataIBsParser;
    chip::TLV::TLVWriter checkpoint;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    AttributeDataIBs::Builder attributeDataIBsBuilder;
    attributeDataIBsBuilder.Init(&writer);

    // encode one attribute element
    AttributeDataIB::Builder & attributeDataIBBuilder1 = attributeDataIBsBuilder.CreateAttributeDataIBBuilder();
    EXPECT_EQ(attributeDataIBsBuilder.GetError(), CHIP_NO_ERROR);
    BuildAttributeDataIB(attributeDataIBBuilder1);
    // checkpoint
    attributeDataIBsBuilder.Checkpoint(checkpoint);
    // encode another attribute element
    AttributeDataIB::Builder & attributeDataIBBuilder2 = attributeDataIBsBuilder.CreateAttributeDataIBBuilder();
    EXPECT_EQ(attributeDataIBsBuilder.GetError(), CHIP_NO_ERROR);
    BuildAttributeDataIB(attributeDataIBBuilder2);
    // rollback to previous checkpoint
    attributeDataIBsBuilder.Rollback(checkpoint);

    attributeDataIBsBuilder.EndOfAttributeDataIBs();
    EXPECT_EQ(attributeDataIBsBuilder.GetError(), CHIP_NO_ERROR);

    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = AttributeDataIBsParser.Init(reader);
    EXPECT_EQ(err, CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_PRETTY_PRINT
    AttributeDataIBsParser.PrettyPrint();
#endif
    while (CHIP_NO_ERROR == (err = AttributeDataIBsParser.Next()))
    {
        ++NumDataElement;
    }

    EXPECT_EQ(NumDataElement, 1u);
}

} // namespace
