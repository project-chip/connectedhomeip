#include "network-commissioning.h"

#include <cstring>

#include "af.h"
#include <gen/callback.h>

#include "gen/att-storage.h"
#include "gen/attribute-id.h"
#include "gen/attribute-type.h"
#include "gen/cluster-id.h"
#include "gen/command-id.h"

/**
 * @brief Network Commissioning Cluster AddThreadNetwork Command callback
 * @param operationalDataset
 * @param breadcrumb
 * @param timeoutMs
 */

bool emberAfNetworkCommissioningClusterAddThreadNetworkCallback(uint8_t * operationalDataset, uint64_t breadcrumb,
                                                                uint32_t timeoutMs)
{
    uint8_t len = operationalDataset[0];
    memmove(operationalDataset, operationalDataset + 1, len);
    operationalDataset[len] = 0;
    chip::app::clusters::NetworkCommissioning::OnAddThreadNetworkCommandCallback(nullptr, emberAfCurrentEndpoint(),
                                                                                 operationalDataset, breadcrumb, timeoutMs);
    return true;
}

/**
 * @brief Network Commissioning Cluster AddWiFiNetwork Command callback
 * @param ssid
 * @param credentials
 * @param breadcrumb
 * @param timeoutMs
 */

bool emberAfNetworkCommissioningClusterAddWiFiNetworkCallback(uint8_t * ssid, uint8_t * credentials, uint64_t breadcrumb,
                                                              uint32_t timeoutMs)
{
    uint8_t len = ssid[0];
    memmove(ssid, ssid + 1, len);
    ssid[len] = 0;

    len = credentials[0];
    memmove(credentials, credentials + 1, len);
    credentials[len] = 0;
    chip::app::clusters::NetworkCommissioning::OnAddWiFiNetworkCommandCallback(nullptr, emberAfCurrentEndpoint(), ssid, credentials,
                                                                               breadcrumb, timeoutMs);
    return true;
}

/**
 * @brief Network Commissioning Cluster EnableNetwork Command callback
 * @param networkID
 * @param breadcrumb
 * @param timeoutMs
 */

bool emberAfNetworkCommissioningClusterEnableNetworkCallback(uint8_t * networkID, uint64_t breadcrumb, uint32_t timeoutMs)
{
    uint8_t len = networkID[0];
    memmove(networkID, networkID + 1, len);
    networkID[len] = 0;
    chip::app::clusters::NetworkCommissioning::OnEnableNetworkCommandCallback(nullptr, emberAfCurrentEndpoint(), networkID,
                                                                              breadcrumb, timeoutMs);
    return true;
}

// TODO: The following commands needed to be implemented.

/**
 * @brief Network Commissioning Cluster DisableNetwork Command callback
 * @param networkID
 * @param breadcrumb
 * @param timeoutMs
 */

bool emberAfNetworkCommissioningClusterDisableNetworkCallback(uint8_t * networkID, uint64_t breadcrumb, uint32_t timeoutMs)
{
    return false;
}

/**
 * @brief Network Commissioning Cluster GetLastNetworkCommissioningResult Command callback
 * @param timeoutMs
 */

bool emberAfNetworkCommissioningClusterGetLastNetworkCommissioningResultCallback(uint32_t timeoutMs)
{
    return false;
}

/**
 * @brief Network Commissioning Cluster RemoveNetwork Command callback
 * @param networkID
 * @param breadcrumb
 * @param timeoutMs
 */

bool emberAfNetworkCommissioningClusterRemoveNetworkCallback(uint8_t * NetworkID, uint64_t Breadcrumb, uint32_t TimeoutMs)
{
    return false;
}

/**
 * @brief Network Commissioning Cluster ScanNetworks Command callback
 * @param ssid
 * @param breadcrumb
 * @param timeoutMs
 */

bool emberAfNetworkCommissioningClusterScanNetworksCallback(uint8_t * ssid, uint64_t breadcrumb, uint32_t timeoutMs)
{
    return false;
}

/**
 * @brief Network Commissioning Cluster UpdateThreadNetwork Command callback
 * @param operationalDataset
 * @param breadcrumb
 * @param timeoutMs
 */

bool emberAfNetworkCommissioningClusterUpdateThreadNetworkCallback(uint8_t * operationalDataset, uint64_t breadcrumb,
                                                                   uint32_t timeoutMs)
{
    return false;
}

/**
 * @brief Network Commissioning Cluster UpdateWiFiNetwork Command callback
 * @param ssid
 * @param credentials
 * @param breadcrumb
 * @param timeoutMs
 */

bool emberAfNetworkCommissioningClusterUpdateWiFiNetworkCallback(uint8_t * ssid, uint8_t * credentials, uint64_t breadcrumb,
                                                                 uint32_t timeoutMs)
{
    return false;
}
