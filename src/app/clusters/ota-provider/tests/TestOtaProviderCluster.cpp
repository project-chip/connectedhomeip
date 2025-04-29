/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/clusters/ota-provider/ota-provider-cluster.h>
#include <lib/core/Optional.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/Span.h>

namespace {

static constexpr chip::EndpointId kRootEndpointId = 0;

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OtaSoftwareUpdateProvider::Commands;
using chip::Protocols::InteractionModel::Status;

class MockDelegate : public OTAProviderDelegate
{
public:
    void HandleQueryImage(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                          const OtaSoftwareUpdateProvider::Commands::QueryImage::DecodableType & commandData) override
    {}

    void
    HandleApplyUpdateRequest(CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                             const OtaSoftwareUpdateProvider::Commands::ApplyUpdateRequest::DecodableType & commandData) override
    {}

    void
    HandleNotifyUpdateApplied(CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                              const OtaSoftwareUpdateProvider::Commands::NotifyUpdateApplied::DecodableType & commandData) override
    {}
};

struct TestOtaProviderLogic : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestOtaProviderLogic, QueryImageValidation)
{
    OtaProviderLogic logic;
    const ConcreteCommandPath kCommandPath{ kRootEndpointId, OtaSoftwareUpdateProvider::Id, QueryImage::Id };

    QueryImage::DecodableType input;

    // Without a delegate, command is unsupported.
    EXPECT_EQ(logic.QueryImage(kCommandPath, input, nullptr /* handler */), Status::UnsupportedCommand);

    MockDelegate mockDelegate;
    logic.SetDelegate(&mockDelegate);

    // Location MUST be 2 bytes.
    static constexpr CharSpan tooLargeLocationSpan = "abc_too_large"_span;
    input.location                                 = MakeOptional(tooLargeLocationSpan);
    EXPECT_EQ(logic.QueryImage(kCommandPath, input, nullptr /* handler */), Status::InvalidCommand);

    // Valid location (empty is ok).
    input.location = NullOptional;

    // Nullopt means delegate was called.
    EXPECT_EQ(logic.QueryImage(kCommandPath, input, nullptr /* handler */), std::nullopt);

    // Test for metadata too large.
    {
        constexpr size_t kTooLargeMetadataBytes = 513;

        Platform::ScopedMemoryBufferWithSize<uint8_t> kHugeBuffer;

        ASSERT_TRUE(kHugeBuffer.Alloc(kTooLargeMetadataBytes));

        input.metadataForProvider = MakeOptional(kHugeBuffer.Span());
        EXPECT_EQ(logic.QueryImage(kCommandPath, input, nullptr /* handler */), Status::InvalidCommand);
        input.metadataForProvider = NullOptional;
    }

    // Valid values, will set some logic.
    input.hardwareVersion     = MakeOptional(static_cast<uint16_t>(1234u));
    input.requestorCanConsent = MakeOptional(true);

    uint8_t someMeta[]        = { 1, 2, 3, 4, 5 };
    input.metadataForProvider = MakeOptional<Span<uint8_t>>(someMeta);
    EXPECT_EQ(logic.QueryImage(kCommandPath, input, nullptr /* handler */), std::nullopt);
}

TEST_F(TestOtaProviderLogic, NotifyUpdateApplied)
{
    OtaProviderLogic logic;
    const ConcreteCommandPath kCommandPath{ kRootEndpointId, OtaSoftwareUpdateProvider::Id, NotifyUpdateApplied::Id };

    NotifyUpdateApplied::DecodableType input;

    // Without a delegate, error out.
    EXPECT_EQ(logic.NotifyUpdateApplied(kCommandPath, input, nullptr /* handler not used */), Status::UnsupportedCommand);

    MockDelegate mockDelegate;
    logic.SetDelegate(&mockDelegate);

    // Update token is not valid.
    EXPECT_EQ(logic.NotifyUpdateApplied(kCommandPath, input, nullptr /* handler not used */), Status::InvalidCommand);

    constexpr uint8_t kUpdateToken[] = { 1, 2, 3, 4, 5, 6, 7, 8 };

    input.softwareVersion = 123;
    input.updateToken     = Span<const uint8_t>(kUpdateToken);
    EXPECT_EQ(logic.NotifyUpdateApplied(kCommandPath, input, nullptr /* handler not used */), std::nullopt);
}

TEST_F(TestOtaProviderLogic, ApplyUpdateRequest)
{
    OtaProviderLogic logic;
    const ConcreteCommandPath kCommandPath{ kRootEndpointId, OtaSoftwareUpdateProvider::Id, ApplyUpdateRequest::Id };

    ApplyUpdateRequest::DecodableType input;

    // Without a delegate, error out.
    EXPECT_EQ(logic.ApplyUpdateRequest(kCommandPath, input, nullptr /* handler not used */), Status::UnsupportedCommand);

    MockDelegate mockDelegate;
    logic.SetDelegate(&mockDelegate);

    // Update token is not valid.
    EXPECT_EQ(logic.ApplyUpdateRequest(kCommandPath, input, nullptr /* handler not used */), Status::InvalidCommand);

    constexpr uint8_t kUpdateToken[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    input.newVersion                 = 124;
    input.updateToken                = Span<const uint8_t>(kUpdateToken);
    EXPECT_EQ(logic.ApplyUpdateRequest(kCommandPath, input, nullptr /* handler not used */), std::nullopt);
}

} // namespace
