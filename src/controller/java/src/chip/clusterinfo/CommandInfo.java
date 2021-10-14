<<<<<<< HEAD
<<<<<<< HEAD
package chip.clusterinfo;

import chip.devicecontroller.ChipClusters.BaseChipCluster;
import java.util.Map;
import java.util.function.Supplier;

/**
 * CommandInfo has a functional interface to invoke arbitrary commands based on cluster, callback
 * and a map of arguments, a Supplier that provides {@link DelegatedClusterCallback}, and maps the
 * parameter and commandParametersInfo.
 */
public class CommandInfo {
  public ClusterCommandFunction commandFunction;
  private Supplier<DelegatedClusterCallback> commandCallbackSupplier;
  private Map<String, CommandParameterInfo> commandParameters;

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

  /**
   * The functional interface provides a uniform way to invoke commands through invokeCommand
   * function. In ClusterInfoMapping, each ClusterCommandFunction was generated using the intended
   * function. By using lambda function, the app component only needs to have cluster, callback,
   * commandArguments to execute the correct function.
   */
  @FunctionalInterface
  public interface ClusterCommandFunction {
    void invokeCommand(
        BaseChipCluster cluster, Object callback, Map<String, Object> commandArguments);
  }
}
=======
package chip.devicecontroller;
=======
package chip.clusterinfo;
>>>>>>> 505e97db3 (change package)


public class CommandInfo {
  public CommandInfo(String name, Function<List<CommandParameter>, ClusterResponse> fn, List<CommandParameter> parameters, Class callbackClass) {
    this.name = name;
    this.fn = fn;
    this.parameters = parameters;
    this.callbackClass = callbackClass;
  }
  public CommandInfo() {}

  public String name;
  public Function<List<CommandParameter>, ClusterResponse> fn;
  public List<CommandParameter> parameters;
  public Class callbackClass;
}
>>>>>>> cdefa1b91 (basic code generation template, but not able to import to tool app)
