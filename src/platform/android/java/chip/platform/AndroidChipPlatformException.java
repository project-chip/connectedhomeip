/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package chip.platform;

/** Exception thrown from AndroidChipPlatform. */
public class AndroidChipPlatformException extends Exception {
  private static final long serialVersionUID = 1L;

  public long errorCode;

  public AndroidChipPlatformException() {}

  public AndroidChipPlatformException(long errorCode, String message) {
    super(message != null ? message : String.format("Error Code %d", errorCode));
    this.errorCode = errorCode;
  }
}
