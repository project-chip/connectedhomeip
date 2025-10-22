/*
 *    Copyright (c) 2024 Project CHIP Authors
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
#pragma once

#include <lib/core/CHIPEncoding.h>
#include <lib/core/CHIPError.h>
#include <stddef.h>
#include <stdint.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {
namespace Encoding {

/*
    Generic buffer used to hold incoming and outgoing data.
    The "in" pointer marks the next address to be written.
    The "out" pointer marks the next address to be read.
    When "out" reaches "in", all incoming data has been read.
    "Size" is the total amount of bytes written, including the part already read.
    "Left" is the number of bytes available for reading.
    "Offset" is the number of bytes read.
    "Spare" is the number of bytes available for writing.
    "Limit" it the total number of bytes allocated (size + spare).
begin            out             in               end
  |---------------v---------------v----------------|
  |.....offset....|......left.....|.....spare......|
  |..............size.............|
  |......................limit.....................|
*/

struct Buffer
{
    Buffer(uint8_t * ptr, size_t size, bool at_end = false) { Init(ptr, size, at_end); }

    void Init(uint8_t * ptr, size_t size, bool at_end = false)
    {
        Finish();
        if (nullptr == ptr)
        {
            ptr       = new uint8_t[size];
            allocated = true;
        }
        this->begin = ptr;
        this->end   = ptr + size;
        this->in    = at_end ? end : begin;
        this->out   = ptr;
    }
    void Finish()
    {
        if (this->begin && allocated)
        {
            delete[] this->begin;
        }
        this->begin = this->end = this->in = this->out = nullptr;
    }

    /** Reset the pointers to initial position. Zero write, zero read. */
    void Clear() { this->in = this->out = this->begin; }
    /** @return Total size allocated for the buffer. */
    size_t Limit() { return (this->end > this->begin) ? (this->end - this->begin) : 0; }
    /** @return Number of bytes written. */
    size_t Size() { return (this->in > this->begin) ? (this->in - this->begin) : 0; }
    /** @return Number of bytes read. */
    size_t Offset() { return (this->out > this->begin) ? (this->out - this->begin) : 0; }
    /** @return Number of bytes available for reading. */
    size_t Left() { return this->Size() - this->Offset(); }
    /** @return Number of bytes available for writing. */
    size_t Spare() { return this->Limit() - this->Size(); }

    CHIP_ERROR Add(uint8_t in);
    CHIP_ERROR Add(uint16_t in);
    CHIP_ERROR Add(uint32_t in);
    CHIP_ERROR Add(int32_t in);
    CHIP_ERROR Add(const uint8_t * in, size_t size);
    CHIP_ERROR Add(Buffer & from, size_t size);
    CHIP_ERROR Get(uint8_t & out);
    CHIP_ERROR Get(uint16_t & out);
    CHIP_ERROR Get(uint32_t & out);
    CHIP_ERROR Get(uint8_t * out, size_t size);
    CHIP_ERROR Get(Buffer & into, size_t size);

    uint8_t * begin = nullptr;
    uint8_t * end   = nullptr;
    uint8_t * in    = nullptr;
    uint8_t * out   = nullptr;
    bool allocated  = false;
};

//------------------------------------------------------------------------------
// Version 1
//------------------------------------------------------------------------------
#ifdef SILABS_PROVISION_PROTOCOL_V1

namespace Version1 {

enum Types : uint8_t
{
    Type_None   = 0x00,
    Type_Int8u  = 0x01,
    Type_Int16u = 0x02,
    Type_Int32u = 0x03,
    Type_Array  = 0x04,
};

CHIP_ERROR Encode(Buffer & arg, uint8_t in);
CHIP_ERROR Encode(Buffer & arg, uint16_t in);
CHIP_ERROR Encode(Buffer & arg, int32_t in);
CHIP_ERROR Encode(Buffer & arg, uint32_t in);
CHIP_ERROR Encode(Buffer & arg, const uint8_t * in, size_t size);
CHIP_ERROR Encode(Buffer & arg, const char * in);

CHIP_ERROR Decode(Buffer & arg, uint8_t & out);
CHIP_ERROR Decode(Buffer & arg, uint16_t & out);
CHIP_ERROR Decode(Buffer & arg, uint32_t & out);
CHIP_ERROR Decode(Buffer & arg, uint8_t * out, size_t limit, size_t & size);
CHIP_ERROR Decode(Buffer & arg, char * out, size_t limit, size_t & size);

namespace Legacy {
CHIP_ERROR DecodeInt8u(Encoding::Buffer & in, uint8_t & out);
CHIP_ERROR DecodeInt16u(Encoding::Buffer & in, uint16_t & out);
CHIP_ERROR DecodeInt32u(Encoding::Buffer & in, uint32_t & out);
} // namespace Legacy

} // namespace Version1

#endif // SILABS_PROVISION_PROTOCOL_V1

//------------------------------------------------------------------------------
// Version 2
//------------------------------------------------------------------------------

namespace Version2 {

enum Types : uint8_t
{
    Type_Binary = 0x00,
    Type_Int8u  = 0x01,
    Type_Int16u = 0x02,
    Type_Int32u = 0x03,
    Type_Int64u = 0x04,
    Type_String = 0x08,
    Type_Int8s  = 0x09,
    Type_Int16s = 0x0a,
    Type_Int32s = 0x0b,
    Type_Int64s = 0x0c,
};

union Value
{
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint32_t u64;
    uint8_t * b;
};

struct Argument : public Buffer
{
    enum States : uint8_t
    {
        State_Flags = 1,
        State_Size  = 3,
        State_Data  = 4,
        State_Ready = 5,
    };

    Argument(uint8_t * ptr = nullptr, size_t size = 0) : Buffer(ptr, size) { Reset(); }

    void Reset()
    {
        this->Clear();
        this->state        = State_Flags;
        this->id           = 0;
        this->type         = 0;
        this->size         = 0;
        this->size_len     = 0;
        this->encoded_size = 0;
        this->is_null      = false;
        this->is_binary    = false;
        this->is_known     = false;
        this->feedback     = false;
        this->offset       = 0;
        memset(&this->value, 0x00, sizeof(Value));
    }

    States state        = State_Flags;
    uint16_t id         = 0;
    uint8_t type        = 0;
    size_t size         = 0;
    uint8_t size_len    = 0;
    size_t encoded_size = 0;
    bool is_null        = false;
    bool is_binary      = false;
    bool is_known       = false;
    bool feedback       = false;
    size_t offset       = 0;
    Value value;
};

CHIP_ERROR Encode(uint16_t id, uint8_t * value, Buffer & out);
CHIP_ERROR Encode(uint16_t id, uint16_t * value, Buffer & out);
CHIP_ERROR Encode(uint16_t id, uint32_t * value, Buffer & out);
CHIP_ERROR Encode(uint16_t id, const uint8_t * value, size_t size, Buffer & out);
CHIP_ERROR Decode(Buffer & in, Argument & arg);
CHIP_ERROR Find(Buffer & in, uint16_t id, Argument & arg);

} // namespace Version2

} // namespace Encoding
} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
