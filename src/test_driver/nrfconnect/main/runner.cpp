/* See Project CHIP LICENSE file for licensing information. */


#include <platform/CHIPDeviceLayer.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/UnitTestRegistration.h>

#include <logging/log.h>
#include <settings/settings.h>

using namespace ::chip;
using namespace ::chip::DeviceLayer;

LOG_MODULE_REGISTER(runner);

void main(void)
{
    VerifyOrDie(settings_subsys_init() == 0);

    LOG_INF("Starting CHIP tests!");
    int status = RunRegisteredUnitTests();
    LOG_INF("CHIP test status: %d", status);

    exit(status);
}
