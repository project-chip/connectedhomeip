#include "ProvisionChannel.h"
#include "ProvisionEncoder.h"
#include "ProvisionProtocol.h"
#include <lib/support/CodeUtils.h>
#include <string.h>
#include <algorithm>


namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {


CHIP_ERROR Channel::Init()
{
    return CHIP_NO_ERROR;
}


CHIP_ERROR Channel::Read(uint8_t *buffer, size_t buffer_length, size_t &bytes_read)
{
    return CHIP_ERROR_READ_FAILED;
}


CHIP_ERROR Channel::Write(const uint8_t *data, size_t data_size)
{
    return CHIP_NO_ERROR;
}


CHIP_ERROR Channel::Update(uint16_t attrib_id)
{
    return CHIP_NO_ERROR;
}


} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
