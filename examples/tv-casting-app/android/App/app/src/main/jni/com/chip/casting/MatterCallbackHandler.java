/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package com.chip.casting;

public abstract class MatterCallbackHandler {
  public abstract void handle(MatterError err);

  public final void handle(int errorCode, String errorMessage) {
    handle(new MatterError(errorCode, errorMessage));
  }
}
