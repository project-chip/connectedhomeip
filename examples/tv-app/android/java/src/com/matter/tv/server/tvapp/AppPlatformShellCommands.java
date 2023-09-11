/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package com.matter.tv.server.tvapp;

public class AppPlatformShellCommands {

  protected static final String TAG = "AppPlatformShellCommands";

  public native String OnExecuteCommand(String[] argv);

  static {
    System.loadLibrary("TvApp");
  }
}
