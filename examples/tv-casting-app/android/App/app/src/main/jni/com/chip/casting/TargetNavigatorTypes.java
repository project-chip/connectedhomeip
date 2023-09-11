/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package com.chip.casting;

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
