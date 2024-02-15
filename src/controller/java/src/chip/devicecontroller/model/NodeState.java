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

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import javax.annotation.Nullable;

/** Class for tracking CHIP node state in a hierarchical manner. */
public final class NodeState {
  private Map<Integer, EndpointState> endpoints;

  public NodeState() {
    this.endpoints = new HashMap<>();
  }

  public Map<Integer, EndpointState> getEndpointStates() {
    return endpoints;
  }

  // Called from native code only, which ignores access modifiers.
  private void addAttribute(
      int endpointId,
      long clusterId,
      long attributeId,
      Object valueObject,
      byte[] tlv,
      String jsonString) {
    addAttribute(
        endpointId, clusterId, attributeId, new AttributeState(valueObject, tlv, jsonString));
  }

  private void addEvent(
      int endpointId,
      long clusterId,
      long eventId,
      long eventNumber,
      int priorityLevel,
      int timestampType,
      long timestampValue,
      Object valueObject,
      byte[] tlv,
      String jsonString) {
    addEvent(
        endpointId,
        clusterId,
        eventId,
        new EventState(
            eventNumber,
            priorityLevel,
            timestampType,
            timestampValue,
            valueObject,
            tlv,
            jsonString));
  }

  private void addAttributeStatus(
      int endpointId,
      long clusterId,
      long attributeId,
      int status,
      @Nullable Integer clusterStatus) {
    addAttributeStatus(
        endpointId, clusterId, attributeId, Status.newInstance(status, clusterStatus));
  }

  private void addEventStatus(
      int endpointId, long clusterId, long eventId, int status, @Nullable Integer clusterStatus) {
    addEventStatus(endpointId, clusterId, eventId, Status.newInstance(status, clusterStatus));
  }

  private void setDataVersion(int endpointId, long clusterId, long dataVersion) {
    EndpointState endpointState = getEndpointState(endpointId);
    ClusterState clusterState = endpointState.getClusterState(clusterId);

    if (clusterState != null) {
      clusterState.setDataVersion(dataVersion);
    }
  }

  private void addAttribute(
      int endpointId, long clusterId, long attributeId, AttributeState attributeStateToAdd) {
    EndpointState endpointState = getEndpointState(endpointId);
    if (endpointState == null) {
      endpointState = new EndpointState(new HashMap<>());
      getEndpointStates().put(endpointId, endpointState);
    }

    ClusterState clusterState = endpointState.getClusterState(clusterId);
    if (clusterState == null) {
      clusterState =
          new ClusterState(new HashMap<>(), new HashMap<>(), new HashMap<>(), new HashMap<>());
      endpointState.getClusterStates().put(clusterId, clusterState);
    }

    if (clusterState.getAttributeStatuses().containsKey(attributeId)) {
      clusterState.getAttributeStatuses().remove(attributeId);
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
      clusterState =
          new ClusterState(new HashMap<>(), new HashMap<>(), new HashMap<>(), new HashMap<>());
      endpointState.getClusterStates().put(clusterId, clusterState);
    }

    if (!clusterState.getEventStates().containsKey(eventId)) {
      clusterState.getEventStates().put(eventId, new ArrayList<EventState>());
    }

    if (clusterState.getEventStatuses().containsKey(eventId)) {
      clusterState.getEventStatuses().remove(eventId);
    }

    clusterState.getEventStates().get(eventId).add(eventStateToAdd);
  }

  private void addAttributeStatus(
      int endpointId, long clusterId, long attributeId, Status statusToAdd) {
    EndpointState endpointState = getEndpointState(endpointId);
    if (endpointState == null) {
      endpointState = new EndpointState(new HashMap<>());
      getEndpointStates().put(endpointId, endpointState);
    }

    ClusterState clusterState = endpointState.getClusterState(clusterId);
    if (clusterState == null) {
      clusterState =
          new ClusterState(new HashMap<>(), new HashMap<>(), new HashMap<>(), new HashMap<>());
      endpointState.getClusterStates().put(clusterId, clusterState);
    }

    if (clusterState.getAttributeStates().containsKey(attributeId)) {
      clusterState.getAttributeStates().remove(attributeId);
    }
    System.out.println("put : " + attributeId + ", " + statusToAdd);
    clusterState.getAttributeStatuses().put(attributeId, statusToAdd);
  }

  private void addEventStatus(int endpointId, long clusterId, long eventId, Status statusToAdd) {
    EndpointState endpointState = getEndpointState(endpointId);
    if (endpointState == null) {
      endpointState = new EndpointState(new HashMap<>());
      getEndpointStates().put(endpointId, endpointState);
    }

    ClusterState clusterState = endpointState.getClusterState(clusterId);
    if (clusterState == null) {
      clusterState =
          new ClusterState(new HashMap<>(), new HashMap<>(), new HashMap<>(), new HashMap<>());
      endpointState.getClusterStates().put(clusterId, clusterState);
    }

    if (!clusterState.getEventStatuses().containsKey(eventId)) {
      clusterState.getEventStatuses().put(eventId, new ArrayList<Status>());
    }

    if (clusterState.getEventStates().containsKey(eventId)) {
      clusterState.getEventStates().remove(eventId);
    }

    clusterState.getEventStatuses().get(eventId).add(statusToAdd);
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
