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

 #include <app/clusters/ota-provider/OTAProviderCluster.h>
 #include <app/server-cluster/testing/ClusterTester.h>
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
 using chip::Testing::ClusterTester;
 
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
 
 struct TestOtaProviderCluster : public ::testing::Test
 {
     static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
     static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
 };
 
 TEST_F(TestOtaProviderCluster, QueryImageValidation)
 {
     OtaProviderServer otaProvider(kRootEndpointId);
     ClusterTester tester(otaProvider);
 
     QueryImage::Type input;
 
     // Without a delegate, command is unsupported.
     auto result = tester.Invoke(QueryImage::Id, input);
     ASSERT_TRUE(result.IsSuccess());
     EXPECT_EQ(result.status->GetStatusCode().GetStatus(), Status::UnsupportedCommand);
 
     MockDelegate mockDelegate;
     otaProvider.SetDelegate(&mockDelegate);
 
     // Location MUST be 2 bytes.
     static constexpr CharSpan tooLargeLocationSpan = "abc_too_large"_span;
     input.location                                 = MakeOptional(tooLargeLocationSpan);
     result                                         = tester.Invoke(QueryImage::Id, input);
     ASSERT_TRUE(result.IsSuccess());
     EXPECT_EQ(result.status->GetStatusCode().GetStatus(), Status::InvalidCommand);
 
     // Valid location (empty is ok).
     input.location = NullOptional;
 
     // Nullopt means delegate was called - this should succeed now
     result = tester.Invoke(QueryImage::Id, input);
     ASSERT_TRUE(result.IsSuccess());
     EXPECT_EQ(result.status->GetStatusCode().GetStatus(), Status::Success);
 
     // Test for metadata too large.
     {
         constexpr size_t kTooLargeMetadataBytes = 513;
 
         Platform::ScopedMemoryBufferWithSize<uint8_t> kHugeBuffer;
 
         ASSERT_TRUE(kHugeBuffer.Alloc(kTooLargeMetadataBytes));
 
         input.metadataForProvider = MakeOptional(kHugeBuffer.Span());
         result                    = tester.Invoke(QueryImage::Id, input);
         ASSERT_TRUE(result.IsSuccess());
         EXPECT_EQ(result.status->GetStatusCode().GetStatus(), Status::InvalidCommand);
         input.metadataForProvider = NullOptional;
     }
 
     // Valid values, will set some logic.
     input.hardwareVersion     = MakeOptional(static_cast<uint16_t>(1234u));
     input.requestorCanConsent = MakeOptional(true);
 
     uint8_t someMeta[]        = { 1, 2, 3, 4, 5 };
     input.metadataForProvider = MakeOptional<Span<uint8_t>>(someMeta);
     result                    = tester.Invoke(QueryImage::Id, input);
     ASSERT_TRUE(result.IsSuccess());
     EXPECT_EQ(result.status->GetStatusCode().GetStatus(), Status::Success);
 }
 
 TEST_F(TestOtaProviderCluster, NotifyUpdateApplied)
 {
     OtaProviderServer otaProvider(kRootEndpointId);
     ClusterTester tester(otaProvider);
 
     NotifyUpdateApplied::Type input;
 
     // Without a delegate, error out.
     auto result = tester.Invoke(NotifyUpdateApplied::Id, input);
     ASSERT_TRUE(result.IsSuccess());
     EXPECT_EQ(result.status->GetStatusCode().GetStatus(), Status::UnsupportedCommand);
 
     MockDelegate mockDelegate;
     otaProvider.SetDelegate(&mockDelegate);
 
     // Update token is not valid.
     result = tester.Invoke(NotifyUpdateApplied::Id, input);
     ASSERT_TRUE(result.IsSuccess());
     EXPECT_EQ(result.status->GetStatusCode().GetStatus(), Status::InvalidCommand);
 
     constexpr uint8_t kUpdateToken[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
 
     input.softwareVersion = 123;
     input.updateToken     = Span<const uint8_t>(kUpdateToken);
     result                = tester.Invoke(NotifyUpdateApplied::Id, input);
     ASSERT_TRUE(result.IsSuccess());
     EXPECT_EQ(result.status->GetStatusCode().GetStatus(), Status::Success);
 }
 
 TEST_F(TestOtaProviderCluster, ApplyUpdateRequest)
 {
     OtaProviderServer otaProvider(kRootEndpointId);
     ClusterTester tester(otaProvider);
 
     ApplyUpdateRequest::Type input;
 
     // Without a delegate, error out.
     auto result = tester.Invoke(ApplyUpdateRequest::Id, input);
     ASSERT_TRUE(result.IsSuccess());
     EXPECT_EQ(result.status->GetStatusCode().GetStatus(), Status::UnsupportedCommand);
 
     MockDelegate mockDelegate;
     otaProvider.SetDelegate(&mockDelegate);
 
     // Update token is not valid.
     result = tester.Invoke(ApplyUpdateRequest::Id, input);
     ASSERT_TRUE(result.IsSuccess());
     EXPECT_EQ(result.status->GetStatusCode().GetStatus(), Status::InvalidCommand);
 
     constexpr uint8_t kUpdateToken[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
     input.newVersion                 = 124;
     input.updateToken                = Span<const uint8_t>(kUpdateToken);
     result                           = tester.Invoke(ApplyUpdateRequest::Id, input);
     ASSERT_TRUE(result.IsSuccess());
     EXPECT_EQ(result.status->GetStatusCode().GetStatus(), Status::Success);
 }
 
 } // namespace