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
#include <app/clusters/messages-server/messages-delegate.h>
#include <app/clusters/messages-server/messages-server.h>
#include <app/server-cluster/testing/MockCommandHandler.h>
#include <app/tests/test-ember-api.h>
#include <lib/support/CHIPMem.h>
#include <protocols/interaction_model/StatusCode.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::Messages;
using chip::Protocols::InteractionModel::Status;

namespace {

constexpr EndpointId kTestEndpointId = 1;

class FakeMessagesDelegate : public Delegate
{
public:
    CHIP_ERROR
    HandlePresentMessagesRequest(const ByteSpan &, const MessagePriorityEnum &, const chip::BitMask<MessageControlBitmap> &,
                                 const DataModel::Nullable<uint32_t> &, const DataModel::Nullable<uint64_t> &, const CharSpan &,
                                 const chip::Optional<DataModel::DecodableList<Structs::MessageResponseOptionStruct::Type>> &,
                                 const chip::Optional<CharSpan> &, const chip::Optional<CharSpan> &) override
    {
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR HandleCancelMessagesRequest(const DataModel::DecodableList<ByteSpan> &) override { return CHIP_NO_ERROR; }

    CHIP_ERROR HandleGetMessages(AttributeValueEncoder &) override { return CHIP_NO_ERROR; }
    CHIP_ERROR HandleGetActiveMessageIds(AttributeValueEncoder &) override { return CHIP_NO_ERROR; }
    CHIP_ERROR HandleGetSupportedLanguageCodes(AttributeValueEncoder &) override { return CHIP_NO_ERROR; }
    CHIP_ERROR HandleGetSupportedMimeTypes(AttributeValueEncoder &) override { return CHIP_NO_ERROR; }

    uint32_t GetFeatureMap(EndpointId) override { return mFeatureMap; }

    void SetFeatureMap(uint32_t featureMap) { mFeatureMap = featureMap; }

private:
    uint32_t mFeatureMap = 0;
};

class TestMessagesCluster : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override
    {
        chip::Testing::numEndpoints = kTestEndpointId + 1;
        SetDefaultDelegate(kTestEndpointId, &mDelegate);
    }

    void TearDown() override { SetDefaultDelegate(kTestEndpointId, nullptr); }

protected:
    FakeMessagesDelegate mDelegate;
};

Commands::PresentMessagesRequest::Type MakeValidRequest()
{
    static const uint8_t kMessageId[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

    Commands::PresentMessagesRequest::Type request;
    request.messageID   = ByteSpan(kMessageId);
    request.priority    = MessagePriorityEnum::kLow;
    request.messageText = "Hello"_span;
    return request;
}

// Encodes `request` to TLV and decodes it back into a DecodableType, mirroring what the
// Interaction Model engine does before invoking a cluster's command callback. This lets tests
// populate DecodableList-based fields (e.g. `responses`) the same way ClusterTester does for
// code-driven clusters, even though messages-server predates that harness.
//
// NOTE: `buf` must outlive `decoded` and the callback invocation below -- the decoded
// ByteSpan/CharSpan/DecodableList fields reference memory inside it, they don't own copies.
Status InvokePresentMessagesRequest(const Commands::PresentMessagesRequest::Type & request)
{
    uint8_t buf[512];
    TLV::TLVWriter writer;
    writer.Init(buf);
    VerifyOrReturnValue(request.Encode(writer, TLV::AnonymousTag()) == CHIP_NO_ERROR, Status::Failure);
    VerifyOrReturnValue(writer.Finalize() == CHIP_NO_ERROR, Status::Failure);

    TLV::TLVReader reader;
    reader.Init(buf, writer.GetLengthWritten());
    VerifyOrReturnValue(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()) == CHIP_NO_ERROR, Status::Failure);

    Commands::PresentMessagesRequest::DecodableType decoded;
    VerifyOrReturnValue(decoded.Decode(reader, kUndefinedFabricIndex) == CHIP_NO_ERROR, Status::Failure);

    chip::Testing::MockCommandHandler mockHandler;
    ConcreteCommandPath commandPath(kTestEndpointId, Id, Commands::PresentMessagesRequest::Id);

    emberAfMessagesClusterPresentMessagesRequestCallback(&mockHandler, commandPath, decoded);

    VerifyOrReturnValue(mockHandler.HasStatus(), Status::Failure);
    return mockHandler.GetLastStatus().status.GetStatus();
}

} // namespace

TEST_F(TestMessagesCluster, ValidRequestSucceeds)
{
    EXPECT_EQ(InvokePresentMessagesRequest(MakeValidRequest()), Status::Success);
}

TEST_F(TestMessagesCluster, InvalidMessageIdLengthRejected)
{
    static const uint8_t kShortId[4] = { 0, 1, 2, 3 };
    auto request                     = MakeValidRequest();
    request.messageID                = ByteSpan(kShortId);
    EXPECT_EQ(InvokePresentMessagesRequest(request), Status::ConstraintError);
}

TEST_F(TestMessagesCluster, MessageTextTooLongRejected)
{
    static char longText[300];
    memset(longText, 'a', sizeof(longText) - 1);
    longText[sizeof(longText) - 1] = '\0';

    auto request        = MakeValidRequest();
    request.messageText = CharSpan::fromCharString(longText);
    EXPECT_EQ(InvokePresentMessagesRequest(request), Status::ConstraintError);
}

TEST_F(TestMessagesCluster, SpokenMessageBitWithoutFeatureRejected)
{
    auto request = MakeValidRequest();
    request.messageControl.Set(MessageControlBitmap::kSpokenMessage);
    request.languageCode = MakeOptional("en-US"_span);
    // mDelegate's feature map defaults to 0 (no SpokenMessages feature).
    EXPECT_EQ(InvokePresentMessagesRequest(request), Status::InvalidCommand);
}

TEST_F(TestMessagesCluster, SpokenMessageBitWithoutLanguageCodeRejected)
{
    mDelegate.SetFeatureMap(to_underlying(Feature::kSpokenMessages));
    auto request = MakeValidRequest();
    request.messageControl.Set(MessageControlBitmap::kSpokenMessage);
    // languageCode intentionally omitted.
    EXPECT_EQ(InvokePresentMessagesRequest(request), Status::ConstraintError);
}

TEST_F(TestMessagesCluster, SpokenMessageWithLanguageCodeSucceeds)
{
    mDelegate.SetFeatureMap(to_underlying(Feature::kSpokenMessages));
    auto request = MakeValidRequest();
    request.messageControl.Set(MessageControlBitmap::kSpokenMessage);
    request.languageCode = MakeOptional("en-US"_span);
    EXPECT_EQ(InvokePresentMessagesRequest(request), Status::Success);
}

TEST_F(TestMessagesCluster, EmptyLanguageCodeRejected)
{
    mDelegate.SetFeatureMap(to_underlying(Feature::kSpokenMessages));
    auto request = MakeValidRequest();
    request.messageControl.Set(MessageControlBitmap::kSpokenMessage);
    request.languageCode = MakeOptional(CharSpan());
    EXPECT_EQ(InvokePresentMessagesRequest(request), Status::ConstraintError);
}

TEST_F(TestMessagesCluster, LanguageCodeTooLongRejected)
{
    static char longCode[64];
    memset(longCode, 'a', sizeof(longCode) - 1);
    longCode[sizeof(longCode) - 1] = '\0';

    mDelegate.SetFeatureMap(to_underlying(Feature::kSpokenMessages));
    auto request = MakeValidRequest();
    request.messageControl.Set(MessageControlBitmap::kSpokenMessage);
    request.languageCode = MakeOptional(CharSpan::fromCharString(longCode));
    EXPECT_EQ(InvokePresentMessagesRequest(request), Status::ConstraintError);
}

TEST_F(TestMessagesCluster, AudioMessageBitWithoutFeatureRejected)
{
    auto request = MakeValidRequest();
    request.messageControl.Set(MessageControlBitmap::kAudioMessage);
    request.messageURI = MakeOptional("https://example.com/msg.mp3"_span);
    EXPECT_EQ(InvokePresentMessagesRequest(request), Status::InvalidCommand);
}

TEST_F(TestMessagesCluster, AudioMessageBitWithoutMessageUriRejected)
{
    mDelegate.SetFeatureMap(to_underlying(Feature::kAudioMessages));
    auto request = MakeValidRequest();
    request.messageControl.Set(MessageControlBitmap::kAudioMessage);
    // messageURI intentionally omitted.
    EXPECT_EQ(InvokePresentMessagesRequest(request), Status::ConstraintError);
}

TEST_F(TestMessagesCluster, AudioMessageWithMessageUriSucceeds)
{
    mDelegate.SetFeatureMap(to_underlying(Feature::kAudioMessages));
    auto request = MakeValidRequest();
    request.messageControl.Set(MessageControlBitmap::kAudioMessage);
    request.messageURI = MakeOptional("https://example.com/msg.mp3"_span);
    EXPECT_EQ(InvokePresentMessagesRequest(request), Status::Success);
}

TEST_F(TestMessagesCluster, EmptyMessageUriRejected)
{
    mDelegate.SetFeatureMap(to_underlying(Feature::kAudioMessages));
    auto request = MakeValidRequest();
    request.messageControl.Set(MessageControlBitmap::kAudioMessage);
    request.messageURI = MakeOptional(CharSpan());
    EXPECT_EQ(InvokePresentMessagesRequest(request), Status::ConstraintError);
}

TEST_F(TestMessagesCluster, MessageUriTooLongRejected)
{
    static char longUri[300];
    memset(longUri, 'a', sizeof(longUri) - 1);
    longUri[sizeof(longUri) - 1] = '\0';

    mDelegate.SetFeatureMap(to_underlying(Feature::kAudioMessages));
    auto request = MakeValidRequest();
    request.messageControl.Set(MessageControlBitmap::kAudioMessage);
    request.messageURI = MakeOptional(CharSpan::fromCharString(longUri));
    EXPECT_EQ(InvokePresentMessagesRequest(request), Status::ConstraintError);
}

TEST_F(TestMessagesCluster, BothBitsWithoutMultiModalFeatureRejected)
{
    mDelegate.SetFeatureMap(to_underlying(Feature::kSpokenMessages) | to_underlying(Feature::kAudioMessages));
    auto request = MakeValidRequest();
    request.messageControl.Set(MessageControlBitmap::kSpokenMessage);
    request.messageControl.Set(MessageControlBitmap::kAudioMessage);
    request.languageCode = MakeOptional("en-US"_span);
    request.messageURI   = MakeOptional("https://example.com/msg.mp3"_span);
    EXPECT_EQ(InvokePresentMessagesRequest(request), Status::InvalidCommand);
}

TEST_F(TestMessagesCluster, BothBitsWithMultiModalFeatureSucceeds)
{
    mDelegate.SetFeatureMap(to_underlying(Feature::kSpokenMessages) | to_underlying(Feature::kAudioMessages) |
                            to_underlying(Feature::kMultiModalMessages));
    auto request = MakeValidRequest();
    request.messageControl.Set(MessageControlBitmap::kSpokenMessage);
    request.messageControl.Set(MessageControlBitmap::kAudioMessage);
    request.languageCode = MakeOptional("en-US"_span);
    request.messageURI   = MakeOptional("https://example.com/msg.mp3"_span);
    EXPECT_EQ(InvokePresentMessagesRequest(request), Status::Success);
}

TEST_F(TestMessagesCluster, ResponsesWithoutConfirmationResponseFeatureRejected)
{
    Structs::MessageResponseOptionStruct::Type option;
    option.messageResponseID = MakeOptional(static_cast<uint32_t>(1));
    option.label             = MakeOptional("Yes"_span);

    auto request = MakeValidRequest();
    // mDelegate's feature map defaults to 0 (no ConfirmationResponse feature).
    request.responses = MakeOptional(DataModel::List<const Structs::MessageResponseOptionStruct::Type>(&option, 1));
    EXPECT_EQ(InvokePresentMessagesRequest(request), Status::InvalidCommand);
}

TEST_F(TestMessagesCluster, ResponsesWithConfirmationResponseFeatureSucceeds)
{
    Structs::MessageResponseOptionStruct::Type option;
    option.messageResponseID = MakeOptional(static_cast<uint32_t>(1));
    option.label             = MakeOptional("Yes"_span);

    mDelegate.SetFeatureMap(to_underlying(Feature::kConfirmationResponse));
    auto request      = MakeValidRequest();
    request.responses = MakeOptional(DataModel::List<const Structs::MessageResponseOptionStruct::Type>(&option, 1));
    EXPECT_EQ(InvokePresentMessagesRequest(request), Status::Success);
}

TEST_F(TestMessagesCluster, TooManyResponseOptionsRejected)
{
    Structs::MessageResponseOptionStruct::Type options[5];
    for (size_t i = 0; i < 5; ++i)
    {
        options[i].messageResponseID = MakeOptional(static_cast<uint32_t>(i + 1));
        options[i].label             = MakeOptional("Option"_span);
    }

    mDelegate.SetFeatureMap(to_underlying(Feature::kConfirmationResponse));
    auto request      = MakeValidRequest();
    request.responses = MakeOptional(DataModel::List<const Structs::MessageResponseOptionStruct::Type>(options, 5));
    EXPECT_EQ(InvokePresentMessagesRequest(request), Status::ConstraintError);
}

TEST_F(TestMessagesCluster, ResponseOptionMissingIdRejected)
{
    Structs::MessageResponseOptionStruct::Type option;
    option.label = MakeOptional("Yes"_span);
    // messageResponseID intentionally omitted.

    mDelegate.SetFeatureMap(to_underlying(Feature::kConfirmationResponse));
    auto request      = MakeValidRequest();
    request.responses = MakeOptional(DataModel::List<const Structs::MessageResponseOptionStruct::Type>(&option, 1));
    EXPECT_EQ(InvokePresentMessagesRequest(request), Status::InvalidCommand);
}

TEST_F(TestMessagesCluster, ResponseOptionLabelTooLongRejected)
{
    static char longLabel[64];
    memset(longLabel, 'a', sizeof(longLabel) - 1);
    longLabel[sizeof(longLabel) - 1] = '\0';

    Structs::MessageResponseOptionStruct::Type option;
    option.messageResponseID = MakeOptional(static_cast<uint32_t>(1));
    option.label             = MakeOptional(CharSpan::fromCharString(longLabel));

    mDelegate.SetFeatureMap(to_underlying(Feature::kConfirmationResponse));
    auto request      = MakeValidRequest();
    request.responses = MakeOptional(DataModel::List<const Structs::MessageResponseOptionStruct::Type>(&option, 1));
    EXPECT_EQ(InvokePresentMessagesRequest(request), Status::ConstraintError);
}
