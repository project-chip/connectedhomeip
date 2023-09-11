/*
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package chip.devicecontroller;

/** Exception class holding error codes defined by clusters. */
public class ChipClusterException extends Exception {
  private static final long serialVersionUID = 1L;

  public long errorCode;

  public ChipClusterException() {}

  public ChipClusterException(long errorCode) {
    super(String.format("CHIP cluster error: %d", errorCode));
    this.errorCode = errorCode;
  }
}
