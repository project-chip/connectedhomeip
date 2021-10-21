<<<<<<< HEAD
<<<<<<< HEAD
package chip.clusterinfo;

<<<<<<< HEAD
<<<<<<< HEAD
import chip.devicecontroller.ChipClusters.BaseChipCluster;
import java.util.Map;
=======
import chip.devicecontroller.ChipClusters.BaseChipCluster;
<<<<<<< HEAD
import java.util.*;
>>>>>>> 46c963914 (Restyled by google-java-format)
=======
import java.util.Map;
>>>>>>> ef97f0702 (resolve type, nullable and format comments)

<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
/** ClusterInfo maps commands and provides a constructor function for a cluster. */
=======
/**
  ClusterInfo maps commands and provides a constructor function for a cluster
 */
>>>>>>> 2185ce827 (add descriptive documentation on each new class)
=======
/** ClusterInfo maps commands and provides a constructor function for a cluster */
>>>>>>> 68e157c68 (Restyled by google-java-format)
=======
/** ClusterInfo maps commands and provides a constructor function for a cluster. */
>>>>>>> 74fe2b5bd (add . at the end of class description)
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
<<<<<<< HEAD
<<<<<<< HEAD
   * The functional interface provides a uniform way to create cluster through create function. In
   * ClusterInfoMapping, each ClusterConstructor was generated using the intended function. Using
   * lambda function, it only needs to have ptr and endpointId to create the intended cluster.
=======
   * The functional interface provides a uniform way to create cluster through create
   * function. In ClusterInfoMapping, each ClusterConstructor was generated using the
   * intended function. Using lambda function, the app component only needs to have ptr
   * and endpointId to create the intended cluster.
>>>>>>> 2185ce827 (add descriptive documentation on each new class)
=======
   * The functional interface provides a uniform way to create cluster through create function. In
   * ClusterInfoMapping, each ClusterConstructor was generated using the intended function. Using
<<<<<<< HEAD
   * lambda function, it only needs to have ptr and endpointId to create the intended
   * cluster.
>>>>>>> e3803522e (Restyled by google-java-format)
=======
   * lambda function, it only needs to have ptr and endpointId to create the intended cluster.
>>>>>>> 68e157c68 (Restyled by google-java-format)
   */
  @FunctionalInterface
  public interface ClusterConstructor {
    BaseChipCluster create(Long devicePtr, int endpointId);
  }
}
<<<<<<< HEAD
=======
package chip.devicecontroller;
=======
package chip.clusterinfo;
>>>>>>> 505e97db3 (change package)
=======
import java.util.*;
<<<<<<< HEAD
>>>>>>> a4fd0282e (no error code generation)
=======
import chip.devicecontroller.ChipClusters.BaseChipCluster;
>>>>>>> 2d2bbd2e1 (new design solution)

public class ClusterInfo {
  private final ClusterConstructor createClusterFunction;
  private final Map<String, CommandInfo> commands;

  public ClusterInfo(ClusterConstructor createClusterFunction, Map<String, CommandInfo> commands) {
    this.createClusterFunction = createClusterFunction;
    this.commands = commands;
  }

<<<<<<< HEAD
}
>>>>>>> cdefa1b91 (basic code generation template, but not able to import to tool app)
=======
  public ClusterConstructor getCreateClusterFunction() {
    return createClusterFunction;
  }

  public Map<String, CommandInfo> getCommands() {
    return commands;
  }

  @FunctionalInterface
  public interface ClusterConstructor {
    BaseChipCluster create(Long devicePtr, int endpointId);
  }
}
>>>>>>> 2d2bbd2e1 (new design solution)
=======
>>>>>>> df6a682b0 (Restyled by whitespace)
