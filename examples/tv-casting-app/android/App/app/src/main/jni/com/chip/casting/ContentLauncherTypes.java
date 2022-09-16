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
