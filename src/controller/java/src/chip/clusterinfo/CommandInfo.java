package chip.clusterinfo;

import chip.devicecontroller.ChipClusters.BaseChipCluster;
import java.util.*;
import java.util.function.Supplier;

public class CommandInfo {
  public ClusterCommandFunction commandFunction;
  private Supplier<DelegatedClusterCallback> commandCallbackSupplier;
  private Map<String, CommandParameter> commandParameters;

  public CommandInfo(
      ClusterCommandFunction commandFunction,
      Supplier<DelegatedClusterCallback> commandCallbackSupplier,
      Map<String, CommandParameter> commandParameters) {
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

  public Map<String, CommandParameter> getCommandParameters() {
    return commandParameters;
  }

  @FunctionalInterface
  public interface ClusterCommandFunction {
    void invokeCommand(
        BaseChipCluster cluster, Object callback, Map<String, Object> commandArguments);
  }
}
