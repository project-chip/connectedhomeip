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
#include <pw_unit_test/framework.h>

#include <app/ConcreteAttributePath.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>
#include <app/AttributeValueDecoder.h>
#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/persistence/AttributePersistence.h>
#include <app/persistence/tests/RamAttributePersistenceProvider.h>
#include <lib/core/StringBuilderAdapters.h>

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
    constexpr uint32_t kValueToStore = 42;
    constexpr uint32_t kOtherValue = 42;
    uint32_t valueRead;

    // Store a fake value
    {
        const uint32_t value = kValueToStore;
        EXPECT_EQ(ramProvider.WriteValue(path, {reinterpret_cast<const uint8_t*>(&value), sizeof(value)}), CHIP_NO_ERROR);
    }

    // Test loading a value
    {
        valueRead = 0;
        ASSERT_TRUE(persistence.LoadNativeEndianValue(path, valueRead, kOtherValue));
        ASSERT_EQ(valueRead, kValueToStore);
    }

    // Test loading a non-existent value
    {
        ConcreteAttributePath wrongPath(1, 2, 4);
        valueRead = 0;
        ASSERT_FALSE(persistence.LoadNativeEndianValue(wrongPath, valueRead, kOtherValue));
        ASSERT_EQ(valueRead, kOtherValue);
    }

    // Test loading a removed value
    {
        EXPECT_EQ(ramProvider.DeleteValue(path), CHIP_NO_ERROR);
        valueRead = 0;
        ASSERT_FALSE(persistence.LoadNativeEndianValue(path, valueRead, kOtherValue));
        ASSERT_EQ(valueRead, kOtherValue);
    }
}

TEST(TestAttributePersistence, TestLoadAndStoreString)
{
    RamAttributePersistenceProvider ramProvider;
    AttributePersistence persistence(ramProvider);
    ConcreteAttributePath path(1, 2, 3);
    char bufferToStore[16];
    char bufferRead[16];
    ShortPascalString stringToStore(bufferToStore);
    ShortPascalString stringRead(bufferRead);

    // Store a fake value
    {
        const char buffer[] = "\x05hello";
        EXPECT_EQ(ramProvider.WriteValue(path, {reinterpret_cast<const uint8_t*>(&buffer), 6}), CHIP_NO_ERROR);
    }

    // Test loading a value
    {
        ASSERT_TRUE(persistence.Load(path, stringRead, std::nullopt));
        ASSERT_TRUE(stringRead.Content().data_equal(CharSpan::fromCharString("hello")));
    }

    // Test loading a non-existent value
    {
        ConcreteAttributePath wrongPath(1, 2, 4);
        ASSERT_FALSE(persistence.Load(wrongPath, stringRead, CharSpan::fromCharString("default")));
        ASSERT_TRUE(stringRead.Content().data_equal(CharSpan::fromCharString("default")));
    }
}

} // namespace
