/*$Id: //dwh/bluetooth/DWC_ble154combo/firmware/rel/2.00a-lca01/firmware/public_inc/ll_fw_config.h#1 $*/
/**
 ********************************************************************************
 * @file    ll_fw_config.h
 * @brief   This file contains the major configurations to the BLE controller.
 ******************************************************************************
 * @copy
 * This Synopsys DWC Bluetooth Low Energy Combo Link Layer/MAC software and
 * associated documentation ( hereinafter the "Software") is an unsupported
 * proprietary work of Synopsys, Inc. unless otherwise expressly agreed to in
 * writing between Synopsys and you. The Software IS NOT an item of Licensed
 * Software or a Licensed Product under any End User Software License Agreement
 * or Agreement for Licensed Products with Synopsys or any supplement thereto.
 * Synopsys is a registered trademark of Synopsys, Inc. Other names included in
 * the SOFTWARE may be the trademarks of their respective owners.
 *
 * Synopsys MIT License:
 * Copyright (c) 2020-Present Synopsys, Inc
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * the Software), to deal in the Software without restriction, including without
 * limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING, BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT, OR OTHERWISE ARISING FROM,
 * OUT OF, OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * */
#ifndef INCLUDE_LL_FW_CONFIG_H
#define INCLUDE_LL_FW_CONFIG_H

/*************************** General Configuration *********************************/
#ifndef SUPPORT_GNRC_SCHDLR_IF
#define SUPPORT_GNRC_SCHDLR_IF                      1 /* Enable\Disable event EXTRNL_GNRC in Ble event manager. Enable:1 - Disable:0 */
#endif /* SUPPORT_GNRC_SCHDLR_IF */

/*************************** BLE Configuration *************************************/
/*Configurations of BLE will apply only when BLE is enabled*/
/* Roles configurations */
#ifndef SUPPORT_EXPLCT_OBSERVER_ROLE
#define SUPPORT_EXPLCT_OBSERVER_ROLE                1 /* Enable\Disable Explicit observer role. Enable:1 - Disable:0 */
#endif /* SUPPORT_EXPLCT_OBSERVER_ROLE */

#ifndef SUPPORT_EXPLCT_BROADCASTER_ROLE
#define SUPPORT_EXPLCT_BROADCASTER_ROLE             1 /* Enable\Disable Explicit broadcaster role. Enable:1 - Disable:0 */
#endif /* SUPPORT_EXPLCT_BROADCASTER_ROLE */

#ifndef SUPPORT_MASTER_CONNECTION
#define SUPPORT_MASTER_CONNECTION                   1 /* Enable\Disable Master connection role. Enable:1 - Disable:0 */
#endif /* SUPPORT_MASTER_CONNECTION */

#ifndef SUPPORT_SLAVE_CONNECTION
#define SUPPORT_SLAVE_CONNECTION                    1 /* Enable\Disable Slave connection role. Enable:1 - Disable:0 */
#endif /* SUPPORT_SLAVE_CONNECTION */

/* Standard features configurations */
#ifndef SUPPORT_LE_ENCRYPTION
#define SUPPORT_LE_ENCRYPTION                       1 /* Enable\Disable Encryption feature. Enable:1 - Disable:0 */
#endif /* SUPPORT_LE_ENCRYPTION */

#ifndef SUPPORT_PRIVACY
#define SUPPORT_PRIVACY                             1 /* Enable\Disable Privacy feature. Enable:1 - Disable:0 */
#endif /* SUPPORT_PRIVACY */

#ifndef SUPPORT_LE_EXTENDED_ADVERTISING
#define SUPPORT_LE_EXTENDED_ADVERTISING             0 /* Enable\Disable Extended advertising feature. Enable:1 - Disable:0 */
#endif /* SUPPORT_LE_EXTENDED_ADVERTISING */

#ifndef SUPPORT_LE_PERIODIC_ADVERTISING
#define SUPPORT_LE_PERIODIC_ADVERTISING             0 /* Enable\Disable Periodic advertising feature. Enable:1 - Disable:0 */
#endif /* SUPPORT_LE_PERIODIC_ADVERTISING */

#ifndef SUPPORT_LE_POWER_CLASS_1
#define SUPPORT_LE_POWER_CLASS_1                    0 /* Enable\Disable Low power class 1 feature. Enable:1 - Disable:0 */
#endif /* SUPPORT_LE_POWER_CLASS_1 */

#ifndef SUPPORT_AOA_AOD
#define SUPPORT_AOA_AOD                             0 /* Enable\Disable AOA_AOD feature. Enable:1 - Disable:0 */
#endif /* SUPPORT_AOA_AOD */

#ifndef SUPPORT_PERIODIC_SYNC_TRANSFER
#define SUPPORT_PERIODIC_SYNC_TRANSFER              0 /* Enable\Disable PAST feature. Enable:1 - Disable:0 */
#endif /* SUPPORT_PERIODIC_SYNC_TRANSFER */

#ifndef SUPPORT_SLEEP_CLOCK_ACCURCY_UPDATES
#define SUPPORT_SLEEP_CLOCK_ACCURCY_UPDATES         1 /* Enable\Disable Sleep Clock Accuracy Updates Feature. Enable:1 - Disable:0 */
#endif /* SUPPORT_SLEEP_CLOCK_ACCURCY_UPDATES */

#ifndef SUPPORT_CONNECTED_ISOCHRONOUS
#define SUPPORT_CONNECTED_ISOCHRONOUS               0 /* Enable\Disable Connected Isochronous Channel Feature. Enable:1 - Disable:0 */
#endif /* SUPPORT_CONNECTED_ISOCHRONOUS */

#ifndef SUPPORT_BRD_ISOCHRONOUS
#define SUPPORT_BRD_ISOCHRONOUS                     0 /* Enable\Disable Broadcast Isochronous Channel Feature. Enable:1 - Disable:0 */
#endif /* SUPPORT_BRD_ISOCHRONOUS */

#ifndef SUPPORT_SYNC_ISOCHRONOUS
#define SUPPORT_SYNC_ISOCHRONOUS                    0 /* Enable\Disable Broadcast Isochronous Synchronizer Channel Feature. Enable:1 - Disable:0 */
#endif /* SUPPORT_SYNC_ISOCHRONOUS */

#ifndef SUPPORT_LE_POWER_CONTROL
#define SUPPORT_LE_POWER_CONTROL                    0 /* Enable\Disable LE Power Control Feature. Enable:1 - Disable:0 */
#endif /* SUPPORT_LE_POWER_CONTROL */

#ifndef SUPPORT_CHANNEL_CLASSIFICATION
#define SUPPORT_CHANNEL_CLASSIFICATION              0
#endif /* SUPPORT_CHANNEL_CLASSIFICATION */

#ifndef SUPPORT_PERIODIC_ADV_ADI
#define SUPPORT_PERIODIC_ADV_ADI                    0
#endif /* SUPPORT_PERIODIC_ADV_ADI */

#ifndef SUPPORT_LE_ENHANCED_CONN_UPDATE
#define SUPPORT_LE_ENHANCED_CONN_UPDATE             0
#endif /* SUPPORT_LE_ENHANCED_CONN_UPDATE */

#ifndef SUPPORT_CSSA
#define SUPPORT_CSSA                                0 /* Enable\Disable Coding Selection Scheme on Advertising Feature. Enable:1 - Disable:0 */
#endif /* SUPPORT_CSSA */

#ifndef SUPPORT_LE_PAWR_ADVERTISER_ROLE
#define SUPPORT_LE_PAWR_ADVERTISER_ROLE             0 /* Enable\Disable PAwR Advertiser role. Enable:1 - Disable:0 */
#endif /* SUPPORT_LE_PAWR_ADVERTISER_ROLE */

#ifndef SUPPORT_LE_PAWR_SYNC_ROLE
#define SUPPORT_LE_PAWR_SYNC_ROLE                   0 /* Enable\Disable PAwR Synchronizer role. Enable:1 - Disable:0 */
#endif /* SUPPORT_LE_PAWR_SYNC_ROLE */

#ifndef SUPPORT_CHANNEL_SOUNDING
#define SUPPORT_CHANNEL_SOUNDING                                        0 /* Enable\Disable Channel Sounding Feature.   Enable:1 - Disable:0 */
#endif /* SUPPORT_CHANNEL_SOUNDING */

#ifndef SUPPORT_FRAME_SPACE_UPDATE
#define SUPPORT_FRAME_SPACE_UPDATE                                      0 /* Enable\Disable Frame Space Update Feature. Enable:1 - Disable:0 */
#endif /* SUPPORT_FRAME_SPACE_UPDATE */

#ifndef SUPPORT_EXT_FEATURE_SET
#define SUPPORT_EXT_FEATURE_SET                                         0 /* Enable\Disable Extended Feature Set Exchange. Enable:1 - Disable:0 */
#endif /* SUPPORT_EXT_FEATURE_SET */

#ifndef SUPPORT_ISO_UNSEG_MODE
#define SUPPORT_ISO_UNSEG_MODE                                          0 /* Enable\Disable Unsegmented Mode for Framed ISO PDUs. Enable: 1 - Disable: 0*/
#endif /* SUPPORT_ISO_UNSEG_MODE */

#ifndef SUPPORT_LE_ADVERTISERS_MONITORING
#define SUPPORT_LE_ADVERTISERS_MONITORING                               0 /* Enable\Disable Advertisers Monitoring Feature. Enable:1 - Disable:0 */
#endif /* SUPPORT_LE_ADVERTISERS_MONITORING */

/* Capabilities configurations */
#ifndef MAX_NUM_CNCRT_STAT_MCHNS
#define MAX_NUM_CNCRT_STAT_MCHNS                    8 /* Set maximum number of states the controller can support */
#endif /* MAX_NUM_CNCRT_STAT_MCHNS */

#ifndef USE_NON_ACCURATE_32K_SLEEP_CLK
#define USE_NON_ACCURATE_32K_SLEEP_CLK              1 /* Allow to drive the sleep clock by sources other than the default crystal oscillator source.*/
                                                      /*LL can use crystal oscillator or RTC or RCO to drive the sleep clock.This selection is done via "DEFAULT_SLEEP_CLOCK_SOURCE" macro. */
#endif /* USE_NON_ACCURATE_32K_SLEEP_CLK */

/* Non-standard features configurations */
#ifndef NUM_OF_CTSM_EMNGR_HNDLS
#define NUM_OF_CTSM_EMNGR_HNDLS                     1 /* Number of custom handles in event manager to be used for app specific needs */
#endif /* NUM_OF_CTSM_EMNGR_HNDLS */

#ifndef SUPPORT_AUGMENTED_BLE_MODE
#define SUPPORT_AUGMENTED_BLE_MODE                  1 /* Enable\Disable Augmented BLE Support. Enable:1 - Disable:0 */
#endif /* SUPPORT_AUGMENTED_BLE_MODE */

#ifndef SUPPORT_PTA
#define SUPPORT_PTA                                 1 /* Enable\Disable PTA Feature. Enable:1 - Disable:0 */
#endif /* SUPPORT_PTA */

#ifndef SUPPORT_CONFIGURABLE_GAIN_FIX
#define SUPPORT_CONFIGURABLE_GAIN_FIX               0 /* Enable\Disable configurable gain fix support */
#endif /* SUPPORT_CONFIGURABLE_GAIN_FIX */

#ifndef CHECK_ANY_MISSED_EVENT_ON_DEEP_SLEEP_EXIT
#define CHECK_ANY_MISSED_EVENT_ON_DEEP_SLEEP_EXIT   1 /* Enable\Disable calling event scheduler handler function at the end of deep sleep exit*/
#endif /* CHECK_ANY_MISSED_EVENT_ON_DEEP_SLEEP_EXIT */

#ifndef LL_BASIC
#define LL_BASIC                                    0
#endif /* LL_BASIC */

/*************************** MAC Configuration *************************************/
/*Configurations of MAC will apply only when MAC is enabled*/
#ifndef FFD_DEVICE_CONFIG
#define FFD_DEVICE_CONFIG                           1 /* Enable\Disable FFD:1 - RFD:0 */
#endif /* FFD_DEVICE_CONFIG */

#ifndef RAL_NUMBER_OF_INSTANCE
#define RAL_NUMBER_OF_INSTANCE                      1 /* The Number of RAL instances supported */
#endif /* RAL_NUMBER_OF_INSTANCE */

#ifndef MAX_NUMBER_OF_INDIRECT_DATA
#define MAX_NUMBER_OF_INDIRECT_DATA                 10 /* The maximum number of supported indirect data buffers */
#endif /* MAX_NUMBER_OF_INDIRECT_DATA */

#ifndef SUPPORT_OPENTHREAD_1_2
#define SUPPORT_OPENTHREAD_1_2                      1 /* Enable / disable FW parts related to new features introduced in openthread 1.2*/
#endif /* SUPPORT_OPENTHREAD_1_2 */

#ifndef SUPPORT_SEC
#define SUPPORT_SEC                                 1 /* The MAC Security Supported : 1 - Not Supported:0 */
#endif /* SUPPORT_SEC */

#ifndef RADIO_CSMA
#define RADIO_CSMA                                  1 /* Enable\Disable CSMA Algorithm in Radio Layer, Must be Enabled if MAC_LAYER_BUILD */
#endif /* RADIO_CSMA */

#ifndef SUPPORT_ANT_DIV
#define SUPPORT_ANT_DIV                             1 /* Enable/Disable Antenna Diversity Feature */
#endif /* SUPPORT_ANT_DIV */

#ifndef SUPPORT_A_MAC
#define SUPPORT_A_MAC                               1
#endif /* SUPPORT_A_MAC */

#ifndef SMPL_PRTCL_TEST_ENABLE
#define SMPL_PRTCL_TEST_ENABLE                      0
#endif /* SMPL_PRTCL_TEST_ENABLE */

#ifndef IEEE_EUI64_VENDOR_SPECIFIC_FUNC
#define IEEE_EUI64_VENDOR_SPECIFIC_FUNC             1 /* Comment to disable EUI-64 vendor specific function, in this case EUI-64 is not unique */
#endif /* IEEE_EUI64_VENDOR_SPECIFIC_FUNC */

/*************************** Zigbee Configuration **********************************/
#ifndef SUPPORT_ZIGBEE_PHY_CERTIFICATION
#define SUPPORT_ZIGBEE_PHY_CERTIFICATION            0 /* Enable support of hci command required to implement zigbee phy Test cases Enable:1 - Disable:0 */
#endif /* SUPPORT_ZIGBEE_PHY_CERTIFICATION */

#endif /* INCLUDE_LL_FW_CONFIG_H */
