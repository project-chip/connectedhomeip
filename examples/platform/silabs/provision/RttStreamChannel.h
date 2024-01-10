#pragma once

#include "ProvisionChannel.h"

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

class RttStreamChannel : public Channel
{
public:
    RttStreamChannel(): Channel(true) {}
    CHIP_ERROR Init() override;
    CHIP_ERROR Read(uint8_t *buffer, size_t buffer_length, size_t &bytes_read) override;
    CHIP_ERROR Write(const uint8_t *buffer, size_t buffer_length) override;
};

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
