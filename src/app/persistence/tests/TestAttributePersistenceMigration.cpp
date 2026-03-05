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
#include <pw_unit_test/framework.h>

#include <app/ConcreteAttributePath.h>
#include <app/DefaultSafeAttributePersistenceProvider.h>
#include <app/persistence/AttributePersistenceMigration.h>
#include <app/persistence/DefaultAttributePersistenceProvider.h>
#include <lib/core/CHIPError.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/Scoped.h>
#include <lib/support/Span.h>
#include <lib/support/TestPersistentStorageDelegate.h>

namespace {

using namespace chip;
using namespace chip::app;

// Used by TestMigrationDeletesFromSafeOnWriteFailure to poison the normal-provider
// key after the initial ReadValue check has already passed.
TestPersistentStorageDelegate * sStorageDelegateForPoisoning = nullptr;

CHIP_ERROR PoisonAfterReadMigrator(const ConcreteAttributePath & attrPath, SafeAttributePersistenceProvider & provider,
                                   MutableByteSpan & buffer)
{
    CHIP_ERROR err = DefaultMigrators::ScalarValue<uint32_t>(attrPath, provider, buffer);
    // Poison the normal provider key now, after ReadValue already returned CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND
    sStorageDelegateForPoisoning->AddPoisonKey(
        DefaultStorageKeyAllocator::AttributeValue(attrPath.mEndpointId, attrPath.mClusterId, attrPath.mAttributeId).KeyName());
    return err;
}

// Single attribute migrated successfully: value appears in AttributePersistence, deleted from SafeAttribute.
TEST(TestAttributePersistenceMigration, TestMigrationSuccess)
{
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider ramProvider;
    DefaultSafeAttributePersistenceProvider safeRamProvider;
    ASSERT_EQ(ramProvider.Init(&storageDelegate), CHIP_NO_ERROR);
    ASSERT_EQ(safeRamProvider.Init(&storageDelegate), CHIP_NO_ERROR);

    const ConcreteAttributePath path(1, 2, 3);
    const ConcreteClusterPath cluster(1, 2);
    constexpr uint32_t kValueToStore = 42;

    // Store a value in the safe provider
    ASSERT_EQ(safeRamProvider.WriteScalarValue(path, kValueToStore), CHIP_NO_ERROR);

    // Run migration
    const AttrMigrationData attributesToMigrate[] = {
        { 3, &DefaultMigrators::ScalarValue<uint32_t> },
    };
    uint8_t buf[128] = {};
    MutableByteSpan buffer(buf);
    EXPECT_EQ(
        MigrateFromSafeToAttributePersistenceProvider(safeRamProvider, ramProvider, cluster, Span(attributesToMigrate), buffer),
        CHIP_NO_ERROR);

    // Value should now exist in the normal provider
    {
        uint8_t readBuf[sizeof(uint32_t)] = {};
        MutableByteSpan readBuffer(readBuf);
        EXPECT_EQ(ramProvider.ReadValue(path, readBuffer), CHIP_NO_ERROR);

        uint32_t readValue = 0;
        memcpy(&readValue, readBuffer.data(), sizeof(uint32_t));
        EXPECT_EQ(readValue, kValueToStore);
    }

    // Value should be deleted from the safe provider
    {
        uint8_t readBuf[sizeof(uint32_t)] = {};
        MutableByteSpan readBuffer(readBuf);
        EXPECT_EQ(safeRamProvider.SafeReadValue(path, readBuffer), CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    }
}

// Attribute not present in SafeAttribute: AttributePersistence untouched.
TEST(TestAttributePersistenceMigration, TestMigrationSkipsAbsentAttribute)
{
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider ramProvider;
    DefaultSafeAttributePersistenceProvider safeRamProvider;
    ASSERT_EQ(ramProvider.Init(&storageDelegate), CHIP_NO_ERROR);
    ASSERT_EQ(safeRamProvider.Init(&storageDelegate), CHIP_NO_ERROR);

    const ConcreteAttributePath path(1, 2, 3);
    const ConcreteClusterPath cluster(1, 2);

    // Don't write anything to the safe provider

    const AttrMigrationData attributesToMigrate[] = {
        { 3, &DefaultMigrators::ScalarValue<uint32_t> },
    };
    uint8_t buf[128] = {};
    MutableByteSpan buffer(buf);
    EXPECT_EQ(
        MigrateFromSafeToAttributePersistenceProvider(safeRamProvider, ramProvider, cluster, Span(attributesToMigrate), buffer),
        CHIP_NO_ERROR);

    // Normal provider should have nothing
    {
        uint8_t readBuf[sizeof(uint32_t)] = {};
        MutableByteSpan readBuffer(readBuf);
        EXPECT_EQ(ramProvider.ReadValue(path, readBuffer), CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    }
}

// When WriteValue to AttributePersistence fails, the value is still deleted from SafeAttribute ("one time" migration guarantee).
// Uses a custom migrator (PoisonAfterReadMigrator) that poisons the normal provider key after the migrator runs, so that the
// initial ReadValue check passes (returns CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND) but the subsequent WriteValue
// hits the poisoned key and fails.
TEST(TestAttributePersistenceMigration, TestMigrationDeletesFromSafeOnWriteFailure)
{
    TestPersistentStorageDelegate storageDelegate;
    ScopedChange ptrGuard(sStorageDelegateForPoisoning, &storageDelegate);

    DefaultAttributePersistenceProvider ramProvider;
    DefaultSafeAttributePersistenceProvider safeRamProvider;
    ASSERT_EQ(ramProvider.Init(&storageDelegate), CHIP_NO_ERROR);
    ASSERT_EQ(safeRamProvider.Init(&storageDelegate), CHIP_NO_ERROR);

    const ConcreteAttributePath path(1, 2, 3);
    const ConcreteClusterPath cluster(1, 2);
    constexpr uint32_t kValueToStore = 42;

    // Store a value in the safe provider
    ASSERT_EQ(safeRamProvider.WriteScalarValue(path, kValueToStore), CHIP_NO_ERROR);

    const AttrMigrationData attributesToMigrate[] = {
        { 3, &PoisonAfterReadMigrator },
    };
    uint8_t buf[128] = {};
    MutableByteSpan buffer(buf);
    EXPECT_NE(
        MigrateFromSafeToAttributePersistenceProvider(safeRamProvider, ramProvider, cluster, Span(attributesToMigrate), buffer),
        CHIP_NO_ERROR);

    // Value should still be deleted from the safe provider
    {
        uint8_t readBuf[sizeof(uint32_t)] = {};
        MutableByteSpan readBuffer(readBuf);
        EXPECT_EQ(safeRamProvider.SafeReadValue(path, readBuffer), CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    }
}

// After a successful migration, calling it again is a no-op (value already deleted from SafeAttribute).
TEST(TestAttributePersistenceMigration, TestMigrationTwice)
{
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider ramProvider;
    DefaultSafeAttributePersistenceProvider safeRamProvider;
    ASSERT_EQ(ramProvider.Init(&storageDelegate), CHIP_NO_ERROR);
    ASSERT_EQ(safeRamProvider.Init(&storageDelegate), CHIP_NO_ERROR);

    const ConcreteAttributePath path(1, 2, 3);
    const ConcreteClusterPath cluster(1, 2);
    constexpr uint32_t kValueToStore = 42;

    ASSERT_EQ(safeRamProvider.WriteScalarValue(path, kValueToStore), CHIP_NO_ERROR);

    const AttrMigrationData attributesToMigrate[] = {
        { 3, &DefaultMigrators::ScalarValue<uint32_t> },
    };
    uint8_t buf[128] = {};
    MutableByteSpan buffer(buf);

    // First migration
    EXPECT_EQ(
        MigrateFromSafeToAttributePersistenceProvider(safeRamProvider, ramProvider, cluster, Span(attributesToMigrate), buffer),
        CHIP_NO_ERROR);

    // Overwrite the value in dstProvider to detect if a second migration would overwrite it
    constexpr uint32_t kNewValue = 99;
    ASSERT_EQ(ramProvider.WriteValue(path, ByteSpan(reinterpret_cast<const uint8_t *>(&kNewValue), sizeof(kNewValue))),
              CHIP_NO_ERROR);

    // Second migration, shouldn't affect the new value and return no error.
    EXPECT_EQ(
        MigrateFromSafeToAttributePersistenceProvider(safeRamProvider, ramProvider, cluster, Span(attributesToMigrate), buffer),
        CHIP_NO_ERROR);

    // The new value should still be intact
    {
        uint8_t readBuf[sizeof(uint32_t)] = {};
        MutableByteSpan readBuffer(readBuf);
        EXPECT_EQ(ramProvider.ReadValue(path, readBuffer), CHIP_NO_ERROR);

        uint32_t readValue = 0;
        ASSERT_EQ(readBuffer.size(), sizeof(uint32_t));
        memcpy(&readValue, readBuffer.data(), sizeof(uint32_t));
        EXPECT_EQ(readValue, kNewValue);
    }
}

// Two attributes in the list, only the second is present in SafeAttribute. First is skipped, second is migrated.
TEST(TestAttributePersistenceMigration, TestMultipleAttributesMixedPresence)
{
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider ramProvider;
    DefaultSafeAttributePersistenceProvider safeRamProvider;
    ASSERT_EQ(ramProvider.Init(&storageDelegate), CHIP_NO_ERROR);
    ASSERT_EQ(safeRamProvider.Init(&storageDelegate), CHIP_NO_ERROR);

    const ConcreteAttributePath pathA(1, 2, 3);
    const ConcreteAttributePath pathB(1, 2, 4);
    const ConcreteClusterPath cluster(1, 2);
    constexpr uint32_t kValueB = 77;

    // Only write attribute B to the safe provider
    ASSERT_EQ(safeRamProvider.WriteScalarValue(pathB, kValueB), CHIP_NO_ERROR);

    const AttrMigrationData attributesToMigrate[] = {
        { 3, &DefaultMigrators::ScalarValue<uint32_t> },
        { 4, &DefaultMigrators::ScalarValue<uint32_t> },
    };
    uint8_t buf[128] = {};
    MutableByteSpan buffer(buf);
    EXPECT_EQ(
        MigrateFromSafeToAttributePersistenceProvider(safeRamProvider, ramProvider, cluster, Span(attributesToMigrate), buffer),
        CHIP_NO_ERROR);

    // Attribute A should not exist in the normal provider
    {
        uint8_t readBuf[sizeof(uint32_t)] = {};
        MutableByteSpan readBuffer(readBuf);
        EXPECT_EQ(ramProvider.ReadValue(pathA, readBuffer), CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    }

    // Attribute B should be migrated to the normal provider
    {
        uint8_t readBuf[sizeof(uint32_t)] = {};
        MutableByteSpan readBuffer(readBuf);
        EXPECT_EQ(ramProvider.ReadValue(pathB, readBuffer), CHIP_NO_ERROR);

        uint32_t readValue = 0;
        ASSERT_EQ(readBuffer.size(), sizeof(uint32_t));
        memcpy(&readValue, readBuffer.data(), sizeof(uint32_t));
        EXPECT_EQ(readValue, kValueB);
    }

    // Attribute B should be deleted from the safe provider
    {
        uint8_t readBuf[sizeof(uint32_t)] = {};
        MutableByteSpan readBuffer(readBuf);
        EXPECT_EQ(safeRamProvider.SafeReadValue(pathB, readBuffer), CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    }
}

// Two attributes both present in SafeAttribute: both are migrated and deleted.
TEST(TestAttributePersistenceMigration, TestMultipleAttributesAllPresent)
{
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider ramProvider;
    DefaultSafeAttributePersistenceProvider safeRamProvider;
    ASSERT_EQ(ramProvider.Init(&storageDelegate), CHIP_NO_ERROR);
    ASSERT_EQ(safeRamProvider.Init(&storageDelegate), CHIP_NO_ERROR);

    const ConcreteAttributePath pathA(1, 2, 3);
    const ConcreteAttributePath pathB(1, 2, 4);
    const ConcreteClusterPath cluster(1, 2);
    constexpr uint32_t kValueA = 10;
    constexpr uint32_t kValueB = 20;

    ASSERT_EQ(safeRamProvider.WriteScalarValue(pathA, kValueA), CHIP_NO_ERROR);
    ASSERT_EQ(safeRamProvider.WriteScalarValue(pathB, kValueB), CHIP_NO_ERROR);

    const AttrMigrationData attributesToMigrate[] = {
        { 3, &DefaultMigrators::ScalarValue<uint32_t> },
        { 4, &DefaultMigrators::ScalarValue<uint32_t> },
    };
    uint8_t buf[128] = {};
    MutableByteSpan buffer(buf);
    EXPECT_EQ(
        MigrateFromSafeToAttributePersistenceProvider(safeRamProvider, ramProvider, cluster, Span(attributesToMigrate), buffer),
        CHIP_NO_ERROR);

    // Both should be in the normal provider
    {
        uint8_t readBuf[sizeof(uint32_t)] = {};
        MutableByteSpan readBuffer(readBuf);
        EXPECT_EQ(ramProvider.ReadValue(pathA, readBuffer), CHIP_NO_ERROR);

        uint32_t readValue = 0;
        ASSERT_EQ(readBuffer.size(), sizeof(uint32_t));
        memcpy(&readValue, readBuffer.data(), sizeof(uint32_t));
        EXPECT_EQ(readValue, kValueA);
    }
    {
        uint8_t readBuf[sizeof(uint32_t)] = {};
        MutableByteSpan readBuffer(readBuf);
        EXPECT_EQ(ramProvider.ReadValue(pathB, readBuffer), CHIP_NO_ERROR);

        uint32_t readValue = 0;
        ASSERT_EQ(readBuffer.size(), sizeof(uint32_t));
        memcpy(&readValue, readBuffer.data(), sizeof(uint32_t));
        EXPECT_EQ(readValue, kValueB);
    }

    // Both should be deleted from the safe provider
    {
        uint8_t readBuf[sizeof(uint32_t)] = {};
        MutableByteSpan readBuffer(readBuf);
        EXPECT_EQ(safeRamProvider.SafeReadValue(pathA, readBuffer), CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    }
    {
        uint8_t readBuf[sizeof(uint32_t)] = {};
        MutableByteSpan readBuffer(readBuf);
        EXPECT_EQ(safeRamProvider.SafeReadValue(pathB, readBuffer), CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    }
}

// Empty attribute list: migration is a no-op, returns CHIP_NO_ERROR.
TEST(TestAttributePersistenceMigration, TestMigrationWithEmptyAttributeList)
{
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider ramProvider;
    DefaultSafeAttributePersistenceProvider safeRamProvider;
    ASSERT_EQ(ramProvider.Init(&storageDelegate), CHIP_NO_ERROR);
    ASSERT_EQ(safeRamProvider.Init(&storageDelegate), CHIP_NO_ERROR);

    const ConcreteClusterPath cluster(1, 2);

    uint8_t buf[128] = {};
    MutableByteSpan buffer(buf);
    EXPECT_EQ(MigrateFromSafeToAttributePersistenceProvider(safeRamProvider, ramProvider, cluster, Span<const AttrMigrationData>(),
                                                            buffer),
              CHIP_NO_ERROR);
}

// Migration using DefaultMigrators::SafeValue which reads raw bytes via SafeReadValue.
TEST(TestAttributePersistenceMigration, TestMigrationWithSafeValueMigrator)
{
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider ramProvider;
    DefaultSafeAttributePersistenceProvider safeRamProvider;
    ASSERT_EQ(ramProvider.Init(&storageDelegate), CHIP_NO_ERROR);
    ASSERT_EQ(safeRamProvider.Init(&storageDelegate), CHIP_NO_ERROR);

    const ConcreteAttributePath path(1, 2, 3);
    const ConcreteClusterPath cluster(1, 2);

    // Write raw bytes to the safe provider
    const uint8_t kRawValue[] = { 0xDE, 0xAD, 0xBE, 0xEF };
    ASSERT_EQ(safeRamProvider.SafeWriteValue(path, ByteSpan(kRawValue)), CHIP_NO_ERROR);

    // Migrate using SafeValue (raw byte migrator)
    const AttrMigrationData attributesToMigrate[] = {
        { 3, &DefaultMigrators::SafeValue },
    };
    uint8_t buf[128] = {};
    MutableByteSpan buffer(buf);
    EXPECT_EQ(
        MigrateFromSafeToAttributePersistenceProvider(safeRamProvider, ramProvider, cluster, Span(attributesToMigrate), buffer),
        CHIP_NO_ERROR);

    // Value should now exist in the normal provider with the same raw bytes
    {
        uint8_t readBuf[sizeof(kRawValue)] = {};
        MutableByteSpan readBuffer(readBuf);
        EXPECT_EQ(ramProvider.ReadValue(path, readBuffer), CHIP_NO_ERROR);
        EXPECT_EQ(readBuffer.size(), sizeof(kRawValue));
        EXPECT_EQ(memcmp(readBuffer.data(), kRawValue, sizeof(kRawValue)), 0);
    }

    // Value should be deleted from the safe provider
    {
        uint8_t readBuf[sizeof(kRawValue)] = {};
        MutableByteSpan readBuffer(readBuf);
        EXPECT_EQ(safeRamProvider.SafeReadValue(path, readBuffer), CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    }
}

// Null migrator: reports failure for the null entry but still migrates the valid entry.
TEST(TestAttributePersistenceMigration, TestMigrationWithNullMigrator)
{
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider ramProvider;
    DefaultSafeAttributePersistenceProvider safeRamProvider;
    ASSERT_EQ(ramProvider.Init(&storageDelegate), CHIP_NO_ERROR);
    ASSERT_EQ(safeRamProvider.Init(&storageDelegate), CHIP_NO_ERROR);

    const ConcreteAttributePath pathA(1, 2, 3);
    const ConcreteAttributePath pathB(1, 2, 4);
    const ConcreteClusterPath cluster(1, 2);
    constexpr uint32_t kValueB = 55;

    // Only attribute B has a value in the safe provider
    ASSERT_EQ(safeRamProvider.WriteScalarValue(pathB, kValueB), CHIP_NO_ERROR);

    // Attribute A has a null migrator, attribute B has a valid one
    const AttrMigrationData attributesToMigrate[] = {
        { 3, nullptr },
        { 4, &DefaultMigrators::ScalarValue<uint32_t> },
    };
    uint8_t buf[128] = {};
    MutableByteSpan buffer(buf);
    // Should report failure due to null migrator
    EXPECT_EQ(
        MigrateFromSafeToAttributePersistenceProvider(safeRamProvider, ramProvider, cluster, Span(attributesToMigrate), buffer),
        CHIP_ERROR_HAD_FAILURES);

    // Attribute A should not exist in the normal provider
    {
        uint8_t readBuf[sizeof(uint32_t)] = {};
        MutableByteSpan readBuffer(readBuf);
        EXPECT_EQ(ramProvider.ReadValue(pathA, readBuffer), CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    }

    // Attribute B should still be migrated successfully
    {
        uint8_t readBuf[sizeof(uint32_t)] = {};
        MutableByteSpan readBuffer(readBuf);
        EXPECT_EQ(ramProvider.ReadValue(pathB, readBuffer), CHIP_NO_ERROR);

        uint32_t readValue = 0;
        ASSERT_EQ(readBuffer.size(), sizeof(uint32_t));
        memcpy(&readValue, readBuffer.data(), sizeof(uint32_t));
        EXPECT_EQ(readValue, kValueB);
    }

    // Attribute B should be deleted from the safe provider
    {
        uint8_t readBuf[sizeof(uint32_t)] = {};
        MutableByteSpan readBuffer(readBuf);
        EXPECT_EQ(safeRamProvider.SafeReadValue(pathB, readBuffer), CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    }
}

} // namespace
