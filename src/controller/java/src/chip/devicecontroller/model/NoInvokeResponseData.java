/*
 *   Copyright (c) 2024 Project CHIP Authors
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
package chip.devicecontroller.model;

import java.util.Locale;
import java.util.logging.Logger;

/** Class for tracking failed invoke response data. */
public final class NoInvokeResponseData {
  private static final Logger logger = Logger.getLogger(NoInvokeResponseData.class.getName());
  private final Integer commandRef;

  private NoInvokeResponseData(int commandRef) {
    this.commandRef = commandRef;
  }

  public Integer getCommandRef() {
    return commandRef;
  }

  public String toString() {
    return String.format(Locale.ENGLISH, "commandRef %s", commandRef.toString());
  }

  public static NoInvokeResponseData newInstance(int commandRef) {
    return new NoInvokeResponseData(commandRef);
  }
}
