package com.google.chip.chiptool.clusterclient;

import chip.devicecontroller.ChipClusters.BaseChipCluster;
import chip.devicecontroller.ChipClusters.DefaultClusterCallback;
import chip.devicecontroller.ChipClusters.OnOffCluster;
import chip.devicecontroller.ChipClusters.OperationalCredentialsCluster.AttestationResponseCallback;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.function.Supplier;

public class ClusterInfo {
  private final ClusterConstructor createClusterFunction;
  private final Map<String, CommandInfo> commands;

  public ClusterInfo(ClusterConstructor createClusterFunction, Map<String, CommandInfo> commands) {
    this.createClusterFunction = createClusterFunction;
    this.commands = commands;
  }

  public ClusterConstructor getCreateClusterFunction() {
    return createClusterFunction;
  }

  public Map<String, CommandInfo> getCommands() {
    return commands;
  }

  @FunctionalInterface
  interface ClusterConstructor {
    BaseChipCluster create(Long devicePtr, int endpointId);
  }
}

class CommandInfo {
  private ClusterCommandFunction commandFunction;
  private Supplier<DelegatedClusterCallback> commandCallbackSupplier;
  private List<CommandParameter> commandParameters;


  public CommandInfo(ClusterCommandFunction commandFunction,
      Supplier<DelegatedClusterCallback> commandCallbackSupplier, List<CommandParameter> commandParameters) {
    this.commandFunction = commandFunction;
    this.commandCallbackSupplier = commandCallbackSupplier;
    this.commandParameters = commandParameters;
  }

  public ClusterCommandFunction getCommandFunction() {
    return commandFunction;
  }

  public Supplier<DelegatedClusterCallback> getCommandCallbackSupplier() {
    return commandCallbackSupplier;
  }

  public List<CommandParameter> getCommandParameters() {
    return commandParameters;
  }

  @FunctionalInterface
  interface ClusterCommandFunction {
    void invokeCommand(BaseChipCluster cluster, Object callback, List<Object> commandArguments);
  }
}

class CommandParameter {

}

// generated code
interface ClusterCommandCallback {
  void onSuccess(List<Object> responseValues);
  void onFailure(Exception exception);
}

interface DelegatedClusterCallback {
  void setCallbackDelegate(ClusterCommandCallback callback);
}

// Demoing DelegatedAttestationResponseCallback since AttestationResponseCallback has non-default success parameters.
// In the template: Generate a Delegated{{... name}} for every cluster callback type.
class DelegatedAttestationResponseCallback implements AttestationResponseCallback, DelegatedClusterCallback {
  private ClusterCommandCallback callback;

  @Override
  public void setCallbackDelegate(ClusterCommandCallback callback) {
    this.callback = callback;
  }

  // Parameters and list-adds here should be generated - refer to the template code that creates each callback interface.
  @Override
  public void onSuccess(byte[] attestationElements, byte[] signature) {
    List<Object> responseValues = new ArrayList<>();
    responseValues.add(attestationElements);
    responseValues.add(signature);
    callback.onSuccess(responseValues);
  }

  @Override
  public void onError(Exception e) {
    callback.onFailure(e);
  }
}

class ExampleCode {
  // GENERATED CODE: Should be created by the template.
  static Map<String, ClusterInfo> getClusterMap() {
    Map<String, ClusterInfo> clusterMap = new HashMap<>();

    // Populate parameters
    List<CommandParameter> commandParams = new ArrayList<>();
    // TODO: fill out parameter types
    CommandParameter effectId = new CommandParameter();
    CommandParameter effectVariant = new CommandParameter();
    commandParams.add(effectId);
    commandParams.add(effectVariant);

    // Populate commands
    Map<String, CommandInfo> onOffClusterCommandInfo = new HashMap<>();
    CommandInfo offWithEffectCommandInfo = new CommandInfo(
        (cluster, callback, commandArguments) -> {
          // TODO: expose index in template helper
          ((OnOffCluster) cluster)
              .offWithEffect((DefaultClusterCallback) callback, (Integer) commandArguments.get(0),
                  (Integer) commandArguments.get(1));
        },
        // DelegatedDefaultClusterCallback is not implemented in this file, but it's just a simpler DelegatedAttestationResponseCallback that passes an empty responseValues list.
        () -> new DelegatedDefaultClusterCallback(),
        commandParams
    );
    onOffClusterCommandInfo.put("OffWithEffect", offWithEffectCommandInfo);

    // Populate cluster
    ClusterInfo onOffClusterInfo = new ClusterInfo(
        (ptr, endpointId) -> new OnOffCluster(ptr, endpointId), onOffClusterCommandInfo);

    clusterMap.put("OnOff", onOffClusterInfo);
    return clusterMap;
  }


  // APP CODE: Should be in the ClusterInteractionFragment instead of here.
  static void accessClusterMap() {
    Map<String, ClusterInfo> clusterMap = getClusterMap();
    // In real code: get the device ptr using ChipClient.getConnectedDevicePointer
    long devicePtr = 0L;
    // This is hardcoded to 1 for most clusters
    int endpointId = 1;
    // In real code: "OnOff" would be selected by the user.
    ClusterInfo clusterInfo = clusterMap.get("OnOff");
    BaseChipCluster onOffCluster = clusterInfo.getCreateClusterFunction().create(devicePtr, endpointId);


    // Imagine user wants to execute the command "OffWithEffect", pass the string here
    CommandInfo offWithEffectInfo = clusterInfo.getCommands().get("OffWithEffect");
    DelegatedClusterCallback callback = offWithEffectInfo.getCommandCallbackSupplier().get();

    callback.setCallbackDelegate(new ClusterCommandCallback() {
      @Override
      public void onSuccess(List<Object> responseValues) {
        // Populate UI based on response values. We know the types from CommandInfo.getCommandResponses().
      }

      @Override
      public void onFailure(Exception exception) {
        // Show some error.
      }
    });
    offWithEffectInfo.getCommandParameters()
    List<Object> commandArguments = new ArrayList<>();
    // "effectId"
    commandArguments.add("effectId", 123);
    // "effectVariant"
    commandArguments.add("effectVariant", 456);
    // How do you know what callback to create?
    offWithEffectInfo.getCommandFunction().invokeCommand(onOffCluster, null, commandArguments);
  }
}