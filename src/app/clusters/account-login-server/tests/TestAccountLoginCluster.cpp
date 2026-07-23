/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/AttributeValueEncoder.h>
#include <app/EventManagement.h>
#include <app/MessageDef/AttributeReportIBs.h>
#include <app/clusters/account-login-server/account-login-delegate.h>
#include <app/clusters/account-login-server/account-login-server.h>
#include <app/server-cluster/testing/MockCommandHandler.h>
#include <app/tests/test-ember-api.h>
#include <lib/support/CHIPCounter.h>
#include <lib/support/CHIPMem.h>
#include <protocols/interaction_model/StatusCode.h>
#include <pw_unit_test/framework.h>

// Not declared in any header included above -- forward-declared here the same way
// TestChimeClusterBackwardsCompatibility.cpp does for its cluster's init callback.
void MatterAccountLoginPluginServerInitCallback();
void MatterAccountLoginPluginServerShutdownCallback();

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::AccountLogin;
using chip::Protocols::InteractionModel::Status;

namespace {

constexpr EndpointId kTestEndpointId = 1;

// EventManagement backing storage, so the LoggedOut event (Critical priority) can actually be
// logged; needed to observe whether emberAfAccountLoginClusterLogoutCallback fires it or not.
uint8_t gDebugEventBuffer[128];
uint8_t gInfoEventBuffer[128];
uint8_t gCritEventBuffer[128];
chip::app::CircularEventBuffer gCircularEventBuffer[3];

class FakeAccountLoginDelegate : public Delegate
{
public:
    void SetSetupPin(char *) override {}

    bool HandleLogin(const CharSpan &, const CharSpan &, const chip::Optional<NodeId> &) override { return mLoginResult; }
    bool HandleLogout(const chip::Optional<NodeId> &) override { return mLogoutResult; }

    void HandleGetSetupPin(CommandResponseHelper<Commands::GetSetupPINResponse::Type> & helper, const CharSpan &) override
    {
        Commands::GetSetupPINResponse::Type response;
        response.setupPIN = mSetupPin;
        TEMPORARY_RETURN_IGNORED helper.Success(response);
    }

    void GetSetupPin(char *, size_t, const CharSpan &) override {}

    void HandleGetDeviceAuthURI(CommandResponseHelper<Commands::GetDeviceAuthURIResponse::Type> & helper) override
    {
        Commands::GetDeviceAuthURIResponse::Type response;
        response.userCode        = mUserCode;
        response.verificationURI = mVerificationURI;
        response.expiresIn       = 1800;
        response.interval        = 5;
        TEMPORARY_RETURN_IGNORED helper.Success(response);
    }

    bool GetOAuthLoggedIn(EndpointId) override { return mOAuthLoggedIn; }

    uint16_t GetClusterRevision(EndpointId) override { return mClusterRevision; }

    bool mLoginResult         = true;
    bool mLogoutResult        = true;
    bool mOAuthLoggedIn       = false;
    uint16_t mClusterRevision = 3;
    CharSpan mSetupPin        = "1234"_span;
    CharSpan mUserCode        = "ABCD-EFGH"_span;
    CharSpan mVerificationURI = "https://example.com/device"_span;
};

class TestAccountLoginCluster : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override
    {
        chip::Testing::numEndpoints = kTestEndpointId + 1;
        SetDefaultDelegate(kTestEndpointId, &mDelegate);
        MatterAccountLoginPluginServerInitCallback();

        const chip::app::LogStorageResources logStorageResources[] = {
            { &gDebugEventBuffer[0], sizeof(gDebugEventBuffer), chip::app::PriorityLevel::Debug },
            { &gInfoEventBuffer[0], sizeof(gInfoEventBuffer), chip::app::PriorityLevel::Info },
            { &gCritEventBuffer[0], sizeof(gCritEventBuffer), chip::app::PriorityLevel::Critical },
        };

        ASSERT_EQ(mEventCounter.Init(0), CHIP_NO_ERROR);
        chip::app::EventManagement::CreateEventManagement(nullptr, MATTER_ARRAY_SIZE(logStorageResources), gCircularEventBuffer,
                                                          logStorageResources, &mEventCounter);
    }

    void TearDown() override
    {
        chip::app::EventManagement::DestroyEventManagement();
        MatterAccountLoginPluginServerShutdownCallback();
        SetDefaultDelegate(kTestEndpointId, nullptr);
    }

protected:
    FakeAccountLoginDelegate mDelegate;
    chip::MonotonicallyIncreasingCounter<chip::EventNumber> mEventCounter;
};

ConcreteCommandPath MakeCommandPath(CommandId commandId)
{
    return ConcreteCommandPath(kTestEndpointId, Id, commandId);
}

// Reads a scalar attribute through the cluster's real, registered AttributeAccessInterface --
// exercising AccountLoginAttrAccess::Read's attribute-ID dispatch, not just the delegate hook it
// eventually calls into.
template <typename T>
CHIP_ERROR ReadScalarAttributeViaAttrAccess(EndpointId endpoint, AttributeId attributeId, T & outValue)
{
    AttributeAccessInterface * attrAccess = AttributeAccessInterfaceRegistry::Instance().Get(endpoint, Id);
    VerifyOrReturnError(attrAccess != nullptr, CHIP_ERROR_NOT_FOUND);

    uint8_t buf[128];
    TLV::TLVWriter writer;
    writer.Init(buf);

    AttributeReportIBs::Builder builder;
    ReturnErrorOnFailure(builder.Init(&writer));

    ConcreteAttributePath path(endpoint, Id, attributeId);
    ConcreteReadAttributePath readPath(path);
    chip::DataVersion dataVersion(0);
    Access::SubjectDescriptor subjectDescriptor;
    AttributeValueEncoder encoder(builder, subjectDescriptor, path, dataVersion);

    ReturnErrorOnFailure(attrAccess->Read(readPath, encoder));
    builder.EndOfContainer();
    ReturnErrorOnFailure(writer.Finalize());

    TLV::TLVReader reader;
    reader.Init(buf, writer.GetLengthWritten());

    TLV::TLVReader attrReportsReader;
    TLV::TLVReader attrReportReader;
    TLV::TLVReader attrDataReader;

    ReturnErrorOnFailure(reader.Next());
    ReturnErrorOnFailure(reader.OpenContainer(attrReportsReader));
    ReturnErrorOnFailure(attrReportsReader.Next());
    ReturnErrorOnFailure(attrReportsReader.OpenContainer(attrReportReader));
    ReturnErrorOnFailure(attrReportReader.Next());
    ReturnErrorOnFailure(attrReportReader.OpenContainer(attrDataReader));

    // Skip to the Data tag (context tag 2) within AttributeDataIB.
    ReturnErrorOnFailure(attrDataReader.Next());
    while (!(TLV::IsContextTag(attrDataReader.GetTag()) && TLV::TagNumFromTag(attrDataReader.GetTag()) == 2))
    {
        ReturnErrorOnFailure(attrDataReader.Next());
    }

    return attrDataReader.Get(outValue);
}

} // namespace

// -----------------------------------------------------------------------------
// GetSetupPIN

TEST_F(TestAccountLoginCluster, GetSetupPinSucceeds)
{
    mDelegate.mSetupPin = "5678"_span;

    Commands::GetSetupPIN::DecodableType request;
    request.tempAccountIdentifier = "account-1"_span;

    chip::Testing::MockCommandHandler mockHandler;
    auto commandPath = MakeCommandPath(Commands::GetSetupPIN::Id);
    emberAfAccountLoginClusterGetSetupPINCallback(&mockHandler, commandPath, request);

    ASSERT_TRUE(mockHandler.HasResponse());
    Commands::GetSetupPINResponse::DecodableType response;
    EXPECT_EQ(mockHandler.DecodeResponse(response), CHIP_NO_ERROR);
    EXPECT_TRUE(response.setupPIN.data_equal("5678"_span));
    EXPECT_FALSE(mockHandler.HasStatus());
}

TEST_F(TestAccountLoginCluster, GetSetupPinWithNoDelegateFails)
{
    SetDefaultDelegate(kTestEndpointId, nullptr);

    Commands::GetSetupPIN::DecodableType request;
    request.tempAccountIdentifier = "account-1"_span;

    chip::Testing::MockCommandHandler mockHandler;
    auto commandPath = MakeCommandPath(Commands::GetSetupPIN::Id);
    emberAfAccountLoginClusterGetSetupPINCallback(&mockHandler, commandPath, request);

    ASSERT_TRUE(mockHandler.HasStatus());
    EXPECT_EQ(mockHandler.GetLastStatus().status.GetStatus(), Status::Failure);
    EXPECT_FALSE(mockHandler.HasResponse());
}

// -----------------------------------------------------------------------------
// GetDeviceAuthURI

TEST_F(TestAccountLoginCluster, GetDeviceAuthUriSucceeds)
{
    Commands::GetDeviceAuthURI::DecodableType request;

    chip::Testing::MockCommandHandler mockHandler;
    auto commandPath = MakeCommandPath(Commands::GetDeviceAuthURI::Id);
    emberAfAccountLoginClusterGetDeviceAuthURICallback(&mockHandler, commandPath, request);

    ASSERT_TRUE(mockHandler.HasResponse());
    Commands::GetDeviceAuthURIResponse::DecodableType response;
    EXPECT_EQ(mockHandler.DecodeResponse(response), CHIP_NO_ERROR);
    EXPECT_TRUE(response.userCode.data_equal("ABCD-EFGH"_span));
    EXPECT_TRUE(response.verificationURI.data_equal("https://example.com/device"_span));
    EXPECT_EQ(response.interval, 5);
    EXPECT_FALSE(mockHandler.HasStatus());
}

TEST_F(TestAccountLoginCluster, GetDeviceAuthUriWithNoDelegateFails)
{
    SetDefaultDelegate(kTestEndpointId, nullptr);

    Commands::GetDeviceAuthURI::DecodableType request;

    chip::Testing::MockCommandHandler mockHandler;
    auto commandPath = MakeCommandPath(Commands::GetDeviceAuthURI::Id);
    emberAfAccountLoginClusterGetDeviceAuthURICallback(&mockHandler, commandPath, request);

    ASSERT_TRUE(mockHandler.HasStatus());
    EXPECT_EQ(mockHandler.GetLastStatus().status.GetStatus(), Status::Failure);
    EXPECT_FALSE(mockHandler.HasResponse());
}

// -----------------------------------------------------------------------------
// Login

TEST_F(TestAccountLoginCluster, LoginSucceedsMapsToSuccess)
{
    mDelegate.mLoginResult = true;

    Commands::Login::DecodableType request;
    request.tempAccountIdentifier = "account-1"_span;
    request.setupPIN              = "1234"_span;

    chip::Testing::MockCommandHandler mockHandler;
    auto commandPath = MakeCommandPath(Commands::Login::Id);
    emberAfAccountLoginClusterLoginCallback(&mockHandler, commandPath, request);

    ASSERT_TRUE(mockHandler.HasStatus());
    EXPECT_EQ(mockHandler.GetLastStatus().status.GetStatus(), Status::Success);
}

TEST_F(TestAccountLoginCluster, LoginFailureMapsToUnsupportedAccess)
{
    mDelegate.mLoginResult = false;

    Commands::Login::DecodableType request;
    request.tempAccountIdentifier = "account-1"_span;
    request.setupPIN              = "wrong"_span;

    chip::Testing::MockCommandHandler mockHandler;
    auto commandPath = MakeCommandPath(Commands::Login::Id);
    emberAfAccountLoginClusterLoginCallback(&mockHandler, commandPath, request);

    ASSERT_TRUE(mockHandler.HasStatus());
    EXPECT_EQ(mockHandler.GetLastStatus().status.GetStatus(), Status::UnsupportedAccess);
}

TEST_F(TestAccountLoginCluster, LoginWithNoDelegateFails)
{
    SetDefaultDelegate(kTestEndpointId, nullptr);

    Commands::Login::DecodableType request;
    request.tempAccountIdentifier = "account-1"_span;
    request.setupPIN              = "1234"_span;

    chip::Testing::MockCommandHandler mockHandler;
    auto commandPath = MakeCommandPath(Commands::Login::Id);
    emberAfAccountLoginClusterLoginCallback(&mockHandler, commandPath, request);

    ASSERT_TRUE(mockHandler.HasStatus());
    EXPECT_EQ(mockHandler.GetLastStatus().status.GetStatus(), Status::Failure);
}

// -----------------------------------------------------------------------------
// Logout
//
// The current implementation logs the LoggedOut event whenever `node` is present,
// regardless of whether HandleLogout succeeded -- these tests pin down that behavior
// explicitly since it's easy to regress silently (event firing on a failure path).

TEST_F(TestAccountLoginCluster, LogoutSucceedsAndFiresEventWhenNodePresent)
{
    mDelegate.mLogoutResult = true;

    Commands::Logout::DecodableType request;
    request.node = chip::MakeOptional(static_cast<NodeId>(1234));

    EventNumber before = EventManagement::GetInstance().GetLastEventNumber();

    chip::Testing::MockCommandHandler mockHandler;
    auto commandPath = MakeCommandPath(Commands::Logout::Id);
    emberAfAccountLoginClusterLogoutCallback(&mockHandler, commandPath, request);

    ASSERT_TRUE(mockHandler.HasStatus());
    EXPECT_EQ(mockHandler.GetLastStatus().status.GetStatus(), Status::Success);
    EXPECT_GT(EventManagement::GetInstance().GetLastEventNumber(), before);
}

TEST_F(TestAccountLoginCluster, LogoutFailureStillFiresEventWhenNodePresent)
{
    mDelegate.mLogoutResult = false;

    Commands::Logout::DecodableType request;
    request.node = chip::MakeOptional(static_cast<NodeId>(1234));

    EventNumber before = EventManagement::GetInstance().GetLastEventNumber();

    chip::Testing::MockCommandHandler mockHandler;
    auto commandPath = MakeCommandPath(Commands::Logout::Id);
    emberAfAccountLoginClusterLogoutCallback(&mockHandler, commandPath, request);

    ASSERT_TRUE(mockHandler.HasStatus());
    EXPECT_EQ(mockHandler.GetLastStatus().status.GetStatus(), Status::Failure);
    // Event fires even though the logout itself failed -- current (arguably surprising) behavior.
    EXPECT_GT(EventManagement::GetInstance().GetLastEventNumber(), before);
}

TEST_F(TestAccountLoginCluster, LogoutWithoutNodeDoesNotFireEvent)
{
    mDelegate.mLogoutResult = true;

    Commands::Logout::DecodableType request;
    // node intentionally omitted.

    EventNumber before = EventManagement::GetInstance().GetLastEventNumber();

    chip::Testing::MockCommandHandler mockHandler;
    auto commandPath = MakeCommandPath(Commands::Logout::Id);
    emberAfAccountLoginClusterLogoutCallback(&mockHandler, commandPath, request);

    ASSERT_TRUE(mockHandler.HasStatus());
    EXPECT_EQ(mockHandler.GetLastStatus().status.GetStatus(), Status::Success);
    EXPECT_EQ(EventManagement::GetInstance().GetLastEventNumber(), before);
}

TEST_F(TestAccountLoginCluster, LogoutWithNoDelegateFails)
{
    SetDefaultDelegate(kTestEndpointId, nullptr);

    Commands::Logout::DecodableType request;
    request.node = chip::MakeOptional(static_cast<NodeId>(1234));

    chip::Testing::MockCommandHandler mockHandler;
    auto commandPath = MakeCommandPath(Commands::Logout::Id);
    emberAfAccountLoginClusterLogoutCallback(&mockHandler, commandPath, request);

    ASSERT_TRUE(mockHandler.HasStatus());
    EXPECT_EQ(mockHandler.GetLastStatus().status.GetStatus(), Status::Failure);
}

// -----------------------------------------------------------------------------
// Attribute reads

TEST_F(TestAccountLoginCluster, ReadClusterRevision)
{
    mDelegate.mClusterRevision = 3;

    uint16_t clusterRevision = 0;
    EXPECT_EQ(ReadScalarAttributeViaAttrAccess(kTestEndpointId, Attributes::ClusterRevision::Id, clusterRevision), CHIP_NO_ERROR);
    EXPECT_EQ(clusterRevision, 3);
}

TEST_F(TestAccountLoginCluster, ReadOAuthLoggedIn)
{
    mDelegate.mOAuthLoggedIn = true;
    bool loggedIn            = false;
    EXPECT_EQ(ReadScalarAttributeViaAttrAccess(kTestEndpointId, Attributes::OAuthLoggedIn::Id, loggedIn), CHIP_NO_ERROR);
    EXPECT_TRUE(loggedIn);

    mDelegate.mOAuthLoggedIn = false;
    EXPECT_EQ(ReadScalarAttributeViaAttrAccess(kTestEndpointId, Attributes::OAuthLoggedIn::Id, loggedIn), CHIP_NO_ERROR);
    EXPECT_FALSE(loggedIn);
}
