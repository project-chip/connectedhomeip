#include "ProvisionManager.h"
#include "ProvisionProtocol.h"
#include "ProvisionEncoder.h"
#include <lib/support/logging/CHIPLogging.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

namespace {
#ifdef SILABS_PROVISION_PROTOCOL_V1
/**
 * Protocol v1.x does not support fragmention. The buffer must
 * hold the entire files (DAC, PAI, CD)
 */
uint8_t rx_buffer[Storage::kArgumentSizeMax];
uint8_t tx_buffer[Storage::kArgumentSizeMax];
#else
/**
 * Protocol v2.0 enforces a maximum package size. Larger files
 * are fragmented in as many packages as needed
 */
uint8_t rx_buffer[Protocol2::kPackageSizeMax];
uint8_t tx_buffer[Protocol2::kPackageSizeMax];
#endif
}// namespace

CHIP_ERROR Manager::Init()
{
    if(CHIP_NO_ERROR != mStore.GetProvisionRequest(mProvisionRequested))
    {
#ifdef SL_PROVISION_CHANNEL_ENABLED
        mProvisionRequested = SL_PROVISION_CHANNEL_ENABLED;
#else
        mProvisionRequested = false;
#endif
    }
    if(mProvisionRequested)
    {
        ChipLogProgress(DeviceLayer, "Bluetooth Provision Enabled!");
        // Disable provision mode for next boot
        mStore.SetProvisionRequest(false);
    }
    return CHIP_NO_ERROR;
}

bool Manager::Step()
{
    size_t bytes_read = 0;
    size_t offset = 0;

    while(CHIP_NO_ERROR == mChannel.Read(&rx_buffer[offset], sizeof(rx_buffer) - offset, bytes_read))
    {
        offset += bytes_read;
        bytes_read = 0;
    }
    if(offset > 0)
    {
        ByteSpan input(rx_buffer, offset);
        MutableByteSpan output(tx_buffer, sizeof(tx_buffer));
        mProvisionRequested = ProcessCommand(input, output);
        mChannel.Write(output.data(), output.size());
    }
    return mProvisionRequested;
}

bool Manager::IsProvisionRequired()
{
    return mProvisionRequested;
}

CHIP_ERROR Manager::SetProvisionRequired(bool do_provision)
{
    mProvisionRequested = do_provision;
    return mStore.SetProvisionRequest(do_provision);
}

bool Manager::ProcessCommand(ByteSpan & req, MutableByteSpan & res)
{
    VerifyOrReturnError(req.size() > 1, false);
    uint8_t prot_id = req.data()[0];
    switch(prot_id)
    {
#ifdef SILABS_PROVISION_PROTOCOL_V1
        case 1:
            return mProtocol1.Execute(req, res);
#endif
        case 2:
            return mProtocol2.Execute(req, res);
        default:
            res.reduce_size(0);
    }
    return false;
}

namespace {
Manager sManager;
}

Manager & Manager::GetInstance() { return sManager; }

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
