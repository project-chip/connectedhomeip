#pragma once

#include "ProvisionStorage.h"
#include <lib/core/CHIPError.h>
#include <stddef.h>
#include <stdint.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

enum EncodingTypes
{
    Type_None = 0,
    Type_UInt8 = 1,
    Type_UInt16 = 2,
    Type_UInt32 = 3,
    Type_Array = 4
};

struct Encoder
{
    Encoder(uint8_t *data, size_t size) :
        _data(data), _size(size), _offset(0)
    {}
    virtual ~Encoder() {}

    // void init(uint8_t *data, size_t size);
    CHIP_ERROR addUint8(uint8_t in);
    CHIP_ERROR getUint8(uint8_t &out);
    CHIP_ERROR addUint16(uint16_t in);
    CHIP_ERROR getUint16(uint16_t &out);
    CHIP_ERROR addInt32(int32_t in);
    CHIP_ERROR addUint32(uint32_t in);
    CHIP_ERROR getUint32(uint32_t &out);
    CHIP_ERROR addArray(const uint8_t *in, size_t size);
    CHIP_ERROR getArray(uint8_t *out, size_t max_size, size_t &size);
    CHIP_ERROR addString(const char *in);
    CHIP_ERROR getString(char *out, size_t max_size, size_t &size);
    void Reset();

    const uint8_t *data() { return _data; }
    size_t offset() { return _offset; }
    size_t size() { return _size; }

    uint8_t *_data = nullptr;
    size_t _size = 0;
    size_t _offset = 0;
};

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
