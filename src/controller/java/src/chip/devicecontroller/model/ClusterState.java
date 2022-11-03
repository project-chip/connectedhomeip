/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package chip.devicecontroller.model;

import java.util.ArrayList;
import java.util.Map;
import java.util.Optional;
import javax.annotation.Nullable;

/** Class for tracking CHIP cluster state in a hierarchical manner. */
public final class ClusterState {
  private Map<Long, AttributeState> attributes;
  private Map<Long, ArrayList<EventState>> events;
  private Optional<Integer> dataVersion;

  public ClusterState(
      Map<Long, AttributeState> attributes, Map<Long, ArrayList<EventState>> events) {
    this.attributes = attributes;
    this.events = events;
    this.dataVersion = Optional.empty();
  }

  public Map<Long, AttributeState> getAttributeStates() {
    return attributes;
  }

  public Map<Long, ArrayList<EventState>> getEventStates() {
    return events;
  }

  public void setDataVersion(int version) {
    dataVersion = Optional.of(version);
  }

  public Optional<Integer> getDataVersion() {
    return dataVersion;
  }

  /**
   * Convenience utility for getting an {@code AttributeState}.
   *
   * @return the {@code AttributeState} for the specified id, or null if not found.
   */
  @Nullable
  public AttributeState getAttributeState(long attributeId) {
    return attributes.get(attributeId);
  }

  /**
   * Convenience utility for getting an {@code ArrayList<EventState> }.
   *
   * @return the {@code ArrayList<EventState>} for the specified id, or null if not found.
   */
  @Nullable
  public ArrayList<EventState> getEventState(long eventId) {
    return events.get(eventId);
  }

  @Override
  public String toString() {
    StringBuilder builder = new StringBuilder();
    attributes.forEach(
        (attributeId, attributeState) -> {
          builder.append("Attribute ");
          builder.append(attributeId);
          builder.append(": ");
          builder.append(
              attributeState.getValue() == null ? "null" : attributeState.getValue().toString());
          builder.append("\n");
        });
    events.forEach(
        (eventId, eventStates) -> {
          eventStates.forEach(
              (eventState) -> {
                builder.append("Event ");
                builder.append(eventId);
                builder.append(": ");
                builder.append(
                    eventState.getValue() == null ? "null" : eventState.getValue().toString());
                builder.append("\n");
              });
        });
    return builder.toString();
  }
}
