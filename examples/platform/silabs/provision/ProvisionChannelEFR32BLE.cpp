/*
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
 */
#include "ProvisionChannel.h"

#include <gatt_db.h>
#include <lib/support/CodeUtils.h>
#include <sl_bt_api.h>

#include <cstring>
#include <headers/ProvisionProtocol.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

namespace {
uint8_t sReceiveBuffer[Protocol2::kPackageSizeMax];
size_t sReceiveSize = 0;
} // namespace

ProvisionChannel & ProvisionChannel::GetInstance()
{
    static ProvisionChannel instance;
    return instance;
}

CHIP_ERROR ProvisionChannel::Init()
{
    sReceiveSize = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ProvisionChannel::Read(uint8_t * buffer, size_t bufferLength, size_t & bytesRead)
{
    VerifyOrReturnError(sReceiveSize > 0, CHIP_ERROR_READ_FAILED);
    VerifyOrReturnError(buffer != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(bufferLength >= sReceiveSize, CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(buffer, sReceiveBuffer, sReceiveSize);
    bytesRead    = sReceiveSize;
    sReceiveSize = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ProvisionChannel::Write(const uint8_t * buffer, size_t bufferLength)
{
    VerifyOrReturnError(buffer != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    const sl_status_t status = sl_bt_gatt_server_write_attribute_value(gattdb_CHIPoBLEChar_Tx, 0, bufferLength, buffer);
    return status == SL_STATUS_OK ? CHIP_NO_ERROR : CHIP_ERROR_WRITE_FAILED;
}

CHIP_ERROR ProvisionChannel::OnDataAvailable()
{
    const sl_status_t status = sl_bt_gatt_server_read_attribute_value(gattdb_CHIPoBLEChar_Rx, 0, Protocol2::kPackageSizeMax,
                                                                      &sReceiveSize, sReceiveBuffer);
    return status == SL_STATUS_OK ? CHIP_NO_ERROR : CHIP_ERROR_READ_FAILED;
}

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
