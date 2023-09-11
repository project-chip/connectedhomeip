/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package com.matter.tv.server.tvapp;

public interface WakeOnLanManager {

  /**
   * Get Wake On Lan Mac address for endpoint
   *
   * @return Mac address in AA:BB:CC:DD:EE
   */
  String getMac();
}
