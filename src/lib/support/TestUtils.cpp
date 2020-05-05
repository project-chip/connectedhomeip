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

#include <stdlib.h>
#include <string.h>
#include <support/TestUtils.h>

const size_t kTest_Suite_Count_Increments = 16;

typedef struct {
    nlTestSuite* test_suites;
    uint32_t test_suites_size;
    uint32_t num_test_suites;
} test_suites_t;

static test_suites_t gs_test_suites = {NULL, 0, 0};

CHIP_ERROR deploy_device_unit_tests(nlTestSuite *tests) {
    if (gs_test_suites.test_suites_size <= gs_test_suites.num_test_suites) {
        uint32_t new_size = gs_test_suites.test_suites_size + kTest_Suite_Count_Increments;
        nlTestSuite* new_suites = (nlTestSuite*)realloc(gs_test_suites.test_suites,
                                   sizeof(nlTestSuite) * new_size);
        if (new_suites == NULL) {
            return CHIP_ERROR_NO_MEMORY;
        }
        gs_test_suites.test_suites = new_suites;
        gs_test_suites.test_suites_size = new_size;
    }
    memcpy(&gs_test_suites.test_suites[gs_test_suites.num_test_suites], tests, sizeof(nlTestSuite));
    gs_test_suites.num_test_suites++;
    return CHIP_NO_ERROR;
}

int run_device_unit_tests() {
    int status = 0;
    for (uint32_t i = 0; i < gs_test_suites.num_test_suites; i++) {
        nlTestRunner(&gs_test_suites.test_suites[i], NULL);
        status |= nlTestRunnerStats(&gs_test_suites.test_suites[i]);
    }
    return status;
}
