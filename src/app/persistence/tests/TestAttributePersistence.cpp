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
#include "app/persistence/PascalString.h"
#include <pw_unit_test/framework.h>

#include <app/AttributeValueDecoder.h>
#include <app/ConcreteAttributePath.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/persistence/AttributePersistence.h>
#include <app/persistence/tests/RamAttributePersistenceProvider.h>
#include <lib/core/CHIPError.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/Span.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Testing;
using namespace chip::app::Storage;
using namespace chip::Testing;

TEST(TestAttributePersistence, TestLoadAndStoreNativeEndian)
{
    RamAttributePersistenceProvider ramProvider;
    AttributePersistence persistence(ramProvider);

    ConcreteAttributePath path(1, 2, 3);
    ConcreteAttributePath wrongPath(1, 2, 4);
    constexpr uint32_t kValueToStore = 42;
    constexpr uint32_t kOtherValue   = 99;

    // Store a fake value
    {
        const uint32_t value = kValueToStore;
        EXPECT_EQ(ramProvider.WriteValue(path, { reinterpret_cast<const uint8_t *>(&value), sizeof(value) }), CHIP_NO_ERROR);
    }

    // Test loading a value
    {
        uint32_t valueRead = 0;

        ASSERT_TRUE(persistence.LoadNativeEndianValue(path, valueRead, kOtherValue));
        ASSERT_EQ(valueRead, kValueToStore);
    }

    // Test loading a non-existent value
    {
        uint32_t valueRead = 0;

        ASSERT_FALSE(persistence.LoadNativeEndianValue(wrongPath, valueRead, kOtherValue));
        ASSERT_EQ(valueRead, kOtherValue);
    }

    // Test loading a removed value
    {
        EXPECT_EQ(ramProvider.DeleteValue(path), CHIP_NO_ERROR);

        uint32_t valueRead = 0;
        ASSERT_FALSE(persistence.LoadNativeEndianValue(path, valueRead, kOtherValue));
        ASSERT_EQ(valueRead, kOtherValue);
    }
}

TEST(TestAttributePersistence, TestLoadAndStoreString)
{
    RamAttributePersistenceProvider ramProvider;
    AttributePersistence persistence(ramProvider);
    ConcreteAttributePath path(1, 2, 3);

    // Store a fake value
    {
        const char buffer[] = "hello";
        EXPECT_EQ(ramProvider.WriteValue(path, { reinterpret_cast<const uint8_t *>(&buffer), 6 }), CHIP_NO_ERROR);
    }

    // Test loading a value
    {
        char bufferRead[16];
        ShortPascalString stringRead(bufferRead);

        ASSERT_TRUE(persistence.Load(path, stringRead, std::nullopt));
        ASSERT_TRUE(stringRead.Content().data_equal(CharSpan::fromCharString("hello")));
    }

    // Test loading a non-existent value
    {
        ConcreteAttributePath wrongPath(1, 2, 4);
        char bufferRead[16];
        ShortPascalString stringRead(bufferRead);

        ASSERT_FALSE(persistence.Load(wrongPath, stringRead, CharSpan::fromCharString("default")));
        ASSERT_TRUE(stringRead.Content().data_equal(CharSpan::fromCharString("default")));
    }
}

TEST(TestAttributePersistence, TestNativeRawValueViaDecoder)
{
    RamAttributePersistenceProvider ramProvider;
    AttributePersistence persistence(ramProvider);

    ConcreteAttributePath path(1, 2, 3);
    ConcreteAttributePath wrongPath(1, 2, 4);
    constexpr uint32_t kValueToStore = 0x12345678;
    constexpr uint32_t kOtherValue   = 0x99887766;
    uint32_t valueRead               = 0;

    // Store a value using an encoder (these are a PAIN to create, so use data model provider helpers)
    {
        WriteOperation writeOp(path);
        AttributeValueDecoder decoder = writeOp.DecoderFor(kValueToStore);
        EXPECT_EQ(persistence.StoreNativeEndianValue(path, decoder, valueRead), CHIP_NO_ERROR);
        EXPECT_EQ(valueRead, kValueToStore);
    }

    {
        valueRead = 0;
        ASSERT_TRUE(persistence.LoadNativeEndianValue(path, valueRead, kOtherValue));
        ASSERT_EQ(valueRead, kValueToStore);
    }
}

TEST(TestAttributePersistence, TestStringViaDecoder)
{
    RamAttributePersistenceProvider ramProvider;
    AttributePersistence persistence(ramProvider);

    ConcreteAttributePath path(1, 2, 3);

    // Store a value using an encoder (these are a PAIN to create, so use data model provider helpers)
    {
        WriteOperation writeOp(path);
        AttributeValueDecoder decoder = writeOp.DecoderFor("hello world"_span);

        char buffer[32];
        EXPECT_EQ(persistence.StorePascalString<Storage::ShortPascalString>(path, decoder, buffer), CHIP_NO_ERROR);

        ShortPascalString stringStored(buffer);
        EXPECT_TRUE(stringStored.Content().data_equal("hello world"_span));
    }

    {
        char bufferRead[32];
        ShortPascalString stringRead(bufferRead);

        ASSERT_TRUE(persistence.Load(path, stringRead, std::nullopt));
        ASSERT_TRUE(stringRead.Content().data_equal(CharSpan::fromCharString("hello world")));
    }
}

} // namespace
