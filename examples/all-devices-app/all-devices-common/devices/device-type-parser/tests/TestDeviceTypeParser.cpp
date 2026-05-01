/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <devices/device-type-parser/DeviceTypeParser.h>
#include <pw_unit_test/framework.h>

using namespace chip;

class TestDeviceTypeParser : public ::testing::Test
{
protected:
    DeviceTypeParser mParser;
};

TEST_F(TestDeviceTypeParser, Parse_ValidTypeOnly)
{
    EXPECT_EQ(mParser.ParseSingleDeviceString("chime"), CHIP_NO_ERROR);
    const auto & configs = mParser.GetDeviceTypeEntries();
    EXPECT_EQ(configs.size(), 1U);
    EXPECT_EQ(configs[0].type, "chime");
    EXPECT_EQ(configs[0].endpoint, 1);
    EXPECT_EQ(configs[0].parentId, kInvalidEndpointId);
}

TEST_F(TestDeviceTypeParser, Parse_ValidTypeAndEndpoint)
{
    EXPECT_EQ(mParser.ParseSingleDeviceString("speaker:2"), CHIP_NO_ERROR);
    const auto & configs = mParser.GetDeviceTypeEntries();
    EXPECT_EQ(configs.size(), 1U);
    EXPECT_EQ(configs[0].type, "speaker");
    EXPECT_EQ(configs[0].endpoint, 2);
    EXPECT_EQ(configs[0].parentId, kInvalidEndpointId);
}

TEST_F(TestDeviceTypeParser, Parse_ValidTypeEndpointAndParent)
{
    EXPECT_EQ(mParser.ParseSingleDeviceString("light:1,parent=2"), CHIP_NO_ERROR);
    const auto & configs = mParser.GetDeviceTypeEntries();
    EXPECT_EQ(configs.size(), 1U);
    EXPECT_EQ(configs[0].type, "light");
    EXPECT_EQ(configs[0].endpoint, 1);
    EXPECT_EQ(configs[0].parentId, 2);
}

TEST_F(TestDeviceTypeParser, Parse_InvalidEndpoint)
{
    EXPECT_NE(mParser.ParseSingleDeviceString("light:abc"), CHIP_NO_ERROR);
}

TEST_F(TestDeviceTypeParser, Parse_InvalidParent)
{
    EXPECT_NE(mParser.ParseSingleDeviceString("light:1,parent=abc"), CHIP_NO_ERROR);
}

TEST_F(TestDeviceTypeParser, Parse_UnknownOption)
{
    EXPECT_NE(mParser.ParseSingleDeviceString("light:1,vendor=123"), CHIP_NO_ERROR);
}

TEST_F(TestDeviceTypeParser, Parse_MissingParentValue)
{
    EXPECT_NE(mParser.ParseSingleDeviceString("light:1,parent="), CHIP_NO_ERROR);
}

TEST_F(TestDeviceTypeParser, Parse_EmptyString)
{
    EXPECT_NE(mParser.ParseSingleDeviceString(""), CHIP_NO_ERROR);
}

TEST_F(TestDeviceTypeParser, AccumulateDevices)
{
    EXPECT_EQ(mParser.ParseSingleDeviceString("chime:1"), CHIP_NO_ERROR);
    EXPECT_EQ(mParser.ParseSingleDeviceString("speaker:2,parent=1"), CHIP_NO_ERROR);

    const auto & configs = mParser.GetDeviceTypeEntries();
    EXPECT_EQ(configs.size(), 2U);
    EXPECT_EQ(configs[0].type, "chime");
    EXPECT_EQ(configs[0].endpoint, 1);

    EXPECT_EQ(configs[1].type, "speaker");
    EXPECT_EQ(configs[1].endpoint, 2);
    EXPECT_EQ(configs[1].parentId, 1);
}
