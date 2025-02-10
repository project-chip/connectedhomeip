/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
 *      This file implements a unit test suite for
 *      <tt>chip::System::PacketBuffer</tt>, a class that provides
 *      structure for network packet buffer management.
 */

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <utility>
#include <vector>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemPacketBuffer.h>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/init.h>
#include <lwip/tcpip.h>
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#if (LWIP_VERSION_MAJOR == 2) && (LWIP_VERSION_MINOR == 0)
#define PBUF_TYPE(pbuf) (pbuf)->type
#else // (LWIP_VERSION_MAJOR == 2) && (LWIP_VERSION_MINOR == 0)
#define PBUF_TYPE(pbuf) (pbuf)->type_internal
#endif // (LWIP_VERSION_MAJOR == 2) && (LWIP_VERSION_MINOR == 0)
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

using ::chip::Encoding::PacketBufferWriter;
using ::chip::System::PacketBuffer;
using ::chip::System::PacketBufferHandle;

#if !CHIP_SYSTEM_CONFIG_USE_LWIP
using ::chip::System::pbuf;
#endif

// Utility functions.

#define TO_LWIP_PBUF(x) (reinterpret_cast<struct pbuf *>(reinterpret_cast<void *>(x)))
#define OF_LWIP_PBUF(x) (reinterpret_cast<PacketBuffer *>(reinterpret_cast<void *>(x)))

namespace {
void ScrambleData(uint8_t * start, size_t length)
{
    for (size_t i = 0; i < length; ++i)
        ++start[i];
}
} // namespace

namespace chip {
namespace System {

/*
 * This class is a friend class of `PacketBuffer` and `PacketBufferHandle` because some tests
 * use or check private methods or properties.
 */
class TestSystemPacketBuffer : public ::testing::Test
{
public:
    static constexpr auto kBlockSize     = PacketBuffer::kBlockSize;
    static constexpr auto kStructureSize = PacketBuffer::kStructureSize;

    static constexpr uint16_t kReservedSizes[] = { 0, 10, 128, 1536, PacketBuffer::kMaxSizeWithoutReserve, kBlockSize };
    static constexpr uint16_t kLengths[]       = { 0, 1, 10, 128, kBlockSize, UINT16_MAX };

    static void SetUpTestSuite()
    {
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
        ASSERT_EQ(chip::DeviceLayer::PlatformMgr().InitChipStack(), CHIP_NO_ERROR);
    }

    static void TearDownTestSuite()
    {
        chip::DeviceLayer::PlatformMgr().Shutdown();
        chip::Platform::MemoryShutdown();

        // Deregister the layer error formatter
        DeregisterCHIPLayerErrorFormatter();
    }

    void SetUp()
    {
        configurations.resize(0);
        // Set up the buffer configuration vector for this suite.
        for (auto size : kReservedSizes)
        {
            configurations.emplace_back<BufferConfiguration>(size);
        }
    }

    void TearDown()
    {
        ASSERT_TRUE(ResetConfigurations());
        ASSERT_TRUE(ResetHandles());
    }

    static void PrintHandle(const char * tag, const PacketBuffer * buffer)
    {
        printf("%s %p ref=%u len=%-4zu next=%p\n", StringOrNullMarker(tag), buffer, buffer ? buffer->ref : 0,
               buffer ? buffer->len : 0, buffer ? buffer->next : nullptr);
    }
    static void PrintHandle(const char * tag, const PacketBufferHandle & handle) { PrintHandle(tag, handle.mBuffer); }

    struct BufferConfiguration
    {
        BufferConfiguration(uint16_t aReservedSize = 0) :
            init_len(0), reserved_size(aReservedSize), start_buffer(nullptr), end_buffer(nullptr), payload_ptr(nullptr),
            handle(nullptr)
        {}

        size_t init_len;
        uint16_t reserved_size;
        uint8_t * start_buffer;
        uint8_t * end_buffer;
        uint8_t * payload_ptr;
        PacketBufferHandle handle;
    };

    static void PrintHandle(const char * tag, const BufferConfiguration & config) { PrintHandle(tag, config.handle); }
    static void PrintConfig(const char * tag, const BufferConfiguration & config)
    {
        printf("%s pay=%-4zu len=%-4zu res=%-4u:", StringOrNullMarker(tag), config.payload_ptr - config.start_buffer,
               config.init_len, config.reserved_size);
        PrintHandle("", config.handle);
    }

    /*
     * Buffers allocated through PrepareTestBuffer with kRecordHandle set will be recorded in `handles` so that their
     * reference counts can be verified by ResetHandles(). Initially they have two refs: the recorded one and the returned one.
     */
    static constexpr int kRecordHandle     = 0x01;
    static constexpr int kAllowHandleReuse = 0x02;
    void PrepareTestBuffer(BufferConfiguration * config, int flags = 0);

    /*
     * Checks and clears the recorded handles. Returns true if it detects no leaks or double frees.
     * Called from `TerminateTest()`, but tests may choose to call it more often to verify reference counts.
     */
    bool ResetConfigurations();
    bool ResetHandles();

    std::vector<BufferConfiguration> configurations;
    std::vector<PacketBufferHandle> handles;

    void CheckAddRef();
    void CheckAddToEnd();
    void CheckCompactHead();
    void CheckConsume();
    void CheckConsumeHead();
    void CheckDataLength();
    void CheckEnsureReservedSize();
    void CheckFree();
    void CheckFreeHead();
    void CheckHandleAdopt();
    void CheckHandleAdvance();
    void CheckHandleCloneData();
    void CheckHandleConstruct();
    void CheckHandleFree();
    void CheckHandleHold();
    void CheckHandleMove();
    void CheckHandleRelease();
    void CheckHandleRetain();
    void CheckHandleRightSize();
    void CheckLast();
    void CheckNew();
    void CheckNext();
    void CheckPopHead();
    void CheckRead();
    void CheckSetDataLength();
    void CheckSetStart();
};

/**
 *  Allocate memory for a test buffer and configure according to test buffer configuration.
 */
void TestSystemPacketBuffer::PrepareTestBuffer(BufferConfiguration * config, int flags)
{
    if (config->handle.IsNull())
    {
        config->handle = PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSizeWithoutReserve, 0);
        VerifyOrDieWithMsg(!config->handle.IsNull(), chipSystemLayer,
                           "NewPacketBuffer: Failed to allocate packet buffer (%u retained): %s",
                           static_cast<unsigned int>(handles.size()), strerror(errno));
        if (flags & kRecordHandle)
        {
            handles.push_back(config->handle.Retain());
        }
    }
    else
    {
        VerifyOrDieWithMsg((flags & kAllowHandleReuse) != 0, chipSystemLayer, "Dirty test configuration");
    }

    const size_t lInitialSize = kStructureSize + config->reserved_size;
    const size_t lAllocSize   = kBlockSize;

    uint8_t * const raw = reinterpret_cast<uint8_t *>(config->handle.Get());
    memset(raw + kStructureSize, 0, lAllocSize - kStructureSize);

    config->start_buffer = raw;
    config->end_buffer   = raw + lAllocSize;

    if (lInitialSize > lAllocSize)
    {
        config->payload_ptr = config->end_buffer;
    }
    else
    {
        config->payload_ptr = config->start_buffer + lInitialSize;
    }

    if (config->handle->HasChainedBuffer())
    {
        // This should not happen.
        PacketBuffer::Free(config->handle->ChainedBuffer());
        config->handle->next = nullptr;
    }
    config->handle->payload = config->payload_ptr;
#if CHIP_SYSTEM_CONFIG_USE_LWIP
    VerifyOrDieWithMsg(chip::CanCastTo<uint16_t>(config->init_len), chipSystemLayer, "Max Length exceeded for LwIP based systems");

    config->handle->len     = static_cast<uint16_t>(config->init_len);
    config->handle->tot_len = static_cast<uint16_t>(config->init_len);
#else
    config->handle->len     = config->init_len;
    config->handle->tot_len = config->init_len;
#endif
}

bool TestSystemPacketBuffer::ResetConfigurations()
{
    // Clear the configurations' buffer handles.
    for (auto & configuration : configurations)
        configuration.handle = nullptr;
    return true;
}

bool TestSystemPacketBuffer::ResetHandles()
{
    // Check against leaks or double-frees in tests: every handle obtained through
    // TestSystemPacketBuffer::NewPacketBuffer should have a reference count of 1.
    bool handles_ok = true;
    for (size_t i = 0; i < handles.size(); ++i)
    {
        const PacketBufferHandle & handle = handles[i];
        if (handle.Get() == nullptr)
        {
            printf("TestTerminate: handle %u null\n", static_cast<unsigned int>(i));
            handles_ok = false;
        }
        else if (handle->ref != 1)
        {
            printf("TestTerminate: handle %u buffer=%p ref=%u\n", static_cast<unsigned int>(i), handle.Get(), handle->ref);
            handles_ok = false;
            while (handle->ref > 1)
            {
                PacketBuffer::Free(handle.Get());
            }
        }
    }
    handles.resize(0);
    return handles_ok;
}

/**
 *  Test PacketBufferHandle::New() function.
 *
 *  Description: For every buffer-configuration from inContext, create a buffer's instance
 *               using the New() method. Then, verify that when the size of the reserved space
 *               passed to New() is greater than PacketBuffer::kMaxSizeWithoutReserve,
 *               the method returns nullptr. Otherwise, check for correctness of initializing
 *               the new buffer's internal state.
 */
TEST_F_FROM_FIXTURE(TestSystemPacketBuffer, CheckNew)
{
    for (const auto & config : configurations)
    {
        const PacketBufferHandle buffer = PacketBufferHandle::New(0, config.reserved_size);

        if (config.reserved_size > PacketBuffer::kMaxAllocSize)
        {
            EXPECT_TRUE(buffer.IsNull());
            continue;
        }

        EXPECT_LE(config.reserved_size, buffer->AllocSize());
        ASSERT_FALSE(buffer.IsNull());

        if (!buffer.IsNull())
        {
            // TODO: the code below seems maybe questionable: OF_LWIP_PBUF is never used
            // NOLINTBEGIN(bugprone-casting-through-void)
            const pbuf * const pb = TO_LWIP_PBUF(buffer.Get());
            // NOLINTEND(bugprone-casting-through-void)

            EXPECT_EQ(pb->len, static_cast<size_t>(0));
            EXPECT_EQ(pb->tot_len, static_cast<size_t>(0));
            EXPECT_EQ(pb->next, nullptr);
            EXPECT_EQ(pb->ref, 1);
        }
    }

#if CHIP_SYSTEM_PACKETBUFFER_FROM_LWIP_POOL || CHIP_SYSTEM_PACKETBUFFER_FROM_CHIP_POOL
    // Use the rest of the buffer space
    std::vector<PacketBufferHandle> allocate_all_the_things;
    for (;;)
    {
        PacketBufferHandle buffer = PacketBufferHandle::New(0, 0);
        if (buffer.IsNull())
        {
            break;
        }
        // Hold on to the buffer, to use up all the buffer space.
        allocate_all_the_things.push_back(std::move(buffer));
    }
#endif // CHIP_SYSTEM_PACKETBUFFER_FROM_LWIP_POOL || CHIP_SYSTEM_PACKETBUFFER_FROM_CHIP_POOL
}

/**
 *  Test PacketBuffer::Start() function.
 */
TEST_F(TestSystemPacketBuffer, CheckStart)
{

    for (auto & config : configurations)
    {
        PrepareTestBuffer(&config, kRecordHandle);
        EXPECT_EQ(config.handle->Start(), config.payload_ptr);
    }
}

/**
 *  Test PacketBuffer::SetStart() function.
 *
 *  Description: For every buffer-configuration from inContext, create a
 *               buffer's instance according to the configuration. Next,
 *               for any offset value from start_offset[], pass it to the
 *               buffer's instance through SetStart method. Then, verify that
 *               the beginning of the buffer has been correctly internally
 *               adjusted according to the offset value passed into the
 *               SetStart() method.
 */
TEST_F_FROM_FIXTURE(TestSystemPacketBuffer, CheckSetStart)
{
    static constexpr ptrdiff_t sSizePacketBuffer = kBlockSize;

    for (auto & config : configurations)
    {
        // clang-format off
        static constexpr ptrdiff_t start_offset[] =
        {
            -sSizePacketBuffer,
            -128,
            -1,
            0,
            1,
            128,
            sSizePacketBuffer
        };
        // clang-format on

        for (ptrdiff_t offset : start_offset)
        {
            PrepareTestBuffer(&config, kRecordHandle | kAllowHandleReuse);
            uint8_t * const test_start = config.payload_ptr + offset;
            uint8_t * verify_start     = test_start;

            config.handle->SetStart(test_start);

            if (verify_start < config.start_buffer + kStructureSize)
            {
                // Set start before valid payload beginning.
                verify_start = config.start_buffer + kStructureSize;
            }

            if (verify_start > config.end_buffer)
            {
                // Set start after valid payload beginning.
                verify_start = config.end_buffer;
            }

            EXPECT_EQ(config.handle->payload, verify_start);

            if (verify_start - config.payload_ptr > static_cast<ptrdiff_t>(config.init_len))
            {
                // Set start to the beginning of payload, right after handle's header.
                EXPECT_EQ(config.handle->len, static_cast<size_t>(0));
            }
            else
            {
                // Set start to somewhere between the end of the handle's
                // header and the end of payload.
                EXPECT_EQ(config.handle->len,
                          static_cast<size_t>((static_cast<int32_t>(config.init_len) - (verify_start - config.payload_ptr))));
            }
        }
    }
}

/**
 *  Test PacketBuffer::DataLength() function.
 */
TEST_F_FROM_FIXTURE(TestSystemPacketBuffer, CheckDataLength)
{
    for (auto & config : configurations)
    {
        PrepareTestBuffer(&config, kRecordHandle);
        EXPECT_EQ(config.handle->DataLength(), config.handle->len);
    }
}

/**
 *  Test PacketBuffer::SetDataLength() function.
 *
 *  Description: Take two initial configurations of PacketBuffer from
 *               inContext and create two PacketBuffer instances based on those
 *               configurations. For any two buffers, call SetDataLength with
 *               different value from sLength[]. If two buffers are created with
 *               the same configuration, test SetDataLength on one buffer,
 *               without specifying the head of the buffer chain. Otherwise,
 *               test SetDataLength with one buffer being down the chain and the
 *               other one being passed as the head of the chain. After calling
 *               the method verify that data lengths were correctly adjusted.
 */
TEST_F_FROM_FIXTURE(TestSystemPacketBuffer, CheckSetDataLength)
{
    for (auto & config_1 : configurations)
    {
        for (auto & config_2 : configurations)
        {
            for (auto length : kLengths)
            {
                PrepareTestBuffer(&config_1, kRecordHandle | kAllowHandleReuse);
                PrepareTestBuffer(&config_2, kRecordHandle | kAllowHandleReuse);

                if (&config_1 == &config_2)
                {
                    // headOfChain (the second arg) is NULL
                    config_2.handle->SetDataLength(length, nullptr);

                    if (length > (config_2.end_buffer - config_2.payload_ptr))
                    {
                        EXPECT_EQ(config_2.handle->len, static_cast<size_t>(config_2.end_buffer - config_2.payload_ptr));
                        EXPECT_EQ(config_2.handle->tot_len, static_cast<size_t>(config_2.end_buffer - config_2.payload_ptr));
                        EXPECT_EQ(config_2.handle.GetNext(), nullptr);
                    }
                    else
                    {
                        EXPECT_EQ(config_2.handle->len, length);
                        EXPECT_EQ(config_2.handle->tot_len, length);
                        EXPECT_EQ(config_2.handle.GetNext(), nullptr);
                    }
                }
                else
                {
                    // headOfChain (the second arg) is config_1.handle
                    config_2.handle->SetDataLength(length, config_1.handle);

                    if (length > (config_2.end_buffer - config_2.payload_ptr))
                    {
                        EXPECT_EQ(config_2.handle->len, static_cast<size_t>(config_2.end_buffer - config_2.payload_ptr));
                        EXPECT_EQ(config_2.handle->tot_len, static_cast<size_t>(config_2.end_buffer - config_2.payload_ptr));
                        EXPECT_EQ(config_2.handle.GetNext(), nullptr);

                        EXPECT_EQ(config_1.handle->tot_len,
                                  static_cast<size_t>(static_cast<int32_t>(config_1.init_len) +
                                                      static_cast<int32_t>(config_2.end_buffer - config_2.payload_ptr) -
                                                      static_cast<int32_t>(config_2.init_len)));
                    }
                    else
                    {
                        EXPECT_EQ(config_2.handle->len, length);
                        EXPECT_EQ(config_2.handle->tot_len, length);
                        EXPECT_EQ(config_2.handle.GetNext(), nullptr);

                        EXPECT_EQ(config_1.handle->tot_len,
                                  static_cast<size_t>(static_cast<int32_t>(config_1.init_len) + static_cast<int32_t>(length) -
                                                      static_cast<int32_t>(config_2.init_len)));
                    }
                }
            }
        }
    }
}

/**
 *  Test PacketBuffer::TotalLength() function.
 */
TEST_F(TestSystemPacketBuffer, CheckTotalLength)
{
    for (auto & config : configurations)
    {
        PrepareTestBuffer(&config, kRecordHandle);
        EXPECT_EQ(config.handle->TotalLength(), config.init_len);
    }
}

/**
 *  Test PacketBuffer::MaxDataLength() function.
 */
TEST_F(TestSystemPacketBuffer, CheckMaxDataLength)
{
    for (auto & config : configurations)
    {
        PrepareTestBuffer(&config, kRecordHandle);
        EXPECT_EQ(config.handle->MaxDataLength(), static_cast<size_t>(config.end_buffer - config.payload_ptr));
    }
}

/**
 *  Test PacketBuffer::AvailableDataLength() function.
 */
TEST_F(TestSystemPacketBuffer, CheckAvailableDataLength)
{
    for (auto & config : configurations)
    {
        PrepareTestBuffer(&config, kRecordHandle);
        EXPECT_EQ(config.handle->AvailableDataLength(),
                  static_cast<size_t>(static_cast<int32_t>(config.end_buffer - config.payload_ptr) -
                                      static_cast<int32_t>(config.init_len)));
    }
}

/**
 *  Test PacketBuffer::ReservedSize() function.
 */
TEST_F(TestSystemPacketBuffer, CheckReservedSize)
{
    for (auto & config : configurations)
    {
        PrepareTestBuffer(&config, kRecordHandle);
        const size_t kAllocSize = config.handle->AllocSize();

        if (config.reserved_size > kAllocSize)
        {
            EXPECT_EQ(config.handle->ReservedSize(), kAllocSize);
        }
        else
        {
            EXPECT_EQ(config.handle->ReservedSize(), config.reserved_size);
        }
    }
}

/**
 *  Test PacketBuffer::HasChainedBuffer() function.
 */
TEST_F(TestSystemPacketBuffer, CheckHasChainedBuffer)
{
    for (auto & config_1 : configurations)
    {
        for (auto & config_2 : configurations)
        {
            if (&config_1 == &config_2)
            {
                continue;
            }

            PrepareTestBuffer(&config_1);
            PrepareTestBuffer(&config_2);

            EXPECT_FALSE(config_1.handle->HasChainedBuffer());
            EXPECT_FALSE(config_2.handle->HasChainedBuffer());

            config_1.handle->AddToEnd(config_2.handle.Retain());
            EXPECT_TRUE(config_1.handle->HasChainedBuffer());
            EXPECT_FALSE(config_2.handle->HasChainedBuffer());

            config_1.handle = nullptr;
            config_2.handle = nullptr;
        }
    }
}

/**
 *  Test PacketBuffer::AddToEnd() function.
 *
 *  Description: Take three initial configurations of PacketBuffer from
 *               inContext, create three PacketBuffers based on those
 *               configurations and then link those buffers together with
 *               PacketBuffer:AddToEnd(). Then, assert that after connecting
 *               buffers together, their internal states are correctly updated.
 *               This test function tests linking any combination of three
 *               buffer-configurations passed within inContext.
 */
TEST_F_FROM_FIXTURE(TestSystemPacketBuffer, CheckAddToEnd)
{
    for (auto & config_1 : configurations)
    {
        for (auto & config_2 : configurations)
        {
            for (auto & config_3 : configurations)
            {
                if (&config_1 == &config_2 || &config_1 == &config_3 || &config_2 == &config_3)
                {
                    continue;
                }

                PrepareTestBuffer(&config_1);
                PrepareTestBuffer(&config_2);
                PrepareTestBuffer(&config_3);
                EXPECT_EQ(config_1.handle->ref, 1);
                EXPECT_EQ(config_2.handle->ref, 1);
                EXPECT_EQ(config_3.handle->ref, 1);

                config_1.handle->AddToEnd(config_2.handle.Retain());
                EXPECT_EQ(config_1.handle->ref, 1); // config_1.handle
                EXPECT_EQ(config_2.handle->ref, 2); // config_2.handle and config_1.handle->next
                EXPECT_EQ(config_3.handle->ref, 1); // config_3.handle

                EXPECT_EQ(config_1.handle->tot_len, (config_1.init_len + config_2.init_len));
                EXPECT_EQ(config_1.handle.GetNext(), config_2.handle.Get());
                EXPECT_EQ(config_2.handle.GetNext(), nullptr);
                EXPECT_EQ(config_3.handle.GetNext(), nullptr);

                config_1.handle->AddToEnd(config_3.handle.Retain());
                EXPECT_EQ(config_1.handle->ref, 1); // config_1.handle
                EXPECT_EQ(config_2.handle->ref, 2); // config_2.handle and config_1.handle->next
                EXPECT_EQ(config_3.handle->ref, 2); // config_3.handle and config_2.handle->next

                EXPECT_EQ(config_1.handle->tot_len, (config_1.init_len + config_2.init_len + config_3.init_len));
                EXPECT_EQ(config_1.handle.GetNext(), config_2.handle.Get());
                EXPECT_EQ(config_2.handle.GetNext(), config_3.handle.Get());
                EXPECT_EQ(config_3.handle.GetNext(), nullptr);

                config_1.handle = nullptr;
                config_2.handle = nullptr;
                config_3.handle = nullptr;
            }
        }
    }
}

/**
 *  Test PacketBuffer::PopHead() function.
 *
 *  Description: Take two initial configurations of PacketBuffer from
 *               inContext and create two PacketBuffer instances based on those
 *               configurations. Next, link those buffers together, with the first
 *               buffer instance pointing to the second one. Then, call PopHead()
 *               on the first buffer to unlink the second buffer. After the call,
 *               verify correct internal state of the first buffer.
 */
TEST_F_FROM_FIXTURE(TestSystemPacketBuffer, CheckPopHead)
{
    // Single buffer test.
    for (auto & config_1 : configurations)
    {
        PrepareTestBuffer(&config_1, kRecordHandle | kAllowHandleReuse);
        EXPECT_EQ(config_1.handle->ref, 2);

        const PacketBuffer * const buffer_1 = config_1.handle.mBuffer;

        const PacketBufferHandle popped = config_1.handle.PopHead();

        EXPECT_TRUE(config_1.handle.IsNull());
        EXPECT_EQ(popped.mBuffer, buffer_1);
        EXPECT_EQ(popped->next, nullptr);
        EXPECT_EQ(popped->tot_len, config_1.init_len);
        EXPECT_EQ(popped->ref, 2);
    }

    ResetHandles();

    // Chained buffers test.
    for (auto & config_1 : configurations)
    {
        for (auto & config_2 : configurations)
        {
            if (&config_1 == &config_2)
            {
                continue;
            }

            PrepareTestBuffer(&config_1, kRecordHandle | kAllowHandleReuse);
            PrepareTestBuffer(&config_2, kRecordHandle | kAllowHandleReuse);

            config_1.handle->AddToEnd(config_2.handle.Retain());

            const PacketBufferHandle popped = config_1.handle.PopHead();

            EXPECT_EQ(config_1.handle, config_2.handle);
            EXPECT_EQ(config_1.handle.GetNext(), nullptr);
            EXPECT_EQ(config_1.handle->tot_len, config_1.init_len);
        }
    }
}

/**
 *  Test PacketBuffer::CompactHead() function.
 *
 *  Description: Take two initial configurations of PacketBuffer from
 *               inContext and create two PacketBuffer instances based on those
 *               configurations. Next, set both buffers' data length to any
 *               combination of values from sLengths[] and link those buffers
 *               into a chain. Then, call CompactHead() on the first buffer in
 *               the chain. After calling the method, verify correctly adjusted
 *               state of the first buffer.
 */
TEST_F_FROM_FIXTURE(TestSystemPacketBuffer, CheckCompactHead)
{
    // Single buffer test.
    for (auto & config : configurations)
    {
        for (auto length : kLengths)
        {
            PrepareTestBuffer(&config, kRecordHandle | kAllowHandleReuse);
            config.handle->SetDataLength(length, config.handle);
            const uint32_t data_length = static_cast<uint32_t>(config.handle->DataLength());

            config.handle->CompactHead();

            EXPECT_EQ(config.handle->payload, (config.start_buffer + kStructureSize));
            EXPECT_EQ(config.handle->tot_len, data_length);
        }

        config.handle = nullptr;
    }

    EXPECT_TRUE(ResetHandles());

    // Chained buffers test.
    for (auto & config_1 : configurations)
    {
        for (auto & config_2 : configurations)
        {
            if (&config_1 == &config_2)
            {
                continue;
            }

            // start with various initial length for the first buffer
            for (auto length_1 : kLengths)
            {
                // start with various initial length for the second buffer
                for (auto length_2 : kLengths)
                {
                    PrepareTestBuffer(&config_1, kRecordHandle | kAllowHandleReuse);
                    EXPECT_EQ(config_1.handle->ref, 2);

                    // CompactHead requires that there be no other references to the chained buffer,
                    // so we manage it manually.
                    PrepareTestBuffer(&config_2);
                    EXPECT_EQ(config_2.handle->ref, 1);
                    PacketBuffer * buffer_2 = std::move(config_2.handle).UnsafeRelease();
                    EXPECT_TRUE(config_2.handle.IsNull());

                    config_1.handle->SetDataLength(length_1, config_1.handle);
                    const uint32_t data_length_1 = static_cast<uint32_t>(config_1.handle->DataLength());

                    // This chain will cause buffer_2 to be freed.
                    config_1.handle->next = buffer_2;

                    // Add various lengths to the second buffer
                    buffer_2->SetDataLength(length_2, config_1.handle);
                    const uint32_t data_length_2 = static_cast<uint32_t>(buffer_2->DataLength());

                    config_1.handle->CompactHead();

                    EXPECT_EQ(config_1.handle->payload, (config_1.start_buffer + kStructureSize));

                    if (config_1.handle->tot_len > config_1.handle->MaxDataLength())
                    {
                        EXPECT_EQ(config_1.handle->len, config_1.handle->MaxDataLength());
                        EXPECT_EQ(buffer_2->len, config_1.handle->tot_len - config_1.handle->MaxDataLength());
                        EXPECT_EQ(config_1.handle.GetNext(), buffer_2);
                        EXPECT_EQ(config_1.handle->ref, 2);
                        EXPECT_EQ(buffer_2->ref, 1);
                    }
                    else
                    {
                        EXPECT_EQ(config_1.handle->len, config_1.handle->tot_len);
                        if (data_length_1 >= config_1.handle->MaxDataLength() && data_length_2 == 0)
                        {
                            /* make sure the second buffer is not freed */
                            EXPECT_EQ(config_1.handle.GetNext(), buffer_2);
                            EXPECT_EQ(buffer_2->ref, 1);
                        }
                        else
                        {
                            /* make sure the second buffer is freed */
                            EXPECT_EQ(config_1.handle.GetNext(), nullptr);
                            buffer_2 = nullptr;
                        }
                    }

                    EXPECT_EQ(config_1.handle->ref, 2);
                    config_1.handle = nullptr;

                    // Verify and release handles.
                    EXPECT_TRUE(ResetHandles());
                }
            }
        }
    }
}

/**
 *  Test PacketBuffer::ConsumeHead() function.
 *
 *  Description: For every buffer-configuration from inContext, create a
 *               buffer's instance according to the configuration. Next,
 *               for any value from sLengths[], pass it to the buffer's
 *               instance through ConsumeHead() method. Then, verify that
 *               the internal state of the buffer has been correctly
 *               adjusted according to the value passed into the method.
 */
TEST_F_FROM_FIXTURE(TestSystemPacketBuffer, CheckConsumeHead)
{
    for (auto & config : configurations)
    {
        for (auto length : kLengths)
        {
            PrepareTestBuffer(&config, kRecordHandle | kAllowHandleReuse);

            config.handle->ConsumeHead(length);

            if (length > config.init_len)
            {
                EXPECT_EQ(config.handle->payload, (config.payload_ptr + config.init_len));
                EXPECT_EQ(config.handle->len, static_cast<size_t>(0));
                EXPECT_EQ(config.handle->tot_len, static_cast<size_t>(0));
            }
            else
            {
                EXPECT_EQ(config.handle->payload, (config.payload_ptr + length));
                EXPECT_EQ(config.handle->len, (config.handle->len - length));
                EXPECT_EQ(config.handle->tot_len, (config.handle->tot_len - length));
            }
        }
    }
}

/**
 *  Test PacketBuffer::Consume() function.
 *
 *  Description: Take two different initial configurations of PacketBuffer from
 *               inContext and create two PacketBuffer instances based on those
 *               configurations. Next, set both buffers' data length to any
 *               combination of values from sLengths[]  and link those buffers
 *               into a chain. Then, call Consume() on the first buffer in
 *               the chain with all values from sLengths[]. After calling the
 *               method, verify correctly adjusted the state of the first
 *               buffer and appropriate return pointer from the method's call.
 */
TEST_F_FROM_FIXTURE(TestSystemPacketBuffer, CheckConsume)
{
    for (auto & config_1 : configurations)
    {
        for (auto & config_2 : configurations)
        {
            if (&config_1 == &config_2)
            {
                continue;
            }

            // consume various amounts of memory
            for (auto consumeLength : kLengths)
            {
                // start with various initial length for the first buffer
                for (auto len_1 : kLengths)
                {
                    // start with various initial length for the second buffer
                    for (auto len_2 : kLengths)
                    {
                        PrepareTestBuffer(&config_1);
                        PrepareTestBuffer(&config_2);
                        EXPECT_EQ(config_1.handle->ref, 1);
                        EXPECT_EQ(config_2.handle->ref, 1);

                        config_1.handle->AddToEnd(config_2.handle.Retain());

                        // Add various lengths to buffers
                        config_1.handle->SetDataLength(len_1, config_1.handle);
                        config_2.handle->SetDataLength(len_2, config_1.handle);

                        const uint32_t buf_1_len = static_cast<uint32_t>(config_1.handle->len);
                        const uint32_t buf_2_len = static_cast<uint32_t>(config_2.handle->len);

                        PacketBufferHandle original_handle_1 = config_1.handle.Retain();
                        EXPECT_EQ(config_1.handle->ref, 2); // config_1.handle and original_handle_1
                        EXPECT_EQ(config_2.handle->ref, 2); // config_2.handle and config_1.handle->next

                        config_1.handle.Consume(consumeLength);

                        if (consumeLength == 0)
                        {
                            EXPECT_EQ(config_1.handle, original_handle_1);
                            EXPECT_EQ(config_1.handle->len, buf_1_len);
                            EXPECT_EQ(config_2.handle->len, buf_2_len);
                            EXPECT_EQ(config_1.handle->ref, 2); // config_1.handle and original_handle_1
                            EXPECT_EQ(config_2.handle->ref, 2); // config_2.handle and config_1.handle->next
                        }
                        else if (consumeLength < buf_1_len)
                        {
                            EXPECT_EQ(config_1.handle, original_handle_1);
                            EXPECT_EQ(config_1.handle->len, buf_1_len - consumeLength);
                            EXPECT_EQ(config_2.handle->len, buf_2_len);
                            EXPECT_EQ(config_1.handle->ref, 2); // config_1.handle and original_handle_1
                            EXPECT_EQ(config_2.handle->ref, 2); // config_2.handle and config_1.handle->next
                        }
                        else if ((consumeLength < buf_1_len + buf_2_len ||
                                  (consumeLength == buf_1_len + buf_2_len && buf_2_len == 0)))
                        {
                            EXPECT_EQ(config_1.handle, config_2.handle);
                            EXPECT_EQ(config_2.handle->len, buf_1_len + buf_2_len - consumeLength);
                            EXPECT_EQ(original_handle_1->ref, 1); // original_handle_1
                            EXPECT_EQ(config_2.handle->ref, 2);   // config_1.handle and config_2.handle
                        }
                        else
                        {
                            EXPECT_TRUE(config_1.handle.IsNull());
                            EXPECT_EQ(original_handle_1->ref, 1); // original_handle_1
                            EXPECT_EQ(config_2.handle->ref, 1);   // config_2.handle
                        }

                        original_handle_1 = nullptr;
                        config_1.handle   = nullptr;
                        config_2.handle   = nullptr;
                    }
                }
            }
        }
    }
}

/**
 *  Test PacketBuffer::EnsureReservedSize() function.
 *
 *  Description: For every buffer-configuration from inContext, create a
 *               buffer's instance according to the configuration. Next,
 *               manually specify how much space is reserved in the buffer.
 *               Then, verify that EnsureReservedSize() method correctly
 *               retrieves the amount of the reserved space.
 */
TEST_F_FROM_FIXTURE(TestSystemPacketBuffer, CheckEnsureReservedSize)
{
    for (auto & config : configurations)
    {
        for (auto length : kLengths)
        {
            PrepareTestBuffer(&config, kRecordHandle | kAllowHandleReuse);
            const uint32_t kAllocSize = static_cast<uint32_t>(config.handle->AllocSize());
            uint16_t reserved_size    = config.reserved_size;

            if (kStructureSize + config.reserved_size > kAllocSize)
            {
                reserved_size = static_cast<uint16_t>(kAllocSize - kStructureSize);
            }

            if (length <= reserved_size)
            {
                EXPECT_EQ(config.handle->EnsureReservedSize(length), true);
                continue;
            }

            if ((length + config.init_len) > (kAllocSize - kStructureSize))
            {
                EXPECT_FALSE(config.handle->EnsureReservedSize(length));
                continue;
            }

            EXPECT_EQ(config.handle->EnsureReservedSize(length), true);
            EXPECT_EQ(config.handle->payload, (config.payload_ptr + length - reserved_size));
        }
    }
}

/**
 *  Test PacketBuffer::AlignPayload() function.
 *
 *  Description: For every buffer-configuration from inContext, create a
 *               buffer's instance according to the configuration. Next,
 *               manually specify how much space is reserved and the
 *               required payload shift. Then, verify that AlignPayload()
 *               method correctly aligns the payload start pointer.
 */
TEST_F(TestSystemPacketBuffer, CheckAlignPayload)
{
    for (auto & config : configurations)
    {
        for (auto length : kLengths)
        {
            PrepareTestBuffer(&config, kRecordHandle | kAllowHandleReuse);
            const uint32_t kAllocSize = static_cast<uint32_t>(config.handle->AllocSize());

            if (length == 0)
            {
                EXPECT_FALSE(config.handle->AlignPayload(length));
                continue;
            }

            uint32_t reserved_size = config.reserved_size;
            if (config.reserved_size > kAllocSize)
            {
                reserved_size = kAllocSize;
            }

            const uint32_t payload_offset = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(config.handle->Start()) % length);
            uint32_t payload_shift        = 0;
            if (payload_offset > 0)
                payload_shift = static_cast<uint32_t>(length - payload_offset);

            if (payload_shift <= kAllocSize - reserved_size)
            {
                EXPECT_EQ(config.handle->AlignPayload(length), true);
                EXPECT_EQ(((unsigned long) config.handle->Start() % length), 0UL);
            }
            else
            {
                EXPECT_FALSE(config.handle->AlignPayload(length));
            }
        }
    }
}

/**
 *  Test PacketBuffer::Next() function.
 */
TEST_F_FROM_FIXTURE(TestSystemPacketBuffer, CheckNext)
{
    for (auto & config_1 : configurations)
    {
        for (auto & config_2 : configurations)
        {
            PrepareTestBuffer(&config_1, kRecordHandle | kAllowHandleReuse);
            PrepareTestBuffer(&config_2, kRecordHandle | kAllowHandleReuse);

            if (&config_1 != &config_2)
            {
                EXPECT_TRUE(config_1.handle->Next().IsNull());

                config_1.handle->AddToEnd(config_2.handle.Retain());

                EXPECT_EQ(config_1.handle->Next(), config_2.handle);
                EXPECT_EQ(config_1.handle->ChainedBuffer(), config_2.handle.Get());
            }
            else
            {
                EXPECT_FALSE(config_1.handle->HasChainedBuffer());
            }

            EXPECT_FALSE(config_2.handle->HasChainedBuffer());
        }
    }
}

/**
 *  Test PacketBuffer::Last() function.
 */
TEST_F_FROM_FIXTURE(TestSystemPacketBuffer, CheckLast)
{
    for (auto & config_1 : configurations)
    {
        for (auto & config_2 : configurations)
        {
            for (auto & config_3 : configurations)
            {
                if (&config_1 == &config_2 || &config_1 == &config_3 || &config_2 == &config_3)
                {
                    continue;
                }

                PrepareTestBuffer(&config_1);
                PrepareTestBuffer(&config_2);
                PrepareTestBuffer(&config_3);

                EXPECT_EQ(config_1.handle->Last(), config_1.handle);
                EXPECT_EQ(config_2.handle->Last(), config_2.handle);
                EXPECT_EQ(config_3.handle->Last(), config_3.handle);

                config_1.handle->AddToEnd(config_2.handle.Retain());

                EXPECT_EQ(config_1.handle->Last(), config_2.handle);
                EXPECT_EQ(config_2.handle->Last(), config_2.handle);
                EXPECT_EQ(config_3.handle->Last(), config_3.handle);

                config_1.handle->AddToEnd(config_3.handle.Retain());

                EXPECT_EQ(config_1.handle->Last(), config_3.handle);
                EXPECT_EQ(config_2.handle->Last(), config_3.handle);
                EXPECT_EQ(config_3.handle->Last(), config_3.handle);

                config_1.handle = nullptr;
                config_2.handle = nullptr;
                config_3.handle = nullptr;
            }
        }
    }
}

/**
 *  Test PacketBuffer::Read() function.
 */
TEST_F_FROM_FIXTURE(TestSystemPacketBuffer, CheckRead)
{
    uint8_t payloads[2 * kBlockSize] = { 1 };
    uint8_t result[2 * kBlockSize];
    for (size_t i = 1; i < sizeof(payloads); ++i)
    {
        payloads[i] = static_cast<uint8_t>(random());
    }

    for (auto & config_1 : configurations)
    {
        for (auto & config_2 : configurations)
        {
            if (&config_1 == &config_2)
            {
                continue;
            }

            PrepareTestBuffer(&config_1, kAllowHandleReuse);
            PrepareTestBuffer(&config_2, kAllowHandleReuse);

            const size_t length_1       = config_1.handle->MaxDataLength();
            const size_t length_2       = config_2.handle->MaxDataLength();
            const size_t length_sum     = length_1 + length_2;
            const uint32_t length_total = static_cast<uint32_t>(length_sum);
            EXPECT_EQ(length_total, length_sum);

            memcpy(config_1.handle->Start(), payloads, length_1);
            memcpy(config_2.handle->Start(), payloads + length_1, length_2);
            config_1.handle->SetDataLength(length_1);
            config_2.handle->SetDataLength(length_2);
            config_1.handle->AddToEnd(config_2.handle.Retain());
            EXPECT_EQ(config_1.handle->TotalLength(), length_total);

            if (length_1 >= 1)
            {
                // Check a read that does not span packet buffers.
                CHIP_ERROR err = config_1.handle->Read(result, 1);
                EXPECT_EQ(err, CHIP_NO_ERROR);
                EXPECT_EQ(result[0], payloads[0]);
            }

            // Check a read that spans packet buffers.
            CHIP_ERROR err = config_1.handle->Read(result, length_total);
            EXPECT_EQ(err, CHIP_NO_ERROR);
            EXPECT_EQ(memcmp(payloads, result, length_total), 0);

            // Check a read that is too long fails.
            err = config_1.handle->Read(result, length_total + 1);
            EXPECT_EQ(err, CHIP_ERROR_BUFFER_TOO_SMALL);

            // Check that running off the end of a corrupt buffer chain is detected.
            if (length_total < UINT32_MAX)
            {
                // First case: TotalLength() is wrong.
                config_1.handle->tot_len = static_cast<uint16_t>(config_1.handle->tot_len + 1);
                err                      = config_1.handle->Read(result, length_total + 1);
                EXPECT_EQ(err, CHIP_ERROR_INTERNAL);
                config_1.handle->tot_len = static_cast<uint16_t>(config_1.handle->tot_len - 1);
            }
            if (length_1 >= 1)
            {
                // Second case: an individual buffer's DataLength() is wrong.
                config_1.handle->len = static_cast<uint16_t>(config_1.handle->len - 1);
                err                  = config_1.handle->Read(result, length_total);
                EXPECT_EQ(err, CHIP_ERROR_INTERNAL);
                config_1.handle->len = static_cast<uint16_t>(config_1.handle->len + 1);
            }

            config_1.handle = nullptr;
            config_2.handle = nullptr;
        }
    }
}

/**
 *  Test PacketBuffer::AddRef() function.
 */
TEST_F_FROM_FIXTURE(TestSystemPacketBuffer, CheckAddRef)
{
    for (auto & config : configurations)
    {
        PrepareTestBuffer(&config, kRecordHandle);
        const auto refs = config.handle->ref;
        config.handle->AddRef();
        EXPECT_EQ(config.handle->ref, refs + 1);
        config.handle->ref = refs; // Don't leak buffers.
    }
}

/**
 *  Test PacketBuffer::Free() function.
 *
 *  Description: Take two different initial configurations of PacketBuffer from
 *               inContext and create two PacketBuffer instances based on those
 *               configurations. Next, chain two buffers together and set each
 *               buffer's reference count to one of the values from
 *               init_ret_count[]. Then, call Free() on the first buffer in
 *               the chain and verify correctly adjusted states of the two
 *               buffers.
 */
TEST_F_FROM_FIXTURE(TestSystemPacketBuffer, CheckFree)
{
    const decltype(PacketBuffer::ref) init_ref_count[] = { 1, 2, 3 };
    constexpr size_t kRefs                             = sizeof(init_ref_count) / sizeof(init_ref_count[0]);

    for (auto & config_1 : configurations)
    {
        for (auto & config_2 : configurations)
        {
            if (&config_1 == &config_2)
            {
                continue;
            }

            // start with various buffer ref counts
            for (size_t r = 0; r < kRefs; r++)
            {
                config_1.handle = PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSizeWithoutReserve, 0);
                config_2.handle = PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSizeWithoutReserve, 0);
                ASSERT_FALSE(config_1.handle.IsNull());
                ASSERT_FALSE(config_2.handle.IsNull());

                PrepareTestBuffer(&config_1, kAllowHandleReuse);
                PrepareTestBuffer(&config_2, kAllowHandleReuse);
                EXPECT_EQ(config_1.handle->ref, 1);
                EXPECT_EQ(config_2.handle->ref, 1);

                // Chain buffers.
                config_1.handle->next = config_2.handle.Get();

                // Add various buffer ref counts.
                const auto initial_refs_1 = config_1.handle->ref = init_ref_count[r];
                const auto initial_refs_2 = config_2.handle->ref = init_ref_count[(r + 1) % kRefs];

                // Free head.
                PacketBuffer::Free(config_1.handle.mBuffer);
                if (initial_refs_1 == 1)
                {
                    config_1.handle.mBuffer = nullptr;
                }

                // Verification.
                if (initial_refs_1 > 1)
                {
                    // Verify that head ref count is decremented.
                    EXPECT_EQ(config_1.handle->ref, initial_refs_1 - 1);
                    // Verify that chain is maintained.
                    EXPECT_EQ(config_1.handle.GetNext(), config_2.handle.Get());
                    // Verify that chained buffer ref count has not changed.
                    EXPECT_EQ(config_2.handle->ref, initial_refs_2);
                }
                else
                {
                    if (initial_refs_2 > 1)
                    {
                        // Verify that chained buffer ref count is decremented.
                        EXPECT_EQ(config_2.handle->ref, initial_refs_2 - 1);
                    }
                    else
                    {
                        // Since the test used fake ref counts, config_2.handle now points
                        // to a freed buffer; clear the handle's internal pointer.
                        config_2.handle.mBuffer = nullptr;
                    }
                }

                // Clean up.
                if (!config_1.handle.IsNull())
                {
                    config_1.handle->next = nullptr;
                    config_1.handle->ref  = 1;
                    config_1.handle       = nullptr;
                }
                if (!config_2.handle.IsNull())
                {
                    config_2.handle->ref = 1;
                    config_2.handle      = nullptr;
                }
            }
        }
    }
}

/**
 *  Test PacketBuffer::FreeHead() function.
 *
 *  Description: Take two different initial configurations of PacketBuffer from
 *               inContext and create two PacketBuffer instances based on those
 *               configurations. Next, chain two buffers together. Then, call
 *               FreeHead() on the first buffer in the chain and verify that
 *               the method returned pointer to the second buffer.
 */
TEST_F_FROM_FIXTURE(TestSystemPacketBuffer, CheckFreeHead)
{
    for (auto & config_1 : configurations)
    {
        for (auto & config_2 : configurations)
        {
            if (&config_1 == &config_2)
            {
                continue;
            }

            // Test PacketBuffer::FreeHead

            PrepareTestBuffer(&config_1, kAllowHandleReuse);
            PrepareTestBuffer(&config_2, kAllowHandleReuse);
            EXPECT_EQ(config_1.handle->ref, 1);
            EXPECT_EQ(config_2.handle->ref, 1);

            PacketBufferHandle handle_1 = config_1.handle.Retain();
            config_1.handle->AddToEnd(config_2.handle.Retain());
            EXPECT_EQ(config_1.handle->ref, 2);
            EXPECT_EQ(config_2.handle->ref, 2); // config_2.handle and config_1.handle->next

            PacketBuffer * const returned = PacketBuffer::FreeHead(std::move(config_1.handle).UnsafeRelease());

            EXPECT_EQ(handle_1->ref, 1);
            EXPECT_EQ(config_2.handle->ref, 2); // config_2.handle and returned
            EXPECT_EQ(returned, config_2.handle.Get());

            config_1.handle = nullptr;
            EXPECT_EQ(config_2.handle->ref, 2);
            config_2.handle = nullptr;
            EXPECT_EQ(returned->ref, 1);
            PacketBuffer::Free(returned);

            // Test PacketBufferHandle::FreeHead

            PrepareTestBuffer(&config_1, kAllowHandleReuse);
            PrepareTestBuffer(&config_2, kAllowHandleReuse);
            EXPECT_EQ(config_1.handle->ref, 1);
            EXPECT_EQ(config_2.handle->ref, 1);

            handle_1 = config_1.handle.Retain();
            config_1.handle->AddToEnd(config_2.handle.Retain());
            EXPECT_EQ(config_1.handle->ref, 2);
            EXPECT_EQ(config_2.handle->ref, 2); // config_2.handle and config_1.handle->next

            PacketBuffer * const buffer_1 = config_1.handle.Get();

            config_1.handle.FreeHead();

            EXPECT_EQ(buffer_1->ref, 1);
            EXPECT_EQ(config_1.handle, config_2.handle);
            EXPECT_EQ(config_2.handle->ref, 2); // config_2.handle and config_1.handle

            config_1.handle = nullptr;
            config_2.handle = nullptr;
        }
    }
}

TEST_F_FROM_FIXTURE(TestSystemPacketBuffer, CheckHandleConstruct)
{
    PacketBufferHandle handle_1;
    EXPECT_TRUE(handle_1.IsNull());

    PacketBufferHandle handle_2(nullptr);
    EXPECT_TRUE(handle_2.IsNull());

    PacketBufferHandle handle_3(PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    ASSERT_FALSE(handle_3.IsNull());

    // Private constructor.
    PacketBuffer * const buffer_3 = std::move(handle_3).UnsafeRelease();
    PacketBufferHandle handle_4(buffer_3);
    EXPECT_EQ(handle_4.Get(), buffer_3);
}

TEST_F_FROM_FIXTURE(TestSystemPacketBuffer, CheckHandleMove)
{
    for (auto & config_1 : configurations)
    {
        for (auto & config_2 : configurations)
        {
            if (&config_1 == &config_2)
            {
                continue;
            }

            PrepareTestBuffer(&config_1, kRecordHandle);
            PrepareTestBuffer(&config_2, kRecordHandle);

            const PacketBuffer * const buffer_1 = config_1.handle.Get();
            const PacketBuffer * const buffer_2 = config_2.handle.Get();
            EXPECT_NE(buffer_1, buffer_2);
            EXPECT_EQ(buffer_1->ref, 2); // test.handles and config_1.handle
            EXPECT_EQ(buffer_2->ref, 2); // test.handles and config_2.handle

            config_1.handle = std::move(config_2.handle);
            EXPECT_EQ(config_1.handle.Get(), buffer_2);
            EXPECT_EQ(config_2.handle.Get(), nullptr);
            EXPECT_EQ(buffer_1->ref, 1); // test.handles
            EXPECT_EQ(buffer_2->ref, 2); // test.handles and config_1.handle

            config_1.handle = nullptr;
        }
        // Verify and release handles.
        EXPECT_TRUE(ResetHandles());
    }
}

TEST_F_FROM_FIXTURE(TestSystemPacketBuffer, CheckHandleRelease)
{
    for (auto & config_1 : configurations)
    {
        PrepareTestBuffer(&config_1);

        PacketBuffer * const buffer_1 = config_1.handle.Get();
        PacketBuffer * const taken_1  = std::move(config_1.handle).UnsafeRelease();

        EXPECT_EQ(buffer_1, taken_1);
        EXPECT_TRUE(config_1.handle.IsNull());
        EXPECT_EQ(buffer_1->ref, 1);
        PacketBuffer::Free(buffer_1);
    }
}

TEST_F_FROM_FIXTURE(TestSystemPacketBuffer, CheckHandleFree)
{
    for (auto & config_1 : configurations)
    {
        PrepareTestBuffer(&config_1, kRecordHandle);

        const PacketBuffer * const buffer_1 = config_1.handle.Get();
        EXPECT_EQ(buffer_1->ref, 2); // test.handles and config_1.handle

        config_1.handle = nullptr;
        EXPECT_TRUE(config_1.handle.IsNull());
        EXPECT_EQ(config_1.handle.Get(), nullptr);
        EXPECT_EQ(buffer_1->ref, 1); // test.handles only
    }
}

TEST_F_FROM_FIXTURE(TestSystemPacketBuffer, CheckHandleRetain)
{
    for (auto & config_1 : configurations)
    {
        PrepareTestBuffer(&config_1, kRecordHandle);

        EXPECT_EQ(config_1.handle->ref, 2); // test.handles and config_1.handle

        PacketBufferHandle handle_1 = config_1.handle.Retain();

        EXPECT_EQ(config_1.handle, handle_1);
        EXPECT_EQ(config_1.handle->ref, 3); // test.handles and config_1.handle and handle_1
    }
}

TEST_F_FROM_FIXTURE(TestSystemPacketBuffer, CheckHandleAdopt)
{
    for (auto & config_1 : configurations)
    {
        PrepareTestBuffer(&config_1, kRecordHandle);
        PacketBuffer * buffer_1 = std::move(config_1.handle).UnsafeRelease();

        EXPECT_TRUE(config_1.handle.IsNull());
        EXPECT_EQ(buffer_1->ref, 2); // test.handles and buffer_1

        config_1.handle = PacketBufferHandle::Adopt(buffer_1);
        EXPECT_EQ(config_1.handle.Get(), buffer_1);
        EXPECT_EQ(config_1.handle->ref, 2); // test.handles and config_1.handle

        config_1.handle = nullptr;
        EXPECT_TRUE(config_1.handle.IsNull());
        EXPECT_EQ(buffer_1->ref, 1); // test.handles only
    }
}

TEST_F_FROM_FIXTURE(TestSystemPacketBuffer, CheckHandleHold)
{
    for (auto & config_1 : configurations)
    {
        PrepareTestBuffer(&config_1, kRecordHandle);
        PacketBuffer * buffer_1 = std::move(config_1.handle).UnsafeRelease();

        EXPECT_TRUE(config_1.handle.IsNull());
        EXPECT_EQ(buffer_1->ref, 2); // test.handles and buffer_1

        config_1.handle = PacketBufferHandle::Hold(buffer_1);
        EXPECT_EQ(config_1.handle.Get(), buffer_1);
        EXPECT_EQ(config_1.handle->ref, 3); // test.handles and config_1.handle and buffer_1

        config_1.handle = nullptr;
        EXPECT_TRUE(config_1.handle.IsNull());
        EXPECT_EQ(buffer_1->ref, 2); // test.handles only and buffer_1

        PacketBuffer::Free(buffer_1);
    }
}

TEST_F_FROM_FIXTURE(TestSystemPacketBuffer, CheckHandleAdvance)
{
    for (auto & config_1 : configurations)
    {
        for (auto & config_2 : configurations)
        {
            for (auto & config_3 : configurations)
            {
                if (&config_1 == &config_2 || &config_1 == &config_3 || &config_2 == &config_3)
                {
                    continue;
                }

                PrepareTestBuffer(&config_1);
                PrepareTestBuffer(&config_2);
                PrepareTestBuffer(&config_3);

                PacketBufferHandle handle_1 = config_1.handle.Retain();
                PacketBufferHandle handle_2 = config_2.handle.Retain();
                PacketBufferHandle handle_3 = config_3.handle.Retain();

                config_1.handle->AddToEnd(config_2.handle.Retain());
                config_1.handle->AddToEnd(config_3.handle.Retain());

                EXPECT_EQ(config_1.handle->ChainedBuffer(), config_2.handle.Get());
                EXPECT_EQ(config_2.handle->ChainedBuffer(), config_3.handle.Get());
                EXPECT_FALSE(config_3.handle->HasChainedBuffer());
                EXPECT_EQ(handle_1->ref, 2); // handle_1 and config_1.handle
                EXPECT_EQ(handle_2->ref, 3); // handle_2 and config_2.handle and config_1.handle->next
                EXPECT_EQ(handle_3->ref, 3); // handle_3 and config_3.handle and config_2.handle->next

                config_1.handle.Advance();

                EXPECT_EQ(config_1.handle, handle_2);
                EXPECT_EQ(handle_1->ref, 1); // handle_1 only
                EXPECT_EQ(handle_2->ref, 4); // handle_2, config_[12].handle, handle_1->next
                EXPECT_EQ(handle_3->ref, 3); // handle_3, config_3.handle, config_2.handle->next

                config_1.handle.Advance();

                EXPECT_EQ(config_1.handle, handle_3);
                EXPECT_EQ(handle_1->ref, 1); // handle_1 only
                EXPECT_EQ(handle_2->ref, 3); // handle_2, config_2.handle, handle_1->next
                EXPECT_EQ(handle_3->ref, 4); // handle_3, config_[13].handle, handle_2->next

                config_1.handle = nullptr;
                config_2.handle = nullptr;
                config_3.handle = nullptr;
            }
        }
    }
}

TEST_F_FROM_FIXTURE(TestSystemPacketBuffer, CheckHandleRightSize)
{
    static const char kPayload[] = "Joy!";
    PacketBufferHandle handle    = PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSizeWithoutReserve, 0);
    PacketBuffer * buffer        = handle.mBuffer;

    memcpy(handle->Start(), kPayload, sizeof kPayload);
    buffer->SetDataLength(sizeof kPayload);
    EXPECT_EQ(handle->ref, 1);

    // RightSize should do nothing if there is another reference to the buffer.
    {
        PacketBufferHandle anotherHandle = handle.Retain();
        handle.RightSize();
        EXPECT_EQ(handle.mBuffer, buffer);
    }

#if CHIP_SYSTEM_PACKETBUFFER_HAS_RIGHTSIZE

    handle.RightSize();
    EXPECT_NE(handle.mBuffer, buffer);
    EXPECT_EQ(handle->DataLength(), sizeof kPayload);
    EXPECT_EQ(memcmp(handle->Start(), kPayload, sizeof kPayload), 0);

#else // CHIP_SYSTEM_PACKETBUFFER_HAS_RIGHTSIZE

    // For this configuration, RightSize() does nothing.
    handle.RightSize();
    EXPECT_EQ(handle.mBuffer, buffer);

#endif // CHIP_SYSTEM_PACKETBUFFER_HAS_RIGHTSIZE
}

TEST_F_FROM_FIXTURE(TestSystemPacketBuffer, CheckHandleCloneData)
{
    uint8_t lPayload[2 * PacketBuffer::kMaxAllocSize];

    for (uint8_t & payload : lPayload)
    {
        payload = static_cast<uint8_t>(random());
    }

    for (auto & config_1 : configurations)
    {
        for (auto & config_2 : configurations)
        {
            if (&config_1 == &config_2)
            {
                continue;
            }

            PrepareTestBuffer(&config_1);
            PrepareTestBuffer(&config_2);

            const uint8_t * payload_1 = lPayload;
            memcpy(config_1.handle->Start(), payload_1, config_1.handle->MaxDataLength());
            config_1.handle->SetDataLength(config_1.handle->MaxDataLength());

            const uint8_t * payload_2 = lPayload + config_1.handle->MaxDataLength();
            memcpy(config_2.handle->Start(), payload_2, config_2.handle->MaxDataLength());
            config_2.handle->SetDataLength(config_2.handle->MaxDataLength());

            // Clone single buffer.
            PacketBufferHandle clone_1 = config_1.handle.CloneData();
            ASSERT_FALSE(clone_1.IsNull());
            EXPECT_EQ(clone_1->DataLength(), config_1.handle->DataLength());
            EXPECT_EQ(memcmp(clone_1->Start(), payload_1, clone_1->DataLength()), 0);
            if (clone_1->DataLength())
            {
                // Verify that modifying the clone does not affect the original.
                ScrambleData(clone_1->Start(), clone_1->DataLength());
                EXPECT_NE(memcmp(clone_1->Start(), payload_1, clone_1->DataLength()), 0);
                EXPECT_EQ(memcmp(config_1.handle->Start(), payload_1, config_1.handle->DataLength()), 0);
            }

            // Clone buffer chain.
            config_1.handle->AddToEnd(config_2.handle.Retain());
            EXPECT_TRUE(config_1.handle->HasChainedBuffer());
            clone_1                         = config_1.handle.CloneData();
            PacketBufferHandle clone_1_next = clone_1->Next();
            ASSERT_FALSE(clone_1.IsNull());
            EXPECT_TRUE(clone_1->HasChainedBuffer());
            EXPECT_EQ(clone_1->DataLength(), config_1.handle->DataLength());
            EXPECT_EQ(clone_1->TotalLength(), config_1.handle->TotalLength());
            EXPECT_EQ(clone_1_next->DataLength(), config_2.handle->DataLength());
            EXPECT_EQ(memcmp(clone_1->Start(), payload_1, clone_1->DataLength()), 0);
            EXPECT_EQ(memcmp(clone_1_next->Start(), payload_2, clone_1_next->DataLength()), 0);
            if (clone_1->DataLength())
            {
                ScrambleData(clone_1->Start(), clone_1->DataLength());
                EXPECT_NE(memcmp(clone_1->Start(), payload_1, clone_1->DataLength()), 0);
                EXPECT_EQ(memcmp(config_1.handle->Start(), payload_1, config_1.handle->DataLength()), 0);
            }
            if (clone_1_next->DataLength())
            {
                ScrambleData(clone_1_next->Start(), clone_1_next->DataLength());
                EXPECT_NE(memcmp(clone_1_next->Start(), payload_2, clone_1_next->DataLength()), 0);
                EXPECT_EQ(memcmp(config_2.handle->Start(), payload_2, config_2.handle->DataLength()), 0);
            }

            config_1.handle = nullptr;
            config_2.handle = nullptr;
        }
    }

#if CHIP_SYSTEM_PACKETBUFFER_FROM_CHIP_HEAP

    // It is possible for a packet buffer allocation to return a larger block than requested (e.g. when using a shared pool)
    // and in particular to return a larger block than it is possible to request from PackBufferHandle::New().
    // In that case, (a) it is incorrect to actually use the extra space, and (b) if it is not used, the clone will
    // be the maximum possible size.
    //
    // This is only testable on heap allocation configurations, where pbuf records the allocation size and we can manually
    // construct an oversize buffer.

    constexpr size_t kOversizeDataSize = PacketBuffer::kMaxAllocSize + 99;
    PacketBuffer * p = reinterpret_cast<PacketBuffer *>(chip::Platform::MemoryAlloc(kStructureSize + kOversizeDataSize));
    ASSERT_NE(p, nullptr);

    p->next       = nullptr;
    p->payload    = reinterpret_cast<uint8_t *>(p) + kStructureSize;
    p->tot_len    = 0;
    p->len        = 0;
    p->ref        = 1;
    p->alloc_size = kOversizeDataSize;

    PacketBufferHandle handle = PacketBufferHandle::Adopt(p);

    // Fill the buffer to maximum and verify that it can be cloned.
    size_t maxSize = PacketBuffer::kMaxAllocSize;

    memset(handle->Start(), 1, maxSize);
    handle->SetDataLength(maxSize);
    EXPECT_EQ(handle->DataLength(), maxSize);

    PacketBufferHandle clone = handle.CloneData();
    ASSERT_FALSE(clone.IsNull());
    EXPECT_EQ(clone->DataLength(), maxSize);
    EXPECT_EQ(memcmp(handle->Start(), clone->Start(), maxSize), 0);

    // Overfill the buffer and verify that it can not be cloned.
    memset(handle->Start(), 2, kOversizeDataSize);
    handle->SetDataLength(kOversizeDataSize);
    EXPECT_EQ(handle->DataLength(), kOversizeDataSize);

    clone = handle.CloneData();
    EXPECT_TRUE(clone.IsNull());

    // Free the packet buffer memory ourselves, since we allocated it ourselves.
    chip::Platform::MemoryFree(std::move(handle).UnsafeRelease());

#endif // CHIP_SYSTEM_PACKETBUFFER_FROM_CHIP_HEAP
}

TEST_F(TestSystemPacketBuffer, CheckPacketBufferWriter)
{
    static const char kPayload[] = "Hello, world!";

    PacketBufferWriter yay(PacketBufferHandle::New(sizeof(kPayload)));
    PacketBufferWriter nay(PacketBufferHandle::New(sizeof(kPayload)), sizeof(kPayload) - 2);
    ASSERT_FALSE(yay.IsNull());
    ASSERT_FALSE(nay.IsNull());

    yay.Put(kPayload);
    yay.Put('\0');
    nay.Put(kPayload);
    nay.Put('\0');
    EXPECT_TRUE(yay.Fit());
    EXPECT_FALSE(nay.Fit());

    PacketBufferHandle yayBuffer = yay.Finalize();
    PacketBufferHandle nayBuffer = nay.Finalize();
    EXPECT_TRUE(yay.IsNull());
    EXPECT_TRUE(nay.IsNull());
    ASSERT_FALSE(yayBuffer.IsNull());
    EXPECT_TRUE(nayBuffer.IsNull());
    EXPECT_EQ(memcmp(yayBuffer->Start(), kPayload, sizeof kPayload), 0);
}

} // namespace System
} // namespace chip
