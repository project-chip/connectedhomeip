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

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
#include <ifaddrs.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <net/if.h>
namespace chip {
namespace Inet {

using namespace chip;

void if_freenameindexImpl(struct if_nameindex * inArray)
{
    if (inArray == nullptr)
    {
        return;
    }

    for (size_t i = 0; inArray[i].if_index != 0; i++)
    {
        if (inArray[i].if_name != nullptr)
        {
            Platform::MemoryFree(inArray[i].if_name);
        }
    }

    Platform::MemoryFree(inArray);
}

struct if_nameindex * if_nameindexImpl()
{
    int err;
    size_t intfIter              = 0;
    size_t maxIntfNum            = 0;
    size_t numIntf               = 0;
    size_t numAddrs              = 0;
    struct if_nameindex * retval = nullptr;
    struct if_nameindex * tmpval = nullptr;
    struct ifaddrs * addrList    = nullptr;
    struct ifaddrs * addrIter    = nullptr;
    const char * lastIntfName    = "";

    err = getifaddrs(&addrList);
    VerifyOrExit(err >= 0, );

    // coalesce on consecutive interface names
    for (addrIter = addrList; addrIter != nullptr; addrIter = addrIter->ifa_next)
    {
        numAddrs++;
        if (strcmp(addrIter->ifa_name, lastIntfName) != 0)
        {
            numIntf++;
            lastIntfName = addrIter->ifa_name;
        }
    }

    tmpval = (struct if_nameindex *) Platform::MemoryAlloc((numIntf + 1) * sizeof(struct if_nameindex));
    VerifyOrExit(tmpval != nullptr, );
    memset(tmpval, 0, (numIntf + 1) * sizeof(struct if_nameindex));

    lastIntfName = "";
    for (addrIter = addrList; addrIter != nullptr; addrIter = addrIter->ifa_next)
    {
        if (strcmp(addrIter->ifa_name, lastIntfName) == 0)
        {
            continue;
        }

        unsigned index = if_nametoindex(addrIter->ifa_name);
        if (index != 0)
        {
            tmpval[intfIter].if_index = index;
            tmpval[intfIter].if_name  = strdup(addrIter->ifa_name);
            intfIter++;
        }
        lastIntfName = addrIter->ifa_name;
    }

    // coalesce on interface index
    maxIntfNum = 0;
    for (size_t i = 0; tmpval[i].if_index != 0; i++)
    {
        if (maxIntfNum < tmpval[i].if_index)
        {
            maxIntfNum = tmpval[i].if_index;
        }
    }

    retval = (struct if_nameindex *) Platform::MemoryAlloc((maxIntfNum + 1) * sizeof(struct if_nameindex));
    VerifyOrExit(retval != nullptr, );
    memset(retval, 0, (maxIntfNum + 1) * sizeof(struct if_nameindex));

    for (size_t i = 0; tmpval[i].if_index != 0; i++)
    {
        struct if_nameindex * intf = &tmpval[i];
        if (retval[intf->if_index - 1].if_index == 0)
        {
            retval[intf->if_index - 1] = *intf;
        }
        else
        {
            free(intf->if_name);
            intf->if_index = 0;
            intf->if_name  = 0;
        }
    }

    intfIter = 0;

    // coalesce potential gaps between indeces
    for (size_t i = 0; i < maxIntfNum; i++)
    {
        if (retval[i].if_index != 0)
        {
            retval[intfIter] = retval[i];
            intfIter++;
        }
    }

    for (size_t i = intfIter; i < maxIntfNum; i++)
    {
        retval[i].if_index = 0;
        retval[i].if_name  = nullptr;
    }

exit:
    if (tmpval != nullptr)
    {
        Platform::MemoryFree(tmpval);
    }

    if (addrList != nullptr)
    {
        freeifaddrs(addrList);
    }

    return retval;
}
} // namespace Inet
} // namespace chip
#endif
