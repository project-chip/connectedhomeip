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

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/Span.h>
#include <system/TLVPacketBufferBackingStore.h>

using ::chip::Platform::ScopedMemoryBuffer;
using ::chip::System::PacketBuffer;
using ::chip::System::PacketBufferHandle;
using ::chip::System::PacketBufferTLVReader;
using ::chip::System::PacketBufferTLVWriter;
using namespace ::chip;

class TestTLVPacketBufferBackingStore : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void WriteUntilRemainingLessThan(PacketBufferTLVWriter & writer, const uint32_t remainingSize)
    {
        uint32_t lengthRemaining = writer.GetRemainingFreeLength();
        while (lengthRemaining >= remainingSize)
        {
            EXPECT_EQ(writer.Put(TLV::AnonymousTag(), static_cast<uint8_t>(7)), CHIP_NO_ERROR);
            lengthRemaining = writer.GetRemainingFreeLength();
        }
    }
};

/**
 * Test that we can do a basic encode to TLV followed by decode.
 */
TEST_F(TestTLVPacketBufferBackingStore, BasicEncodeDecode)
{
    auto buffer = PacketBufferHandle::New(PacketBuffer::kMaxSizeWithoutReserve, 0);

    PacketBufferTLVWriter writer;
    writer.Init(std::move(buffer));

    TLV::TLVType outerContainerType;
    CHIP_ERROR error = writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, outerContainerType);
    EXPECT_EQ(error, CHIP_NO_ERROR);

    error = writer.Put(TLV::AnonymousTag(), static_cast<uint8_t>(7));
    EXPECT_EQ(error, CHIP_NO_ERROR);

    error = writer.Put(TLV::AnonymousTag(), static_cast<uint8_t>(8));
    EXPECT_EQ(error, CHIP_NO_ERROR);

    error = writer.Put(TLV::AnonymousTag(), static_cast<uint8_t>(9));
    EXPECT_EQ(error, CHIP_NO_ERROR);

    error = writer.EndContainer(outerContainerType);
    EXPECT_EQ(error, CHIP_NO_ERROR);

    error = writer.Finalize(&buffer);
    EXPECT_EQ(error, CHIP_NO_ERROR);

    // Array start/end is 2 bytes.  Each entry is also 2 bytes: control +
    // value.  So 8 bytes total.
    EXPECT_FALSE(buffer->HasChainedBuffer());
    EXPECT_EQ(buffer->TotalLength(), static_cast<size_t>(8));
    EXPECT_EQ(buffer->DataLength(), static_cast<size_t>(8));

    PacketBufferTLVReader reader;
    reader.Init(std::move(buffer));

    error = reader.Next(TLV::kTLVType_Array, TLV::AnonymousTag());
    EXPECT_EQ(error, CHIP_NO_ERROR);

    error = reader.EnterContainer(outerContainerType);
    EXPECT_EQ(error, CHIP_NO_ERROR);

    error = reader.Next(TLV::kTLVType_UnsignedInteger, TLV::AnonymousTag());
    EXPECT_EQ(error, CHIP_NO_ERROR);

    uint8_t value;
    error = reader.Get(value);
    EXPECT_EQ(error, CHIP_NO_ERROR);
    EXPECT_EQ(value, 7);

    error = reader.Next(TLV::kTLVType_UnsignedInteger, TLV::AnonymousTag());
    EXPECT_EQ(error, CHIP_NO_ERROR);

    error = reader.Get(value);
    EXPECT_EQ(error, CHIP_NO_ERROR);
    EXPECT_EQ(value, 8);

    error = reader.Next(TLV::kTLVType_UnsignedInteger, TLV::AnonymousTag());
    EXPECT_EQ(error, CHIP_NO_ERROR);

    error = reader.Get(value);
    EXPECT_EQ(error, CHIP_NO_ERROR);
    EXPECT_EQ(value, 9);

    error = reader.Next();
    EXPECT_EQ(error, CHIP_END_OF_TLV);

    error = reader.ExitContainer(outerContainerType);
    EXPECT_EQ(error, CHIP_NO_ERROR);

    error = reader.Next();
    EXPECT_EQ(error, CHIP_END_OF_TLV);
}

/**
 * Test that we can do an encode that's going to split across multiple buffers correctly.
 */
TEST_F(TestTLVPacketBufferBackingStore, MultiBufferEncode)
{
    // Start with a too-small buffer.
    auto buffer = PacketBufferHandle::New(2, 0);

    PacketBufferTLVWriter writer;
    writer.Init(std::move(buffer), /* useChainedBuffers = */ true);

    TLV::TLVType outerContainerType;
    CHIP_ERROR error = writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, outerContainerType);
    EXPECT_EQ(error, CHIP_NO_ERROR);

    error = writer.Put(TLV::AnonymousTag(), static_cast<uint8_t>(7));
    EXPECT_EQ(error, CHIP_NO_ERROR);

    error = writer.Put(TLV::AnonymousTag(), static_cast<uint8_t>(8));
    EXPECT_EQ(error, CHIP_NO_ERROR);

    // Something to make sure we have 3 buffers.
    uint8_t bytes[2000] = { 0 };
    error               = writer.Put(TLV::AnonymousTag(), ByteSpan(bytes));
    EXPECT_EQ(error, CHIP_NO_ERROR);

    error = writer.EndContainer(outerContainerType);
    EXPECT_EQ(error, CHIP_NO_ERROR);

    error = writer.Finalize(&buffer);
    EXPECT_EQ(error, CHIP_NO_ERROR);

    // Array start/end is 2 bytes.  First two entries are 2 bytes each.
    // Third entry is 1 control byte, 2 length bytes, 2000 bytes of data,
    // for a total of 2009 bytes.
    constexpr size_t totalSize = 2009;
#if CHIP_SYSTEM_PACKETBUFFER_FROM_LWIP_STANDARD_POOL || CHIP_SYSTEM_PACKETBUFFER_FROM_CHIP_POOL
    // In case of pool allocation, the buffer size is always the maximum size.
    constexpr size_t bufferSizes[] = { PacketBuffer::kMaxSizeWithoutReserve, totalSize - PacketBuffer::kMaxSizeWithoutReserve };
#elif CHIP_SYSTEM_PACKETBUFFER_FROM_CHIP_HEAP
    constexpr size_t bufferSizes[] = { 2, PacketBuffer::kMaxSizeWithoutReserve,
                                       totalSize - 2 - PacketBuffer::kMaxSizeWithoutReserve };
#else
    // Skipping the test for other configurations because the allocation method is unknown.
    GTEST_SKIP();
#endif
    size_t checkedSize = 0;
    auto bufferTmp     = buffer.Retain();
    for (const auto size : bufferSizes)
    {
        ASSERT_FALSE(bufferTmp.IsNull());
        EXPECT_EQ(bufferTmp->TotalLength(), totalSize - checkedSize);
        EXPECT_EQ(bufferTmp->DataLength(), size);
        bufferTmp = bufferTmp->Next();
        checkedSize += size;
    }
    // There should be no more buffers.
    ASSERT_TRUE(bufferTmp.IsNull());

    // PacketBufferTLVReader cannot handle non-contiguous buffers, and our
    // buffers are too big to stick into a single packet buffer.
    ScopedMemoryBuffer<uint8_t> buf;
    EXPECT_TRUE(buf.Calloc(totalSize));
    size_t offset = 0;
    while (!buffer.IsNull())
    {
        memcpy(buf.Get() + offset, buffer->Start(), buffer->DataLength());
        offset += buffer->DataLength();
        buffer.Advance();
        EXPECT_TRUE(offset < totalSize || (offset == totalSize && buffer.IsNull()));
    }

    TLV::TLVReader reader;
    reader.Init(buf.Get(), totalSize);

    error = reader.Next(TLV::kTLVType_Array, TLV::AnonymousTag());
    EXPECT_EQ(error, CHIP_NO_ERROR);

    error = reader.EnterContainer(outerContainerType);
    EXPECT_EQ(error, CHIP_NO_ERROR);

    error = reader.Next(TLV::kTLVType_UnsignedInteger, TLV::AnonymousTag());
    EXPECT_EQ(error, CHIP_NO_ERROR);

    uint8_t value;
    error = reader.Get(value);
    EXPECT_EQ(error, CHIP_NO_ERROR);
    EXPECT_EQ(value, 7);

    error = reader.Next(TLV::kTLVType_UnsignedInteger, TLV::AnonymousTag());
    EXPECT_EQ(error, CHIP_NO_ERROR);

    error = reader.Get(value);
    EXPECT_EQ(error, CHIP_NO_ERROR);
    EXPECT_EQ(value, 8);

    error = reader.Next(TLV::kTLVType_ByteString, TLV::AnonymousTag());
    EXPECT_EQ(error, CHIP_NO_ERROR);

    ByteSpan byteValue;
    error = reader.Get(byteValue);
    EXPECT_EQ(error, CHIP_NO_ERROR);
    EXPECT_EQ(byteValue.size(), sizeof(bytes));

    error = reader.Next();
    EXPECT_EQ(error, CHIP_END_OF_TLV);

    error = reader.ExitContainer(outerContainerType);
    EXPECT_EQ(error, CHIP_NO_ERROR);

    error = reader.Next();
    EXPECT_EQ(error, CHIP_END_OF_TLV);
}

TEST_F(TestTLVPacketBufferBackingStore, NonChainedBufferCanReserve)
{
    // Start with a too-small buffer.
    uint32_t smallSize             = 5;
    uint32_t smallerSizeToReserver = smallSize - 1;

    auto buffer = PacketBufferHandle::New(smallSize, /* aReservedSize = */ 0);

    PacketBufferTLVWriter writer;
    writer.Init(std::move(buffer), /* useChainedBuffers = */ false);

    CHIP_ERROR error = writer.ReserveBuffer(smallerSizeToReserver);
    EXPECT_EQ(error, CHIP_NO_ERROR);
}

// This test previously was created to show that there was an overflow bug, now this test mainly
// just checks that you cannot reserve this type of TLVBackingStorage buffer.
TEST_F(TestTLVPacketBufferBackingStore, TestWriterReserveUnreserveDoesNotOverflow)
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
        EXPECT_EQ(lengthRemaining, 1U);
        // Lets try to overflow by getting next buffer in the chain,
        // unreserving then writing until the end of the current buffer.
        error = writer.Put(TLV::AnonymousTag(), static_cast<uint8_t>(7));
        EXPECT_EQ(error, CHIP_NO_ERROR);

        lengthRemaining = writer.GetRemainingFreeLength();
        EXPECT_GT(lengthRemaining, smallerSizeToReserver);

        WriteUntilRemainingLessThan(writer, 2);

        lengthRemaining = writer.GetRemainingFreeLength();
        EXPECT_NE(lengthRemaining, 0U);
        EXPECT_LT(lengthRemaining, smallerSizeToReserver);

        error = writer.UnreserveBuffer(smallerSizeToReserver);
        EXPECT_EQ(error, CHIP_NO_ERROR);

        lengthRemaining = writer.GetRemainingFreeLength();
        EXPECT_GT(lengthRemaining, smallerSizeToReserver);

        // This is where we get overflow.
        WriteUntilRemainingLessThan(writer, 2);

        // If we get here then the overflow condition we were expecting did not happen. If that is the case,
        // either we have fixed reservation for chained buffers, or expected failure didn't hit on this
        // platform.
        //
        // If there is a fix please add reservation for chained buffers, please make sure you account for
        // what happens if TLVWriter::WriteData fails to get a new buffer but we are not at max size, do
        // you actually have space for what was supposed to be reserved.
        FAIL();
    }

    // We no longer allow non-contigous buffers to be reserved.
    EXPECT_EQ(error, CHIP_ERROR_INCORRECT_STATE);
}

#if CHIP_SYSTEM_PACKETBUFFER_FROM_CHIP_HEAP
TEST_F(TestTLVPacketBufferBackingStore, TestWriterReserve)
{
    // Start with a too-small buffer.
    uint32_t smallSize             = 5;
    uint32_t smallerSizeToReserver = smallSize - 1;

    auto buffer = PacketBufferHandle::New(smallSize, 0);

    PacketBufferTLVWriter writer;
    writer.Init(std::move(buffer), /* useChainedBuffers = */ false);

    CHIP_ERROR error = writer.ReserveBuffer(smallerSizeToReserver);
    EXPECT_EQ(error, CHIP_NO_ERROR);

    error = writer.Put(TLV::AnonymousTag(), static_cast<uint8_t>(7));
    EXPECT_EQ(error, CHIP_ERROR_NO_MEMORY);

    error = writer.UnreserveBuffer(smallerSizeToReserver);
    EXPECT_EQ(error, CHIP_NO_ERROR);

    error = writer.Put(TLV::AnonymousTag(), static_cast<uint8_t>(7));
    EXPECT_EQ(error, CHIP_NO_ERROR);
}
#endif
