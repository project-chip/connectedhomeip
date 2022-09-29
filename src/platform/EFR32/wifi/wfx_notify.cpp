/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "em_bus.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_ldma.h"
#include "em_usart.h"
#include "gpiointerrupt.h"

#include "AppConfig.h"

#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"

#include "wfx_host_events.h"

#ifdef RS911X_WIFI
#include "wfx_rsi.h"
#endif

#include <platform/CHIPDeviceLayer.h>
//#include <app/server/Mdns.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>

using namespace ::chip;
using namespace ::chip::DeviceLayer;

/*
 * Notifications to the upper-layer
 * All done in the context of the RSI/WiFi task (rsi_if.c)
 */

/***********************************************************************************
 * @fn  wfx_started_notify()
 * @brief
 *       Wifi device started notification
 * @param[in]: None
 * @return None
 *************************************************************************************/
void wfx_started_notify()
{
    sl_wfx_startup_ind_t evt;
    sl_wfx_mac_address_t mac;

    EFR32_LOG("%s: started.", __func__);

    memset(&evt, 0, sizeof(evt));
    evt.header.id     = SL_WFX_STARTUP_IND_ID;
    evt.header.length = sizeof evt;
    evt.body.status   = 0;
    wfx_get_wifi_mac_addr(SL_WFX_STA_INTERFACE, &mac);
    memcpy(&evt.body.mac_addr[0], &mac.octet[0], MAC_ADDRESS_FIRST_OCTET);

    PlatformMgrImpl().HandleWFXSystemEvent(WIFI_EVENT, (sl_wfx_generic_message_t *) &evt);
}

/***********************************************************************************
 * @fn  void wfx_connected_notify(int32_t status, sl_wfx_mac_address_t *ap)
 * @brief
 * For now we are not notifying anything other than AP Mac -
 * Other stuff such as DTIM etc. may be required for later
 * @param[in] status:
 * @param[in] ap: access point
 * @return None
 *************************************************************************************/
void wfx_connected_notify(int32_t status, sl_wfx_mac_address_t * ap)
{
    sl_wfx_connect_ind_t evt;

    EFR32_LOG("%s: started.", __func__);

    if (status != SUCCESS_STATUS)
    {
        EFR32_LOG("%s: error: failed status: %d.", __func__, status);
        return;
    }

    EFR32_LOG("%s: connected.", __func__);

    memset(&evt, 0, sizeof(evt));
    evt.header.id     = SL_WFX_CONNECT_IND_ID;
    evt.header.length = sizeof evt;

#ifdef RS911X_WIFI
    evt.body.channel = wfx_rsi.ap_chan;
#endif
    memcpy(&evt.body.mac[0], &ap->octet[0], MAC_ADDRESS_FIRST_OCTET);

    PlatformMgrImpl().HandleWFXSystemEvent(WIFI_EVENT, (sl_wfx_generic_message_t *) &evt);
}

/**************************************************************************************
 * @fn  void wfx_disconnected_notify(int32_t status)
 * @brief
 *    notification of disconnection
 * @param[in] status:
 * @return None
 ********************************************************************************************/
void wfx_disconnected_notify(int32_t status)
{
    sl_wfx_disconnect_ind_t evt;

    EFR32_LOG("%s: started.", __func__);

    memset(&evt, 0, sizeof(evt));
    evt.header.id     = SL_WFX_DISCONNECT_IND_ID;
    evt.header.length = sizeof evt;
    evt.body.reason   = status;
    PlatformMgrImpl().HandleWFXSystemEvent(WIFI_EVENT, (sl_wfx_generic_message_t *) &evt);
}

/**************************************************************************************
 * @fn  void wfx_ipv6_notify(int got_ip)
 * @brief
 *      notification of ipv6
 * @param[in]  got_ip:
 * @return None
 ********************************************************************************************/
void wfx_ipv6_notify(int got_ip)
{
    sl_wfx_generic_message_t eventData;

    EFR32_LOG("%s: started.", __func__);

    memset(&eventData, 0, sizeof(eventData));
    eventData.header.id     = got_ip ? IP_EVENT_GOT_IP6 : IP_EVENT_STA_LOST_IP;
    eventData.header.length = sizeof(eventData.header);
    PlatformMgrImpl().HandleWFXSystemEvent(IP_EVENT, &eventData);

    /* So the other threads can run and have the connectivity OK */
    if (got_ip)
    {
        /* Should remember this */
        vTaskDelay(1);
        chip::DeviceLayer::PlatformMgr().LockChipStack();
        chip::app::DnssdServer::Instance().StartServer(/*Dnssd::CommissioningMode::kEnabledBasic*/);
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    }
}

/**************************************************************************************
 * @fn   void wfx_ip_changed_notify(int got_ip)
 * @brief
 *      notification of ip change
 * @param[in]  got_ip:
 * @return None
 ********************************************************************************************/
void wfx_ip_changed_notify(int got_ip)
{
    sl_wfx_generic_message_t eventData;

    EFR32_LOG("%s: started.", __func__);

    memset(&eventData, 0, sizeof(eventData));
    eventData.header.id     = got_ip ? IP_EVENT_STA_GOT_IP : IP_EVENT_STA_LOST_IP;
    eventData.header.length = sizeof(eventData.header);
    PlatformMgrImpl().HandleWFXSystemEvent(IP_EVENT, &eventData);

    /* So the other threads can run and have the connectivity OK */
    if (got_ip)
    {
        /* Should remember this */
        vTaskDelay(1);
        chip::DeviceLayer::PlatformMgr().LockChipStack();
        chip::app::DnssdServer::Instance().StartServer(/*Dnssd::CommissioningMode::kEnabledBasic*/);
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    }
}
