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

public abstract class MatterCallbackHandler {
  public abstract void handle(Status status);

  public final void handle(int errorCode, String errorMessage) {
    handle(new Status(errorCode, errorMessage));
  }

  public static class Status {
    private int errorCode;
    private String errorMessage;

    public static final Status SUCCESS = new Status(0, null);

    public Status(int errorCode, String errorMessage) {
      this.errorCode = errorCode;
      this.errorMessage = errorMessage;
    }

    public boolean isSuccess() {
      return this.equals(SUCCESS);
    }

    public int getErrorCode() {
      return errorCode;
    }

    public String getErrorMessage() {
      return errorMessage;
    }

    @Override
    public String toString() {
      return "Status{"
          + (isSuccess()
              ? "Success"
              : "errorCode=" + errorCode + ", errorMessage='" + errorMessage + '\'')
          + '}';
    }

    @Override
    public boolean equals(Object o) {
      if (this == o) return true;
      if (o == null || getClass() != o.getClass()) return false;
      Status status = (Status) o;
      return errorCode == status.errorCode;
    }

    @Override
    public int hashCode() {
      return Objects.hash(errorCode);
    }
  }
}
