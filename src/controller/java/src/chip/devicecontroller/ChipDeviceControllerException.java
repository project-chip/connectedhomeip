/*
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package chip.devicecontroller;

/** Exception thrown from CHIPDeviceController. */
public class ChipDeviceControllerException extends RuntimeException {
  private static final long serialVersionUID = 1L;

  public long errorCode;

  public ChipDeviceControllerException() {}

  public ChipDeviceControllerException(long errorCode, String message) {
    super(message != null ? message : String.format("Error Code %d", errorCode));
    this.errorCode = errorCode;
  }
}
