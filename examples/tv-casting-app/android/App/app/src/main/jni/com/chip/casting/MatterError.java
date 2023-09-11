/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package com.chip.casting;

import java.util.Objects;

public class MatterError {
  private long errorCode;
  private String errorMessage;

  public static final MatterError DISCOVERY_SERVICE_LOST =
      new MatterError(4L, "Discovery service was lost.");

  public static final MatterError NO_ERROR = new MatterError(0, null);

  public MatterError(long errorCode, String errorMessage) {
    this.errorCode = errorCode;
    this.errorMessage = errorMessage;
  }

  public boolean isNoError() {
    return this.equals(NO_ERROR);
  }

  public long getErrorCode() {
    return errorCode;
  }

  public String getErrorMessage() {
    return errorMessage;
  }

  @Override
  public String toString() {
    return "MatterError{"
        + (isNoError()
            ? "No error"
            : "errorCode=" + errorCode + ", errorMessage='" + errorMessage + '\'')
        + '}';
  }

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;
    MatterError matterError = (MatterError) o;
    return errorCode == matterError.errorCode;
  }

  @Override
  public int hashCode() {
    return Objects.hash(errorCode);
  }
}
