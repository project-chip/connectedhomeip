/*
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <pw_unit_test/framework.h>

#include <app/CommandHandler.h>
#include <app/clusters/webrtc-transport-provider-server/WebRTCTransportProviderCluster.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model/Decode.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/WebRTCTransportProvider/Attributes.h>
#include <clusters/WebRTCTransportProvider/Commands.h>
#include <clusters/WebRTCTransportProvider/Enums.h>
#include <clusters/WebRTCTransportProvider/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <protocols/interaction_model/StatusCode.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WebRTCTransportProvider;
using namespace chip::Testing;

using chip::app::ClusterShutdownType;

using ICEServerDecodableStruct = Clusters::Globals::Structs::ICEServerStruct::DecodableType;
using WebRTCSessionStruct      = Clusters::Globals::Structs::WebRTCSessionStruct::Type;
using ICECandidateStruct       = Clusters::Globals::Structs::ICECandidateStruct::Type;
using StreamUsageEnum          = Clusters::Globals::StreamUsageEnum;
using WebRTCEndReasonEnum      = Clusters::Globals::WebRTCEndReasonEnum;

static constexpr EndpointId kTestEndpointId = 1;

// Mock delegate for testing WebRTCTransportProvider
class MockWebRTCTransportProviderDelegate : public Delegate
{
public:
    MockWebRTCTransportProviderDelegate() = default;

    CHIP_ERROR HandleSolicitOffer(const OfferRequestArgs & args, WebRTCSessionStruct & outSession, bool & outDeferredOffer) override
    {
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR HandleProvideOffer(const ProvideOfferRequestArgs & args, WebRTCSessionStruct & outSession) override
    {
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR HandleProvideAnswer(uint16_t sessionId, const std::string & sdpAnswer) override { return CHIP_NO_ERROR; }

    CHIP_ERROR HandleProvideICECandidates(uint16_t sessionId, const std::vector<ICECandidateStruct> & candidates) override
    {
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR HandleEndSession(uint16_t sessionId, WebRTCEndReasonEnum reasonCode) override { return CHIP_NO_ERROR; }

    CHIP_ERROR ValidateStreamUsage(StreamUsageEnum streamUsage, Optional<std::vector<uint16_t>> & videoStreams,
                                   Optional<std::vector<uint16_t>> & audioStreams) override
    {
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ValidateVideoStreamID(uint16_t videoStreamId) override { return CHIP_NO_ERROR; }

    CHIP_ERROR ValidateAudioStreamID(uint16_t audioStreamId) override { return CHIP_NO_ERROR; }

    CHIP_ERROR ValidateVideoStreams(const std::vector<uint16_t> & videoStreams) override { return CHIP_NO_ERROR; }

    CHIP_ERROR ValidateAudioStreams(const std::vector<uint16_t> & audioStreams) override { return CHIP_NO_ERROR; }

    CHIP_ERROR IsStreamUsageSupported(Globals::StreamUsageEnum streamUsage) override { return CHIP_NO_ERROR; }

    CHIP_ERROR IsHardPrivacyModeActive(bool & isActive) override
    {
        isActive = false;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR IsSoftRecordingPrivacyModeActive(bool & isActive) override
    {
        isActive = false;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR IsSoftLivestreamPrivacyModeActive(bool & isActive) override
    {
        isActive = false;
        return CHIP_NO_ERROR;
    }

    bool HasAllocatedVideoStreams() override { return true; }

    bool HasAllocatedAudioStreams() override { return true; }

    CHIP_ERROR ValidateSFrameConfig(uint16_t cipherSuite, size_t baseKeyLength) override { return CHIP_NO_ERROR; }

    CHIP_ERROR IsUTCTimeNull(bool & isNull) override
    {
        isNull = false;
        return CHIP_NO_ERROR;
    }
};

// initialize memory as ReadOnlyBufferBuilder may allocate
struct TestWebRTCTransportProviderCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }
};

TEST_F(TestWebRTCTransportProviderCluster, TestAttributes)
{
    MockWebRTCTransportProviderDelegate mockDelegate;
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate);

    ASSERT_TRUE(IsAttributesListEqualTo(server, { WebRTCTransportProvider::Attributes::CurrentSessions::kMetadataEntry }));
}

TEST_F(TestWebRTCTransportProviderCluster, TestCommands)
{
    MockWebRTCTransportProviderDelegate mockDelegate;
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate);

    ASSERT_TRUE(IsAcceptedCommandsListEqualTo(server,
                                              {
                                                  WebRTCTransportProvider::Commands::SolicitOffer::kMetadataEntry,
                                                  WebRTCTransportProvider::Commands::ProvideOffer::kMetadataEntry,
                                                  WebRTCTransportProvider::Commands::ProvideAnswer::kMetadataEntry,
                                                  WebRTCTransportProvider::Commands::ProvideICECandidates::kMetadataEntry,
                                                  WebRTCTransportProvider::Commands::EndSession::kMetadataEntry,
                                              }));
}

TEST_F(TestWebRTCTransportProviderCluster, TestCurrentSessionsAttribute)
{
    MockWebRTCTransportProviderDelegate mockDelegate;
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate);

    // Initially, no sessions should exist
    auto sessions = server.GetCurrentSessions();
    EXPECT_TRUE(sessions.empty());
}

TEST_F(TestWebRTCTransportProviderCluster, TestSessionManagement)
{
    MockWebRTCTransportProviderDelegate mockDelegate;
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate);

    // Verify initial state
    auto sessions = server.GetCurrentSessions();
    EXPECT_TRUE(sessions.empty());

    // Test that RemoveSession on non-existent session is safe
    server.RemoveSession(999);
    sessions = server.GetCurrentSessions();
    EXPECT_TRUE(sessions.empty());
}

TEST_F(TestWebRTCTransportProviderCluster, TestReadCurrentSessionsAttribute)
{
    TestServerClusterContext context;
    MockWebRTCTransportProviderDelegate mockDelegate;
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate);
    ASSERT_EQ(server.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(server);

    // Test reading empty sessions
    WebRTCTransportProvider::Attributes::CurrentSessions::TypeInfo::DecodableType sessions;
    auto status = tester.ReadAttribute(WebRTCTransportProvider::Attributes::CurrentSessions::Id, sessions);
    EXPECT_TRUE(status.IsSuccess());

    auto iter = sessions.begin();
    EXPECT_FALSE(iter.Next()); // Should be empty

    server.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWebRTCTransportProviderCluster, TestReadClusterRevisionAttribute)
{
    TestServerClusterContext context;
    MockWebRTCTransportProviderDelegate mockDelegate;
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate);
    ASSERT_EQ(server.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(server);

    // Test reading cluster revision
    Globals::Attributes::ClusterRevision::TypeInfo::DecodableType clusterRevision = 0;
    auto status = tester.ReadAttribute(Globals::Attributes::ClusterRevision::Id, clusterRevision);
    EXPECT_TRUE(status.IsSuccess());
    EXPECT_EQ(clusterRevision, WebRTCTransportProvider::kRevision);

    server.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWebRTCTransportProviderCluster, TestReadUnsupportedAttribute)
{
    TestServerClusterContext context;
    MockWebRTCTransportProviderDelegate mockDelegate;
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate);
    ASSERT_EQ(server.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(server);

    // Test reading unsupported attribute
    uint32_t dummyValue;
    auto status = tester.ReadAttribute(0xFFFF /* Invalid attribute ID */, dummyValue);
    EXPECT_FALSE(status.IsSuccess());
    EXPECT_EQ(status.GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::UnsupportedAttribute);

    server.Shutdown(ClusterShutdownType::kClusterShutdown);
}

constexpr FabricIndex kCandidateFabric      = 1;
constexpr NodeId kCandidatePeer             = 0x12345;
constexpr uint16_t kCandidateVideoStreams[] = { 1 };
constexpr CharSpan kTestCandidate           = "candidate:1 1 UDP 2130706431 192.0.2.1 5000 typ host"_span;

/** A delegate that creates an owned session and records candidate delivery without retaining decoded spans. */
class CandidateRecordingDelegate : public MockWebRTCTransportProviderDelegate
{
public:
    /** Populate a session from the validated offer arguments and stable fixture stream storage. */
    CHIP_ERROR HandleProvideOffer(const ProvideOfferRequestArgs & args, WebRTCSessionStruct & outSession) override
    {
        outSession.id         = args.sessionId;
        outSession.peerNodeID = args.peerNodeId;
        outSession.SetFabricIndex(args.fabricIndex);
        outSession.peerEndpointID = args.originatingEndpointId;
        outSession.streamUsage    = args.streamUsage;
        outSession.videoStreams.SetValue(DataModel::List<const uint16_t>(kCandidateVideoStreams));
        return CHIP_NO_ERROR;
    }

    /** Copy candidate strings during the callback and record its session and count. */
    CHIP_ERROR HandleProvideICECandidates(uint16_t sessionId, const std::vector<ICECandidateStruct> & candidates) override
    {
        ++mCalls;
        mSessionId = sessionId;
        mCandidateStrings.clear();
        for (const auto & candidate : candidates)
        {
            mCandidateStrings.emplace_back(candidate.candidate.data(), candidate.candidate.size());
        }
        return CHIP_NO_ERROR;
    }

    uint32_t mCalls     = 0;
    uint16_t mSessionId = 0;
    std::vector<std::string> mCandidateStrings;
};

/** Encode an array containing a non-struct item to exercise lazy list-iterator errors. */
struct MalformedCandidateRequest
{
    uint16_t sessionId;
    bool prependValidCandidate;

    /** Preserve valid command fields while injecting an invalid ICECandidates array element. */
    CHIP_ERROR Encode(TLV::TLVWriter & writer, TLV::Tag tag) const
    {
        TLV::TLVType outer, array;
        ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(0), sessionId));
        ReturnErrorOnFailure(writer.StartContainer(TLV::ContextTag(1), TLV::kTLVType_Array, array));
        if (prependValidCandidate)
        {
            ICECandidateStruct candidate;
            candidate.candidate = kTestCandidate;
            ReturnErrorOnFailure(candidate.Encode(writer, TLV::AnonymousTag()));
        }
        ReturnErrorOnFailure(writer.Put(TLV::AnonymousTag(), uint32_t(1)));
        ReturnErrorOnFailure(writer.EndContainer(array));
        return writer.EndContainer(outer);
    }
};

/** Command-path fixture with a session established through ProvideOffer under a matching CASE subject. */
struct TestProvideICECandidates : public TestWebRTCTransportProviderCluster
{
    /** Start the server and establish a new session through the public command interface. */
    void SetUp() override
    {
        ASSERT_EQ(server.Startup(context.Get()), CHIP_NO_ERROR);
        SetSubject(kCandidateFabric, kCandidatePeer);
        Commands::ProvideOffer::Type request;
        request.sdp                   = "v=0\r\n"_span;
        request.originatingEndpointID = kTestEndpointId;
        request.streamUsage           = StreamUsageEnum::kLiveView;
        uint16_t videoStreams[]       = { 1 };
        request.videoStreams.SetValue(DataModel::List<uint16_t>(videoStreams));
        auto response = tester.Invoke<Commands::ProvideOffer::Type, Commands::ProvideOfferResponse::DecodableType>(
            Commands::ProvideOffer::Id, request);
        ASSERT_TRUE(response.IsSuccess());
        if (!response.response.has_value())
        {
            FAIL();
            return;
        }
        sessionId = response.response->webRTCSessionID;
        ASSERT_EQ(server.GetCurrentSessions().size(), 1u);
        EXPECT_EQ(server.GetCurrentSessions()[0].peerNodeID, kCandidatePeer);
        EXPECT_EQ(server.GetCurrentSessions()[0].GetFabricIndex(), kCandidateFabric);
        context.ChangeListener().DirtyList().clear();
    }

    /** Shut down while the context and delegate remain valid. */
    void TearDown() override { server.Shutdown(ClusterShutdownType::kClusterShutdown); }

    /** Select the invoking peer and fabric in the mock secure-session subject descriptor. */
    void SetSubject(FabricIndex fabric, NodeId peer, Access::AuthMode authMode = Access::AuthMode::kCase)
    {
        tester.SetSubjectDescriptor(Access::SubjectDescriptor{ .fabricIndex = fabric, .authMode = authMode, .subject = peer });
    }

    /** Verify rejection preserves the session through both the API and attribute TLV readback. */
    void ExpectSessionUnchanged()
    {
        const auto & current = server.GetCurrentSessions();
        ASSERT_EQ(current.size(), 1u);
        EXPECT_EQ(current[0].id, sessionId);
        EXPECT_EQ(current[0].peerNodeID, kCandidatePeer);
        EXPECT_EQ(current[0].GetFabricIndex(), kCandidateFabric);
        EXPECT_TRUE(context.ChangeListener().DirtyList().empty());
        tester.SetFabricIndex(kCandidateFabric);
        Attributes::CurrentSessions::TypeInfo::DecodableType sessions;
        ASSERT_EQ(tester.ReadAttribute(Attributes::CurrentSessions::Id, sessions), CHIP_NO_ERROR);
        auto it = sessions.begin();
        ASSERT_TRUE(it.Next());
        EXPECT_EQ(it.GetValue().id, sessionId);
        EXPECT_EQ(it.GetValue().peerNodeID, kCandidatePeer);
        EXPECT_EQ(it.GetValue().GetFabricIndex(), kCandidateFabric);
        EXPECT_FALSE(it.Next());
        EXPECT_EQ(it.GetStatus(), CHIP_NO_ERROR);
    }

    CandidateRecordingDelegate delegate;
    TestServerClusterContext context;
    WebRTCTransportProviderCluster server{ kTestEndpointId, delegate };
    ClusterTester tester{ server };
    uint16_t sessionId = 0;
};

/** The min-one list constraint rejects an empty list before delegate effects, including repeated requests. */
TEST_F(TestProvideICECandidates, RejectsEmptyList)
{
    Commands::ProvideICECandidates::Type request;
    request.webRTCSessionID = sessionId;
    request.ICECandidates   = DataModel::List<ICECandidateStruct>();
    for (unsigned attempt = 0; attempt < 2; ++attempt)
    {
        auto response = tester.Invoke<Commands::ProvideICECandidates::Type, DataModel::NullObjectType>(
            Commands::ProvideICECandidates::Id, request);
        EXPECT_EQ(response.GetStatusCode(),
                  Protocols::InteractionModel::ClusterStatusCode(Protocols::InteractionModel::Status::ConstraintError));
        EXPECT_EQ(delegate.mCalls, 0u);
        ExpectSessionUnchanged();
    }
    ICECandidateStruct candidate;
    candidate.candidate   = kTestCandidate;
    request.ICECandidates = DataModel::List<ICECandidateStruct>(&candidate, 1);
    auto valid =
        tester.Invoke<Commands::ProvideICECandidates::Type, DataModel::NullObjectType>(Commands::ProvideICECandidates::Id, request);
    ASSERT_TRUE(valid.IsSuccess());
    EXPECT_EQ(delegate.mCalls, 1u);
    EXPECT_EQ(delegate.mSessionId, sessionId);
}

/** One and multiple candidate entries continue to reach the owned-session delegate in order. */
TEST_F(TestProvideICECandidates, AcceptsNonemptyLists)
{
    ICECandidateStruct candidates[2];
    candidates[0].candidate = kTestCandidate;
    candidates[0].SDPMid.SetNonNull("0"_span);
    candidates[0].SDPMLineIndex.SetNonNull(0);
    candidates[1].candidate = "candidate:2 1 UDP 2130706430 192.0.2.2 5001 typ host"_span;
    Commands::ProvideICECandidates::Type request;
    request.webRTCSessionID = sessionId;
    for (size_t count : { size_t(1), size_t(2) })
    {
        request.ICECandidates = DataModel::List<ICECandidateStruct>(candidates, count);
        auto response         = tester.Invoke<Commands::ProvideICECandidates::Type, DataModel::NullObjectType>(
            Commands::ProvideICECandidates::Id, request);
        ASSERT_TRUE(response.IsSuccess());
        ASSERT_EQ(delegate.mCandidateStrings.size(), count);
        EXPECT_EQ(delegate.mSessionId, sessionId);
        for (size_t i = 0; i < count; ++i)
        {
            EXPECT_EQ(delegate.mCandidateStrings[i], std::string(candidates[i].candidate.data(), candidates[i].candidate.size()));
        }
    }
    EXPECT_EQ(delegate.mCalls, 2u);
    ExpectSessionUnchanged();
}

/** An entry with an empty candidate string remains valid for end-of-candidates signaling. */
TEST_F(TestProvideICECandidates, AcceptsEndOfCandidatesEntry)
{
    ICECandidateStruct candidate;
    candidate.candidate = ""_span;
    Commands::ProvideICECandidates::Type request;
    request.webRTCSessionID = sessionId;
    request.ICECandidates   = DataModel::List<ICECandidateStruct>(&candidate, 1);
    auto response =
        tester.Invoke<Commands::ProvideICECandidates::Type, DataModel::NullObjectType>(Commands::ProvideICECandidates::Id, request);
    ASSERT_TRUE(response.IsSuccess());
    EXPECT_EQ(delegate.mCalls, 1u);
    EXPECT_EQ(delegate.mSessionId, sessionId);
    ASSERT_EQ(delegate.mCandidateStrings.size(), 1u);
    EXPECT_TRUE(delegate.mCandidateStrings[0].empty());
    ExpectSessionUnchanged();
}

/** Malformed lists preserve InvalidCommand even when zero valid entries were decoded. */
TEST_F(TestProvideICECandidates, RejectsMalformedList)
{
    for (bool prependValid : { false, true })
    {
        MalformedCandidateRequest request{ sessionId, prependValid };
        auto response =
            tester.Invoke<MalformedCandidateRequest, DataModel::NullObjectType>(Commands::ProvideICECandidates::Id, request);
        EXPECT_EQ(response.GetStatusCode(),
                  Protocols::InteractionModel::ClusterStatusCode(Protocols::InteractionModel::Status::InvalidCommand));
        EXPECT_EQ(delegate.mCalls, 0u);
        ExpectSessionUnchanged();
    }
}

/** Present but empty SDPMid remains a constraint error before delegate invocation. */
TEST_F(TestProvideICECandidates, RejectsEmptySDPMid)
{
    ICECandidateStruct candidate;
    candidate.candidate = kTestCandidate;
    candidate.SDPMid.SetNonNull(""_span);
    Commands::ProvideICECandidates::Type request;
    request.webRTCSessionID = sessionId;
    request.ICECandidates   = DataModel::List<ICECandidateStruct>(&candidate, 1);
    auto response =
        tester.Invoke<Commands::ProvideICECandidates::Type, DataModel::NullObjectType>(Commands::ProvideICECandidates::Id, request);
    EXPECT_EQ(response.GetStatusCode(),
              Protocols::InteractionModel::ClusterStatusCode(Protocols::InteractionModel::Status::ConstraintError));
    EXPECT_EQ(delegate.mCalls, 0u);
    ExpectSessionUnchanged();
}

/** Nonempty lists retain NotFound for missing sessions and mismatched fabric, peer, or authentication mode. */
TEST_F(TestProvideICECandidates, RejectsUnmatchedSessions)
{
    ICECandidateStruct candidate;
    candidate.candidate = kTestCandidate;
    Commands::ProvideICECandidates::Type request;
    request.webRTCSessionID                    = sessionId;
    request.ICECandidates                      = DataModel::List<ICECandidateStruct>(&candidate, 1);
    const Access::SubjectDescriptor subjects[] = {
        { .fabricIndex = kCandidateFabric, .authMode = Access::AuthMode::kCase, .subject = kCandidatePeer + 1 },
        { .fabricIndex = kCandidateFabric + 1, .authMode = Access::AuthMode::kCase, .subject = kCandidatePeer },
        { .fabricIndex = kCandidateFabric, .authMode = Access::AuthMode::kPase, .subject = kCandidatePeer },
    };
    for (const auto & subject : subjects)
    {
        tester.SetSubjectDescriptor(subject);
        auto response = tester.Invoke<Commands::ProvideICECandidates::Type, DataModel::NullObjectType>(
            Commands::ProvideICECandidates::Id, request);
        EXPECT_EQ(response.GetStatusCode(),
                  Protocols::InteractionModel::ClusterStatusCode(Protocols::InteractionModel::Status::NotFound));
        EXPECT_EQ(delegate.mCalls, 0u);
        ExpectSessionUnchanged();
    }
    SetSubject(kCandidateFabric, kCandidatePeer);
    request.webRTCSessionID = 65534;
    auto response =
        tester.Invoke<Commands::ProvideICECandidates::Type, DataModel::NullObjectType>(Commands::ProvideICECandidates::Id, request);
    EXPECT_EQ(response.GetStatusCode(),
              Protocols::InteractionModel::ClusterStatusCode(Protocols::InteractionModel::Status::NotFound));
    EXPECT_EQ(delegate.mCalls, 0u);
    ExpectSessionUnchanged();
}

} // namespace
