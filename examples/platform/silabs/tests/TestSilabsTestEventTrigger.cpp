/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>

#include <SilabsTestEventTriggerDelegate.h>
#include <headers/ProvisionedDataProvider.h>
#include <lib/support/Span.h>

using namespace chip;
using namespace chip::DeviceLayer::Silabs::Provision;

namespace {
const uint8_t kTestEnableKey1[TestEventTriggerDelegate::kEnableKeyLength]       = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                                                                    0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };
const uint8_t kTestEnableKey2[TestEventTriggerDelegate::kEnableKeyLength]       = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                                                                    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10 };
const uint8_t kInvalidEnableKey[TestEventTriggerDelegate::kEnableKeyLength - 1] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                                                                    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
const uint8_t kZeroEnableKey[TestEventTriggerDelegate::kEnableKeyLength]        = { 0 };

class ProviderStub : public ProvisionedDataProvider
{
public:
    CHIP_ERROR GetTestEventTriggerKey(MutableByteSpan & keySpan) override
    {
        VerifyOrReturnError(!forceError, CHIP_ERROR_INTERNAL);

        ByteSpan enableKeySpan = ByteSpan(mEnableKey, TestEventTriggerDelegate::kEnableKeyLength);
        CopySpanToMutableSpan(enableKeySpan, keySpan);
        return CHIP_NO_ERROR;
    }

    void SetEnableKey(const uint8_t * key, size_t length)
    {
        if (length == sizeof(mEnableKey))
        {
            memcpy(mEnableKey, key, length);
        }
    }

    void SetForceError(bool value) { forceError = value; }

private:
    uint8_t mEnableKey[TestEventTriggerDelegate::kEnableKeyLength] = { 0 };
    bool forceError                                                = false;
};

} // namespace

// Test that a valid key matches
TEST(TestSilabsTestEventTriggerDelegate, TestDoesEnableKeyMatch_ValidKey)
{
    ProviderStub provider;
    provider.SetEnableKey(kTestEnableKey1, TestEventTriggerDelegate::kEnableKeyLength);

    SilabsTestEventTriggerDelegate delegate;
    delegate.Init(&provider);

    ByteSpan validKeySpan(kTestEnableKey1);
    EXPECT_TRUE(delegate.DoesEnableKeyMatch(validKeySpan));
}

// Test that an invalid key does not match
TEST(TestSilabsTestEventTriggerDelegate, TestDoesEnableKeyMatch_InvalidKey)
{
    ProviderStub provider;
    provider.SetEnableKey(kTestEnableKey1, TestEventTriggerDelegate::kEnableKeyLength);

    SilabsTestEventTriggerDelegate delegate;
    delegate.Init(&provider);

    ByteSpan invalidKeySpan(kInvalidEnableKey, TestEventTriggerDelegate::kEnableKeyLength - 1);
    EXPECT_FALSE(delegate.DoesEnableKeyMatch(invalidKeySpan));
}

// Test that an empty key does not match
TEST(TestSilabsTestEventTriggerDelegate, TestDoesEnableKeyMatch_EmptyKey)
{
    ProviderStub provider;
    provider.SetEnableKey(kTestEnableKey1, TestEventTriggerDelegate::kEnableKeyLength);

    SilabsTestEventTriggerDelegate delegate;
    delegate.Init(&provider);

    EXPECT_FALSE(delegate.DoesEnableKeyMatch(ByteSpan(kZeroEnableKey)));
}

// Test that a different valid key does not match
TEST(TestSilabsTestEventTriggerDelegate, TestDoesEnableKeyMatch_DifferentValidKey)
{
    ProviderStub provider;
    provider.SetEnableKey(kTestEnableKey1, TestEventTriggerDelegate::kEnableKeyLength);

    SilabsTestEventTriggerDelegate delegate;
    delegate.Init(&provider);

    ByteSpan differentValidKeySpan(kTestEnableKey2);
    EXPECT_FALSE(delegate.DoesEnableKeyMatch(differentValidKeySpan));
}

// Test that an empty key matchs when no enable key is set
TEST(TestSilabsTestEventTriggerDelegate, TestDoesEnableKeyMatch_NoKeySet_EmptyKey)
{
    ProviderStub provider;

    SilabsTestEventTriggerDelegate delegate;
    delegate.Init(&provider);

    EXPECT_TRUE(delegate.DoesEnableKeyMatch(ByteSpan(kZeroEnableKey)));
}

// Test that a valid key does not match when no enable key is set
TEST(TestSilabsTestEventTriggerDelegate, TestDoesEnableKeyMatch_NoKeySet_ValidKey)
{
    ProviderStub provider;

    SilabsTestEventTriggerDelegate delegate;
    delegate.Init(&provider);

    ByteSpan validKeySpan(kTestEnableKey1);
    EXPECT_FALSE(delegate.DoesEnableKeyMatch(validKeySpan));
}

// Test that a valid key does not match when provider is not set
TEST(TestSilabsTestEventTriggerDelegate, TestDoesEnableKeyMatch_NoStorage_ValidKey)
{
    SilabsTestEventTriggerDelegate delegate;

    ByteSpan validKeySpan(kTestEnableKey1);
    EXPECT_FALSE(delegate.DoesEnableKeyMatch(validKeySpan));
}

// Test that an invalid key does not match when provider is not set
TEST(TestSilabsTestEventTriggerDelegate, TestDoesEnableKeyMatch_NoStorage_InvalidKey)
{
    SilabsTestEventTriggerDelegate delegate;

    ByteSpan invalidKeySpan(kInvalidEnableKey);
    EXPECT_FALSE(delegate.DoesEnableKeyMatch(invalidKeySpan));
}

// Test that an empty key matchs when provider is not set
TEST(TestSilabsTestEventTriggerDelegate, TestDoesEnableKeyMatch_NoStorage_EmptyKey)
{
    SilabsTestEventTriggerDelegate delegate;

    EXPECT_TRUE(delegate.DoesEnableKeyMatch(ByteSpan(kZeroEnableKey)));
}

// Test that a valid key does not match when GetTestEventTriggerKey returns an error
TEST(TestSilabsTestEventTriggerDelegate, TestDoesEnableKeyMatch_GetKeyError_ValidKey)
{
    ProviderStub provider;
    provider.SetForceError(true);

    SilabsTestEventTriggerDelegate delegate;
    delegate.Init(&provider);

    ByteSpan validKeySpan(kTestEnableKey1);
    EXPECT_FALSE(delegate.DoesEnableKeyMatch(validKeySpan));
}

// Test that an invalid key does not match when GetTestEventTriggerKey returns an error
TEST(TestSilabsTestEventTriggerDelegate, TestDoesEnableKeyMatch_GetKeyError_InvalidKey)
{
    ProviderStub provider;
    provider.SetForceError(true);

    SilabsTestEventTriggerDelegate delegate;
    delegate.Init(&provider);

    ByteSpan invalidKeySpan(kInvalidEnableKey);
    EXPECT_FALSE(delegate.DoesEnableKeyMatch(invalidKeySpan));
}

// Test that an empty key matchs when GetTestEventTriggerKey returns an error
TEST(TestSilabsTestEventTriggerDelegate, TestDoesEnableKeyMatch_GetKeyError_EmptyKey)
{
    ProviderStub provider;
    provider.SetForceError(true);

    SilabsTestEventTriggerDelegate delegate;
    delegate.Init(&provider);

    EXPECT_TRUE(delegate.DoesEnableKeyMatch(ByteSpan(kZeroEnableKey)));
}

// Test that Init function initializes the delegate with a valid provider
TEST(TestSilabsTestEventTriggerDelegate, TestInit_ValidProvider)
{
    ProviderStub provider;
    SilabsTestEventTriggerDelegate delegate;
    EXPECT_EQ(delegate.Init(&provider), CHIP_NO_ERROR);
}

// Test that Init function returns an error when initialized with a null provider
TEST(TestSilabsTestEventTriggerDelegate, TestInit_NullProvider)
{
    SilabsTestEventTriggerDelegate delegate;
    EXPECT_EQ(delegate.Init(nullptr), CHIP_ERROR_INVALID_ARGUMENT);
}
