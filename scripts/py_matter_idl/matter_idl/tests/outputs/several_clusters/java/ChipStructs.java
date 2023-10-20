/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
package chip.devicecontroller;

import javax.annotation.Nullable;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Optional;

public class ChipStructs {
public static class SecondClusterFabricDescriptorStruct {
  public byte[] rootPublicKey;
  public Integer vendorID;
  public Long fabricID;
  public Long nodeID;
  public String label;
  public Integer fabricIndex;
  public SecondClusterFabricDescriptorStruct(
    byte[] rootPublicKey,
    Integer vendorID,
    Long fabricID,
    Long nodeID,
    String label,
    Integer fabricIndex
  ) {
    this.rootPublicKey = rootPublicKey;
    this.vendorID = vendorID;
    this.fabricID = fabricID;
    this.nodeID = nodeID;
    this.label = label;
    this.fabricIndex = fabricIndex;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("SecondClusterFabricDescriptorStruct {\n");
    output.append("\trootPublicKey: ");
    output.append(Arrays.toString(rootPublicKey));
    output.append("\n");
    output.append("\tvendorID: ");
    output.append(vendorID);
    output.append("\n");
    output.append("\tfabricID: ");
    output.append(fabricID);
    output.append("\n");
    output.append("\tnodeID: ");
    output.append(nodeID);
    output.append("\n");
    output.append("\tlabel: ");
    output.append(label);
    output.append("\n");
    output.append("\tfabricIndex: ");
    output.append(fabricIndex);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
}
