package chip.clusterinfo;

import java.util.*;
import chip.devicecontroller.ChipClusters.BaseChipCluster;

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

  @FunctionalInterface
  public interface ClusterConstructor {
    BaseChipCluster create(Long devicePtr, int endpointId);
  }
}
