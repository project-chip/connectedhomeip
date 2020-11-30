/******************************************************************************

 Group: CMCU, LPC
 Target Device: cc13x2_26x2

 ******************************************************************************

 Copyright (c) 2017-2020, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************


 *****************************************************************************/


#include <openthread/config.h>

#include <openthread/platform/misc.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/sys_ctrl.h)

/**
 * Function documented in platform/misc.h
 */
void otPlatReset(otInstance *aInstance)
{
    (void)aInstance;
    SysCtrlSystemReset();
}

/**
 * Function documented in platform/misc.h
 */
otPlatResetReason otPlatGetResetReason(otInstance *aInstance)
{
    (void)aInstance;

    switch (SysCtrlResetSourceGet())
    {
        case RSTSRC_PWR_ON:
        {
            return OT_PLAT_RESET_REASON_POWER_ON;
        }

        case RSTSRC_PIN_RESET:
        {
            return OT_PLAT_RESET_REASON_EXTERNAL;
        }

        case RSTSRC_VDDS_LOSS:
        case RSTSRC_VDDR_LOSS:
        case RSTSRC_CLK_LOSS:
        {
            return OT_PLAT_RESET_REASON_CRASH;
        }

        case RSTSRC_WARMRESET:
        case RSTSRC_SYSRESET:
        case RSTSRC_WAKEUP_FROM_SHUTDOWN:
        {
            return OT_PLAT_RESET_REASON_SOFTWARE;
        }

        default:
        {
            return OT_PLAT_RESET_REASON_UNKNOWN;
        }
    }
}

void otPlatWakeHost(void)
{
    // TODO: implement an operation to wake the host from sleep state.
}
