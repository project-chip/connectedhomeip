/*
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package com.tcl.chip.chiptest;

/** Exception thrown from TestEngine. */
public class TestEngineException extends Exception {
  private static final long serialVersionUID = 1L;

  public int errorCode;

  public TestEngineException() {}

  public TestEngineException(int errorCode, String message) {
    super(message != null ? message : String.format("Error Code %d", errorCode));
    this.errorCode = errorCode;
  }
}
