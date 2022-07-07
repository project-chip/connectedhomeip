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
import java.util.HashMap;
import java.util.Map;

/** Class for tracking CHIP node state in a hierarchical manner. */
public final class NodeState {
  private Map<Integer, EndpointState> endpoints;

  public NodeState(Map<Integer, EndpointState> endpoints) {
    this.endpoints = endpoints;
  }

  public Map<Integer, EndpointState> getEndpointStates() {
    return endpoints;
  }

  // Called from native code only, which ignores access modifiers.
  private void addAttribute(
      int endpointId, long clusterId, long attributeId, AttributeState attributeStateToAdd) {
    EndpointState endpointState = getEndpointState(endpointId);
    if (endpointState == null) {
      endpointState = new EndpointState(new HashMap<>());
      getEndpointStates().put(endpointId, endpointState);
    }

    ClusterState clusterState = endpointState.getClusterState(clusterId);
    if (clusterState == null) {
      clusterState = new ClusterState(new HashMap<>(), new HashMap<>());
      endpointState.getClusterStates().put(clusterId, clusterState);
    }

    // This will overwrite previous attributes.
    clusterState.getAttributeStates().put(attributeId, attributeStateToAdd);
  }

  private void addEvent(int endpointId, long clusterId, long eventId, EventState eventStateToAdd) {
    EndpointState endpointState = getEndpointState(endpointId);
    if (endpointState == null) {
      endpointState = new EndpointState(new HashMap<>());
      getEndpointStates().put(endpointId, endpointState);
    }

    ClusterState clusterState = endpointState.getClusterState(clusterId);
    if (clusterState == null) {
      clusterState = new ClusterState(new HashMap<>(), new HashMap<>());
      endpointState.getClusterStates().put(clusterId, clusterState);
    }

    // This will overwrite previous events.
    clusterState.getEventStates().put(eventId, eventStateToAdd);
  }

  @Override
  public String toString() {
    StringBuilder builder = new StringBuilder();
    endpoints.forEach(
        (endpointId, endpointState) -> {
          builder.append("Endpoint ");
          builder.append(endpointId);
          builder.append(": {\n");
          builder.append(endpointState.toString());
          builder.append("}\n");
        });
    return builder.toString();
  }

  /**
   * Convenience utility for getting an {@code EndpointState}.
   *
   * @return the {@code EndpointState} for the specified id, or null if not found.
   */
  @Nullable
  public EndpointState getEndpointState(int endpointId) {
    return endpoints.get(endpointId);
  }
}
