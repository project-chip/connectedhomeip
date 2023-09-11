/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package com.chip.casting;

import java.util.ArrayList;
import java.util.Optional;

public class ContentLauncherTypes {
  public static class AdditionalInfo {
    public String name;
    public String value;

    public AdditionalInfo(String name, String value) {
      this.name = name;
      this.value = value;
    }

    @Override
    public String toString() {
      StringBuilder output = new StringBuilder();
      output.append("AdditionalInfo {\n");
      output.append("\tname: ");
      output.append(name);
      output.append("\n");
      output.append("\tvalue: ");
      output.append(value);
      output.append("\n");
      output.append("}\n");
      return output.toString();
    }
  }

  public static class Parameter {
    public Integer type;
    public String value;
    public Optional<ArrayList<AdditionalInfo>> externalIDList;

    public Parameter(
        Integer type, String value, Optional<ArrayList<AdditionalInfo>> externalIDList) {
      this.type = type;
      this.value = value;
      this.externalIDList = externalIDList;
    }

    @Override
    public String toString() {
      StringBuilder output = new StringBuilder();
      output.append("Parameter {\n");
      output.append("\ttype: ");
      output.append(type);
      output.append("\n");
      output.append("\tvalue: ");
      output.append(value);
      output.append("\n");
      output.append("\texternalIDList: ");
      output.append(externalIDList);
      output.append("\n");
      output.append("}\n");
      return output.toString();
    }
  }

  public static class ContentSearch {
    public ArrayList<Parameter> parameterList;

    public ContentSearch(ArrayList<Parameter> parameterList) {
      this.parameterList = parameterList;
    }

    @Override
    public String toString() {
      StringBuilder output = new StringBuilder();
      output.append("ContentSearch {\n");
      output.append("\tparameterList: ");
      output.append(parameterList);
      output.append("\n");
      output.append("}\n");
      return output.toString();
    }
  }
}
