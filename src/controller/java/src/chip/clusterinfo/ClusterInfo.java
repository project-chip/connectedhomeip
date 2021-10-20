package chip.clusterinfo;

import chip.devicecontroller.ChipClusters.BaseChipCluster;
import java.util.Map;

/**
 * ClusterInfo includes a functional interface to create arbitrary clusters and a map <String,
 * CommandInfo> to retrieve CommandInfo object from clusterInteractionFragment.kt
 */
public class ClusterInfo {
  private final ClusterConstructor createClusterFunction;
  private final Map<String, CommandInfo> commands;

  /**
   * Constructor
   *
   * @param createClusterFunction the function that can construct Cluster object
   * @param commands the Map that can provide easy access to CommandInfo
   */
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
   * lambda function, the app component only needs to have ptr and endpointId to create the intended
   * cluster.
   */
  @FunctionalInterface
  public interface ClusterConstructor {
    BaseChipCluster create(Long devicePtr, int endpointId);
  }
}
