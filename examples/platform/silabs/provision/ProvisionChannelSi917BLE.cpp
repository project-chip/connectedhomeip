/*
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
 */
#include "ProvisionChannel.h"

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

ProvisionChannel & ProvisionChannel::GetInstance()
{
    static ProvisionChannel instance;
    return instance;
}

CHIP_ERROR ProvisionChannel::Init()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR ProvisionChannel::Read(uint8_t *, size_t, size_t &)
{
    return CHIP_ERROR_READ_FAILED;
}

CHIP_ERROR ProvisionChannel::Write(const uint8_t *, size_t)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR ProvisionChannel::OnDataAvailable()
{
    return CHIP_NO_ERROR;
}

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
