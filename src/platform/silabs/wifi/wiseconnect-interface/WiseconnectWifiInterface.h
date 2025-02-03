/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#pragma once

#include <platform/silabs/wifi/WifiInterface.h>
#include <sl_status.h>

#define WFX_RSI_DHCP_POLL_INTERVAL (250) /* Poll interval in ms for DHCP */

enum class WifiPlatformEvent : uint8_t
{
    kStationConnect    = 0,
    kStationDisconnect = 1,
    kAPStart           = 2,
    kAPStop            = 3,
    kScan              = 4, /* This combines the scan start and scan result events  */
    kStationStartJoin  = 5,
    kStationDoDhcp     = 6,
    kStationDhcpDone   = 7,
    kStationDhcpPoll   = 8,
};

/**
 * @brief Function calls the underlying platforms disconnection API.
 *
 * @note This abstraction layer here is used to reduce the duplication for wiseconnect platforms.
 *       Since the only difference is the disconnection API, the common implementation is in the WiseconnectWifiInterface
 *       which calls this abstraction function that is implemented by the different platforms.
 *
 * @return sl_status_t SL_STATUS_OK, the Wi-Fi disconnection was succesfully triggered
 *                     SL_STATUS_FAILURE, otherwise
 */
sl_status_t TriggerPlatformWifiDisconnection();

/**
 * @brief Callback function for the DHCP timer event.
 *
 * TODO: Once the class structure is done, move this to the protected section. Should not be public.
 */
void DHCPTimerEventHandler(void * arg);

/**
 * @brief Function cancels the DHCP timer if it is running.
 *        If the timer isn't running, function doesn't do anything.
 *
 * TODO: Once the class structure is done, move this to the protected section. Should not be public.
 */
void CancelDHCPTimer(void);

/**
 * @brief Function starts the DHCP timer with the given timeout.
 *
 * TODO: Once the class structure is done, move this to the protected section. Should not be public.
 *
 * @param timeout timer duration in milliseconds
 */
void StartDHCPTimer(uint32_t timeout);

/**
 * @brief Reset the flags that are used to notify the application about DHCP connectivity
 *        and emits a WifiPlatformEvent::kStationDoDhcp event to trigger DHCP polling checks.
 *
 * TODO: This function should be moved to the protected section once the class structure is done.
 */
void ResetDHCPNotificationFlags();

/**
 * @brief Function creates the DHCP timer
 *
 * @note This function is necessary for the time being since the WifiInterface don't leverage inheritance for the time being and as
 *       such don't have access to all data structures. Once the class structure is done, this function will not be necessary
 *       anymore.
 *
 * @return sl_status_t SL_STATUS_OK, the timer was successfully created
 */
sl_status_t CreateDHCPTimer();

/**
 * @brief Notify the application about the connectivity status if it has not been notified yet.
 *
 * TODO: This function should be moved to the protected section once the class structure is done.
 */
void NotifyConnectivity(void);

/**
 * @brief Posts an event to the Wi-Fi task
 *
 * TODO: Once the class structure is in place, the function implementation can be in the protected section of this class instead of
 *       implemented twice.
 *
 * @param[in] event Event to process.
 */
void PostWifiPlatformEvent(WifiPlatformEvent event);

/**
 * @brief Main worker function for the Matter Wi-Fi task responsible of processing Wi-Fi platform events.
 *        Function is used in the StartWifiTask.
 *
 * @param[in] arg context pointer
 */
void MatterWifiTask(void * arg);
