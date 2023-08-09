/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "mpc_datastore_fixt.h"

// Test includes
#include "unity.h"

// Unify component includes
#include "datastore_fixt_mock.h"
#include "mpc_config_mock.h"

// Generic includes
#include <string.h>

// Static test variables
mpc_config_t mpc_config;

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
    return num_failures;
}

/// Called before each and every test
void setUp()
{
    memset(&mpc_config, 0, sizeof(mpc_config));
}

void test_mpc_datastore_test()
{
    mpc_config.datastore_file = "Test_file.db";
    // Not much to test, just run the init
    sl_status_t test_status = SL_STATUS_NONE_WAITING;

    mpc_get_config_ExpectAndReturn(&mpc_config);
    datastore_fixt_setup_and_handle_version_ExpectAndReturn(mpc_config.datastore_file, MPC_DATASTORE_VERSION, test_status);

    TEST_ASSERT_EQUAL(test_status, mpc_datastore_fixt_setup());
}
