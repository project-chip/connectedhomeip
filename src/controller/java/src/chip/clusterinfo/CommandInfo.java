package chip.clusterinfo;

import chip.devicecontroller.ChipClusters.BaseChipCluster;
import java.util.Map;
import java.util.function.Supplier;

/**
 * CommandInfo includes a functional interface to invoke arbitrary commands based on cluster,
 * callback and a map of arguments, a Supplier that provides DelegatedClusterCallback, and a
 * Map that includes the name of the parameter and commandParametersInfo.
 */

 public class CommandInfo {
  public ClusterCommandFunction commandFunction;
  private Supplier<DelegatedClusterCallback> commandCallbackSupplier;
  private Map<String, CommandParameterInfo> commandParameters;

  /**
   * Constructor
   *
   * @param commandFunction the function that can construct invoke arbitrary cluster command
   * @param commandCallbackSupplier the Supplier that provides DelegatedClusterCallback
   * @param commandParameters the Map that contains parameter names and CommandParameterInfo
   */
  public CommandInfo(
      ClusterCommandFunction commandFunction,
      Supplier<DelegatedClusterCallback> commandCallbackSupplier,
      Map<String, CommandParameterInfo> commandParameters) {
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

  public Map<String, CommandParameterInfo> getCommandParameters() {
    return commandParameters;
  }


  /** The functional interface provides a uniform way to invoke commands through invokeCommand
  * function. In ClusterInfoMapping, each ClusterCommandFunction was generated using the
  * intended function. Using lambda function, the app component only needs to have cluster,
  * callback, commandArguments to execute the correct function.
  */
  @FunctionalInterface
  public interface ClusterCommandFunction {
    void invokeCommand(
        BaseChipCluster cluster, Object callback, Map<String, Object> commandArguments);
  }
}
