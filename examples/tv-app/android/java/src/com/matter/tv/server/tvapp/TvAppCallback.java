/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package com.matter.tv.server.tvapp;

public interface TvAppCallback {
  void onClusterInit(TvApp app, long clusterId, int endpoint);
}
