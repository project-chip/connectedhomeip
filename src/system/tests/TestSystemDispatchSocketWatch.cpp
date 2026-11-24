/*
 *
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

#include <gtest/gtest.h>
#include <pw_unit_test/framework.h>

#include <lib/support/CodeUtils.h>
#include <system/SystemConfig.h>
#include <system/SystemError.h>
#include <system/SystemLayer.h>
#include <system/SystemLayerImpl.h>

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH && CHIP_SYSTEM_CONFIG_USE_SOCKETS

#include <atomic>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <mach-o/dyld.h>
#include <poll.h>
#include <signal.h>
#include <spawn.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

extern char ** environ;

using namespace chip;
using namespace chip::System;

namespace {

constexpr char kStrictChildEnv[] = "CHIP_TEST_DISPATCH_SOCKET_WATCH_STRICT_CHILD";

enum class Watch
{
    kRead,
    kWrite,
    kReadWrite,
};

void NoopSocketCallback(SocketEvents, intptr_t) {}

void CountingSocketCallback(SocketEvents, intptr_t data)
{
    reinterpret_cast<std::atomic<int> *>(data)->fetch_add(1);
}

int OpenDescriptorCount()
{
    int count = 0;
    for (int fd = 0; fd < getdtablesize(); fd++)
    {
        count += (::fcntl(fd, F_GETFD) != -1) ? 1 : 0;
    }
    return count;
}

template <typename F>
void RunOn(dispatch_queue_t queue, F && f)
{
    dispatch_sync_f(queue, &f, [](void * context) { (*static_cast<F *>(context))(); });
}

CHIP_ERROR StartWatch(LayerImplDispatch & layer, dispatch_queue_t queue, int fd, Watch watch, SocketWatchToken & token,
                      SocketWatchCallback callback = NoopSocketCallback, intptr_t data = 0)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    RunOn(queue, [&] {
        err = layer.StartWatchingSocket(fd, &token);
        VerifyOrReturn(err == CHIP_NO_ERROR);
        err = layer.SetCallback(token, callback, data);
        VerifyOrReturn(err == CHIP_NO_ERROR);
        if (watch != Watch::kWrite)
        {
            err = layer.RequestCallbackOnPendingRead(token);
            VerifyOrReturn(err == CHIP_NO_ERROR);
        }
        if (watch != Watch::kRead)
        {
            err = layer.RequestCallbackOnPendingWrite(token);
        }
    });
    return err;
}

// Endpoint teardown order: StopWatchingSocket, then close() in the same queue block.
// Returns true once the peer observes EOF, i.e. no descriptor for the socket remains open.
bool StopThenCloseReleasesSocket(LayerImplDispatch & layer, dispatch_queue_t queue, Watch watch)
{
    int fds[2];
    VerifyOrReturnValue(::socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0, false);

    SocketWatchToken token;
    bool released = false;
    if (StartWatch(layer, queue, fds[0], watch, token) == CHIP_NO_ERROR)
    {
        RunOn(queue, [&] {
            TEMPORARY_RETURN_IGNORED layer.StopWatchingSocket(&token);
            ::close(fds[0]);
        });
        char buf[8];
        pollfd peer = { fds[1], POLLIN, 0 };
        released    = ::poll(&peer, 1, 1000) == 1 && ::recv(fds[1], buf, sizeof(buf), MSG_DONTWAIT) == 0;
    }
    else
    {
        ::close(fds[0]);
    }
    ::close(fds[1]);
    return released;
}

bool RunStopThenCloseCycles(LayerImplDispatch & layer, dispatch_queue_t queue)
{
    for (int i = 0; i < 60; i++)
    {
        VerifyOrReturnValue(StopThenCloseReleasesSocket(layer, queue, static_cast<Watch>(i % 3)), false);
    }
    return true;
}

// Runs before main() in the child spawned by StopThenCloseDoesNotAbortUnderStrictDispatch, so no other
// test in the binary executes under LIBDISPATCH_STRICT.
[[maybe_unused]] const bool gStrictChildRan = [] {
    if (getenv(kStrictChildEnv) == nullptr)
    {
        return false;
    }
    LayerImplDispatch layer;
    dispatch_queue_t queue = dispatch_queue_create("org.csa-iot.matter.test.socketwatch.strict", DISPATCH_QUEUE_SERIAL);
    bool ok                = layer.Init() == CHIP_NO_ERROR;
    layer.SetDispatchQueue(queue);
    ok = ok && RunStopThenCloseCycles(layer, queue);
    RunOn(queue, [&] { layer.Shutdown(); });
    _exit(ok ? 0 : 1);
}();

class TestSystemDispatchSocketWatch : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_EQ(mLayer.Init(), CHIP_NO_ERROR);
        mQueue = dispatch_queue_create("org.csa-iot.matter.test.socketwatch", DISPATCH_QUEUE_SERIAL);
        ASSERT_NE(mQueue, nullptr);
        mLayer.SetDispatchQueue(mQueue);
    }

    void TearDown() override
    {
        VerifyOrReturn(mQueue != nullptr);
        RunOn(mQueue, [this] { mLayer.Shutdown(); });
    }

    LayerImplDispatch mLayer;
    dispatch_queue_t mQueue = nullptr;
};

TEST_F(TestSystemDispatchSocketWatch, StopThenCloseCyclesReleaseSockets)
{
    EXPECT_TRUE(RunStopThenCloseCycles(mLayer, mQueue));
}

TEST_F(TestSystemDispatchSocketWatch, StopThenCloseDoesNotAbortUnderStrictDispatch)
{
    char path[4096];
    uint32_t pathSize = sizeof(path);
    ASSERT_EQ(_NSGetExecutablePath(path, &pathSize), 0);

    std::vector<char *> env;
    char strict[] = "LIBDISPATCH_STRICT=1";
    char child[]  = "CHIP_TEST_DISPATCH_SOCKET_WATCH_STRICT_CHILD=1";
    for (char ** e = environ; *e != nullptr; e++)
    {
        if (strncmp(*e, "LIBDISPATCH_STRICT=", strlen("LIBDISPATCH_STRICT=")) != 0 &&
            strncmp(*e, kStrictChildEnv, strlen(kStrictChildEnv)) != 0)
        {
            env.push_back(*e);
        }
    }
    env.push_back(strict);
    env.push_back(child);
    env.push_back(nullptr);

    posix_spawn_file_actions_t actions;
    ASSERT_EQ(posix_spawn_file_actions_init(&actions), 0);
    ASSERT_EQ(posix_spawn_file_actions_addopen(&actions, STDOUT_FILENO, "/dev/null", O_WRONLY, 0), 0);
    ASSERT_EQ(posix_spawn_file_actions_addopen(&actions, STDERR_FILENO, "/dev/null", O_WRONLY, 0), 0);

    char * argv[] = { path, nullptr };
    pid_t pid;
    int spawnErr = posix_spawn(&pid, path, &actions, nullptr, argv, env.data());
    posix_spawn_file_actions_destroy(&actions);
    ASSERT_EQ(spawnErr, 0);

    int status   = 0;
    pid_t waited = 0;
    for (int i = 0; i < 3000 && (waited = waitpid(pid, &status, WNOHANG)) == 0; i++)
    {
        usleep(10 * 1000);
    }
    if (waited == 0)
    {
        kill(pid, SIGKILL);
        waitpid(pid, &status, 0);
        FAIL() << "strict child did not exit within 30s";
    }
    ASSERT_EQ(waited, pid);
    EXPECT_FALSE(WIFSIGNALED(status));
    EXPECT_TRUE(WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

TEST_F(TestSystemDispatchSocketWatch, StopClosesOnlyPrivateDescriptors)
{
    int fds[2];
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    int noSigPipe = 1;
    ASSERT_EQ(setsockopt(fds[1], SOL_SOCKET, SO_NOSIGPIPE, &noSigPipe, sizeof(noSigPipe)), 0);
    int baseline = OpenDescriptorCount();

    SocketWatchToken token;
    ASSERT_EQ(StartWatch(mLayer, mQueue, fds[0], Watch::kReadWrite, token), CHIP_NO_ERROR);
    EXPECT_EQ(OpenDescriptorCount(), baseline + 2);

    CHIP_ERROR err = CHIP_NO_ERROR;
    RunOn(mQueue, [&] { err = mLayer.StopWatchingSocket(&token); });
    ASSERT_EQ(err, CHIP_NO_ERROR);
    for (int i = 0; i < 100 && OpenDescriptorCount() != baseline; i++)
    {
        usleep(10 * 1000);
    }
    EXPECT_EQ(OpenDescriptorCount(), baseline);

    char out = 'x';
    char in  = 0;
    EXPECT_EQ(::send(fds[1], &out, 1, 0), static_cast<ssize_t>(1));
    EXPECT_EQ(::recv(fds[0], &in, 1, 0), static_cast<ssize_t>(1));
    EXPECT_EQ(in, out);
    ::close(fds[0]);
    ::close(fds[1]);
}

TEST_F(TestSystemDispatchSocketWatch, DupFailureReturnsErrorAndLaterRequestSucceeds)
{
    int fds[2];
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);

    SocketWatchToken token;
    std::atomic<int> callbacks{ 0 };
    CHIP_ERROR err = CHIP_NO_ERROR;
    RunOn(mQueue, [&] {
        err = mLayer.StartWatchingSocket(fds[0], &token);
        VerifyOrReturn(err == CHIP_NO_ERROR);
        err = mLayer.SetCallback(token, CountingSocketCallback, reinterpret_cast<intptr_t>(&callbacks));
    });
    ASSERT_EQ(err, CHIP_NO_ERROR);

    rlimit saved;
    ASSERT_EQ(getrlimit(RLIMIT_NOFILE, &saved), 0);
    rlimit lowered   = saved;
    lowered.rlim_cur = static_cast<rlim_t>(fds[1] + 1);
    ASSERT_EQ(setrlimit(RLIMIT_NOFILE, &lowered), 0);
    std::vector<int> fillers;
    for (int fd; (fd = ::fcntl(fds[1], F_DUPFD, 0)) >= 0;)
    {
        fillers.push_back(fd);
    }

    CHIP_ERROR failedErr = CHIP_NO_ERROR;
    RunOn(mQueue, [&] { failedErr = mLayer.RequestCallbackOnPendingWrite(token); });

    for (int fd : fillers)
    {
        ::close(fd);
    }
    ASSERT_EQ(setrlimit(RLIMIT_NOFILE, &saved), 0);
    EXPECT_EQ(failedErr, CHIP_ERROR_POSIX(EMFILE));

    usleep(50 * 1000);
    EXPECT_EQ(callbacks.load(), 0);

    RunOn(mQueue, [&] { err = mLayer.RequestCallbackOnPendingWrite(token); });
    EXPECT_EQ(err, CHIP_NO_ERROR);
    for (int i = 0; i < 100 && callbacks.load() == 0; i++)
    {
        usleep(10 * 1000);
    }
    EXPECT_GT(callbacks.load(), 0);

    RunOn(mQueue, [&] {
        err = mLayer.StopWatchingSocket(&token);
        ::close(fds[0]);
    });
    EXPECT_EQ(err, CHIP_NO_ERROR);
    ::close(fds[1]);
}

} // namespace

#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH && CHIP_SYSTEM_CONFIG_USE_SOCKETS
