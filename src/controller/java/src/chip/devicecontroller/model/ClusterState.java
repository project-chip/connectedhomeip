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

import android.util.Log;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Stream;
import javax.annotation.Nullable;
import org.json.JSONException;
import org.json.JSONObject;

/** Class for tracking CHIP cluster state in a hierarchical manner. */
public final class ClusterState {
  private static final String TAG = "ClusterState";
  private Map<Long, AttributeState> attributes;
  private Map<Long, ArrayList<EventState>> events;
  private Map<Long, Status> attributeStatuses;
  private Map<Long, ArrayList<Status>> eventStatuses;
  private Optional<Long> dataVersion;

  protected ClusterState(
      Map<Long, AttributeState> attributes,
      Map<Long, ArrayList<EventState>> events,
      Map<Long, Status> attributeStatuses,
      Map<Long, ArrayList<Status>> eventStatuses) {
    this.attributes = attributes;
    this.events = events;
    this.attributeStatuses = attributeStatuses;
    this.eventStatuses = eventStatuses;
    this.dataVersion = Optional.empty();
  }

  public Map<Long, AttributeState> getAttributeStates() {
    return attributes;
  }

  public Map<Long, Status> getAttributeStatuses() {
    return attributeStatuses;
  }

  public Map<Long, ArrayList<EventState>> getEventStates() {
    return events;
  }

  public Map<Long, ArrayList<Status>> getEventStatuses() {
    return eventStatuses;
  }

  public void setDataVersion(long version) {
    dataVersion = Optional.of(version);
  }

  public Optional<Long> getDataVersion() {
    return dataVersion;
  }

  /**
   * Convenience utility for getting all attributes in Json string format.
   *
   * @return all attributes in Json string format., or empty string if not found.
   */
  public String getAttributesJson() {
    JSONObject combinedObject = new JSONObject();
    Stream<JSONObject> attributeJsons =
        attributes.values().stream().map(it -> it.getJson()).filter(it -> it != null);

    attributeJsons.forEach(
        attributes -> {
          for (Iterator<String> iterator = attributes.keys(); iterator.hasNext(); ) {
            String key = iterator.next();
            if (combinedObject.has(key)) {
              Log.e(TAG, "Conflicting attribute tag Id is found: " + key);
              continue;
            }
            try {
              Object value = attributes.get(key);
              combinedObject.put(key, value);
            } catch (JSONException ex) {
              Log.e(TAG, "receive attribute json exception: " + ex);
            }
          }
        });
    return combinedObject.toString();
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
              attributeState.getJson() == null ? "null" : attributeState.getJson().toString());
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
                    eventState.getJson() == null ? "null" : eventState.getJson().toString());
                builder.append("\n");
              });
        });
    attributeStatuses.forEach(
        (attributeId, status) -> {
          builder.append("Attribute Status ");
          builder.append(attributeId);
          builder.append(": ");
          builder.append(status.toString());
          builder.append("\n");
        });
    eventStatuses.forEach(
        (eventId, status) -> {
          status.forEach(
              (eventState) -> {
                builder.append("Event Status");
                builder.append(eventId);
                builder.append(": ");
                builder.append(status.toString());
                builder.append("\n");
              });
        });
    return builder.toString();
  }
}
