/* See Project CHIP LICENSE file for licensing information. */


#include "DeviceNetworkProvisioningDelegateImpl.h"

#if CHIP_ENABLE_OPENTHREAD
#include <platform/ThreadStackManager.h>
#endif

namespace chip {
namespace DeviceLayer {

CHIP_ERROR DeviceNetworkProvisioningDelegateImpl::_ProvisionThreadNetwork(DeviceLayer::Internal::DeviceNetworkInfo & threadData)
{
#if CHIP_ENABLE_OPENTHREAD
    CHIP_ERROR error = CHIP_NO_ERROR;

    SuccessOrExit(error = ThreadStackMgr().SetThreadEnabled(false));
    SuccessOrExit(error = ThreadStackMgr().SetThreadProvision(threadData));
    SuccessOrExit(error = ThreadStackMgr().SetThreadEnabled(true));
exit:
    return error;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // CHIP_ENABLE_OPENTHREAD
}

} // namespace DeviceLayer
} // namespace chip
