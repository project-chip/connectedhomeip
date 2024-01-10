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
    Channel(bool active): mActive(active) {}
    virtual ~Channel() = default;
    virtual CHIP_ERROR Init() = 0;
    virtual CHIP_ERROR Read(uint8_t *buffer, size_t buffer_length, size_t &bytes_read) = 0;
    virtual CHIP_ERROR Write(const uint8_t *buffer, size_t buffer_length) = 0;

    void SetActive(bool active) {
        mActive = active;
    }

    bool IsActive() {
        return mActive;
    }

private:
    bool mActive = false;
};

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
