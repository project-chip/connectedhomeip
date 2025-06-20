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
#include <pw_unit_test/framework.h>

#include <app/storage/PascalString.h>
#include <app/util/persistence/AttributePersistenceProvider.h>
#include <app/util/persistence/DefaultAttributePersistenceProvider.h>
#include <data-model-providers/codegen/EmberAttributeStorageImpl.h>
#include <lib/core/CHIPError.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/TestPersistentStorageDelegate.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Storage;

TEST(EmberAttributeStorageImpl, TestStorage)
{
    DefaultAttributePersistenceProvider persistenceProvider;
    TestPersistentStorageDelegate testPersistenceDelegate;

    ASSERT_EQ(persistenceProvider.Init(&testPersistenceDelegate), CHIP_NO_ERROR);
    SetAttributePersistenceProvider(&persistenceProvider);

    // persistence is set up globally, ember implementation should now work

    EmberAttributeStorageImpl storage;

    {
        char buff[8];
        ShortPascalString str(buff);
        EXPECT_EQ(storage.Read({ 0, 1, 2 }, str), CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

        EXPECT_TRUE(str.SetValue("foo"_span));
        EXPECT_EQ(storage.Write({ 0, 1, 2 }, str), CHIP_NO_ERROR);
        EXPECT_TRUE(str.SetValue("barbaz"_span));
        EXPECT_TRUE(str.Content().data_equal("barbaz"_span));
        EXPECT_EQ(storage.Read({ 0, 1, 2 }, str), CHIP_NO_ERROR);
        EXPECT_TRUE(str.Content().data_equal("foo"_span));
    }

    {
        uint8_t buff[8];
        constexpr uint8_t kData1[]{ 1, 2, 3, 4 };
        constexpr uint8_t kData2[]{ 1, 2, 7, 8 };

        LongPascalString str(buff);
        EXPECT_EQ(storage.Read({ 0, 2, 2 }, str), CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

        EXPECT_TRUE(str.SetValue(ByteSpan(kData1)));
        EXPECT_EQ(storage.Write({ 0, 2, 2 }, str), CHIP_NO_ERROR);
        EXPECT_TRUE(str.SetValue(ByteSpan(kData2)));
        EXPECT_TRUE(str.Content().data_equal(ByteSpan(kData2)));
        EXPECT_EQ(storage.Read({ 0, 2, 2 }, str), CHIP_NO_ERROR);
        EXPECT_TRUE(str.Content().data_equal(ByteSpan(kData1)));
    }

    SetAttributePersistenceProvider(nullptr);
}

} // namespace
