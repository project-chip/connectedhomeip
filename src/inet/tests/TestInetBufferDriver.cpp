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
 *      This file implements a standalone/native program executable
 *      test driver for the CHIP Internet (inet) library buffer unit
 *      tests.
 *
 */

#include "TestInetLayer.h"

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/tcpip.h>
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#include <nlunit-test.h>

#include <inet/InetConfig.h>

int main(void)
{
#if INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

    // Init lwip
#if INET_LWIP
    tcpip_init(NULL, NULL);
#endif

    // Generate machine-readable, comma-separated value (CSV) output.
    nlTestSetOutputStyle(OUTPUT_CSV);

#endif // INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

    return (TestInetBuffer());
}
