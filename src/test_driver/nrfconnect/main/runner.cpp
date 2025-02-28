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

#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTest.h>
#include <platform/CHIPDeviceLayer.h>
#include <pw_unit_test/framework.h>

#include <unistd.h>
#include <vector>

#include <nsi_cmdline.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>

using namespace ::chip;
using namespace ::chip::DeviceLayer;

LOG_MODULE_REGISTER(runner, CONFIG_MATTER_LOG_LEVEL);

int main(void)
{
    VerifyOrDie(settings_subsys_init() == 0);

    int argc;
    char ** argv;
    nsi_get_test_cmd_line_args(&argc, &argv);

    std::vector<std::string_view> suites_to_run;
    for (int i = 0; i < argc; ++i)
    {
        suites_to_run.push_back(argv[i]);
    }

    pw::unit_test::SetTestSuitesToRun(suites_to_run);

    LOG_INF("Starting CHIP tests!");
    int status = 0;
    status += chip::test::RunAllTests();
    LOG_INF("CHIP test status: %d", status);

    _exit(status);
}
