package chip.clusterinfo;

import java.util.function.Function;
import java.util.*;
import java.util.function.Supplier;
import chip.clusterinfo.DelegatedClusterCallback;
import chip.devicecontroller.ChipClusters.BaseChipCluster;

public class CommandInfo {
  public ClusterCommandFunction commandFunction;
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