// Morse Micro's customization of the application configuration.
// Inherit from the original configuration.

/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    All rights reserved.
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
 *          Example project configuration file for CHIP.
 *
 *          This is a place to put application or project-specific overrides
 *          to the default configuration values for general CHIP features.
 *
 */

#pragma once

// include the CHIPProjectConfig from config/standalone
#include <CHIPProjectConfig.h>

#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY 0

// Bulbs do not typically use this - enabled so we can use shell to discover commissioners
#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT 1

#define CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY 1

#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DEVICE_TYPE 1
#undef CHIP_DEVICE_CONFIG_DEVICE_TYPE
#define CHIP_DEVICE_CONFIG_DEVICE_TYPE 0x0092 // Proxy Commissioner 146

#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DEVICE_NAME 1

#define CHIP_DEVICE_ENABLE_PORT_PARAMS 1

#undef CHIP_DEVICE_CONFIG_DEVICE_NAME
#define CHIP_DEVICE_CONFIG_DEVICE_NAME "Morse Micro Proxy Commissioner"

// Sleep to allow the Wi-Fi to settle
// udhcpc by default rebroadcasts DISCOVER after the following times:
// ~0.5s
// ~5s
// ~10s
// ~15s
// and empirical testing in the EU domain shows a lease is typically acquired
// between 15.5s and 30.5s
// The following is inspired by https://bitbucket.org/morsemicro/morse-linux-one-file-config/src/master/morse/scripts/morse_init.sh
// -t 60: Send up to 60 DHCP DISCOVER packets - because we know Wi-Fi should be up
// -T 2 : Pause between packets - 2 seconds, because we know Wi-Fi should be up
// -f   : Keep in foreground so that debug continues to come out to the console
#define CHIP_DEVICE_CONFIG_LINUX_DHCPC_CMD "( udhcpc -B -t 60 -T 2 -f -i %s ) &"

// Leave Concurrent mode on until support percolates down from the
// major commissioner vendors. Otherwise we will have compatibility issues
#define CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION 1

#define CHIP_DEVICE_CONFIG_MIMIC_CONCURRENT 0

#define CHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME "mlan0"

/**
 * MM_ADDITIONAL_DELAYED_ACTION_TIME
 *
 * Additional delay to add to the delayedActionTime IE received
 * in OTA messages.
 */
#define MM_ADDITIONAL_DELAYED_ACTION_TIME 0

/**
 * CHIP_CONFIG_MM_SUBSCRIPTION_OPTIMISATION
 *
 * For test TC-IDM-6.2 account for the time to reboot and reconnect when persistence
 * is enabled and a start-up subscription is in use
 *
 */
#define CHIP_CONFIG_MM_SUBSCRIPTION_OPTIMISATION 0

/**
 * CHIP_CONFIG_MM_SUBSCRIBE_REBOOT_MAX
 *
 * For test TC-IDM-6.2 this is the number of subscriptions that having timing
 * saved over a reset
 *
 */
#define CHIP_CONFIG_MM_SUBSCRIBE_REBOOT_MAX 5

/**
 * CHIP_DEVICE_CONFIG, Default channel Frequency in MHZ. S1G needs to override
 * the 24G channel Linux definition here to avoid modifying the core file
 * src/platform/Linux/ConnectivityManagerImpl.cpp. However for production this
 * will require a change to the core file as S1G will not have a default
 * frequency due to not having a harmonise world wide band.
 *
 * As wpa_supplicant currently override the 5GHz frequency, we have defaulted
 * to:
 *    5660 = 902.5MHz, US, Lowest S1G Freq in regulatory domain
 *    5660 = 863.5MHz, EU, Lowest S1G Freq in regulatory domain
 *       EU Covers 28 counties including GB
 *    5660 = 918.5MHz, KR, Lowest S1G Freq in regulatory domain
 *    AU, NZ, IN, JP, SG do not use this band. This is lowest common
 *    denominator for a default
 *
 */
//#define CHIP_DEVICE_CONFIG_WIFIPAF_24G_DEFAUTL_CHNL 5660

#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONING_PROXY 1