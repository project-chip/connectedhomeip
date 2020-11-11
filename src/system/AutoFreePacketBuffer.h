/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "SystemPacketBuffer.h"

namespace chip {
namespace System {

/// Ensures that a System::PacketBuffer is freed when it goes out of scope.
class AutoFreePacketBuffer
{
public:
    AutoFreePacketBuffer() : mBuffer(nullptr) {}
    explicit AutoFreePacketBuffer(PacketBuffer * p) : mBuffer(p) {}

    AutoFreePacketBuffer(const AutoFreePacketBuffer &) = delete;
    AutoFreePacketBuffer & operator=(const AutoFreePacketBuffer &) = delete;

    ~AutoFreePacketBuffer() { Adopt(nullptr); }

    inline PacketBuffer * operator->() { return mBuffer; }
    inline PacketBuffer & operator*() { return *mBuffer; }

    inline const PacketBuffer * operator->() const { return mBuffer; }
    inline const PacketBuffer & operator*() const { return *mBuffer; }

    inline void Adopt(PacketBuffer * v)
    {
        if (mBuffer != nullptr)
        {
            PacketBuffer::Free(mBuffer);
        }
        mBuffer = v;
    }

    inline PacketBuffer * Release()
    {
        PacketBuffer * r = mBuffer;
        mBuffer          = nullptr;
        return r;
    }

    inline PacketBuffer * Get_NoRelease() { return mBuffer; }

    inline bool IsNull() const { return mBuffer == nullptr; }

private:
    PacketBuffer * mBuffer;
};

} // namespace System
} // namespace chip
