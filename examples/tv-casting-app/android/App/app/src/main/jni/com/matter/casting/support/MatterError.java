/*
 *   Copyright (c) 2023 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

package com.matter.casting.support;

import java.util.Objects;

public class MatterError {
  private long errorCode;
  private String errorMessage;

  public static final MatterError NO_ERROR = new MatterError(0, null);

  public static final MatterError CHIP_ERROR_INVALID_ARGUMENT =
      new MatterError(0x2f, "CHIP_ERROR_INVALID_ARGUMENT");

  public static final MatterError CHIP_ERROR_INCORRECT_STATE =
      new MatterError(0x03, "CHIP_ERROR_INCORRECT_STATE");

  public MatterError(long errorCode, String errorMessage) {
    this.errorCode = errorCode;
    this.errorMessage = errorMessage;
  }

  public boolean hasError() {
    return !this.equals(NO_ERROR);
  }

  public boolean hasNoError() {
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
        + (hasNoError()
            ? "No error"
            : "errorCode=" + errorCode + ", errorMessage='" + errorMessage + '\'')
        + '}';
  }

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;
    MatterError matterError = (MatterError) o;
    return errorCode == matterError.getErrorCode();
  }

  @Override
  public int hashCode() {
    return Objects.hash(errorCode);
  }
}
