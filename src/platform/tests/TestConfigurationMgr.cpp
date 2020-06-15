/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *    @file
 *      This file implements a unit test suite for the Configuration Manager
 *      code functionality.
 *
 */

#include "TestConfigurationMgr.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nlunit-test.h>
#include <support/CodeUtils.h>

#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::Logging;
using namespace chip::Inet;
using namespace chip::DeviceLayer;

// =================================
//      Unit tests
// =================================

static void TestPlatformMgr_Init(nlTestSuite * inSuite, void * inContext)
{
    // ConfigurationManager is initialized from PlatformManager indirectly
    CHIP_ERROR err = PlatformMgr().InitChipStack();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

static void TestConfigurationMgr_FabricId(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    uint64_t fabricId = 0;

    err = ConfigurationMgr().StoreFabricId(2006255910626445ULL);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = ConfigurationMgr().GetFabricId(fabricId);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, fabricId == 2006255910626445ULL);
}

static void TestConfigurationMgr_ServiceConfig(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint8_t buf[1024];
    size_t serviceConfigLen    = 0;
    const char * serviceConfig = "1QAADwABADYBFTABCQCoNCLp2XXkVSQCBDcDJxMBAADu7jC0GBgmBJUjqRkmBRXB0iw3BicTAQAA7"
                                 "u4wtBgYJAcCJggVAFojMAoxBHhS4pySunAZWEZtrhhySvtDDfYHKTMNYVXlZUaOug2lP7UXwEdkRA"
                                 "IYT6gRJFDUezWDKQEpAhg1gikBJAJgGDWBMAIIQgys9rRkceYYNYAwAghCDKz2tGRx5hg1DDABGQC"
                                 "+DtqhY1qO8VIXRYC93JQS1MwcLDNOKdwwAhkAi+fuLhEXFK6S2is7bS/XXZ5fzbi6L2V2GBgVMAEI"
                                 "cQIn0yh6aI8kAgQ3AywBCDMwMzI0OTUzGCYEeo6eJSYFejfEODcGLAEIMzAzMjQ5NTMYJAcCJgglA"
                                 "FojMAo5BLZ6ok8Qck/nD1V/uwxBHadUxpBwP+AtAjO9iBxp+CrnCdp8iT1k0rpICd/yVB5J79FpKN"
                                 "Zk81aVNYMpARg1gikBJAIFGDWEKQE2AgQCBAEYGDWBMAIITs6h0PVqSKQYNYAwAghOzqHQ9WpIpBg"
                                 "1DDABHGd5WOwcKPLXFHW97Y3skfyC8PHj0LgiadIPQx4wAh0AnzaRJWzznB07Q0cghoIGZTIPZjZA"
                                 "0kFZ3uvMPxgYGDUCJwEBAAAAAjC0GDYCFSwBGWZyb250ZG9vci5xYS5uZXN0bGFicy5jb20lAlcrG"
                                 "BgYGA==";

    err = ConfigurationMgr().StoreServiceConfig((uint8_t *) serviceConfig, strlen(serviceConfig));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = ConfigurationMgr().GetServiceConfig(buf, 1024, serviceConfigLen);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, serviceConfigLen == strlen(serviceConfig));
    NL_TEST_ASSERT(inSuite, memcmp(buf, serviceConfig, serviceConfigLen) == 0);
}

static void TestConfigurationMgr_PairedAccountId(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    char buf[64];
    size_t accountIdLen          = 0;
    const char * pairedAccountId = "USER_016CB664A86A888D";

    err = ConfigurationMgr().StorePairedAccountId(pairedAccountId, strlen(pairedAccountId));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = ConfigurationMgr().GetPairedAccountId(buf, 64, accountIdLen);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, accountIdLen == strlen(pairedAccountId));
    NL_TEST_ASSERT(inSuite, strcmp(buf, "USER_016CB664A86A888D") == 0);
}

/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = {

    NL_TEST_DEF("Test PlatformMgr::Init", TestPlatformMgr_Init),
    NL_TEST_DEF("Test ConfigurationMgr::FabricId", TestConfigurationMgr_FabricId),
    NL_TEST_DEF("Test ConfigurationMgr::ServiceConfig", TestConfigurationMgr_ServiceConfig),
    NL_TEST_DEF("Test ConfigurationMgr::PairedAccountId", TestConfigurationMgr_PairedAccountId), NL_TEST_SENTINEL()
};

int TestConfigurationMgr(void)
{
    nlTestSuite theSuite = { "CHIP DeviceLayer time tests", &sTests[0], NULL, NULL };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, NULL);
    return nlTestRunnerStats(&theSuite);
}
