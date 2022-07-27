/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <platform/Tizen/ConnectivityUtils.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

// XXX: This is a workaround for a bug in the Tizen SDK header files. It is not
//      possible to include both <net/if.h> and <linux/if.h> at the same time.
//      This will cause warning that struct ifmap is redefined. On Linux, this
//      is not a problem, because in <linux/if.h> the struct is guarded with
//      ifdef. To prevent this, we will define _LINUX_IF_H, so the <linux/if.h>
//      will not be included.
#define _LINUX_IF_H

#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <linux/wireless.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

using namespace ::chip::app::Clusters::GeneralDiagnostics;

namespace chip {
namespace DeviceLayer {
namespace Internal {

InterfaceType ConnectivityUtils::GetInterfaceConnectionType(const char * ifname)
{
    InterfaceType ret = InterfaceType::EMBER_ZCL_INTERFACE_TYPE_UNSPECIFIED;
    int sock          = -1;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        ChipLogError(DeviceLayer, "Failed to open socket");
        return ret;
    }

    // Test wireless extensions for CONNECTION_WIFI
    struct iwreq pwrq = {};
    strncpy(pwrq.ifr_name, ifname, IFNAMSIZ - 1);

    if (ioctl(sock, SIOCGIWNAME, &pwrq) != -1)
    {
        ret = InterfaceType::EMBER_ZCL_INTERFACE_TYPE_WI_FI;
    }
    else if ((strncmp(ifname, "en", 2) == 0) || (strncmp(ifname, "eth", 3) == 0))
    {
        struct ethtool_cmd ecmd = {};
        ecmd.cmd                = ETHTOOL_GSET;
        struct ifreq ifr        = {};
        ifr.ifr_data            = reinterpret_cast<char *>(&ecmd);
        strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);

        if (ioctl(sock, SIOCETHTOOL, &ifr) != -1)
            ret = InterfaceType::EMBER_ZCL_INTERFACE_TYPE_ETHERNET;
    }

    close(sock);

    return ret;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
