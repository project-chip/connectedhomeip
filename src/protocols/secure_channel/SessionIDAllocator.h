/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <core/CHIPError.h>

namespace chip {

class SessionIDAllocator
{
public:
    SessionIDAllocator() {}
    ~SessionIDAllocator() {}

    CHIP_ERROR Allocate(uint16_t & id);
    void Free(uint16_t id);
    CHIP_ERROR Reserve(uint16_t id);
    CHIP_ERROR ReserveUpTo(uint16_t id);
    uint16_t Peek();

private:
    // Session ID is a 15 bit value (16th bit indicates unicast/group key)
    static constexpr uint16_t kMaxSessionID = (1 << 15) - 1;
    uint16_t mNextAvailable                 = 0;
};

} // namespace chip
