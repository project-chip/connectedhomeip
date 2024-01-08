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

/**
 * @file
 *  A collection of mocks and pseudo application context that help testing each
 *  protocol controller functionality locally.
 *
 */

#ifndef UNIFY_MPC_CONTEXT_H_
#define UNIFY_MPC_CONTEXT_H_

#include <app/tests/AppTestContext.h>

// Mocks

// Third party library
#include <nlunit-test.h>

namespace unify::mpc {
namespace Test {

/**
 * @brief
 *  A base class for mocking CHIPTool client. In a test context it is already paird
 *  to Unify MPC and you can instantly start interaction with command/attribute
 *  handlers.
 *
 */
class UnifyMPCContext : public chip::Test::AppContext
{
    typedef chip::Test::AppContext Super;

public:
    UnifyMPCContext() {}

    /// Initialize the underlying layers.
    CHIP_ERROR Initialize();

    // Shutdown all layers, finalize operations
    void Finalize();
};

} // namespace Test
} // namespace unify::mpc

#endif // UNIFY_MPC_CONTEXT_H_
