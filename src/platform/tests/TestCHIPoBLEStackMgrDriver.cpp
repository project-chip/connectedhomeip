/* See Project CHIP LICENSE file for licensing information. */

#include "TestCHIPoBLEStackMgr.h"
#include <stdlib.h>

int main(int argc, char * argv[])
{
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    if (argc == 2 && atoi(argv[1]) == 1)
    {
        return TestCHIPoBLEStackManager();
    }
    return 0;
#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
}
