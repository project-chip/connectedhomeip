/*
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
 */
#pragma once

#include <headers/ProvisionChannelInterface.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

class ProvisionChannel final : public IProvisionChannel
{
public:
    static ProvisionChannel & GetInstance();

    ProvisionChannel(const ProvisionChannel &)             = delete;
    ProvisionChannel & operator=(const ProvisionChannel &) = delete;

    CHIP_ERROR Init() override;
    CHIP_ERROR Read(uint8_t * buffer, size_t bufferLength, size_t & bytesRead) override;
    CHIP_ERROR Write(const uint8_t * buffer, size_t bufferLength) override;
    CHIP_ERROR OnDataAvailable() override;

private:
    ProvisionChannel() = default;
};

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
