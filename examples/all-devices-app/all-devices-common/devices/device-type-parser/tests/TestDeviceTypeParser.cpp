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

TEST_F(TestDeviceTypeParser, ExpandWildcards_NoWildcard)
{
    EXPECT_EQ(mParser.ParseSingleDeviceString("chime:1"), CHIP_NO_ERROR);
    EXPECT_EQ(mParser.ParseSingleDeviceString("speaker:2"), CHIP_NO_ERROR);

    std::vector<std::string> deviceTypes = { "chime", "speaker", "light" };
    mParser.ExpandWildcards(deviceTypes);

    const auto & configs = mParser.GetDeviceTypeEntries();
    EXPECT_EQ(configs.size(), 2U);
    EXPECT_EQ(configs[0].type, "chime");
    EXPECT_EQ(configs[0].endpoint, 1);
    EXPECT_EQ(configs[1].type, "speaker");
    EXPECT_EQ(configs[1].endpoint, 2);
}

TEST_F(TestDeviceTypeParser, ExpandWildcards_DefaultEndpoint)
{
    EXPECT_EQ(mParser.ParseSingleDeviceString("*"), CHIP_NO_ERROR);

    std::vector<std::string> deviceTypes = { "chime", "speaker", "light" };
    mParser.ExpandWildcards(deviceTypes);

    const auto & configs = mParser.GetDeviceTypeEntries();
    EXPECT_EQ(configs.size(), 3U);
    EXPECT_EQ(configs[0].type, "chime");
    EXPECT_EQ(configs[0].endpoint, 1);
    EXPECT_EQ(configs[1].type, "speaker");
    EXPECT_EQ(configs[1].endpoint, 2);
    EXPECT_EQ(configs[2].type, "light");
    EXPECT_EQ(configs[2].endpoint, 3);
}

TEST_F(TestDeviceTypeParser, ExpandWildcards_ExplicitEndpoint)
{
    EXPECT_EQ(mParser.ParseSingleDeviceString("*:10"), CHIP_NO_ERROR);

    std::vector<std::string> deviceTypes = { "chime", "speaker" };
    mParser.ExpandWildcards(deviceTypes);

    const auto & configs = mParser.GetDeviceTypeEntries();
    EXPECT_EQ(configs.size(), 2U);
    EXPECT_EQ(configs[0].type, "chime");
    EXPECT_EQ(configs[0].endpoint, 10);
    EXPECT_EQ(configs[1].type, "speaker");
    EXPECT_EQ(configs[1].endpoint, 11);
}

TEST_F(TestDeviceTypeParser, ExpandWildcards_ParentPropagation)
{
    EXPECT_EQ(mParser.ParseSingleDeviceString("*:5,parent=2"), CHIP_NO_ERROR);

    std::vector<std::string> deviceTypes = { "chime", "speaker" };
    mParser.ExpandWildcards(deviceTypes);

    const auto & configs = mParser.GetDeviceTypeEntries();
    EXPECT_EQ(configs.size(), 2U);
    EXPECT_EQ(configs[0].type, "chime");
    EXPECT_EQ(configs[0].endpoint, 5);
    EXPECT_EQ(configs[0].parentId, 2);
    EXPECT_EQ(configs[1].type, "speaker");
    EXPECT_EQ(configs[1].endpoint, 6);
    EXPECT_EQ(configs[1].parentId, 2);
}

TEST_F(TestDeviceTypeParser, ExpandWildcards_MixedOrdering)
{
    EXPECT_EQ(mParser.ParseSingleDeviceString("light:5"), CHIP_NO_ERROR);
    EXPECT_EQ(mParser.ParseSingleDeviceString("*"), CHIP_NO_ERROR);

    std::vector<std::string> deviceTypes = { "chime", "speaker" };
    mParser.ExpandWildcards(deviceTypes);

    const auto & configs = mParser.GetDeviceTypeEntries();
    EXPECT_EQ(configs.size(), 3U);
    EXPECT_EQ(configs[0].type, "light");
    EXPECT_EQ(configs[0].endpoint, 5);
    EXPECT_EQ(configs[1].type, "chime");
    EXPECT_EQ(configs[1].endpoint, 6);
    EXPECT_EQ(configs[2].type, "speaker");
    EXPECT_EQ(configs[2].endpoint, 7);
}

TEST_F(TestDeviceTypeParser, ExpandWildcards_MultipleWildcards)
{
    EXPECT_EQ(mParser.ParseSingleDeviceString("*"), CHIP_NO_ERROR);
    EXPECT_EQ(mParser.ParseSingleDeviceString("*"), CHIP_NO_ERROR);

    std::vector<std::string> deviceTypes = { "chime", "speaker" };
    mParser.ExpandWildcards(deviceTypes);

    const auto & configs = mParser.GetDeviceTypeEntries();
    EXPECT_EQ(configs.size(), 4U);
    // First wildcard
    EXPECT_EQ(configs[0].type, "chime");
    EXPECT_EQ(configs[0].endpoint, 1);
    EXPECT_EQ(configs[1].type, "speaker");
    EXPECT_EQ(configs[1].endpoint, 2);
    // Second wildcard
    EXPECT_EQ(configs[2].type, "chime");
    EXPECT_EQ(configs[2].endpoint, 3);
    EXPECT_EQ(configs[3].type, "speaker");
    EXPECT_EQ(configs[3].endpoint, 4);
}

TEST_F(TestDeviceTypeParser, ExpandWildcards_MixedOrderingAfter)
{
    EXPECT_EQ(mParser.ParseSingleDeviceString("*"), CHIP_NO_ERROR);
    EXPECT_EQ(mParser.ParseSingleDeviceString("light:10"), CHIP_NO_ERROR);

    std::vector<std::string> deviceTypes = { "chime", "speaker" };
    mParser.ExpandWildcards(deviceTypes);

    const auto & configs = mParser.GetDeviceTypeEntries();
    EXPECT_EQ(configs.size(), 3U);
    // Wildcard expanded starting at maxEp + 1 = 11
    EXPECT_EQ(configs[0].type, "chime");
    EXPECT_EQ(configs[0].endpoint, 11);
    EXPECT_EQ(configs[1].type, "speaker");
    EXPECT_EQ(configs[1].endpoint, 12);
    // light explicit endpoint at 10
    EXPECT_EQ(configs[2].type, "light");
    EXPECT_EQ(configs[2].endpoint, 10);
}

TEST_F(TestDeviceTypeParser, ExpandWildcards_Subtrees)
{
    EXPECT_EQ(mParser.ParseSingleDeviceString("aggregator:1"), CHIP_NO_ERROR);
    EXPECT_EQ(mParser.ParseSingleDeviceString("*,parent=1"), CHIP_NO_ERROR);
    EXPECT_EQ(mParser.ParseSingleDeviceString("aggregator:20"), CHIP_NO_ERROR);
    EXPECT_EQ(mParser.ParseSingleDeviceString("*,parent=20"), CHIP_NO_ERROR);

    std::vector<std::string> deviceTypes = { "chime", "speaker" };
    mParser.ExpandWildcards(deviceTypes);

    const auto & configs = mParser.GetDeviceTypeEntries();
    EXPECT_EQ(configs.size(), 6U);

    // aggregator:1
    EXPECT_EQ(configs[0].type, "aggregator");
    EXPECT_EQ(configs[0].endpoint, 1);

    // first wildcard block starts at maxEp + 1 = 21
    EXPECT_EQ(configs[1].type, "chime");
    EXPECT_EQ(configs[1].endpoint, 21);
    EXPECT_EQ(configs[1].parentId, 1);
    EXPECT_EQ(configs[2].type, "speaker");
    EXPECT_EQ(configs[2].endpoint, 22);
    EXPECT_EQ(configs[2].parentId, 1);

    // aggregator:20
    EXPECT_EQ(configs[3].type, "aggregator");
    EXPECT_EQ(configs[3].endpoint, 20);

    // second wildcard block starts at nextAvailableEp = 23
    EXPECT_EQ(configs[4].type, "chime");
    EXPECT_EQ(configs[4].endpoint, 23);
    EXPECT_EQ(configs[4].parentId, 20);
    EXPECT_EQ(configs[5].type, "speaker");
    EXPECT_EQ(configs[5].endpoint, 24);
    EXPECT_EQ(configs[5].parentId, 20);
}

TEST_F(TestDeviceTypeParser, ExpandWildcards_WildcardExplicitMaxEp)
{
    EXPECT_EQ(mParser.ParseSingleDeviceString("*,parent=1"), CHIP_NO_ERROR);
    EXPECT_EQ(mParser.ParseSingleDeviceString("*:10,parent=20"), CHIP_NO_ERROR);

    std::vector<std::string> deviceTypes = { "chime", "speaker" };
    mParser.ExpandWildcards(deviceTypes);

    const auto & configs = mParser.GetDeviceTypeEntries();
    EXPECT_EQ(configs.size(), 4U);

    // Because *:10 is parsed, maxEp is 10.
    // Therefore, the first wildcard starts at maxEp + 1 = 11.
    EXPECT_EQ(configs[0].type, "chime");
    EXPECT_EQ(configs[0].endpoint, 11);
    EXPECT_EQ(configs[0].parentId, 1);
    EXPECT_EQ(configs[1].type, "speaker");
    EXPECT_EQ(configs[1].endpoint, 12);
    EXPECT_EQ(configs[1].parentId, 1);

    // The second wildcard starts at its explicit endpoint 10.
    EXPECT_EQ(configs[2].type, "chime");
    EXPECT_EQ(configs[2].endpoint, 10);
    EXPECT_EQ(configs[2].parentId, 20);
    EXPECT_EQ(configs[3].type, "speaker");
    EXPECT_EQ(configs[3].endpoint, 11);
    EXPECT_EQ(configs[3].parentId, 20);
}
