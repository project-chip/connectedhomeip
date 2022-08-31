/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#pragma once

#include <lib/dnssd/minimal_mdns/core/QName.h>
#include <lib/support/CodeUtils.h>

namespace testing {

/// Convenience class for testing.
///
/// Allows defining of constants for tests for FullQName and SerializedQNameIterator.
/// Usage:
///
/// const auto kTestHostName = testing::TestQName<2>({ "abcd", "local" });
///
/// kTestHostName.Serialized(); // SerializedQNameIterator: abcd.local
/// kTestHostName.Full();       // FullQName: abcd.local
template <size_t N>
class TestQName
{
public:
    TestQName(const std::array<const char *, N> & data)
    {
        size_t neededSize = 1; // for end terminator
        for (size_t i = 0; i < N; i++)
        {
            VerifyOrDie(strlen(data[i]) <= UINT8_MAX);
            neededSize += strlen(data[i]) + 1; // lenght + data
            mStrings[i] = strdup(data[i]);
        }

        mSerializedQNameBuffer = new (std::nothrow) uint8_t[neededSize];
        VerifyOrDie(mSerializedQNameBuffer != nullptr);

        chip::Encoding::BigEndian::BufferWriter writer(mSerializedQNameBuffer, neededSize);
        for (size_t i = 0; i < N; i++)
        {
            writer.Put(static_cast<uint8_t>(strlen(data[i])));
            writer.Put(data[i]);
        }
        writer.Put(static_cast<uint8_t>(0));
        VerifyOrDie(writer.Fit());

        mSerializedBufferSize = neededSize;
    }

    ~TestQName()
    {
        delete[] mSerializedQNameBuffer;
        for (size_t i = 0; i < N; i++)
        {
            free(mStrings[i]);
        }
    }

    mdns::Minimal::SerializedQNameIterator Serialized() const
    {
        return mdns::Minimal::SerializedQNameIterator(
            mdns::Minimal::BytesRange(mSerializedQNameBuffer, mSerializedQNameBuffer + mSerializedBufferSize),
            mSerializedQNameBuffer);
    }

    mdns::Minimal::FullQName Full() const
    {
        mdns::Minimal::FullQName result;

        result.names     = mStrings;
        result.nameCount = N;

        return result;
    }

private:
    char * mStrings[N];
    uint8_t * mSerializedQNameBuffer;
    size_t mSerializedBufferSize;
};

} // namespace testing
