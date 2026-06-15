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

TEST_F(TestDeviceTypeParser, Parse_BridgedOption)
{
    EXPECT_EQ(mParser.ParseSingleDeviceString("chime:2,bridged"), CHIP_NO_ERROR);
    const auto & configs = mParser.GetDeviceTypeEntries();
    EXPECT_EQ(configs.size(), 1U);
    EXPECT_EQ(configs[0].type, "chime");
    EXPECT_EQ(configs[0].endpoint, 2);
    EXPECT_EQ(configs[0].parentId, kInvalidEndpointId);
    EXPECT_TRUE(configs[0].bridged);
}

TEST_F(TestDeviceTypeParser, Parse_MultipleOptions)
{
    EXPECT_EQ(mParser.ParseSingleDeviceString("chime:2,parent=1,bridged"), CHIP_NO_ERROR);
    const auto & configs = mParser.GetDeviceTypeEntries();
    EXPECT_EQ(configs.size(), 1U);
    EXPECT_EQ(configs[0].type, "chime");
    EXPECT_EQ(configs[0].endpoint, 2);
    EXPECT_EQ(configs[0].parentId, 1);
    EXPECT_TRUE(configs[0].bridged);
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

TEST_F(TestDeviceTypeParser, ExpandWildcards_ExplicitBridged)
{
    EXPECT_EQ(mParser.ParseSingleDeviceString("chime:2,parent=1,bridged"), CHIP_NO_ERROR);

    std::vector<std::string> deviceTypes = { "chime" };
    mParser.ExpandWildcards(deviceTypes);

    const auto & configs = mParser.GetDeviceTypeEntries();
    EXPECT_EQ(configs.size(), 2U);

    // Explicit chime:2,bridged is expanded into a parent bridged-node and the chime child.
    EXPECT_EQ(configs[0].type, "bridged-node");
    EXPECT_EQ(configs[0].endpoint, 2);
    EXPECT_EQ(configs[0].parentId, 1);
    EXPECT_TRUE(configs[0].bridged);

    EXPECT_EQ(configs[1].type, "chime");
    EXPECT_EQ(configs[1].endpoint, 3);
    EXPECT_EQ(configs[1].parentId, 2);
    EXPECT_TRUE(configs[1].bridged);
}

TEST_F(TestDeviceTypeParser, ExpandWildcards_WildcardBridged)
{
    EXPECT_EQ(mParser.ParseSingleDeviceString("aggregator:1"), CHIP_NO_ERROR);
    EXPECT_EQ(mParser.ParseSingleDeviceString("*,parent=1,bridged"), CHIP_NO_ERROR);

    std::vector<std::string> deviceTypes = { "chime", "speaker" };
    mParser.ExpandWildcards(deviceTypes);

    const auto & configs = mParser.GetDeviceTypeEntries();
    EXPECT_EQ(configs.size(), 5U);

    EXPECT_EQ(configs[0].type, "aggregator");
    EXPECT_EQ(configs[0].endpoint, 1);
    EXPECT_FALSE(configs[0].bridged);

    // Wildcard block starts at maxEp + 1 = 2.
    // For each device in wildcard, we get [bridged-node, device]
    EXPECT_EQ(configs[1].type, "bridged-node");
    EXPECT_EQ(configs[1].endpoint, 2);
    EXPECT_EQ(configs[1].parentId, 1);
    EXPECT_TRUE(configs[1].bridged);

    EXPECT_EQ(configs[2].type, "chime");
    EXPECT_EQ(configs[2].endpoint, 3);
    EXPECT_EQ(configs[2].parentId, 2);
    EXPECT_TRUE(configs[2].bridged);

    EXPECT_EQ(configs[3].type, "bridged-node");
    EXPECT_EQ(configs[3].endpoint, 4);
    EXPECT_EQ(configs[3].parentId, 1);
    EXPECT_TRUE(configs[3].bridged);

    EXPECT_EQ(configs[4].type, "speaker");
    EXPECT_EQ(configs[4].endpoint, 5);
    EXPECT_EQ(configs[4].parentId, 4);
    EXPECT_TRUE(configs[4].bridged);
}

TEST_F(TestDeviceTypeParser, ValidateConfig_Valid)
{
    std::vector<DeviceTypeParser::Entry> entries = { { .type = "aggregator", .endpoint = 1, .parentId = kInvalidEndpointId },
                                                     { .type = "bridged-node", .endpoint = 2, .parentId = 1 },
                                                     { .type = "chime", .endpoint = 3, .parentId = 2 } };
    EXPECT_EQ(DeviceTypeParser::ValidateConfig(entries), CHIP_NO_ERROR);
}

TEST_F(TestDeviceTypeParser, ValidateConfig_DuplicateEndpoints)
{
    std::vector<DeviceTypeParser::Entry> entries = { { .type = "chime", .endpoint = 1, .parentId = kInvalidEndpointId },
                                                     { .type = "speaker", .endpoint = 1, .parentId = kInvalidEndpointId } };
    EXPECT_NE(DeviceTypeParser::ValidateConfig(entries), CHIP_NO_ERROR);
}

TEST_F(TestDeviceTypeParser, ValidateConfig_BridgedNodeNoParent)
{
    std::vector<DeviceTypeParser::Entry> entries = { { .type = "bridged-node", .endpoint = 2, .parentId = kInvalidEndpointId } };
    EXPECT_NE(DeviceTypeParser::ValidateConfig(entries), CHIP_NO_ERROR);
}

TEST_F(TestDeviceTypeParser, ValidateConfig_ParentDoesNotExist)
{
    std::vector<DeviceTypeParser::Entry> entries = { { .type = "chime", .endpoint = 2, .parentId = 1 } };
    EXPECT_NE(DeviceTypeParser::ValidateConfig(entries), CHIP_NO_ERROR);
}

TEST_F(TestDeviceTypeParser, ValidateConfig_BridgedNodeParentNotAggregator)
{
    std::vector<DeviceTypeParser::Entry> entries = { { .type = "chime", .endpoint = 1, .parentId = kInvalidEndpointId },
                                                     { .type = "bridged-node", .endpoint = 2, .parentId = 1 } };
    EXPECT_NE(DeviceTypeParser::ValidateConfig(entries), CHIP_NO_ERROR);
}

TEST_F(TestDeviceTypeParser, Parse_MultipleCommas)
{
    // Verifies that empty option segments (e.g., from multiple commas) are gracefully ignored.
    EXPECT_EQ(mParser.ParseSingleDeviceString("chime:1,,bridged"), CHIP_NO_ERROR);
    const auto & configs = mParser.GetDeviceTypeEntries();
    EXPECT_EQ(configs.size(), 1U);
    EXPECT_TRUE(configs[0].bridged);
}

TEST_F(TestDeviceTypeParser, ExpandWildcards_WildcardBridgedSequential)
{
    // Verifies that bridged wildcards correctly increment endpoints in pairs (node + device).
    EXPECT_EQ(mParser.ParseSingleDeviceString("aggregator:1"), CHIP_NO_ERROR);
    EXPECT_EQ(mParser.ParseSingleDeviceString("*:10,parent=1,bridged"), CHIP_NO_ERROR);

    std::vector<std::string> deviceTypes = { "chime", "speaker" };
    mParser.ExpandWildcards(deviceTypes);

    const auto & configs = mParser.GetDeviceTypeEntries();
    EXPECT_EQ(configs.size(), 5U); // 1 (aggregator) + 2 * (bridged-node + device)

    // First pair starting at 10
    EXPECT_EQ(configs[1].type, "bridged-node");
    EXPECT_EQ(configs[1].endpoint, 10);
    EXPECT_EQ(configs[2].type, "chime");
    EXPECT_EQ(configs[2].endpoint, 11);

    // Second pair starting at 12
    EXPECT_EQ(configs[3].type, "bridged-node");
    EXPECT_EQ(configs[3].endpoint, 12);
    EXPECT_EQ(configs[4].type, "speaker");
    EXPECT_EQ(configs[4].endpoint, 13);
}

TEST_F(TestDeviceTypeParser, ValidateConfig_CycleDetected)
{
    // Verifies that cyclic parent-child dependencies are detected and rejected.
    std::vector<DeviceTypeParser::Entry> entries = { { .type = "chime", .endpoint = 2, .parentId = 3 },
                                                     { .type = "speaker", .endpoint = 3, .parentId = 2 } };
    EXPECT_NE(DeviceTypeParser::ValidateConfig(entries), CHIP_NO_ERROR);
}
