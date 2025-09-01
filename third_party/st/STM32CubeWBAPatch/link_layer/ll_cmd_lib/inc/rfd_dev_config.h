/*$Id: //dwh/bluetooth/DWC_ble154combo/firmware/rel/2.00a-lca01/firmware/public_inc/rfd_dev_config.h#1 $*/
/**
 ********************************************************************************
 * @file    rfd_dev_config.h
 * @brief Configurations for reduced function device and full function device
 *
 *
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


#ifndef MAC_CONTROLLER_INC_RFD_DEV_CONFIG_H_
#define MAC_CONTROLLER_INC_RFD_DEV_CONFIG_H_

#include "ll_fw_config.h"


#if (!FFD_DEVICE_CONFIG)  /* RFD Device Configuration */

#define RFD_SUPPORT_ACTIVE_SCAN						1 /* Enable\Disable :RFD supports Active Scanning Enable:1 - Disable:0 */
#define RFD_SUPPORT_ENERGY_DETECT					1 /* Enable\Disable :RFD supports Energy Detection Enable:1 - Disable:0 */
#define RFD_SUPPORT_DATA_PURGE						0 /* Enable\Disable :RFD supports Data Purge Primitive Enable:1 - Disable:0 */
#define RFD_SUPPORT_ASSOCIATION_IND_RSP				0 /* Enable\Disable :RFD supports Association Indication and Response Primitives Enable:1 - Disable:0 */
#define RFD_SUPPORT_ORPHAN_IND_RSP					1 /* Enable\Disable :RFD supports Orphan Indication and Response Primitives Enable:1 - Disable:0 */
#define RFD_SUPPORT_START_PRIM						1 /* Enable\Disable :RFD supports Start Primitive Enable:1 - Disable:0 */
#define RFD_SUPPORT_PROMISCUOUS_MODE				1 /* Enable\Disable :RFD supports Promiscuous Mode Enable:1 - Disable:0 */
#define RFD_SUPPORT_SEND_BEACON						1 /* Enable\Disable :RFD supports Sending Beacons if Coordinator Enable:1 - Disable:0 */
#define RFD_SUPPORT_PANID_CONFLICT_RSLN				1 /* Enable\Disable :RFD supports Pan Id conflict detection and resolution Enable:1 - Disable:0 */

#endif


#endif /* MAC_CONTROLLER_INC_RFD_DEV_CONFIG_H_ */
