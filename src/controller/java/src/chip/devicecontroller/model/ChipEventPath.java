/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

  public ChipPathId getEndpointId() {
    return endpointId;
  }

  public ChipPathId getClusterId() {
    return clusterId;
  }

  public ChipPathId getEventId() {
    return eventId;
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
