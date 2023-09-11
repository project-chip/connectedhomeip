/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package com.matter.tv.server.tvapp;

public class UserPrompterResolver {

  protected static final String TAG = "UserPrompterResolver";

  public native void OnPinCodeEntered(int pinCode);

  public native void OnPinCodeDeclined();

  public native void OnPromptAccepted();

  public native void OnPromptDeclined();

  static {
    System.loadLibrary("TvApp");
  }
}
