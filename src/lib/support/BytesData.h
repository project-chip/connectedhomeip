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

#include <cstdint>
#include <cstdlib>

namespace chip {

/**
 * @brief A wrapper class for holding bytes buffer and its length, without the ownership of buffer.
 */
class BytesData
{
public:
    BytesData() : mDataBuf(nullptr), mDataLen(0) {}
    BytesData(const uint8_t * databuf, size_t datalen) : mDataBuf(databuf), mDataLen(datalen) {}

    const uint8_t * Data() const { return mDataBuf; }
    size_t Length() const { return mDataLen; }

private:
    const uint8_t * mDataBuf;
    size_t mDataLen;
};

} // namespace chip
