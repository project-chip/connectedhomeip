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

import java.util.Locale;
import java.util.Objects;

/** An event path that should be used for requests. */
public class ChipEventPath {
  private final ChipPathId endpointId, clusterId, eventId;
  private final boolean isUrgent;

  private ChipEventPath(
      ChipPathId endpointId, ChipPathId clusterId, ChipPathId eventId, boolean isUrgent) {
    this.endpointId = endpointId;
    this.clusterId = clusterId;
    this.eventId = eventId;
    this.isUrgent = isUrgent;
  }

  ChipEventPath(int endpointId, long clusterId, long eventId, boolean isUrgent) {
    this.endpointId = ChipPathId.forId(endpointId);
    this.clusterId = ChipPathId.forId(clusterId);
    this.eventId = ChipPathId.forId(eventId);
    this.isUrgent = isUrgent;
  }

  public ChipPathId getEndpointId() {
    return endpointId;
  }

  public ChipPathId getClusterId() {
    return clusterId;
  }

  public ChipPathId getEventId() {
    return eventId;
  }

  // For use in JNI.
  private long getEndpointId(long wildcardValue) {
    return endpointId.getId(wildcardValue);
  }

  private long getClusterId(long wildcardValue) {
    return clusterId.getId(wildcardValue);
  }

  private long getEventId(long wildcardValue) {
    return eventId.getId(wildcardValue);
  }

  public boolean isUrgent() {
    return isUrgent;
  }

  @Override
  public boolean equals(Object object) {
    if (object instanceof ChipEventPath) {
      ChipEventPath that = (ChipEventPath) object;
      return Objects.equals(this.endpointId, that.endpointId)
          && Objects.equals(this.clusterId, that.clusterId)
          && Objects.equals(this.eventId, that.eventId)
          && (this.isUrgent == that.isUrgent);
    }
    return false;
  }

  @Override
  public int hashCode() {
    return Objects.hash(endpointId, clusterId, eventId, isUrgent);
  }

  @Override
  public String toString() {
    return String.format(
        Locale.ENGLISH,
        "Endpoint %s, cluster %s, event %s, isUrgent %s",
        endpointId,
        clusterId,
        eventId,
        isUrgent ? "true" : "false");
  }

  public static ChipEventPath newInstance(
      ChipPathId endpointId, ChipPathId clusterId, ChipPathId eventId) {
    return new ChipEventPath(endpointId, clusterId, eventId, false);
  }

  /** Create a new {@link ChipEventPath} with only concrete ids. */
  public static ChipEventPath newInstance(int endpointId, long clusterId, long eventId) {
    return new ChipEventPath(
        ChipPathId.forId(endpointId),
        ChipPathId.forId(clusterId),
        ChipPathId.forId(eventId),
        false);
  }

  public static ChipEventPath newInstance(
      ChipPathId endpointId, ChipPathId clusterId, ChipPathId eventId, boolean isUrgent) {
    return new ChipEventPath(endpointId, clusterId, eventId, isUrgent);
  }

  /** Create a new {@link ChipEventPath} with only concrete ids. */
  public static ChipEventPath newInstance(
      int endpointId, long clusterId, long eventId, boolean isUrgent) {
    return new ChipEventPath(
        ChipPathId.forId(endpointId),
        ChipPathId.forId(clusterId),
        ChipPathId.forId(eventId),
        isUrgent);
  }
}
