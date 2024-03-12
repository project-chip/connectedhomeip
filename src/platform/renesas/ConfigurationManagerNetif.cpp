/**
 * @copyright Copyright 2024, (C) Sensorfy B.V.
 */

#include "ConfigurationManagerImpl.h"
#include "ConfigurationManagerNetif.h"
struct netif;

void ConfigurationManagerNetif_register(struct netif* netif)
{
        static_cast<chip::DeviceLayer::ConfigurationManagerImpl&>(chip::DeviceLayer::ConfigurationMgrImpl()).RegisterNetif(netif);
}
