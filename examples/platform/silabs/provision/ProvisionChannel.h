#pragma once

#include <lib/core/CHIPError.h>
#include <stdint.h>
#include <stddef.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

class Channel
{
public:
    Channel() = default;
    ~Channel() = default;

    CHIP_ERROR Init();
    CHIP_ERROR Read(uint8_t *buffer, size_t buffer_length, size_t &bytes_read);
    CHIP_ERROR Write(const uint8_t *buffer, size_t buffer_length);

    static CHIP_ERROR Update(uint16_t handle);
};

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
