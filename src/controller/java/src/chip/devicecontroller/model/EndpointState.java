/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package chip.devicecontroller.model;

import java.util.Map;
import javax.annotation.Nullable;

/** Class for tracking CHIP endpoint state in a hierarchical manner. */
public final class EndpointState {
  private Map<Long, ClusterState> clusters;

  public EndpointState(Map<Long, ClusterState> clusters) {
    this.clusters = clusters;
  }

  public Map<Long, ClusterState> getClusterStates() {
    return clusters;
  }

  /**
   * Convenience utility for getting an {@code ClusterState}.
   *
   * @return the {@code ClusterState} for the specified id, or null if not found.
   */
  @Nullable
  public ClusterState getClusterState(long clusterId) {
    return clusters.get(clusterId);
  }

  @Override
  public String toString() {
    StringBuilder builder = new StringBuilder();
    clusters.forEach(
        (clusterId, clusterState) -> {
          builder.append("Cluster ");
          builder.append(clusterId);
          builder.append(": {\n");
          builder.append(clusterState.toString());
          builder.append("}\n");
        });
    return builder.toString();
  }
}
