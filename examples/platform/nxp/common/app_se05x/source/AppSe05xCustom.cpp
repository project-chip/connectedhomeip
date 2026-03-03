/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#if CONFIG_CHIP_SE05X
#include "AppSe05x.h"
#include <app/FailSafeContext.h>
#include <app/server/Server.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/nxp/crypto/se05x/mcu_kvs/NFCDataRetrievalInfo.hpp>
#include <platform/nxp/crypto/se05x/kvs_utilities/CHIPCryptoPALHsm_se05x_readClusters.h>

namespace chip {
namespace NXP {
namespace App {
namespace Se05x {

CHIP_ERROR Init()
{
    return chip::DeviceLayer::NFCCommissioning::NFCDataRetrievalInfoMgr().Init();
}

CHIP_ERROR PostInit()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogDetail(DeviceLayer, "SE05x - Check for SE05x fail safe timer");

    uint16_t fail_safe_time = DeviceLayer::NFCCommissioning::NFCDataRetrievalInfoMgr().GetRemainingFailSafeTimerForSE05x();

    if (fail_safe_time != 0)
    {
        chip::app::FailSafeContext & failSafeContext = chip::Server::GetInstance().GetFailSafeContext();

        err = failSafeContext.ArmFailSafe(se05x_get_fabric_id(), chip::System::Clock::Seconds16(fail_safe_time));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(NotSpecified, "SE05x - Error in starting Fail Safe timer for SE05x NFC commissioned fabric");
        }
        else
        {
            ChipLogDetail(NotSpecified, "SE05x - Started Fail Safe timer for SE05x NFC commissioned fabric");
        }
    }

    return err;
}

} // namespace Se05x
} // namespace App
} // namespace NXP
} // namespace chip

#endif // CONFIG_CHIP_SE05X
