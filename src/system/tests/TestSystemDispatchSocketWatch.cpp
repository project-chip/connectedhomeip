/*
 *
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

/**
 *    @file
 *      Unit tests for the libdispatch-backed socket watch in
 *      chip::System::LayerImplDispatch.
 *
 *      A libdispatch READ/WRITE source requires the descriptor it monitors to
 *      remain valid until the source's asynchronous cancellation has completed.
 *      If the layer monitors the caller's fd directly, a caller that closes
 *      that fd during socket / endpoint teardown -- before the async source
 *      cancellation completes -- makes the descriptor vanish underneath
 *      libdispatch, which can abort the whole process.
 *
 *      The layer must therefore monitor a private duplicate of the caller's fd
 *      that stays open until the source is torn down. These tests assert that
 *      invariant behaviorally: once a watch is installed, closing the caller's
 *      fd must not tear down the underlying socket description (the peer end
 *      stays connected). That is observable and deterministic, and does not
 *      depend on reproducing the platform abort.
 */

#include <gtest/gtest.h>
#include <pw_unit_test/framework.h>

#include <lib/support/CodeUtils.h>
#include <system/SystemConfig.h>
#include <system/SystemError.h>
#include <system/SystemLayer.h>
#include <system/SystemLayerImpl.h>

// The dispatch-source fd-vanish hazard only exists in the libdispatch-backed
// system layer (Darwin). On select-based builds LayerImplDispatch is not the
// active implementation, so the whole suite is compiled out there.
#if CHIP_SYSTEM_CONFIG_USE_DISPATCH && CHIP_SYSTEM_CONFIG_USE_SOCKETS

#include <cerrno>
#include <sys/socket.h>
#include <unistd.h>

using namespace chip;
using namespace chip::System;

namespace {

void NoopSocketCallback(SocketEvents, intptr_t) {}

class TestSystemDispatchSocketWatch : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_EQ(mLayer.Init(), CHIP_NO_ERROR);

        // A live (non-suspended) serial queue so RequestCallback installs and
        // activates a real dispatch source for the watched fd.
        mQueue = dispatch_queue_create("org.csa-iot.matter.test.socketwatch", DISPATCH_QUEUE_SERIAL);
        ASSERT_NE(mQueue, nullptr);
        mLayer.SetDispatchQueue(mQueue);
    }

    void TearDown() override { mLayer.Shutdown(); }

    // Installs a watch on one end of a socketpair, requests the given event
    // callback (which installs the dispatch source), then closes the caller's
    // fd while the watch is still active. Asserts that the underlying socket
    // description is still open afterwards -- i.e. the peer end is still
    // connected rather than at EOF. The dispatch source must monitor a private
    // duplicate that outlives the caller's close(); if it monitors the caller's
    // fd directly, close() tears down the only reference and the peer sees EOF.
    void ExpectWatchedDescriptorOutlivesCallerClose(bool wantRead)
    {
        int fds[2];
        ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);

        SocketWatchToken token;
        ASSERT_EQ(mLayer.StartWatchingSocket(fds[0], &token), CHIP_NO_ERROR);
        ASSERT_EQ(mLayer.SetCallback(token, NoopSocketCallback, 0), CHIP_NO_ERROR);
        if (wantRead)
        {
            ASSERT_EQ(mLayer.RequestCallbackOnPendingRead(token), CHIP_NO_ERROR);
        }
        else
        {
            ASSERT_EQ(mLayer.RequestCallbackOnPendingWrite(token), CHIP_NO_ERROR);
        }

        // Caller closes its fd during teardown, before the watch is stopped.
        ASSERT_EQ(::close(fds[0]), 0);

        // The socket description the dispatch source monitors must still be
        // open, so the peer end is still connected: a non-blocking recv blocks
        // (EAGAIN) rather than returning 0 (EOF). Pre-fix the source monitors
        // the caller's fd directly, so this close() dropped the only reference
        // and the peer sees EOF (recv == 0) -> this assertion fails (RED).
        char buf[8];
        ssize_t received = ::recv(fds[1], buf, sizeof(buf), MSG_DONTWAIT);
        EXPECT_EQ(received, static_cast<ssize_t>(-1))
            << "peer saw EOF after caller close(); watched descriptor did not outlive the caller's fd";
        EXPECT_TRUE(errno == EAGAIN || errno == EWOULDBLOCK) << "unexpected errno " << errno << " from peer recv";

        ASSERT_EQ(mLayer.StopWatchingSocket(&token), CHIP_NO_ERROR);
        ::close(fds[1]);
    }

    LayerImplDispatch mLayer;
    dispatch_queue_t mQueue = nullptr;
};

// A read watch must monitor a descriptor that outlives the caller's close().
TEST_F(TestSystemDispatchSocketWatch, ReadWatchedDescriptorOutlivesCallerClose)
{
    ExpectWatchedDescriptorOutlivesCallerClose(/* wantRead = */ true);
}

// Same invariant for the write source path (DISPATCH_SOURCE_TYPE_WRITE).
TEST_F(TestSystemDispatchSocketWatch, WriteWatchedDescriptorOutlivesCallerClose)
{
    ExpectWatchedDescriptorOutlivesCallerClose(/* wantRead = */ false);
}

} // namespace

#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH && CHIP_SYSTEM_CONFIG_USE_SOCKETS
