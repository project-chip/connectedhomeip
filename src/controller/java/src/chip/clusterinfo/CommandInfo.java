package chip.clusterinfo;

import java.util.function.Function;
import java.util.*;
import java.util.function.Supplier;
import chip.clusterinfo.DelegatedClusterCallback;
import chip.devicecontroller.ChipClusters.BaseChipCluster;

public class CommandInfo {
  public ClusterCommandFunction commandFunction;
  private Supplier<DelegatedClusterCallback> commandCallbackSupplier;
  private Map<String, CommandParameter> commandParameters;


  public CommandInfo(ClusterCommandFunction commandFunction,
      Supplier<DelegatedClusterCallback> commandCallbackSupplier, Map<String, CommandParameter> commandParameters) {
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
    void invokeCommand(BaseChipCluster cluster, Object callback, Map<String, Object> commandArguments);
  }
}