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

class StorageReaderStub : public IProvisionStorageReader
{
public:
    CHIP_ERROR GetSerialNumber(char *, size_t) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetVendorId(uint16_t &) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetVendorName(char *, size_t) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetProductId(uint16_t &) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetProductName(char *, size_t) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetProductLabel(char *, size_t) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetProductURL(char *, size_t) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetPartNumber(char *, size_t) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetHardwareVersion(uint16_t &) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetHardwareVersionString(char *, size_t) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetManufacturingDate(uint8_t *, size_t, size_t &) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetPersistentUniqueId(uint8_t *, size_t, size_t &) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetSetupDiscriminator(uint16_t &) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetSpake2pIterationCount(uint32_t &) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetSpake2pSalt(char *, size_t, size_t &) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetSpake2pVerifier(char *, size_t, size_t &) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetSetupPayload(uint8_t *, size_t, size_t &) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetFirmwareInformation(MutableByteSpan &) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetCertificationDeclaration(MutableByteSpan &) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetProductAttestationIntermediateCert(MutableByteSpan &) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetDeviceAttestationCert(MutableByteSpan &) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetProvisionVersion(char *, size_t, size_t &) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetOtaTlvEncryptionKeyId(uint32_t & value) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR DecryptUsingOtaTlvEncryptionKey(MutableByteSpan & block, uint32_t & ivOffset) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    CHIP_ERROR GetTestEventTriggerKey(MutableByteSpan & keySpan) override
    {
        VerifyOrReturnError(!forceError, CHIP_ERROR_INTERNAL);

        ByteSpan enableKeySpan = ByteSpan(mEnableKey, TestEventTriggerDelegate::kEnableKeyLength);
        TEMPORARY_RETURN_IGNORED CopySpanToMutableSpan(enableKeySpan, keySpan);
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
    StorageReaderStub provider;
    provider.SetEnableKey(kTestEnableKey1, TestEventTriggerDelegate::kEnableKeyLength);

    SilabsTestEventTriggerDelegate delegate;
    TEMPORARY_RETURN_IGNORED delegate.Init(&provider);

    ByteSpan validKeySpan(kTestEnableKey1);
    EXPECT_TRUE(delegate.DoesEnableKeyMatch(validKeySpan));
}

// Test that an invalid key does not match
TEST(TestSilabsTestEventTriggerDelegate, TestDoesEnableKeyMatch_InvalidKey)
{
    StorageReaderStub provider;
    provider.SetEnableKey(kTestEnableKey1, TestEventTriggerDelegate::kEnableKeyLength);

    SilabsTestEventTriggerDelegate delegate;
    TEMPORARY_RETURN_IGNORED delegate.Init(&provider);

    ByteSpan invalidKeySpan(kInvalidEnableKey, TestEventTriggerDelegate::kEnableKeyLength - 1);
    EXPECT_FALSE(delegate.DoesEnableKeyMatch(invalidKeySpan));
}

// Test that an empty key does not match
TEST(TestSilabsTestEventTriggerDelegate, TestDoesEnableKeyMatch_EmptyKey)
{
    StorageReaderStub provider;
    provider.SetEnableKey(kTestEnableKey1, TestEventTriggerDelegate::kEnableKeyLength);

    SilabsTestEventTriggerDelegate delegate;
    TEMPORARY_RETURN_IGNORED delegate.Init(&provider);

    EXPECT_FALSE(delegate.DoesEnableKeyMatch(ByteSpan(kZeroEnableKey)));
}

// Test that a different valid key does not match
TEST(TestSilabsTestEventTriggerDelegate, TestDoesEnableKeyMatch_DifferentValidKey)
{
    StorageReaderStub provider;
    provider.SetEnableKey(kTestEnableKey1, TestEventTriggerDelegate::kEnableKeyLength);

    SilabsTestEventTriggerDelegate delegate;
    TEMPORARY_RETURN_IGNORED delegate.Init(&provider);

    ByteSpan differentValidKeySpan(kTestEnableKey2);
    EXPECT_FALSE(delegate.DoesEnableKeyMatch(differentValidKeySpan));
}

// Test that an empty key matchs when no enable key is set
TEST(TestSilabsTestEventTriggerDelegate, TestDoesEnableKeyMatch_NoKeySet_EmptyKey)
{
    StorageReaderStub provider;

    SilabsTestEventTriggerDelegate delegate;
    TEMPORARY_RETURN_IGNORED delegate.Init(&provider);

    EXPECT_TRUE(delegate.DoesEnableKeyMatch(ByteSpan(kZeroEnableKey)));
}

// Test that a valid key does not match when no enable key is set
TEST(TestSilabsTestEventTriggerDelegate, TestDoesEnableKeyMatch_NoKeySet_ValidKey)
{
    StorageReaderStub provider;

    SilabsTestEventTriggerDelegate delegate;
    TEMPORARY_RETURN_IGNORED delegate.Init(&provider);

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
    StorageReaderStub provider;
    provider.SetForceError(true);

    SilabsTestEventTriggerDelegate delegate;
    TEMPORARY_RETURN_IGNORED delegate.Init(&provider);

    ByteSpan validKeySpan(kTestEnableKey1);
    EXPECT_FALSE(delegate.DoesEnableKeyMatch(validKeySpan));
}

// Test that an invalid key does not match when GetTestEventTriggerKey returns an error
TEST(TestSilabsTestEventTriggerDelegate, TestDoesEnableKeyMatch_GetKeyError_InvalidKey)
{
    StorageReaderStub provider;
    provider.SetForceError(true);

    SilabsTestEventTriggerDelegate delegate;
    TEMPORARY_RETURN_IGNORED delegate.Init(&provider);

    ByteSpan invalidKeySpan(kInvalidEnableKey);
    EXPECT_FALSE(delegate.DoesEnableKeyMatch(invalidKeySpan));
}

// Test that an empty key matchs when GetTestEventTriggerKey returns an error
TEST(TestSilabsTestEventTriggerDelegate, TestDoesEnableKeyMatch_GetKeyError_EmptyKey)
{
    StorageReaderStub provider;
    provider.SetForceError(true);

    SilabsTestEventTriggerDelegate delegate;
    TEMPORARY_RETURN_IGNORED delegate.Init(&provider);

    EXPECT_TRUE(delegate.DoesEnableKeyMatch(ByteSpan(kZeroEnableKey)));
}

// Test that Init function initializes the delegate with a valid provider
TEST(TestSilabsTestEventTriggerDelegate, TestInit_ValidProvider)
{
    StorageReaderStub provider;
    SilabsTestEventTriggerDelegate delegate;
    EXPECT_EQ(delegate.Init(&provider), CHIP_NO_ERROR);
}

// Test that Init function returns an error when initialized with a null provider
TEST(TestSilabsTestEventTriggerDelegate, TestInit_NullProvider)
{
    SilabsTestEventTriggerDelegate delegate;
    EXPECT_EQ(delegate.Init(nullptr), CHIP_ERROR_INVALID_ARGUMENT);
}
