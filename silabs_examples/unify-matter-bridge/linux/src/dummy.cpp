
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <platform/Linux/NetworkCommissioningDriver.h>

using namespace chip;
using namespace chip::app;
using namespace chip::DeviceLayer;

/*constexpr EndpointId kNetworkCommissioningEndpointMain      = 0;
NetworkCommissioning::LinuxEthernetDriver sLinuxEthernetDriver;
Clusters::NetworkCommissioning::Instance sEthernetNetworkCommissioningInstance(kNetworkCommissioningEndpointMain,
                                                                               &sLinuxEthernetDriver);
*/
void ApplicationInit()
{
    // sEthernetNetworkCommissioningInstance.Init();
}

void ApplicationShutdown() {}

void MatterThreadNetworkDiagnosticsPluginServerInitCallback() {}