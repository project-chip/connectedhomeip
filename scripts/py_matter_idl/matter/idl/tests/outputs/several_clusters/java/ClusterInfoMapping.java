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
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import javax.annotation.Nullable;
import chip.clusterinfo.ClusterInfo;
import chip.clusterinfo.InteractionInfo;
import chip.clusterinfo.CommandParameterInfo;
import chip.clusterinfo.DelegatedClusterCallback;
import chip.clusterinfo.ClusterCommandCallback;
import chip.clusterinfo.CommandResponseInfo;
import chip.devicecontroller.ChipClusters.DefaultClusterCallback;
import chip.devicecontroller.ClusterReadMapping;
import chip.devicecontroller.ClusterWriteMapping;

public class ClusterInfoMapping {

  public static class DelegatedCharStringAttributeCallback implements ChipClusters.CharStringAttributeCallback, DelegatedClusterCallback {
    /** Indicates a successful read for a CHAR_STRING attribute. */
    private ClusterCommandCallback callback;

    @Override
    public void setCallbackDelegate(ClusterCommandCallback callback) {
      this.callback = callback;
    }

    @Override
    public void onSuccess(String value) {
      Map<CommandResponseInfo, Object> responseValues = new LinkedHashMap<>();
      CommandResponseInfo setupPINResponseValue = new CommandResponseInfo("value", "String");
      responseValues.put(setupPINResponseValue, value);
      callback.onSuccess(responseValues);
    }

    @Override
    public void onError(Exception error) {
      callback.onFailure(error);
    }
  }

  public static class DelegatedOctetStringAttributeCallback implements ChipClusters.OctetStringAttributeCallback, DelegatedClusterCallback {
    /** Indicates a successful read for an OCTET_STRING attribute. */
    private ClusterCommandCallback callback;

    @Override
    public void setCallbackDelegate(ClusterCommandCallback callback) {
      this.callback = callback;
    }

    @Override
    public void onSuccess(byte[] value) {
      Map<CommandResponseInfo, Object> responseValues = new LinkedHashMap<>();
      CommandResponseInfo setupPINResponseValue = new CommandResponseInfo("value", "byte[]");
      responseValues.put(setupPINResponseValue, value);
      callback.onSuccess(responseValues);
    }

    @Override
    public void onError(Exception error) {
      callback.onFailure(error);
    }
  }

  public static class DelegatedIntegerAttributeCallback implements ChipClusters.IntegerAttributeCallback, DelegatedClusterCallback {
    private ClusterCommandCallback callback;

    @Override
    public void setCallbackDelegate(ClusterCommandCallback callback) {
      this.callback = callback;
    }

    @Override
    public void onSuccess(int value) {
      Map<CommandResponseInfo, Object> responseValues = new LinkedHashMap<>();
      CommandResponseInfo setupPINResponseValue = new CommandResponseInfo("value", "int");
      responseValues.put(setupPINResponseValue, value);
      callback.onSuccess(responseValues);
    }

    @Override
    public void onError(Exception error) {
      callback.onFailure(error);
    }
  }

  public static class DelegatedLongAttributeCallback implements ChipClusters.LongAttributeCallback, DelegatedClusterCallback {
    private ClusterCommandCallback callback;

    @Override
    public void setCallbackDelegate(ClusterCommandCallback callback) {
      this.callback = callback;
    }

    @Override
    public void onSuccess(long value) {
      Map<CommandResponseInfo, Object> responseValues = new LinkedHashMap<>();
      CommandResponseInfo setupPINResponseValue = new CommandResponseInfo("value", "long");
      responseValues.put(setupPINResponseValue, value);
      callback.onSuccess(responseValues);
    }

    @Override
    public void onError(Exception error) {
      callback.onFailure(error);
    }
  }

  public static class DelegatedBooleanAttributeCallback implements ChipClusters.BooleanAttributeCallback, DelegatedClusterCallback {
    private ClusterCommandCallback callback;

    @Override
    public void setCallbackDelegate(ClusterCommandCallback callback) {
      this.callback = callback;
    }

    @Override
    public void onSuccess(boolean value) {
      Map<CommandResponseInfo, Object> responseValues = new LinkedHashMap<>();
      CommandResponseInfo setupPINResponseValue = new CommandResponseInfo("value", "boolean");
      responseValues.put(setupPINResponseValue, value);
      callback.onSuccess(responseValues);
    }

    @Override
    public void onError(Exception error) {
      callback.onFailure(error);
    }
  }

  public static class DelegatedFloatAttributeCallback implements ChipClusters.FloatAttributeCallback, DelegatedClusterCallback {
    private ClusterCommandCallback callback;

    @Override
    public void setCallbackDelegate(ClusterCommandCallback callback) {
      this.callback = callback;
    }

    @Override
    public void onSuccess(float value) {
      Map<CommandResponseInfo, Object> responseValues = new LinkedHashMap<>();
      CommandResponseInfo setupPINResponseValue = new CommandResponseInfo("value", "float");
      responseValues.put(setupPINResponseValue, value);
      callback.onSuccess(responseValues);
    }

    @Override
    public void onError(Exception error) {
      callback.onFailure(error);
    }
  }

  public static class DelegatedDoubleAttributeCallback implements ChipClusters.DoubleAttributeCallback, DelegatedClusterCallback {
    private ClusterCommandCallback callback;

    @Override
    public void setCallbackDelegate(ClusterCommandCallback callback) {
      this.callback = callback;
    }

    @Override
    public void onSuccess(double value) {
      Map<CommandResponseInfo, Object> responseValues = new LinkedHashMap<>();
      CommandResponseInfo setupPINResponseValue = new CommandResponseInfo("value", "double");
      responseValues.put(setupPINResponseValue, value);
      callback.onSuccess(responseValues);
    }

    @Override
    public void onError(Exception error) {
      callback.onFailure(error);
    }
  }

  public static class DelegatedDefaultClusterCallback implements DefaultClusterCallback, DelegatedClusterCallback {
    private ClusterCommandCallback callback;

    @Override
    public void setCallbackDelegate(ClusterCommandCallback callback) {
      this.callback = callback;
    }

    // Parameters and list-adds here should be generated - refer to the template code that creates each callback interface.
    @Override
    public void onSuccess() {
      Map<CommandResponseInfo, Object> responseValues = new LinkedHashMap<>();
      callback.onSuccess(responseValues);
    }

    @Override
    public void onError(Exception e) {
      callback.onFailure(e);
    }
  }

  public static class DelegatedSecondClusterFabricsAttributeCallback implements ChipClusters.SecondCluster.FabricsAttributeCallback, DelegatedClusterCallback {
    private ClusterCommandCallback callback;
    @Override
    public void setCallbackDelegate(ClusterCommandCallback callback) {
      this.callback = callback;
    }

    @Override
    public void onSuccess(List<ChipStructs.SecondClusterFabricDescriptorStruct> valueList) {
      Map<CommandResponseInfo, Object> responseValues = new LinkedHashMap<>();
      CommandResponseInfo commandResponseInfo = new CommandResponseInfo("valueList", "List<ChipStructs.SecondClusterFabricDescriptorStruct>");
      responseValues.put(commandResponseInfo, valueList);
      callback.onSuccess(responseValues);
    }

    @Override
    public void onError(Exception ex) {
      callback.onFailure(ex);
    }
  }


  public Map<String, ClusterInfo> getClusterMap() {
    Map<String, ClusterInfo> clusterMap = initializeClusterMap();
    Map<String, Map<String, InteractionInfo>> commandMap = getCommandMap();
    combineCommand(clusterMap, commandMap);
    Map<String, Map<String, InteractionInfo>> readAttributeMap = new ClusterReadMapping().getReadAttributeMap();
    combineCommand(clusterMap, readAttributeMap);
    Map<String, Map<String, InteractionInfo>> writeAttributeMap = new ClusterWriteMapping().getWriteAttributeMap();
    combineCommand(clusterMap, writeAttributeMap);
    return clusterMap;
 }

  public Map<String, ClusterInfo> initializeClusterMap() {
    Map<String, ClusterInfo> clusterMap = new HashMap<>();

    ClusterInfo firstClusterInfo = new ClusterInfo(
      (ptr, endpointId) -> new ChipClusters.FirstCluster(ptr, endpointId), new HashMap<>());
    clusterMap.put("first", firstClusterInfo);

    ClusterInfo secondClusterInfo = new ClusterInfo(
      (ptr, endpointId) -> new ChipClusters.SecondCluster(ptr, endpointId), new HashMap<>());
    clusterMap.put("second", secondClusterInfo);

    ClusterInfo thirdClusterInfo = new ClusterInfo(
      (ptr, endpointId) -> new ChipClusters.ThirdCluster(ptr, endpointId), new HashMap<>());
    clusterMap.put("third", thirdClusterInfo);

    return clusterMap;
  }

  public void combineCommand(Map<String, ClusterInfo> destination, Map<String, Map<String, InteractionInfo>> source) {
    destination.get("first").combineCommands(source.get("first"));
    destination.get("second").combineCommands(source.get("second"));
    destination.get("third").combineCommands(source.get("third"));
  }

 @SuppressWarnings("unchecked")
  public Map<String, Map<String, InteractionInfo>> getCommandMap() {
    Map<String, Map<String, InteractionInfo>> commandMap = new HashMap<>();

    Map<String, InteractionInfo> firstClusterInteractionInfoMap = new LinkedHashMap<>();

    commandMap.put("first", firstClusterInteractionInfoMap);

    Map<String, InteractionInfo> secondClusterInteractionInfoMap = new LinkedHashMap<>();

    commandMap.put("second", secondClusterInteractionInfoMap);

    Map<String, InteractionInfo> thirdClusterInteractionInfoMap = new LinkedHashMap<>();

    commandMap.put("third", thirdClusterInteractionInfoMap);

    return commandMap;
  }
}
