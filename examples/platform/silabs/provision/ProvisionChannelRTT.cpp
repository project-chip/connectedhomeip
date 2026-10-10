/*
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
 */
#include "ProvisionChannel.h"

#include <SEGGER_RTT.h>
#include <lib/support/CodeUtils.h>

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
    VerifyOrReturnError(SEGGER_RTT_ConfigUpBuffer(0, nullptr, nullptr, 0, SEGGER_RTT_MODE_NO_BLOCK_TRIM) >= 0, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(SEGGER_RTT_ConfigDownBuffer(0, nullptr, nullptr, 0, SEGGER_RTT_MODE_NO_BLOCK_TRIM) >= 0,
                        CHIP_ERROR_INTERNAL);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ProvisionChannel::Read(uint8_t * buffer, size_t bufferLength, size_t & bytesRead)
{
    VerifyOrReturnError(buffer != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    bytesRead                     = 0;
    const unsigned bytesAvailable = SEGGER_RTT_HasData(0);
    VerifyOrReturnError(bytesAvailable > 0, CHIP_ERROR_READ_FAILED);

    const unsigned toRead = bytesAvailable < bufferLength ? bytesAvailable : static_cast<unsigned>(bufferLength);
    bytesRead             = SEGGER_RTT_Read(0, buffer, toRead);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ProvisionChannel::Write(const uint8_t * buffer, size_t bufferLength)
{
    VerifyOrReturnError(buffer != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    size_t sent = 0;
    while (sent < bufferLength)
    {
        const unsigned written = SEGGER_RTT_Write(0, buffer + sent, bufferLength - sent);
        VerifyOrReturnError(written > 0, CHIP_ERROR_WRITE_FAILED);
        sent += written;
    }
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
