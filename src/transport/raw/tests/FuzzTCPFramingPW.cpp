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

/**
 *    @file
 *      FuzzTest harness for the TCP transport's message framing
 *      (TCPBase::ProcessReceivedBuffer / ProcessSingleMessage).
 *
 *      The transport reassembles length-prefixed Matter messages from a byte stream that
 *      the socket layer may deliver in any segmentation. Each input is fed through the same
 *      function the socket receive callback calls, with the stream split into packet
 *      buffers at fuzzer-chosen points, so that:
 *        - a length prefix can span two buffers,
 *        - a buffer can end before its message does,
 *        - one buffer can hold several messages.
 *
 *      Real TCP transport, ActiveTCPConnectionState, PacketBuffer chains and Inet endpoint
 *      objects; only the socket itself is absent (the endpoint is never bound).
 */

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <utility>
#include <vector>

#include <fuzztest/fuzztest_core.h>
#include <gtest/gtest.h>

#include <inet/IPAddress.h>
#include <inet/TCPEndPoint.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/support/CodeUtils.h>
#include <system/SystemPacketBuffer.h>
#include <transport/raw/PeerAddress.h>
#include <transport/raw/TCP.h>
#include <transport/raw/tests/NetworkTestHelpers.h>
#include <transport/raw/tests/TCPBaseTestAccess.h>

namespace {

using namespace chip;
using namespace chip::Transport;
using namespace fuzztest;

constexpr size_t kActiveConnections = 2;
constexpr size_t kPendingPackets    = 2;
using TCPImpl                       = TCP<kActiveConnections, kPendingPackets>;
using TestAccess                    = TCPBaseTestAccess<kActiveConnections, kPendingPackets>;

// Mirrors the file-local constants in TCP.cpp.
constexpr size_t kPrefixSize          = 4;
constexpr uint32_t kMaxTCPMessageSize = static_cast<uint32_t>(System::PacketBuffer::kLargeBufMaxSizeWithoutReserve - kPrefixSize);
constexpr size_t kMaxChunkSize        = System::PacketBuffer::kMaxSizeWithoutReserve;
// Maximum number of packet buffers whose size the fuzzer chooses per input. Bytes after
// them go in buffers of kMaxChunkSize. Appending a buffer traverses the whole receive
// queue, so cost is quadratic in this count.
constexpr size_t kMaxChunks = 64;
// libFuzzer-compatible mode caps the serialized input at -max_len (4096 by default);
// a larger domain bound is never reached. Sizes near the limit are fuzzed as integers.
constexpr size_t kMaxStreamSize = 4000;
// Only a static pool can run out of packet buffers on a valid stream.
constexpr bool kStaticPool = CHIP_SYSTEM_PACKETBUFFER_FROM_CHIP_POOL;

// Records every message the transport hands up so a property can compare it with the
// frames it encoded.
class RecordingDelegate : public RawTransportDelegate
{
public:
    void HandleMessageReceived(const PeerAddress &, System::PacketBufferHandle && msg, MessageTransportContext *) override
    {
        std::vector<uint8_t> bytes(msg->TotalLength());
        if (!bytes.empty())
        {
            VerifyOrDie(msg->Read(bytes.data(), bytes.size()) == CHIP_NO_ERROR);
        }
        mMessages.push_back(std::move(bytes));
    }

    void HandleConnectionClosed(ActiveTCPConnectionState &, CHIP_ERROR) override { mClosedCount++; }

    void Reset()
    {
        mMessages.clear();
        mClosedCount = 0;
    }

    std::vector<std::vector<uint8_t>> mMessages;
    size_t mClosedCount = 0;
};

struct Fixture
{
    Testing::IOContext io;
    TCPImpl tcp;
    RecordingDelegate delegate;
    PeerAddress peer;
};

Fixture * gFixture = nullptr;

Fixture & GetFixture()
{
    static std::once_flag once;
    std::call_once(once, [] {
        auto * fx = new Fixture();
        VerifyOrDie(fx->io.Init() == CHIP_NO_ERROR);
        fx->tcp.SetDelegate(&fx->delegate);

        Inet::IPAddress addr;
        VerifyOrDie(Inet::IPAddress::FromString("::1", addr));
        fx->peer = PeerAddress::TCP(addr, CHIP_PORT);

        gFixture = fx;

        // The fixture is leaked, but the file-static Inet EndPointManagers VerifyOrDie in
        // their destructors unless shut down first, which reads as a crash.
        std::atexit([] {
            if (gFixture != nullptr)
            {
                gFixture->tcp.CloseActiveConnections();
                gFixture->io.Shutdown();
            }
        });
    });
    return *gFixture;
}

// A connection slot bound to a fresh, never-connected endpoint, released at scope exit so
// each input starts with an empty receive queue.
class ScopedConnection
{
public:
    explicit ScopedConnection(Fixture & fx) : mFx(fx)
    {
        VerifyOrDie(fx.io.GetTCPEndPointManager()->NewEndPoint(mEndPoint) == CHIP_NO_ERROR);
        // Allocated as connecting: the connected-state lookup inside AllocateConnection
        // asks the endpoint for peer info, which an endpoint without a socket cannot answer.
        mConnection = TestAccess::AllocateConnection(fx.tcp, mEndPoint, fx.peer, TCPState::kConnecting);
        VerifyOrDie(mConnection);
        mConnection.Handle()->mConnectionState = TCPState::kConnected;
    }

    ~ScopedConnection() { mFx.tcp.CloseActiveConnections(); }

    CHIP_ERROR Feed(System::PacketBufferHandle && buffer)
    {
        return TestAccess::ProcessReceivedBuffer(mFx.tcp, mEndPoint, mFx.peer, std::move(buffer));
    }

    bool IsOpen() { return !TestAccess::GetEndpoint(mConnection).IsNull(); }

private:
    Fixture & mFx;
    Inet::TCPEndPointHandle mEndPoint;
    TestAccess::Connection mConnection;
};

// Set when the harness itself could not get a packet buffer (static-pool builds), so a
// property can stop without reading that as a transport outcome.
bool gHarnessAllocFailed = false;

System::PacketBufferHandle MakeBuffer(const uint8_t * data, size_t size)
{
    System::PacketBufferHandle buf = System::PacketBufferHandle::New(size, 0);
    if (buf.IsNull())
    {
        gHarnessAllocFailed = true;
        return buf;
    }
    if (size > 0)
    {
        memcpy(buf->Start(), data, size);
    }
    buf->SetDataLength(size);
    return buf;
}

// Splits `stream` into packet buffers of the given chunk sizes, then bytes after them into
// buffers of at most kMaxChunkSize, and feeds them in order. Stops at the first error.
CHIP_ERROR FeedSegmented(ScopedConnection & conn, const std::vector<uint8_t> & stream, const std::vector<uint16_t> & chunkSizes)
{
    size_t offset = 0;
    for (uint16_t chunk : chunkSizes)
    {
        if (offset >= stream.size())
        {
            break;
        }
        size_t size                    = std::min<size_t>(chunk, stream.size() - offset);
        System::PacketBufferHandle buf = MakeBuffer(stream.data() + offset, size);
        VerifyOrReturnError(!buf.IsNull(), CHIP_ERROR_NO_MEMORY);
        ReturnErrorOnFailure(conn.Feed(std::move(buf)));
        offset += size;
    }
    while (offset < stream.size())
    {
        size_t size                    = std::min(kMaxChunkSize, stream.size() - offset);
        System::PacketBufferHandle buf = MakeBuffer(stream.data() + offset, size);
        VerifyOrReturnError(!buf.IsNull(), CHIP_ERROR_NO_MEMORY);
        ReturnErrorOnFailure(conn.Feed(std::move(buf)));
        offset += size;
    }
    return CHIP_NO_ERROR;
}

void AppendPrefix(std::vector<uint8_t> & stream, uint32_t length)
{
    uint8_t prefix[kPrefixSize];
    Encoding::LittleEndian::Put32(prefix, length);
    stream.insert(stream.end(), prefix, prefix + kPrefixSize);
}

// Raw bytes, segmented arbitrarily: the framing code sees whatever a peer could send.
void ArbitraryStreamDoesNotCrash(const std::vector<uint8_t> & stream, const std::vector<uint16_t> & chunkSizes)
{
    Fixture & fx = GetFixture();
    fx.delegate.Reset();
    gHarnessAllocFailed = false;
    ScopedConnection conn(fx);

    CHIP_ERROR err = FeedSegmented(conn, stream, chunkSizes);
    if (gHarnessAllocFailed)
    {
        return;
    }

    // Every delivered message was announced by a non-zero prefix within the size limit.
    for (const auto & msg : fx.delegate.mMessages)
    {
        ASSERT_GT(msg.size(), 0u);
        ASSERT_LE(msg.size(), kMaxTCPMessageSize);
    }
    if (err == CHIP_ERROR_MESSAGE_TOO_LONG)
    {
        ASSERT_FALSE(conn.IsOpen());
        ASSERT_EQ(fx.delegate.mClosedCount, 1u);
        return;
    }
    ASSERT_TRUE(err == CHIP_NO_ERROR || err == CHIP_ERROR_INVALID_MESSAGE_LENGTH || (kStaticPool && err == CHIP_ERROR_NO_MEMORY));
    ASSERT_EQ(fx.delegate.mClosedCount, 0u);
    if (err == CHIP_NO_ERROR)
    {
        ASSERT_TRUE(conn.IsOpen());
    }
}

FUZZ_TEST(FuzzTCPFramingPW, ArbitraryStreamDoesNotCrash)
    .WithDomains(VectorOf(Arbitrary<uint8_t>()).WithMaxSize(kMaxStreamSize),
                 VectorOf(InRange<uint16_t>(1, static_cast<uint16_t>(kMaxChunkSize))).WithMaxSize(kMaxChunks));

// A message may span more than one packet buffer, within the transport limit. The count is
// how many one-buffer messages fit in kMaxStreamSize.
constexpr size_t kMaxFramedMessageSize = std::min<size_t>(2 * kMaxChunkSize, kMaxTCPMessageSize);
constexpr size_t kMaxFramedMessages    = kMaxStreamSize / kMaxChunkSize + 1;

// Well-formed frames, segmented arbitrarily: the delegate must get exactly the encoded
// messages, in order, byte for byte, regardless of where the buffers were cut.
void FramedMessagesSurviveSegmentation(const std::vector<std::vector<uint8_t>> & messages, const std::vector<uint16_t> & chunkSizes)
{
    Fixture & fx = GetFixture();
    fx.delegate.Reset();
    gHarnessAllocFailed = false;
    ScopedConnection conn(fx);

    std::vector<uint8_t> stream;
    for (const auto & msg : messages)
    {
        AppendPrefix(stream, static_cast<uint32_t>(msg.size()));
        stream.insert(stream.end(), msg.begin(), msg.end());
    }

    CHIP_ERROR err = FeedSegmented(conn, stream, chunkSizes);
    if (gHarnessAllocFailed)
    {
        return;
    }
    if (kStaticPool && err == CHIP_ERROR_NO_MEMORY)
    {
        // Messages delivered before the pool ran out must be an exact prefix of the input.
        ASSERT_LE(fx.delegate.mMessages.size(), messages.size());
        ASSERT_TRUE(std::equal(fx.delegate.mMessages.begin(), fx.delegate.mMessages.end(), messages.begin()));
        return;
    }
    ASSERT_EQ(err, CHIP_NO_ERROR);
    ASSERT_EQ(fx.delegate.mMessages, messages);
    ASSERT_EQ(fx.delegate.mClosedCount, 0u);
    ASSERT_TRUE(conn.IsOpen());
}

FUZZ_TEST(FuzzTCPFramingPW, FramedMessagesSurviveSegmentation)
    .WithDomains(
        VectorOf(VectorOf(Arbitrary<uint8_t>()).WithMinSize(1).WithMaxSize(kMaxFramedMessageSize)).WithMaxSize(kMaxFramedMessages),
        VectorOf(InRange<uint16_t>(1, static_cast<uint16_t>(kMaxChunkSize))).WithMaxSize(kMaxChunks));

// One frame whose declared length and delivered byte count are fuzzed as integers, so the
// size-limit boundary and the partial-message arm are reached without building a 64 KB
// seed. The expected outcome is a function of the two numbers.
void LengthPrefixBoundary(uint32_t declaredLength, uint32_t presentBytes, uint16_t chunkSize)
{
    Fixture & fx = GetFixture();
    fx.delegate.Reset();
    gHarnessAllocFailed = false;
    ScopedConnection conn(fx);

    // Filler past the declared length would be parsed as the next frame's prefix, so only an
    // over-limit prefix (never consumed) may be followed by more bytes than it declares.
    if (declaredLength <= kMaxTCPMessageSize)
    {
        presentBytes = std::min(presentBytes, declaredLength);
    }

    std::vector<uint8_t> stream;
    AppendPrefix(stream, declaredLength);
    stream.resize(stream.size() + presentBytes, 0x5A);

    std::vector<uint16_t> chunkSizes(std::min<size_t>(stream.size() / chunkSize + 1, kMaxChunks), chunkSize);
    CHIP_ERROR err = FeedSegmented(conn, stream, chunkSizes);
    if (gHarnessAllocFailed)
    {
        return;
    }

    if (declaredLength > kMaxTCPMessageSize)
    {
        ASSERT_EQ(err, CHIP_ERROR_MESSAGE_TOO_LONG);
        ASSERT_EQ(fx.delegate.mMessages.size(), 0u);
        ASSERT_FALSE(conn.IsOpen());
        ASSERT_EQ(fx.delegate.mClosedCount, 1u);
        return;
    }
    if (presentBytes < declaredLength)
    {
        ASSERT_EQ(err, CHIP_NO_ERROR);
        ASSERT_EQ(fx.delegate.mMessages.size(), 0u);
        ASSERT_TRUE(conn.IsOpen());
        ASSERT_EQ(fx.delegate.mClosedCount, 0u);
        return;
    }
    if (declaredLength == 0)
    {
        ASSERT_EQ(err, CHIP_ERROR_INVALID_MESSAGE_LENGTH);
        ASSERT_EQ(fx.delegate.mMessages.size(), 0u);
        ASSERT_TRUE(conn.IsOpen());
        ASSERT_EQ(fx.delegate.mClosedCount, 0u);
        return;
    }
    // The pool ran out before the contiguous copy; nothing truncated may be delivered.
    if (kStaticPool && err == CHIP_ERROR_NO_MEMORY)
    {
        ASSERT_EQ(fx.delegate.mMessages.size(), 0u);
        return;
    }
    ASSERT_EQ(err, CHIP_NO_ERROR);
    ASSERT_EQ(fx.delegate.mMessages.size(), 1u);
    ASSERT_EQ(fx.delegate.mMessages[0].size(), declaredLength);
    ASSERT_TRUE(conn.IsOpen());
    ASSERT_EQ(fx.delegate.mClosedCount, 0u);
}

// Sizes as integers so the size limit is reachable under the -max_len cap (see kMaxStreamSize).
FUZZ_TEST(FuzzTCPFramingPW, LengthPrefixBoundary)
    .WithDomains(Arbitrary<uint32_t>().WithSeeds({ 0, 1, kMaxTCPMessageSize, kMaxTCPMessageSize + 1, 0xFFFFFFFFu }),
                 InRange<uint32_t>(0, kMaxTCPMessageSize + 8).WithSeeds({ 0, kMaxTCPMessageSize, kMaxTCPMessageSize + 1 }),
                 InRange<uint16_t>(1, static_cast<uint16_t>(kMaxChunkSize)));

} // namespace
