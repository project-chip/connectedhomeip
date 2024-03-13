#pragma once

#include <lib/core/CHIPError.h>
#include <stddef.h>
#include <stdint.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

struct GenericStorage
{
    virtual ~GenericStorage() = default;

    virtual CHIP_ERROR Set(uint16_t id, const uint8_t *value) = 0;
    virtual CHIP_ERROR Get(uint16_t id, uint8_t &value) = 0;
    virtual CHIP_ERROR Set(uint16_t id, const uint16_t *value) = 0;
    virtual CHIP_ERROR Get(uint16_t id, uint16_t &value) = 0;
    virtual CHIP_ERROR Set(uint16_t id, const uint32_t *value) = 0;
    virtual CHIP_ERROR Get(uint16_t id, uint32_t &value) = 0;
    virtual CHIP_ERROR Set(uint16_t id, const uint64_t *value) = 0;
    virtual CHIP_ERROR Get(uint16_t id, uint64_t &value) = 0;
    virtual CHIP_ERROR Get(uint16_t id, uint8_t *value, size_t max_size, size_t &size) = 0;
    virtual CHIP_ERROR Set(uint16_t id, const uint8_t *value, size_t size) = 0;
};


} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
