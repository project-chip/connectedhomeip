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

import chip.clusterinfo.CommandParameterInfo;
import chip.clusterinfo.InteractionInfo;
import chip.devicecontroller.ChipClusters.DefaultClusterCallback;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.Map;

public class ClusterWriteMapping {
  public Map<String, Map<String, InteractionInfo>> getWriteAttributeMap() {
    Map<String, Map<String, InteractionInfo>> writeAttributeMap = new HashMap<>();
    Map<String, InteractionInfo> writeFirstInteractionInfo = new LinkedHashMap<>();
    Map<String, CommandParameterInfo> writeFirstSomeIntegerCommandParams = new LinkedHashMap<String, CommandParameterInfo>();
    CommandParameterInfo firstsomeIntegerCommandParameterInfo =
        new CommandParameterInfo(
            "value", 
            Integer.class, 
            Integer.class 
        );
    writeFirstSomeIntegerCommandParams.put(
        "value",
        firstsomeIntegerCommandParameterInfo
    );
    InteractionInfo writeFirstSomeIntegerAttributeInteractionInfo = new InteractionInfo(
      (cluster, callback, commandArguments) -> {
        ((ChipClusters.FirstCluster) cluster).writeSomeIntegerAttribute(
          (DefaultClusterCallback) callback,
          (Integer) commandArguments.get("value")
        );
      },
      () -> new ClusterInfoMapping.DelegatedDefaultClusterCallback(),
      writeFirstSomeIntegerCommandParams
    );
    writeFirstInteractionInfo.put("writeSomeIntegerAttribute", writeFirstSomeIntegerAttributeInteractionInfo);
    writeAttributeMap.put("first", writeFirstInteractionInfo);
    Map<String, InteractionInfo> writeSecondInteractionInfo = new LinkedHashMap<>();
    writeAttributeMap.put("second", writeSecondInteractionInfo);
    Map<String, InteractionInfo> writeThirdInteractionInfo = new LinkedHashMap<>();
    Map<String, CommandParameterInfo> writeThirdSomeEnumCommandParams = new LinkedHashMap<String, CommandParameterInfo>();
    CommandParameterInfo thirdsomeEnumCommandParameterInfo =
        new CommandParameterInfo(
            "value", 
            Integer.class, 
            Integer.class 
        );
    writeThirdSomeEnumCommandParams.put(
        "value",
        thirdsomeEnumCommandParameterInfo
    );
    InteractionInfo writeThirdSomeEnumAttributeInteractionInfo = new InteractionInfo(
      (cluster, callback, commandArguments) -> {
        ((ChipClusters.ThirdCluster) cluster).writeSomeEnumAttribute(
          (DefaultClusterCallback) callback,
          (Integer) commandArguments.get("value")
        );
      },
      () -> new ClusterInfoMapping.DelegatedDefaultClusterCallback(),
      writeThirdSomeEnumCommandParams
    );
    writeThirdInteractionInfo.put("writeSomeEnumAttribute", writeThirdSomeEnumAttributeInteractionInfo);
    Map<String, CommandParameterInfo> writeThirdOptionsCommandParams = new LinkedHashMap<String, CommandParameterInfo>();
    CommandParameterInfo thirdoptionsCommandParameterInfo =
        new CommandParameterInfo(
            "value", 
            Integer.class, 
            Integer.class 
        );
    writeThirdOptionsCommandParams.put(
        "value",
        thirdoptionsCommandParameterInfo
    );
    InteractionInfo writeThirdOptionsAttributeInteractionInfo = new InteractionInfo(
      (cluster, callback, commandArguments) -> {
        ((ChipClusters.ThirdCluster) cluster).writeOptionsAttribute(
          (DefaultClusterCallback) callback,
          (Integer) commandArguments.get("value")
        );
      },
      () -> new ClusterInfoMapping.DelegatedDefaultClusterCallback(),
      writeThirdOptionsCommandParams
    );
    writeThirdInteractionInfo.put("writeOptionsAttribute", writeThirdOptionsAttributeInteractionInfo);
    writeAttributeMap.put("third", writeThirdInteractionInfo);return writeAttributeMap;
  }
}
