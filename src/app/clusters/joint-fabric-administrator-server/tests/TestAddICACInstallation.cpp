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

/**
 * Tests for the JointFabricAdministrator cluster's AddICAC certificate
 * installation path, specifically the StoreCrossSignedICAC delegate interface.
 */

#include <app/server/JointFabricAdministrator.h>
#include <credentials/CHIPCert.h>
#include <credentials/tests/CHIPCert_test_vectors.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app;
using namespace chip::Credentials;
using namespace chip::TestCerts;

namespace {

/**
 * A test delegate that captures the cross-signed ICAC passed to StoreCrossSignedICAC.
 */
class TestJFADelegate : public JointFabricAdministrator::Delegate
{
public:
    CHIP_ERROR GetIcacCsr(MutableByteSpan & icacCsr) override { return CHIP_NO_ERROR; }

    CHIP_ERROR StoreCrossSignedICAC(const ByteSpan & crossSignedICAC) override
    {
        mStoreCallCount++;
        mStoredICACLen = crossSignedICAC.size();
        if (crossSignedICAC.size() <= sizeof(mStoredICACBuf))
        {
            memcpy(mStoredICACBuf, crossSignedICAC.data(), crossSignedICAC.size());
        }
        return mReturnError;
    }

    uint8_t mStoredICACBuf[Credentials::kMaxDERCertLength] = { 0 };
    size_t mStoredICACLen                                  = 0;
    int mStoreCallCount                                    = 0;
    CHIP_ERROR mReturnError                                = CHIP_NO_ERROR;
};

/**
 * A test delegate that fails on StoreCrossSignedICAC.
 */
class FailingJFADelegate : public JointFabricAdministrator::Delegate
{
public:
    CHIP_ERROR GetIcacCsr(MutableByteSpan & icacCsr) override { return CHIP_NO_ERROR; }

    CHIP_ERROR StoreCrossSignedICAC(const ByteSpan & crossSignedICAC) override { return CHIP_ERROR_PERSISTED_STORAGE_FAILED; }
};

} // namespace

class TestAddICACInstallation : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * Verify that the default JointFabricAdministrator::Delegate::StoreCrossSignedICAC
 * returns CHIP_NO_ERROR (no-op base implementation).
 */
TEST_F(TestAddICACInstallation, DefaultDelegateStoreCrossSignedICACIsNoop)
{
    JointFabricAdministrator::Delegate delegate;
    ByteSpan testICACSpan(sTestCert_ICA01_Chip);
    EXPECT_EQ(delegate.StoreCrossSignedICAC(testICACSpan), CHIP_NO_ERROR);
}

/**
 * Verify that a custom delegate receives the cross-signed ICAC bytes correctly.
 */
TEST_F(TestAddICACInstallation, DelegateStoreCrossSignedICACReceivesCorrectBytes)
{
    TestJFADelegate delegate;
    ByteSpan testICACSpan(sTestCert_ICA01_Chip);

    EXPECT_EQ(delegate.StoreCrossSignedICAC(testICACSpan), CHIP_NO_ERROR);
    EXPECT_EQ(delegate.mStoreCallCount, 1);
    EXPECT_EQ(delegate.mStoredICACLen, testICACSpan.size());
    EXPECT_EQ(memcmp(delegate.mStoredICACBuf, testICACSpan.data(), testICACSpan.size()), 0);
}

/**
 * Verify that StoreCrossSignedICAC is only called once per AddICAC invocation.
 */
TEST_F(TestAddICACInstallation, DelegateStoreCrossSignedICACCalledOnce)
{
    TestJFADelegate delegate;
    ByteSpan testICACSpan(sTestCert_ICA01_Chip);

    // First call succeeds
    EXPECT_EQ(delegate.StoreCrossSignedICAC(testICACSpan), CHIP_NO_ERROR);
    EXPECT_EQ(delegate.mStoreCallCount, 1);

    // If called again (e.g., retry scenario), the count increases
    EXPECT_EQ(delegate.StoreCrossSignedICAC(testICACSpan), CHIP_NO_ERROR);
    EXPECT_EQ(delegate.mStoreCallCount, 2);
}

/**
 * Verify that an error returned from StoreCrossSignedICAC is propagated.
 * This mirrors the expected behavior in HandleAddICAC when installation fails.
 */
TEST_F(TestAddICACInstallation, DelegateStoreCrossSignedICACFailurePropagates)
{
    TestJFADelegate delegate;
    delegate.mReturnError = CHIP_ERROR_PERSISTED_STORAGE_FAILED;

    ByteSpan testICACSpan(sTestCert_ICA01_Chip);
    CHIP_ERROR err = delegate.StoreCrossSignedICAC(testICACSpan);
    EXPECT_EQ(err, CHIP_ERROR_PERSISTED_STORAGE_FAILED);
}

/**
 * Verify that an empty ICAC span can be passed without crashing.
 * Expected behavior: CHIP_NO_ERROR (or implementation-specific error), no crash.
 */
TEST_F(TestAddICACInstallation, DelegateStoreCrossSignedICACWithEmptySpan)
{
    TestJFADelegate delegate;
    ByteSpan emptySpan;

    CHIP_ERROR err = delegate.StoreCrossSignedICAC(emptySpan);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(delegate.mStoredICACLen, 0u);
}

/**
 * Verify that a second ICA certificate (ICA02) can also be stored correctly.
 * Expected behavior: tests backward-compatibility with different cert sizes.
 */
TEST_F(TestAddICACInstallation, DelegateStoreCrossSignedICACWithICA02)
{
    TestJFADelegate delegate;
    ByteSpan testICACSpan(sTestCert_ICA02_Chip);

    EXPECT_EQ(delegate.StoreCrossSignedICAC(testICACSpan), CHIP_NO_ERROR);
    EXPECT_EQ(delegate.mStoredICACLen, testICACSpan.size());
    EXPECT_EQ(memcmp(delegate.mStoredICACBuf, testICACSpan.data(), testICACSpan.size()), 0);
}

/**
 * Verify that the JointFabricAdministrator singleton can have a delegate set
 * and retrieved correctly.
 */
TEST_F(TestAddICACInstallation, JointFabricAdministratorSetGetDelegate)
{
    TestJFADelegate delegate;
    auto & jfa = JointFabricAdministrator::GetInstance();

    EXPECT_EQ(jfa.SetDelegate(&delegate), CHIP_NO_ERROR);
    EXPECT_EQ(jfa.GetDelegate(), &delegate);
}
