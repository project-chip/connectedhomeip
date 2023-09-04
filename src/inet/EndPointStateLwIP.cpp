/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <inet/EndPointStateLwIP.h>

#include <lwip/sys.h>
#include <lwip/tcpip.h>

#include <platform/LockTracker.h>

namespace chip {
namespace Inet {

err_t EndPointStateLwIP::RunOnTCPIPRet(std::function<err_t()> fn)
{
    assertChipStackLockedByCurrentThread();
#if LWIP_TCPIP_CORE_LOCKING
    err_t err;
    LOCK_TCPIP_CORE();
    err = fn();
    UNLOCK_TCPIP_CORE();
    return err;
#else
    // Post a message to the TCPIP task and wait for it to run.
    static sys_sem_t sTCPIPSem;
    static bool sTCPIPSemInited = false;
    if (!sTCPIPSemInited)
    {
        err_t err = sys_sem_new(&sTCPIPSem, 0);
        if (err != ERR_OK)
        {
            return err;
        }
        sTCPIPSemInited = true;
    }

    // tcpip_callback takes a C function pointer, so we can't pass a capturing lambda to it.
    // Just store the state the function we pass to it needs in statics.
    // This should be safe, since that function will execute before we return and there is no
    // re-entry into this method.
    static std::function<err_t()> sTCPIPFunction;
    static err_t sTCPIPFunctionResult;
    VerifyOrDie(sTCPIPFunction == nullptr);

    sTCPIPFunction  = fn;
    const err_t err = tcpip_callback(
        [](void * aCtx) {
            sTCPIPFunctionResult = sTCPIPFunction();
            sys_sem_signal(&sTCPIPSem);
        },
        nullptr);
    if (err != ERR_OK)
    {
        return err;
    }
    sys_arch_sem_wait(&sTCPIPSem, 0);
    sTCPIPFunction = nullptr;
    return sTCPIPFunctionResult;
#endif
}

void EndPointStateLwIP::RunOnTCPIP(std::function<void()> fn)
{
    VerifyOrDie(RunOnTCPIPRet([&fn]() {
                    fn();
                    return ERR_OK;
                }) == ERR_OK);
}

} // namespace Inet
} // namespace chip
