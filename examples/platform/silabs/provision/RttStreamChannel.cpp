#include "RttStreamChannel.h"
#include <sl_iostream_rtt.h>
#include <sl_iostream_init_instances.h>
// #include <hardware/board/inc/sl_board_control.h>
#include "em_chip.h"

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

CHIP_ERROR RttStreamChannel::Init()
{
    CHIP_Init();
    sl_iostream_rtt_init();
    return CHIP_NO_ERROR;
}

int count = 0;

CHIP_ERROR RttStreamChannel::Read(uint8_t *buffer, size_t buffer_length, size_t &bytes_read)
{
    sl_status_t err = sl_iostream_read(sl_iostream_rtt_handle, buffer, buffer_length, &bytes_read);
    return err ? CHIP_ERROR_NOT_FOUND : CHIP_NO_ERROR;
}

CHIP_ERROR RttStreamChannel::Write(const uint8_t *buffer, size_t buffer_length)
{
    sl_iostream_write(sl_iostream_rtt_handle, buffer, buffer_length);
    // sl_iostream_write(sl_iostream_rtt_handle, buffer, buffer_length);
    return CHIP_NO_ERROR;
}

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
