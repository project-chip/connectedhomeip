/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
// Includes from this component
#include "mpc_attribute_parser_fwk.h"
#include "mpc_attribute_resolver_callbacks.h"
#include "mpc_attribute_resolver_rules.h"
#include "mpc_attribute_resolver_send.h"

// Unify Components includes
#include "attribute_resolver.h"

sl_status_t mpc_attribute_resolver_init()
{
    sl_status_t init_status = SL_STATUS_OK;

    attribute_resolver_config_t attribute_resolver_config = { .send_init = &mpc_attribute_resolver_send_init,
                                                              .send      = &mpc_attribute_resolver_send,
                                                              // .abort     = &attribute_resolver_abort_pending_resolution,
                                                              // Minimal timespan before retrying a get
                                                              .get_retry_timeout = 6000,
                                                              // Number of times to retry sending a get
                                                              .get_retry_count = 10 };
    init_status |= attribute_resolver_init(attribute_resolver_config);

    // MPC resolver rule init
    init_status |= mpc_attribute_resolver_rules_init();

    // MPC attribute parser init
    init_status |= mpc_attribute_parser_init();
    return init_status;
}

int mpc_attribute_resolver_teardown()
{
    // mpc_attribute_resolver_group_teardown();
    return attribute_resolver_teardown();
}
