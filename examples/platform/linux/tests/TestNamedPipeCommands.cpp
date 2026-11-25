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

#include <NamedPipeCommands.h>
#include <lib/support/Defer.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <pw_unit_test/framework.h>

#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <fcntl.h>
#include <mutex>
#include <string>
#include <unistd.h>

namespace {

class TestNamedPipeCommands : public ::testing::Test, public NamedPipeCommandDelegate
{
public:
    void SetUp() override
    {
        ASSERT_NE(mkdtemp(mDirectory), nullptr);
        mPath = std::string(mDirectory) + "/commands";
        ASSERT_SUCCESS(mPipe.Start(mPath, this));
    }

    void TearDown() override
    {
        EXPECT_SUCCESS(mPipe.Stop());
        EXPECT_EQ(rmdir(mDirectory), 0);
    }

    void OnEventCommandReceived(const char * json) override
    {
        std::lock_guard<std::mutex> lock(mMutex);
        ++mCommandCount;
        mLastCommand = json;
        mReceived.notify_one();
    }

protected:
    void SendCommandAndWait(const std::string & command)
    {
        int fd = open(mPath.c_str(), O_RDWR | O_CLOEXEC);
        ASSERT_GE(fd, 0);
        auto closeFd          = MakeDefer([fd] { EXPECT_EQ(close(fd), 0); });
        const ssize_t written = write(fd, command.data(), command.size());
        ASSERT_EQ(written, static_cast<ssize_t>(command.size()));

        std::unique_lock<std::mutex> lock(mMutex);
        ASSERT_TRUE(mReceived.wait_for(lock, std::chrono::seconds(5), [this] { return mCommandCount > 0; }));
    }

    char mDirectory[64] = "/tmp/chip-named-pipe-test-XXXXXX";
    std::string mPath;
    std::mutex mMutex;
    std::condition_variable mReceived;
    size_t mCommandCount = 0;
    std::string mLastCommand;
    NamedPipeCommands mPipe;
};

TEST_F(TestNamedPipeCommands, StopDoesNotDispatchWakeup)
{
    const std::string command = R"({"Name":"Test"})";
    SendCommandAndWait(command);
    ASSERT_FALSE(HasFailure());

    ASSERT_SUCCESS(mPipe.Stop());
    EXPECT_EQ(mCommandCount, 1u);
    EXPECT_EQ(mLastCommand, command);
    EXPECT_EQ(access(mPath.c_str(), F_OK), -1);
}

TEST_F(TestNamedPipeCommands, StopWithoutCommandsDoesNotCallDelegate)
{
    ASSERT_SUCCESS(mPipe.Stop());
    EXPECT_EQ(mCommandCount, 0u);
}

} // namespace
