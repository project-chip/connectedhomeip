#include "ProvisionEncoder.h"
#include "ProvisionProtocol.h"
#include <lib/core/CHIPEncoding.h>
#include <lib/support/CodeUtils.h>
#include <string.h>
#include <algorithm>

using namespace chip::Encoding;

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {
namespace Encoding {

/*
begin            out             in               end
  |---------------v---------------v----------------|
  |.....offset....|......left.....|.....spare......|
  |..............size.............|
  |......................limit.....................|
*/

CHIP_ERROR Buffer::Add(uint8_t in)
{
    VerifyOrReturnError(nullptr != this->in, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(this->Spare() >= 1, CHIP_ERROR_BUFFER_TOO_SMALL);
    Write8(this->in, in);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Buffer::Add(uint16_t in)
{
    VerifyOrReturnError(nullptr != this->in, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(this->Spare() >= 2, CHIP_ERROR_BUFFER_TOO_SMALL);
    BigEndian::Write16(this->in, in);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Buffer::Add(uint32_t in)
{
    VerifyOrReturnError(nullptr != this->in, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(this->Spare() >= 4, CHIP_ERROR_BUFFER_TOO_SMALL);
    BigEndian::Write32(this->in, in);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Buffer::Add(int32_t in)
{
    return (in < 0) ? Add((uint32_t)(0x80000000 | (-in))) : Add((uint32_t)in);
}

CHIP_ERROR Buffer::Add(const uint8_t *in, size_t size)
{
    VerifyOrReturnError(nullptr != in, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(nullptr != this->in, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(this->Spare() >= size, CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(this->in, in, size);
    this->in += size;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Buffer::Add(Buffer & from, size_t size)
{
    VerifyOrReturnError(this->Spare() >= size, CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(from.Left() >= size, CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(this->in, from.out, size);
    this->in += size;
    from.out += size;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Buffer::Get(uint8_t &out)
{
    VerifyOrReturnError(nullptr != this->out, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(this->Left() > 0, CHIP_ERROR_BUFFER_TOO_SMALL);
    out = Read8(this->out);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Buffer::Get(uint16_t &out)
{
    VerifyOrReturnError(nullptr != this->out, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(this->Left() >= 2, CHIP_ERROR_BUFFER_TOO_SMALL);
    out = BigEndian::Read16(this->out);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Buffer::Get(uint32_t &out)
{
    VerifyOrReturnError(nullptr != this->out, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(this->Left() >= 4, CHIP_ERROR_BUFFER_TOO_SMALL);
    out = BigEndian::Read32(this->out);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Buffer::Get(uint8_t *out, size_t size)
{
    VerifyOrReturnError(nullptr != out, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(nullptr != this->out, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(this->Left() >= size, CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(out, this->out, size);
    this->out += size;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Buffer::Get(Buffer & into, size_t size)
{
    VerifyOrReturnError(into.Spare() >= size, CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(this->Left() >= size, CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(into.in, this->out, size);
    into.in += size;
    this->out += size;
    return CHIP_NO_ERROR;
}


//------------------------------------------------------------------------------
// Version 1
//------------------------------------------------------------------------------

#ifdef SILABS_PROVISION_PROTOCOL_V1

namespace Version1 {

CHIP_ERROR Encode(Buffer &arg, uint8_t in)
{
    ReturnErrorOnFailure(arg.Add(Type_Int8u));
    return arg.Add(in);
}

CHIP_ERROR Encode(Buffer &arg, uint16_t in)
{
    ReturnErrorOnFailure(arg.Add(Type_Int16u));
    return arg.Add(in);
}

CHIP_ERROR Encode(Buffer &arg, int32_t in)
{
    if(in < 0)
    {
        return Encode(arg, (uint32_t)(0x80000000 | (-in)));
    }
    else
    {
        return Encode(arg, (uint32_t)in);
    }
}

CHIP_ERROR Encode(Buffer &arg, uint32_t in)
{
    ReturnErrorOnFailure(arg.Add(Type_Int32u));
    return arg.Add(in);
}

CHIP_ERROR Encode(Buffer &arg, const uint8_t *in, size_t size)
{
    ReturnErrorOnFailure(arg.Add(Type_Array));
    ReturnErrorOnFailure(arg.Add((uint16_t) size));
    return arg.Add(in, size);
}

CHIP_ERROR Encode(Buffer &arg, const char *in)
{
    return Encode(arg, (uint8_t*)in, strlen(in) + 1);
}


CHIP_ERROR Decode(Buffer &arg, uint8_t &out)
{
    uint8_t type = 0;
    ReturnErrorOnFailure(arg.Get(type));
    VerifyOrReturnError(Type_Int8u == type, CHIP_ERROR_WRONG_KEY_TYPE);
    return arg.Get(out);
}

CHIP_ERROR Decode(Buffer &arg, uint16_t &out)
{
    uint8_t type = 0;
    ReturnErrorOnFailure(arg.Get(type));
    VerifyOrReturnError(Type_Int16u == type, CHIP_ERROR_WRONG_KEY_TYPE);
    return arg.Get(out);
}

CHIP_ERROR Decode(Buffer &arg, uint32_t &out)
{
    uint8_t type = 0;
    ReturnErrorOnFailure(arg.Get(type));
    VerifyOrReturnError(Type_Int32u == type, CHIP_ERROR_WRONG_KEY_TYPE);
    return arg.Get(out);
}

CHIP_ERROR Decode(Buffer &arg, uint8_t *out, size_t limit, size_t &size)
{
    uint8_t type = 0;
    ReturnErrorOnFailure(arg.Get(type));
    VerifyOrReturnError(Type_Array == type, CHIP_ERROR_WRONG_KEY_TYPE);
    uint16_t sz = 0;
    ReturnErrorOnFailure(arg.Get(sz));
    VerifyOrReturnError(sz <= limit, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorOnFailure(arg.Get(out, sz));
    size = sz;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Decode(Buffer &arg, char *out, size_t limit, size_t &size)
{
    VerifyOrReturnError(limit > 0, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(Decode(arg, (uint8_t *)out, limit - 1, size));
    out[size] = 0;
    return CHIP_NO_ERROR;
}

namespace Legacy {

CHIP_ERROR DecodeInt8u(Encoding::Buffer &in, uint8_t &out)
{
    uint8_t type = in.out[0];
    switch(type)
    {
    case Encoding::Version1::Type_Int8u:
        return Encoding::Version1::Decode(in, out);
    case Encoding::Version1::Type_Int32u:
    {
        uint32_t u32 = 0;
        ReturnErrorOnFailure(Encoding::Version1::Decode(in, u32));
        out = u32;
        return CHIP_NO_ERROR;
    }
    }
    return CHIP_ERROR_WRONG_KEY_TYPE;
}

CHIP_ERROR DecodeInt16u(Encoding::Buffer &in, uint16_t &out)
{
    uint8_t type = in.out[0];
    switch(type)
    {
    case Encoding::Version1::Type_Int16u:
        return Encoding::Version1::Decode(in, out);
    case Encoding::Version1::Type_Int32u:
    {
        uint32_t u32 = 0;
        ReturnErrorOnFailure(Encoding::Version1::Decode(in, u32));
        out = u32;
        return CHIP_NO_ERROR;
    }
    }
    return CHIP_ERROR_WRONG_KEY_TYPE;
}

CHIP_ERROR DecodeInt32u(Encoding::Buffer &in, uint32_t &out)
{
    uint8_t type = in.out[0];
    switch(type)
    {
    case Encoding::Version1::Type_Int32u:
        return Encoding::Version1::Decode(in, out);
    case Encoding::Version1::Type_Array:
    {
        uint8_t temp[sizeof(uint32_t)] = { 0 };
        size_t size = 0;
        ReturnErrorOnFailure(Encoding::Version1::Decode(in, temp, sizeof(temp), size));
        out = chip::Encoding::LittleEndian::Get32(temp);
        return CHIP_NO_ERROR;
    }
    }
    return CHIP_ERROR_WRONG_KEY_TYPE;
}

} // namespace Legacy

} // namespace Version1

#endif // SILABS_PROVISION_PROTOCOL_V1

//------------------------------------------------------------------------------
// Version 2
//------------------------------------------------------------------------------

namespace Version2 {

namespace {

uint8_t EncodeSizeLength(size_t size)
{
    if(size < 1) return 0;
    if(size < 0xff) return 1;
    if(size < 0xffff) return 2;
    return 3;
}

CHIP_ERROR EncodeHeader(uint16_t id, uint8_t type, size_t size, Buffer &out)
{
    // Flags
    uint8_t size_len = EncodeSizeLength(size);
    uint16_t flags = (id & Protocol2::kIdMask);
    flags |= ((size_len << Protocol2::kSizeBit) & Protocol2::kSizeMask);
    flags |= ((type << Protocol2::kTypeBit) & Protocol2::kTypeMask);
    ReturnErrorOnFailure(out.Add(flags));
    // Size
    if((Type_Binary == type) && (size_len > 0))
    {
        // Binary type (well-known or not)
        switch(size_len)
        {
        case 1:
            ReturnErrorOnFailure(out.Add((uint8_t) size));
            break;
        case 2:
            ReturnErrorOnFailure(out.Add((uint16_t) size));
            break;
        default:
            ReturnErrorOnFailure(out.Add((uint32_t) size));
            break;
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Defragment(Argument &arg, Buffer &in, size_t size)
{
    if(arg.Left() < size)
    {
        ReturnErrorOnFailure(arg.Add(in, std::min(in.Left(), size - arg.Left())));
    }
    ReturnErrorCodeIf(arg.Left() < size, CHIP_ERROR_MESSAGE_INCOMPLETE);
    return CHIP_NO_ERROR;
}

size_t DecodeSizeLength(uint16_t flags)
{
    uint8_t sz = (flags & Protocol2::kSizeMask) >> Protocol2::kSizeBit;
    return (sz < 3) ? sz : 4;
}

CHIP_ERROR DecodeFlags(Argument &arg, Buffer &in)
{
    uint16_t flags = 0;
    ReturnErrorOnFailure(in.Get(flags));
    arg.id        = (flags & Protocol2::kIdMask);
    arg.feedback  = (flags & Protocol2::kFeedbackMask);
    arg.type      = (flags & Protocol2::kTypeMask) >> Protocol2::kTypeBit;
    arg.size_len  = DecodeSizeLength(flags);
    arg.size      = 0;
    arg.is_null   = (0 == arg.size_len);
    arg.is_binary = (Type_Binary == arg.type);
    arg.is_known  = (flags & Protocol2::kWellKnownMask) > 0;
    return CHIP_NO_ERROR;
}

size_t DecodeFixedSize(uint8_t type)
{
    switch (type)
    {
    case Type_Int8u:
        return 1;
    case Type_Int16u:
        return 2;
    case Type_Int32u:
        return 4;
    default:
        return 0;
    }
}

CHIP_ERROR DecodeVariableSize(Argument &arg, Buffer &in)
{
    if(1 == arg.size_len)
    {
        uint8_t sz = 0;
        ReturnErrorOnFailure(in.Get(sz));
        arg.size = sz;
    }
    else if(2 == arg.size_len)
    {
        uint16_t sz = 0;
        ReturnErrorOnFailure(in.Get(sz));
        arg.size = sz;
    }
    else
    {
        uint32_t sz = 0;
        ReturnErrorOnFailure(in.Get(sz));
        arg.size = sz;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR DecodeValue(Argument &arg, Buffer &in)
{
    memset(&arg.value, 0x00, sizeof(Value));
    switch (arg.type)
    {
    case Type_Int8u:
        ReturnErrorOnFailure(in.Get(arg.value.u8));
        break;
    case Type_Int16u:
        ReturnErrorOnFailure(in.Get(arg.value.u16));
        break;
    case Type_Int32u:
        ReturnErrorOnFailure(in.Get(arg.value.u32));
        break;
    case Type_Binary:
        arg.value.b = in.out;
        break;
    default:
        return CHIP_ERROR_WRONG_TLV_TYPE;
    }
    return CHIP_NO_ERROR;
}

} // namespace


CHIP_ERROR Encode(uint16_t id, uint8_t *value, Buffer &out)
{
    ReturnErrorOnFailure(EncodeHeader(id, Type_Int8u, (nullptr == value ? 0 : 1), out));
    if(nullptr != value)
    {
        ReturnErrorOnFailure(out.Add(*value));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Encode(uint16_t id, uint16_t *value, Buffer &out)
{
    ReturnErrorOnFailure(EncodeHeader(id, Type_Int16u, (nullptr == value ? 0 : 2), out));
    if(nullptr != value)
    {
        ReturnErrorOnFailure(out.Add(*value));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Encode(uint16_t id, uint32_t *value, Buffer &out)
{
    ReturnErrorOnFailure(EncodeHeader(id, Type_Int32u, (nullptr == value ? 0 : 4), out));
    if(nullptr != value)
    {
        ReturnErrorOnFailure(out.Add(*value));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Encode(uint16_t id, const uint8_t *value, size_t size, Buffer &out)
{
    ReturnErrorOnFailure(EncodeHeader(id, Type_Binary, (nullptr == value ? 0 : size), out));
    if(nullptr != value)
    {
        ReturnErrorOnFailure(out.Add(value, size));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Decode(Buffer &in, Argument &arg)
{
    while((in.Left() > 0) && (Argument::State_Ready != arg.state))
    {
        switch(arg.state)
        {
        case Argument::State_Flags:
            // Decode flags
            ReturnErrorOnFailure(Defragment(arg, in, sizeof(uint16_t)));
            ReturnErrorOnFailure(DecodeFlags(arg, arg));
            arg.state = (arg.is_null) ? Argument::State_Ready : Argument::State_Size;
            break;

        case Argument::State_Size:
            // Decode size
            if(arg.is_binary)
            {
                ReturnErrorOnFailure(Defragment(arg, in, arg.size_len));
                ReturnErrorOnFailure(DecodeVariableSize(arg, arg));
            }
            else
            {
                arg.size = DecodeFixedSize(arg.type);
            }
            // Total encoded size
            arg.encoded_size = sizeof(uint16_t) + (arg.is_binary ? arg.size_len : 0) + arg.size;
            arg.state = Argument::State_Data;
            break;

        case Argument::State_Data:
            // Argument Value
            VerifyOrReturnError(!arg.is_null, CHIP_ERROR_INVALID_TLV_ELEMENT);
            VerifyOrReturnError(arg.size > 0, CHIP_ERROR_INVALID_MESSAGE_LENGTH);
            ReturnErrorOnFailure(Defragment(arg, in, arg.size));
            ReturnErrorOnFailure(DecodeValue(arg, arg)); // The context itself contains the value
            arg.state = Argument::State_Ready;
            break;

        default:
            return CHIP_ERROR_INCORRECT_STATE;
        }
    }

    VerifyOrReturnError(Argument::State_Ready == arg.state, CHIP_ERROR_MESSAGE_INCOMPLETE);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Find(Buffer &in, uint16_t id, Argument &arg)
{
    while(in.Left() > 0)
    {
        arg.Clear();
        arg.offset = in.Offset();
        // Decode flags
        ReturnErrorOnFailure(DecodeFlags(arg, in));
        // Decode size
        if(arg.is_null)
        {
            arg.size = 0;
        }
        else if(arg.is_binary)
        {
            ReturnErrorOnFailure(DecodeVariableSize(arg, in));
        }
        else
        {
            arg.size = DecodeFixedSize(arg.type);
        }
        // Total encoded size
        arg.encoded_size = sizeof(uint16_t) + (arg.is_binary ? arg.size_len : 0) + arg.size;
        if(arg.id == id)
        {
            // Decode the value from the input buffer
            return DecodeValue(arg, in);
        }
        // Id doesn't match, skip value
        ReturnErrorCodeIf(in.Left() < arg.size, CHIP_ERROR_MESSAGE_INCOMPLETE);
        in.out += arg.size;
    }
    arg.offset = in.Offset();
    return CHIP_ERROR_NOT_FOUND;
}

} // namespace Version2

} // namespace Encoding
} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
