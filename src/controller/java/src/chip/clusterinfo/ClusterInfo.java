package chip.clusterinfo;

import chip.devicecontroller.ChipClusters.BaseChipCluster;
import java.util.Map;

/** ClusterInfo maps commands and provides a constructor function for a cluster. */
public class ClusterInfo {
  private final ClusterConstructor createClusterFunction;
  private final Map<String, InteractionInfo> commands;

  public ClusterInfo(
      ClusterConstructor createClusterFunction, Map<String, InteractionInfo> commands) {
    this.createClusterFunction = createClusterFunction;
    this.commands = commands;
  }

  public ClusterConstructor getCreateClusterFunction() {
    return createClusterFunction;
  }

  public Map<String, InteractionInfo> getCommands() {
    return commands;
  }

  public void combineCommands(Map<String, InteractionInfo> newCommands) {
    this.commands.putAll(newCommands);
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
