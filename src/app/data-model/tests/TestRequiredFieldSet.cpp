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

#include <app/data-model/RequiredFieldSet.h>
#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>

using chip::app::DataModel::RequiredFieldSet;

TEST(TestRequiredFieldSet, NoRequiredFields)
{
    RequiredFieldSet<0> fields;
    EXPECT_EQ(fields.Check(), CHIP_NO_ERROR);
}

TEST(TestRequiredFieldSet, EveryRequiredFieldMustBePresent)
{
    RequiredFieldSet<3> fields;
    EXPECT_EQ(fields.Check(), CHIP_ERROR_MISSING_TLV_ELEMENT);
    fields.MarkPresent(2);
    EXPECT_TRUE(fields.IsPresent(2));
    EXPECT_FALSE(fields.IsPresent(0));
    EXPECT_EQ(fields.Check(), CHIP_ERROR_MISSING_TLV_ELEMENT);
    fields.MarkPresent(0);
    EXPECT_EQ(fields.Check(), CHIP_ERROR_MISSING_TLV_ELEMENT);
    fields.MarkPresent(1);
    EXPECT_EQ(fields.Check(), CHIP_NO_ERROR);
}

TEST(TestRequiredFieldSet, RepeatedFieldCannotReplaceAnotherRequiredField)
{
    RequiredFieldSet<2> fields;
    fields.MarkPresent(0);
    fields.MarkPresent(0);
    EXPECT_EQ(fields.Check(), CHIP_ERROR_MISSING_TLV_ELEMENT);
    fields.MarkPresent(1);
    EXPECT_EQ(fields.Check(), CHIP_NO_ERROR);
}

TEST(TestRequiredFieldSet, MoreThanOneMachineWord)
{
    RequiredFieldSet<65> fields;
    for (size_t index = 0; index < 64; index++)
    {
        fields.MarkPresent(index);
    }
    EXPECT_EQ(fields.Check(), CHIP_ERROR_MISSING_TLV_ELEMENT);
    fields.MarkPresent(64);
    EXPECT_EQ(fields.Check(), CHIP_NO_ERROR);
}
