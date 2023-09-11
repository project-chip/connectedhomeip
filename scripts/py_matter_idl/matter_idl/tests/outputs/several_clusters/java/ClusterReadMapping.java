/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package chip.devicecontroller;

import chip.clusterinfo.CommandParameterInfo;
import chip.clusterinfo.InteractionInfo;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.Map;

public class ClusterReadMapping {


    private static Map<String, InteractionInfo> readFirstInteractionInfo() {
       Map<String, InteractionInfo> result = new LinkedHashMap<>();Map<String, CommandParameterInfo> readFirstSomeIntegerCommandParams = new LinkedHashMap<String, CommandParameterInfo>();
        InteractionInfo readFirstSomeIntegerAttributeInteractionInfo = new InteractionInfo(
          (cluster, callback, commandArguments) -> {
            ((ChipClusters.FirstCluster) cluster).readSomeIntegerAttribute(
              (ChipClusters.IntegerAttributeCallback) callback
            );
          },
          () -> new ClusterInfoMapping.DelegatedIntegerAttributeCallback(),
          readFirstSomeIntegerCommandParams
        );
        result.put("readSomeIntegerAttribute", readFirstSomeIntegerAttributeInteractionInfo);
     
       return result;
    }
    private static Map<String, InteractionInfo> readSecondInteractionInfo() {
       Map<String, InteractionInfo> result = new LinkedHashMap<>();Map<String, CommandParameterInfo> readSecondFabricsCommandParams = new LinkedHashMap<String, CommandParameterInfo>();
        InteractionInfo readSecondFabricsAttributeInteractionInfo = new InteractionInfo(
          (cluster, callback, commandArguments) -> {
            ((ChipClusters.SecondCluster) cluster).readFabricsAttribute(
              (ChipClusters.SecondCluster.FabricsAttributeCallback) callback
            );
          },
          () -> new ClusterInfoMapping.DelegatedSecondClusterFabricsAttributeCallback(),
          readSecondFabricsCommandParams
        );
        result.put("readFabricsAttribute", readSecondFabricsAttributeInteractionInfo);
     Map<String, CommandParameterInfo> readSecondSomeBytesCommandParams = new LinkedHashMap<String, CommandParameterInfo>();
        InteractionInfo readSecondSomeBytesAttributeInteractionInfo = new InteractionInfo(
          (cluster, callback, commandArguments) -> {
            ((ChipClusters.SecondCluster) cluster).readSomeBytesAttribute(
              (ChipClusters.OctetStringAttributeCallback) callback
            );
          },
          () -> new ClusterInfoMapping.DelegatedOctetStringAttributeCallback(),
          readSecondSomeBytesCommandParams
        );
        result.put("readSomeBytesAttribute", readSecondSomeBytesAttributeInteractionInfo);
     
       return result;
    }
    private static Map<String, InteractionInfo> readThirdInteractionInfo() {
       Map<String, InteractionInfo> result = new LinkedHashMap<>();Map<String, CommandParameterInfo> readThirdSomeEnumCommandParams = new LinkedHashMap<String, CommandParameterInfo>();
        InteractionInfo readThirdSomeEnumAttributeInteractionInfo = new InteractionInfo(
          (cluster, callback, commandArguments) -> {
            ((ChipClusters.ThirdCluster) cluster).readSomeEnumAttribute(
              (ChipClusters.IntegerAttributeCallback) callback
            );
          },
          () -> new ClusterInfoMapping.DelegatedIntegerAttributeCallback(),
          readThirdSomeEnumCommandParams
        );
        result.put("readSomeEnumAttribute", readThirdSomeEnumAttributeInteractionInfo);
     Map<String, CommandParameterInfo> readThirdOptionsCommandParams = new LinkedHashMap<String, CommandParameterInfo>();
        InteractionInfo readThirdOptionsAttributeInteractionInfo = new InteractionInfo(
          (cluster, callback, commandArguments) -> {
            ((ChipClusters.ThirdCluster) cluster).readOptionsAttribute(
              (ChipClusters.IntegerAttributeCallback) callback
            );
          },
          () -> new ClusterInfoMapping.DelegatedIntegerAttributeCallback(),
          readThirdOptionsCommandParams
        );
        result.put("readOptionsAttribute", readThirdOptionsAttributeInteractionInfo);
     
       return result;
    }
    @SuppressWarnings("serial")
    public Map<String, Map<String, InteractionInfo>> getReadAttributeMap() {

        return new HashMap<String, Map<String, InteractionInfo>>(){{
            put("first", readFirstInteractionInfo());
            put("second", readSecondInteractionInfo());
            put("third", readThirdInteractionInfo());}};
    }
}

