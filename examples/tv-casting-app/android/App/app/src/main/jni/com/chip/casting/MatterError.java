/*
 *   Copyright (c) 2022 Project CHIP Authors
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
