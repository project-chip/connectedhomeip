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

/** @deprecated Use the APIs described in /examples/tv-casting-app/APIs.md instead. */
@Deprecated
public class TargetNavigatorTypes {
  public static class TargetInfo {
    public Integer identifier;
    public String name;

    public TargetInfo(Integer identifier, String name) {
      this.identifier = identifier;
      this.name = name;
    }

    @Override
    public String toString() {
      StringBuilder output = new StringBuilder();
      output.append("TargetInfo {\n");
      output.append("\tidentifier: ");
      output.append(identifier);
      output.append("\n");
      output.append("\tname: ");
      output.append(name);
      output.append("\n");
      output.append("}\n");
      return output.toString();
    }
  }
}
