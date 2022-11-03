/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package com.matter.tv.server.tvapp;

public interface TvAppCallback {
  void onClusterInit(TvApp app, int clusterId, int endpoint);
}
