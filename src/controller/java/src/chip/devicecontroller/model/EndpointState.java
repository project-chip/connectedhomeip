/*
 *   Copyright (c) 2022 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package chip.devicecontroller.model;

import androidx.annotation.Nullable;
import java.util.Map;

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
