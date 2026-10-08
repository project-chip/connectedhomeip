/*
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
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
#include <inet/InetInterfaceImpl.h>
#include <lib/core/CHIPConfig.h>

#if CHIP_MEMORY_SANITIZER_ENABLED
#include <sanitizer/msan_interface.h>
#endif

#if (CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK) && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
#include <net/if.h>
namespace chip {
namespace Inet {
struct if_nameindex * if_nameindexImpl()
{
    struct if_nameindex * result = if_nameindex();
#if CHIP_MEMORY_SANITIZER_ENABLED
    if (result)
    {
        // if_nameindex() lives in libc, which is not MSan-instrumented, and the structs it returns are populated
        // from kernel data MSan cannot observe. Manually unpoisoning the result is the only way to avoid false positives.
        for (size_t i = 0;; i++)
        {
            __msan_unpoison(&result[i], sizeof(result[i]));
            if (result[i].if_index == 0)
                break;
            __msan_unpoison(result[i].if_name, IF_NAMESIZE);
        }
    }
#endif
    return result;
}
void if_freenameindexImpl(struct if_nameindex * inArray)
{
    if_freenameindex(inArray);
}
} // namespace Inet
} // namespace chip
#endif
