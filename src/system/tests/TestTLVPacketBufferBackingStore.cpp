/*
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/Span.h>
#include <lib/support/UnitTestRegistration.h>
#include <system/TLVPacketBufferBackingStore.h>

#include <nlunit-test.h>

using ::chip::Platform::ScopedMemoryBuffer;
using ::chip::System::PacketBuffer;
using ::chip::System::PacketBufferHandle;
using ::chip::System::PacketBufferTLVReader;
using ::chip::System::PacketBufferTLVWriter;
using namespace ::chip;

namespace {

void WriteUntilRemainingLessThan(nlTestSuite * inSuite, PacketBufferTLVWriter & writer, const uint32_t remainingSize)
{
    uint32_t lengthRemaining = writer.GetRemainingFreeLength();
    while (lengthRemaining >= remainingSize)
    {
        NL_TEST_ASSERT(inSuite, writer.Put(TLV::AnonymousTag(), static_cast<uint8_t>(7)) == CHIP_NO_ERROR);
        lengthRemaining = writer.GetRemainingFreeLength();
    }
}

class TLVPacketBufferBackingStoreTest
{
public:
    static int TestSetup(void * inContext);
    static int TestTeardown(void * inContext);

    static void BasicEncodeDecode(nlTestSuite * inSuite, void * inContext);
    static void MultiBufferEncode(nlTestSuite * inSuite, void * inContext);
    static void NonChainedBufferCanReserve(nlTestSuite * inSuite, void * inContext);
    static void TestWriterReserveUnreserveDoesNotOverflow(nlTestSuite * inSuite, void * inContext);
    static void TestWriterReserve(nlTestSuite * inSuite, void * inContext);
};

int TLVPacketBufferBackingStoreTest::TestSetup(void * inContext)
{
    chip::Platform::MemoryInit();

    return SUCCESS;
}

int TLVPacketBufferBackingStoreTest::TestTeardown(void * inContext)
{
    chip::Platform::MemoryShutdown();

    return SUCCESS;
}

/**
 * Test that we can do a basic encode to TLV followed by decode.
 */
void TLVPacketBufferBackingStoreTest::BasicEncodeDecode(nlTestSuite * inSuite, void * inContext)
{
    auto buffer = PacketBufferHandle::New(PacketBuffer::kMaxSizeWithoutReserve, 0);

    PacketBufferTLVWriter writer;
    writer.Init(std::move(buffer));

    TLV::TLVType outerContainerType;
    CHIP_ERROR error = writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, outerContainerType);
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

    error = writer.Put(TLV::AnonymousTag(), static_cast<uint8_t>(7));
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

    error = writer.Put(TLV::AnonymousTag(), static_cast<uint8_t>(8));
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

    error = writer.Put(TLV::AnonymousTag(), static_cast<uint8_t>(9));
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

    error = writer.EndContainer(outerContainerType);
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

    error = writer.Finalize(&buffer);
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

    // Array start/end is 2 bytes.  Each entry is also 2 bytes: control +
    // value.  So 8 bytes total.
    NL_TEST_ASSERT(inSuite, !buffer->HasChainedBuffer());
    NL_TEST_ASSERT(inSuite, buffer->TotalLength() == 8);
    NL_TEST_ASSERT(inSuite, buffer->DataLength() == 8);

    PacketBufferTLVReader reader;
    reader.Init(std::move(buffer));

    error = reader.Next(TLV::kTLVType_Array, TLV::AnonymousTag());
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

    error = reader.EnterContainer(outerContainerType);
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

    error = reader.Next(TLV::kTLVType_UnsignedInteger, TLV::AnonymousTag());
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

    uint8_t value;
    error = reader.Get(value);
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, value == 7);

    error = reader.Next(TLV::kTLVType_UnsignedInteger, TLV::AnonymousTag());
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

    error = reader.Get(value);
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, value == 8);

    error = reader.Next(TLV::kTLVType_UnsignedInteger, TLV::AnonymousTag());
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

    error = reader.Get(value);
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, value == 9);

    error = reader.Next();
    NL_TEST_ASSERT(inSuite, error == CHIP_END_OF_TLV);

    error = reader.ExitContainer(outerContainerType);
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

    error = reader.Next();
    NL_TEST_ASSERT(inSuite, error == CHIP_END_OF_TLV);
}

/**
 * Test that we can do an encode that's going to split across multiple buffers correctly.
 */
void TLVPacketBufferBackingStoreTest::MultiBufferEncode(nlTestSuite * inSuite, void * inContext)
{
    // Start with a too-small buffer.
    auto buffer = PacketBufferHandle::New(2, 0);

    PacketBufferTLVWriter writer;
    writer.Init(std::move(buffer), /* useChainedBuffers = */ true);

    TLV::TLVType outerContainerType;
    CHIP_ERROR error = writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, outerContainerType);
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

    error = writer.Put(TLV::AnonymousTag(), static_cast<uint8_t>(7));
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

    error = writer.Put(TLV::AnonymousTag(), static_cast<uint8_t>(8));
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

    // Something to make sure we have 3 buffers.
    uint8_t bytes[2000] = { 0 };
    error               = writer.Put(TLV::AnonymousTag(), ByteSpan(bytes));
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

    error = writer.EndContainer(outerContainerType);
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

    error = writer.Finalize(&buffer);
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

    // Array start/end is 2 bytes.  First two entries are 2 bytes each.
    // Third entry is 1 control byte, 2 length bytes, 2000 bytes of data,
    // for a total of 2009 bytes.
    constexpr size_t totalSize = 2009;
    NL_TEST_ASSERT(inSuite, buffer->HasChainedBuffer());
    NL_TEST_ASSERT(inSuite, buffer->TotalLength() == totalSize);
    NL_TEST_ASSERT(inSuite, buffer->DataLength() == 2);
    auto nextBuffer = buffer->Next();
    NL_TEST_ASSERT(inSuite, nextBuffer->HasChainedBuffer());
    NL_TEST_ASSERT(inSuite, nextBuffer->TotalLength() == totalSize - 2);
    NL_TEST_ASSERT(inSuite, nextBuffer->DataLength() == PacketBuffer::kMaxSizeWithoutReserve);
    nextBuffer = nextBuffer->Next();
    NL_TEST_ASSERT(inSuite, !nextBuffer->HasChainedBuffer());
    NL_TEST_ASSERT(inSuite, nextBuffer->TotalLength() == nextBuffer->DataLength());
    NL_TEST_ASSERT(inSuite, nextBuffer->DataLength() == totalSize - 2 - PacketBuffer::kMaxSizeWithoutReserve);

    // PacketBufferTLVReader cannot handle non-contiguous buffers, and our
    // buffers are too big to stick into a single packet buffer.
    ScopedMemoryBuffer<uint8_t> buf;
    NL_TEST_ASSERT(inSuite, buf.Calloc(totalSize));
    size_t offset = 0;
    while (!buffer.IsNull())
    {
        memcpy(buf.Get() + offset, buffer->Start(), buffer->DataLength());
        offset += buffer->DataLength();
        buffer.Advance();
        NL_TEST_ASSERT(inSuite, offset < totalSize || (offset == totalSize && buffer.IsNull()));
    }

    TLV::TLVReader reader;
    reader.Init(buf.Get(), totalSize);

    error = reader.Next(TLV::kTLVType_Array, TLV::AnonymousTag());
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

    error = reader.EnterContainer(outerContainerType);
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

    error = reader.Next(TLV::kTLVType_UnsignedInteger, TLV::AnonymousTag());
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

    uint8_t value;
    error = reader.Get(value);
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, value == 7);

    error = reader.Next(TLV::kTLVType_UnsignedInteger, TLV::AnonymousTag());
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

    error = reader.Get(value);
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, value == 8);

    error = reader.Next(TLV::kTLVType_ByteString, TLV::AnonymousTag());
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

    ByteSpan byteValue;
    error = reader.Get(byteValue);
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, byteValue.size() == sizeof(bytes));

    error = reader.Next();
    NL_TEST_ASSERT(inSuite, error == CHIP_END_OF_TLV);

    error = reader.ExitContainer(outerContainerType);
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

    error = reader.Next();
    NL_TEST_ASSERT(inSuite, error == CHIP_END_OF_TLV);
}

void TLVPacketBufferBackingStoreTest::NonChainedBufferCanReserve(nlTestSuite * inSuite, void * inContext)
{
    // Start with a too-small buffer.
    uint32_t smallSize             = 5;
    uint32_t smallerSizeToReserver = smallSize - 1;

    auto buffer = PacketBufferHandle::New(smallSize, /* aReservedSize = */ 0);

    PacketBufferTLVWriter writer;
    writer.Init(std::move(buffer), /* useChainedBuffers = */ false);

    CHIP_ERROR error = writer.ReserveBuffer(smallerSizeToReserver);
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
}

// This test previously was created to show that there was an overflow bug, now this test mainly
// just checks that you cannot reserve this type of TLVBackingStorage buffer.
void TLVPacketBufferBackingStoreTest::TestWriterReserveUnreserveDoesNotOverflow(nlTestSuite * inSuite, void * inContext)
{
    // Start with a too-small buffer.
    uint32_t smallSize             = 100;
    uint32_t smallerSizeToReserver = smallSize - 1;

    auto buffer = PacketBufferHandle::New(smallSize, 0);

    PacketBufferTLVWriter writer;
    writer.Init(std::move(buffer), /* useChainedBuffers = */ true);

    CHIP_ERROR error = writer.ReserveBuffer(smallerSizeToReserver);
    if (error == CHIP_NO_ERROR)
    {
        uint32_t lengthRemaining = writer.GetRemainingFreeLength();
        NL_TEST_ASSERT(inSuite, lengthRemaining == 1);
        // Lets try to overflow by getting next buffer in the chain,
        // unreserving then writing until the end of the current buffer.
        error = writer.Put(TLV::AnonymousTag(), static_cast<uint8_t>(7));
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

        lengthRemaining = writer.GetRemainingFreeLength();
        NL_TEST_ASSERT(inSuite, lengthRemaining > smallerSizeToReserver);

        WriteUntilRemainingLessThan(inSuite, writer, 2);

        lengthRemaining = writer.GetRemainingFreeLength();
        NL_TEST_ASSERT(inSuite, lengthRemaining != 0);
        NL_TEST_ASSERT(inSuite, lengthRemaining < smallerSizeToReserver);

        error = writer.UnreserveBuffer(smallerSizeToReserver);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

        lengthRemaining = writer.GetRemainingFreeLength();
        NL_TEST_ASSERT(inSuite, lengthRemaining > smallerSizeToReserver);

        // This is where we get overflow.
        WriteUntilRemainingLessThan(inSuite, writer, 2);

        // If we get here then the overflow condition we were expecting did not happen. If that is the case,
        // either we have fixed reservation for chained buffers, or expected failure didn't hit on this
        // platform.
        //
        // If there is a fix please add reservation for chained buffers, please make sure you account for
        // what happens if TLVWriter::WriteData fails to get a new buffer but we are not at max size, do
        // you actually have space for what was supposed to be reserved.
        NL_TEST_ASSERT(inSuite, false);
    }

    // We no longer allow non-contigous buffers to be reserved.
    NL_TEST_ASSERT(inSuite, error == CHIP_ERROR_INCORRECT_STATE);
}

void TLVPacketBufferBackingStoreTest::TestWriterReserve(nlTestSuite * inSuite, void * inContext)
{
    // Start with a too-small buffer.
    uint32_t smallSize             = 5;
    uint32_t smallerSizeToReserver = smallSize - 1;

    auto buffer = PacketBufferHandle::New(smallSize, 0);

    PacketBufferTLVWriter writer;
    writer.Init(std::move(buffer), /* useChainedBuffers = */ false);

    CHIP_ERROR error = writer.ReserveBuffer(smallerSizeToReserver);
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

    error = writer.Put(TLV::AnonymousTag(), static_cast<uint8_t>(7));
    NL_TEST_ASSERT(inSuite, error == CHIP_ERROR_NO_MEMORY);

    error = writer.UnreserveBuffer(smallerSizeToReserver);
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

    error = writer.Put(TLV::AnonymousTag(), static_cast<uint8_t>(7));
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
}

/**
 *   Test Suite. It lists all the test functions.
 */
// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("BasicEncodeDecode",                         TLVPacketBufferBackingStoreTest::BasicEncodeDecode),
    NL_TEST_DEF("MultiBufferEncode",                         TLVPacketBufferBackingStoreTest::MultiBufferEncode),
    NL_TEST_DEF("NonChainedBufferCanReserve",                TLVPacketBufferBackingStoreTest::NonChainedBufferCanReserve),
    NL_TEST_DEF("TestWriterReserveUnreserveDoesNotOverflow", TLVPacketBufferBackingStoreTest::TestWriterReserveUnreserveDoesNotOverflow),
    NL_TEST_DEF("TestWriterReserve",                         TLVPacketBufferBackingStoreTest::TestWriterReserve),

    NL_TEST_SENTINEL()
};
// clang-format on

} // anonymous namespace

int TestTLVPacketBufferBackingStore()
{
    // clang-format off
    nlTestSuite theSuite = {
        .name ="chip-tlv-packet-buffer-backing-store",
        .tests = &sTests[0],
        .setup = TLVPacketBufferBackingStoreTest::TestSetup,
        .tear_down = TLVPacketBufferBackingStoreTest::TestTeardown,
    };
    // clang-format on

    // Run test suite.
    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestTLVPacketBufferBackingStore)
