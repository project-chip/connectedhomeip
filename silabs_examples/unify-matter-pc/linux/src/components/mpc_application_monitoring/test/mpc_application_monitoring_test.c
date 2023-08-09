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
#include "mpc_application_monitoring.h"
#include "uic_version.h"
#include "unity.h"

#include "unify_application_monitoring_mock.h"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
    return num_failures;
}

/// Called before each and every test
void setUp() {}

void test_mpc_application_monitoring()
{
    unify_application_monitoring_set_application_name_Expect(MPC_APPLICATION_NAME);
    unify_application_monitoring_set_application_version_Expect(UIC_VERSION);
    unify_application_monitoring_init_ExpectAndReturn(SL_STATUS_OK);
    TEST_ASSERT_EQUAL(SL_STATUS_OK, mpc_application_monitoring_init());
}
