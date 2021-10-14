<<<<<<< HEAD
<<<<<<< HEAD
package chip.clusterinfo;

<<<<<<< HEAD
import chip.devicecontroller.ChipClusters.BaseChipCluster;
import java.util.Map;

/** ClusterInfo maps commands and provides a constructor function for a cluster. */
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

  /**
   * The functional interface provides a uniform way to create cluster through create function. In
   * ClusterInfoMapping, each ClusterConstructor was generated using the intended function. Using
   * lambda function, it only needs to have ptr and endpointId to create the intended cluster.
   */
  @FunctionalInterface
  public interface ClusterConstructor {
    BaseChipCluster create(Long devicePtr, int endpointId);
  }
}
=======
package chip.devicecontroller;
=======
package chip.clusterinfo;
>>>>>>> 505e97db3 (change package)
=======
import java.util.*;
>>>>>>> a4fd0282e (no error code generation)

public class ClusterInfo {

  public ClusterInfo(String name) {
    this.name = name;
    this.commands = new ArrayList<CommandInfo>();
  }
  public String name;
  public Class<?> clusterClass;
  public List<CommandInfo> commands;

}
>>>>>>> cdefa1b91 (basic code generation template, but not able to import to tool app)
