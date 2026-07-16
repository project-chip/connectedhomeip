/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/tests/AppTestContext.h>
#include <credentials/jcm/TrustVerification.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::Credentials::JCM;

class TestTrustVerification : public chip::Testing::AppContext
{
public:
    // Performs shared setup for all tests in the test suite.  Run once for the whole suite.
    static void SetUpTestSuite()
    {
        ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);

        chip::Testing::AppContext::SetUpTestSuite();
    }

    // Performs shared teardown for all tests in the test suite.  Run once for the whole suite.
    static void TearDownTestSuite()
    {
        AppContext::TearDownTestSuite();

        Platform::MemoryShutdown();
    }

    void TestEnumToStringError();
    void TestEnumToStringStage();
    void TestTrustVerificationInfoCleanup();

protected:
    void SetUp() override { AppContext::SetUp(); }

    void TearDown() override { AppContext::TearDown(); }
};

TEST_F_FROM_FIXTURE(TestTrustVerification, TestEnumToStringError)
{
    EXPECT_EQ(EnumToString(TrustVerificationError::kSuccess), std::string("SUCCESS"));
    EXPECT_EQ(EnumToString(TrustVerificationError::kAsync), std::string("ASYNC_OPERATION"));
    EXPECT_EQ(EnumToString(TrustVerificationError::kInvalidAdministratorEndpointId),
              std::string("INVALID_ADMINISTRATOR_ENDPOINT_ID"));
    EXPECT_EQ(EnumToString(TrustVerificationError::kInvalidAdministratorFabricIndex),
              std::string("INVALID_ADMINISTRATOR_FABRIC_INDEX"));
    EXPECT_EQ(EnumToString(TrustVerificationError::kInvalidAdministratorCAT), std::string("INVALID_ADMINISTRATOR_CAT"));
    EXPECT_EQ(EnumToString(TrustVerificationError::kTrustVerificationDelegateNotSet),
              std::string("TRUST_VERIFICATION_DELEGATE_NOT_SET"));
    EXPECT_EQ(EnumToString(TrustVerificationError::kUserDeniedConsent), std::string("USER_DENIED_CONSENT"));
    EXPECT_EQ(EnumToString(TrustVerificationError::kVendorIdVerificationFailed), std::string("VENDOR_ID_VERIFICATION_FAILED"));
    EXPECT_EQ(EnumToString(TrustVerificationError::kInternalError), std::string("INTERNAL_ERROR"));
}

TEST_F_FROM_FIXTURE(TestTrustVerification, TestEnumToStringStage)
{
    EXPECT_EQ(EnumToString(kIdle), std::string("IDLE"));
    EXPECT_EQ(EnumToString(kVerifyingAdministratorInformation), std::string("VERIFYING_ADMINISTRATOR_INFORMATION"));
    EXPECT_EQ(EnumToString(kPerformingVendorIDVerification), std::string("PERFORMING_VENDOR_ID_VERIFICATION_PROCEDURE"));
    EXPECT_EQ(EnumToString(kAskingUserForConsent), std::string("ASKING_USER_FOR_CONSENT"));
    EXPECT_EQ(EnumToString(kComplete), std::string("COMPLETE"));
    EXPECT_EQ(EnumToString(kError), std::string("ERROR"));
}

TEST_F_FROM_FIXTURE(TestTrustVerification, TestTrustVerificationInfoCleanup)
{
    TrustVerificationInfo info;

    // Set non-defaults
    info.adminEndpointId  = 5;
    info.adminFabricIndex = 7;
    info.adminVendorId    = VendorId::TestVendor1;
    info.adminFabricId    = static_cast<FabricId>(0xdeadbeef);

    // Allocate some memory into the buffers
    info.rootPublicKey.Calloc(16);
    info.adminRCAC.Calloc(8);
    info.adminICAC.Calloc(12);
    info.adminNOC.Calloc(20);

    // Sanity check allocations succeeded
    EXPECT_GT(info.rootPublicKey.AllocatedSize(), 0u);
    EXPECT_GT(info.adminRCAC.AllocatedSize(), 0u);
    EXPECT_GT(info.adminICAC.AllocatedSize(), 0u);
    EXPECT_GT(info.adminNOC.AllocatedSize(), 0u);

    // Call cleanup and verify defaults and that buffers were freed
    info.Cleanup();

    EXPECT_EQ(info.adminEndpointId, kInvalidEndpointId);
    EXPECT_EQ(info.adminFabricIndex, kUndefinedFabricIndex);
    EXPECT_EQ(info.adminVendorId, VendorId::Common);
    EXPECT_EQ(info.adminFabricId, kUndefinedFabricId);

    EXPECT_EQ(info.rootPublicKey.AllocatedSize(), 0u);
    EXPECT_EQ(info.adminRCAC.AllocatedSize(), 0u);
    EXPECT_EQ(info.adminICAC.AllocatedSize(), 0u);
    EXPECT_EQ(info.adminNOC.AllocatedSize(), 0u);
}
