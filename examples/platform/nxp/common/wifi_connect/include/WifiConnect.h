/*
 *  Copyright 2023-2024 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __WIFICONNECT_H__
#define __WIFICONNECT_H__
namespace chip {
namespace NXP {
namespace App {

/*
 * Function allowing to join a Wi-Fi network based on Wi-Fi build credentials
 * Must be called after completing Wi-Fi driver initialization
 */
void WifiConnectAtboot(void);

} // namespace App
} // namespace NXP
} // namespace chip
#endif /* __WIFICONNECT_H__ */
