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

#include <lib/core/CHIPError.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <tracing/json/json_tracing.h>

#include <unistd.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>

using namespace chip;
using namespace chip::Tracing::Json;

namespace {

class TestJsonTracing : public ::testing::Test
{
protected:
    void SetUp() override
    {
        std::string pattern = (std::filesystem::temp_directory_path() / "TestJsonTracing-XXXXXX").string();
        ASSERT_NE(mkdtemp(pattern.data()), nullptr);
        mTempDir = pattern;
    }

    void TearDown() override
    {
        std::error_code ec;
        std::filesystem::remove_all(mTempDir, ec);
    }

    // Traces one instant event to `path` and returns the file content.
    static std::string TraceToFile(const std::filesystem::path & path)
    {
        JsonBackend backend;
        EXPECT_SUCCESS(backend.OpenFile(path.c_str()));
        backend.TraceInstant("TestLabel", "TestGroup");
        backend.CloseFile();

        std::ifstream file(path);
        std::stringstream content;
        content << file.rdbuf();
        return content.str();
    }

    std::filesystem::path mTempDir;
};

TEST_F(TestJsonTracing, TestOpenFileInExistingDirectory)
{
    std::filesystem::path path = mTempDir / "trace.json";

    std::string content = TraceToFile(path);

    EXPECT_NE(content.find("TestLabel"), std::string::npos);
}

TEST_F(TestJsonTracing, TestOpenFileCreatesMissingDirectories)
{
    std::filesystem::path path = mTempDir / "new" / "nested" / "trace.json";

    std::string content = TraceToFile(path);

    EXPECT_NE(content.find("TestLabel"), std::string::npos);
}

TEST_F(TestJsonTracing, TestOpenFileTwiceInSameDirectory)
{
    std::filesystem::path directory = mTempDir / "new";

    TraceToFile(directory / "first.json");
    std::string content = TraceToFile(directory / "second.json");

    EXPECT_NE(content.find("TestLabel"), std::string::npos);
}

TEST_F(TestJsonTracing, TestOpenFileWithoutDirectory)
{
    std::filesystem::path previousDirectory = std::filesystem::current_path();
    std::filesystem::current_path(mTempDir);

    std::string content = TraceToFile("trace.json");

    std::filesystem::current_path(previousDirectory);
    EXPECT_NE(content.find("TestLabel"), std::string::npos);
}

TEST_F(TestJsonTracing, TestOpenFileUnderRegularFileFails)
{
    std::filesystem::path regularFile = mTempDir / "file";
    std::ofstream(regularFile) << "not a directory";

    JsonBackend backend;
    EXPECT_NE(backend.OpenFile((regularFile / "trace.json").c_str()), CHIP_NO_ERROR);
}

TEST_F(TestJsonTracing, TestOpenFileRejectsEmptyPathWithoutClosingOpenFile)
{
    std::filesystem::path path = mTempDir / "trace.json";

    JsonBackend backend;
    ASSERT_SUCCESS(backend.OpenFile(path.c_str()));
    EXPECT_EQ(backend.OpenFile(nullptr), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(backend.OpenFile(""), CHIP_ERROR_INVALID_ARGUMENT);
    backend.TraceInstant("TestLabel", "TestGroup");
    backend.CloseFile();

    std::ifstream file(path);
    std::stringstream content;
    content << file.rdbuf();
    EXPECT_NE(content.str().find("TestLabel"), std::string::npos);
}

} // namespace
