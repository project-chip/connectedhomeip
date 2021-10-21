<<<<<<< HEAD
<<<<<<< HEAD
package chip.clusterinfo;

<<<<<<< HEAD
<<<<<<< HEAD
import chip.devicecontroller.ChipClusters.BaseChipCluster;
import java.util.Map;
import java.util.function.Supplier;

/**
<<<<<<< HEAD
<<<<<<< HEAD
 * CommandInfo has a functional interface to invoke arbitrary commands based on cluster, callback
 * and a map of arguments, a Supplier that provides {@link DelegatedClusterCallback}, and maps the
 * parameter and commandParametersInfo.
 */
public class CommandInfo {
=======
 * CommandInfo includes a functional interface to invoke arbitrary commands based on cluster,
 * callback and a map of arguments, a Supplier that provides DelegatedClusterCallback, and a Map
 * that includes the name of the parameter and commandParametersInfo.
=======
 * CommandInfo has a functional interface to invoke arbitrary commands based on cluster,
 * callback and a map of arguments, a Supplier that provides {@link DelegatedClusterCallback}, and
 * maps the parameter and commandParametersInfo.
>>>>>>> 4054a1371 (modify description of each new added class)
 */
<<<<<<< HEAD

 public class CommandInfo {
>>>>>>> 2185ce827 (add descriptive documentation on each new class)
=======
public class CommandInfo {
>>>>>>> e3803522e (Restyled by google-java-format)
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

<<<<<<< HEAD
<<<<<<< HEAD
  /**
   * The functional interface provides a uniform way to invoke commands through invokeCommand
   * function. In ClusterInfoMapping, each ClusterCommandFunction was generated using the intended
   * function. By using lambda function, the app component only needs to have cluster, callback,
   * commandArguments to execute the correct function.
   */
=======

  /** The functional interface provides a uniform way to invoke commands through invokeCommand
  * function. In ClusterInfoMapping, each ClusterCommandFunction was generated using the
  * intended function. Using lambda function, the app component only needs to have cluster,
  * callback, commandArguments to execute the correct function.
  */
>>>>>>> 2185ce827 (add descriptive documentation on each new class)
=======
  /**
   * The functional interface provides a uniform way to invoke commands through invokeCommand
   * function. In ClusterInfoMapping, each ClusterCommandFunction was generated using the intended
   * function. By using lambda function, the app component only needs to have cluster, callback,
   * commandArguments to execute the correct function.
   */
>>>>>>> e3803522e (Restyled by google-java-format)
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
=======
import java.util.function.Function;
=======
import chip.devicecontroller.ChipClusters.BaseChipCluster;
>>>>>>> 46c963914 (Restyled by google-java-format)
import java.util.*;
<<<<<<< HEAD
>>>>>>> a4fd0282e (no error code generation)

=======
import java.util.function.Supplier;
<<<<<<< HEAD
import chip.clusterinfo.DelegatedClusterCallback;
import chip.devicecontroller.ChipClusters.BaseChipCluster;
>>>>>>> 2d2bbd2e1 (new design solution)
=======
>>>>>>> 46c963914 (Restyled by google-java-format)

public class CommandInfo {
  public ClusterCommandFunction commandFunction;
  private Supplier<DelegatedClusterCallback> commandCallbackSupplier;
  private Map<String, CommandParameterInfo> commandParameters;

<<<<<<< HEAD
<<<<<<< HEAD
  public String name;
  public Function<List<CommandParameter>, Class<?>> fn;
  public List<CommandParameter> parameters;
<<<<<<< HEAD
  public Class callbackClass;
}
>>>>>>> cdefa1b91 (basic code generation template, but not able to import to tool app)
=======
  public Class<?> callbackClass;
}
>>>>>>> a4fd0282e (no error code generation)
=======

  public CommandInfo(ClusterCommandFunction commandFunction,
      Supplier<DelegatedClusterCallback> commandCallbackSupplier, Map<String, CommandParameter> commandParameters) {
=======
  public CommandInfo(
      ClusterCommandFunction commandFunction,
      Supplier<DelegatedClusterCallback> commandCallbackSupplier,
<<<<<<< HEAD
      Map<String, CommandParameter> commandParameters) {
>>>>>>> 46c963914 (Restyled by google-java-format)
=======
      Map<String, CommandParameterInfo> commandParameters) {
>>>>>>> fde1d13c0 (fix comments)
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

  @FunctionalInterface
  public interface ClusterCommandFunction {
    void invokeCommand(
        BaseChipCluster cluster, Object callback, Map<String, Object> commandArguments);
  }
}
<<<<<<< HEAD
>>>>>>> 2d2bbd2e1 (new design solution)
=======
>>>>>>> df6a682b0 (Restyled by whitespace)
