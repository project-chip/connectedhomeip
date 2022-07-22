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

/** Class for tracking CHIP cluster state in a hierarchical manner. */
public final class ClusterState {
  private Map<Long, AttributeState> attributes;
  private Map<Long, EventState> events;

  public ClusterState(Map<Long, AttributeState> attributes, Map<Long, EventState> events) {
    this.attributes = attributes;
    this.events = events;
  }

  public Map<Long, AttributeState> getAttributeStates() {
    return attributes;
  }

  public Map<Long, EventState> getEventStates() {
    return events;
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
   * Convenience utility for getting an {@code EventState}.
   *
   * @return the {@code EventState} for the specified id, or null if not found.
   */
  @Nullable
  public EventState getEventState(long eventId) {
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
        (eventId, eventState) -> {
          builder.append("Event ");
          builder.append(eventId);
          builder.append(": ");
          builder.append(eventState.getValue() == null ? "null" : eventState.getValue().toString());
          builder.append("\n");
        });
    return builder.toString();
  }
}
