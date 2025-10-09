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
package chip.devicecontroller;

public enum DiagnosticLogType {
  EndUserSupport(0),
  NetworkDiagnostics(1),
  CrashLogs(2);

  private final int value;

  DiagnosticLogType(int value) {
    this.value = value;
  }

  public static DiagnosticLogType value(int value) {
    for (DiagnosticLogType type : DiagnosticLogType.values()) {
      if (type.value == value) {
        return type;
      }
    }
    throw new IllegalArgumentException("Invalid value: " + value);
  }

  public static DiagnosticLogType value(String value) {
    for (DiagnosticLogType type : DiagnosticLogType.values()) {
      if (type.toString().equals(value)) {
        return type;
      }
    }
    throw new IllegalArgumentException("Invalid value: " + value);
  }

  public int getValue() {
    return value;
  }
}
