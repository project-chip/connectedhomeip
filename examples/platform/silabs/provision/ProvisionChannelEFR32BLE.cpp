#include "ProvisionChannel.h"
#include "ProvisionEncoder.h"
#include "ProvisionProtocol.h"
#include <lib/support/CodeUtils.h>
#ifndef SLI_SI91X_MCU_INTERFACEX_MCU_INTERFACE
#include <sl_bluetooth.h>
#include <gatt_db.h>
#endif
#include <string.h>
#include <algorithm>


namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

namespace {
uint8_t rx_buffer[Protocol2::kPackageSizeMax];
size_t rx_size = 0;
} // namespace


CHIP_ERROR Channel::Init()
{
    return CHIP_NO_ERROR;
}


CHIP_ERROR Channel::Read(uint8_t *buffer, size_t buffer_length, size_t &bytes_read)
{
    VerifyOrReturnError(rx_size > 0, CHIP_ERROR_READ_FAILED);
    ChipLogProgress(Zcl, "Bluetooth Channel RX(%u/%u)", (unsigned)rx_size, (unsigned)buffer_length);
    ReturnErrorCodeIf(nullptr == buffer, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorCodeIf(buffer_length < rx_size, CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(buffer, rx_buffer, rx_size);
    bytes_read = rx_size;
    rx_size = 0;
    return CHIP_NO_ERROR;
}


CHIP_ERROR Channel::Write(const uint8_t *data, size_t data_size)
{
    ChipLogProgress(Zcl, "Bluetooth Channel TX(%u)", (unsigned)data_size);
    sl_bt_gatt_server_write_attribute_value(gattdb_CHIPoBLEChar_Tx, 0, data_size, data);
    return CHIP_NO_ERROR;
}


CHIP_ERROR Channel::Update(uint16_t attrib_id)
{
    sl_bt_gatt_server_read_attribute_value(attrib_id, 0, Protocol2::kPackageSizeMax, &rx_size, rx_buffer);
    return CHIP_NO_ERROR;
}


} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
