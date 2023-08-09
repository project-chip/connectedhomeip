/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "UnifyMPCContext.h"

namespace unify::mpc {
namespace Test {

CHIP_ERROR UnifyMPCContext::Init()
{
    ReturnErrorOnFailure(Super::Init());

    return CHIP_NO_ERROR;
}

void UnifyMPCContext::Shutdown()
{
    Super::Shutdown();
}

} // namespace Test
} // namespace unify::mpc

void __attribute__((weak)) mpc_schedule_contiki() {
    // do nothing since this is mock for test when MPC uic_loop scheduling is not needed
}
