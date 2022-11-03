/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package chip.devicecontroller.model;

import java.util.Locale;
import java.util.Objects;

/** An event path that should be used for requests. */
public class ChipEventPath {
  private ChipPathId endpointId, clusterId, eventId;

  private ChipEventPath(ChipPathId endpointId, ChipPathId clusterId, ChipPathId eventId) {
    this.endpointId = endpointId;
    this.clusterId = clusterId;
    this.eventId = eventId;
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

  @Override
  public boolean equals(Object object) {
    if (object instanceof ChipEventPath) {
      ChipEventPath that = (ChipEventPath) object;
      return Objects.equals(this.endpointId, that.endpointId)
          && Objects.equals(this.clusterId, that.clusterId)
          && Objects.equals(this.eventId, that.eventId);
    }
    return false;
  }

  @Override
  public int hashCode() {
    return Objects.hash(endpointId, clusterId, eventId);
  }

  @Override
  public String toString() {
    return String.format(
        Locale.ENGLISH, "Endpoint %s, cluster %s, event %s", endpointId, clusterId, eventId);
  }

  public static ChipEventPath newInstance(
      ChipPathId endpointId, ChipPathId clusterId, ChipPathId eventId) {
    return new ChipEventPath(endpointId, clusterId, eventId);
  }

  /** Create a new {@link ChipEventPath} with only concrete ids. */
  public static ChipEventPath newInstance(long endpointId, long clusterId, long eventId) {
    return new ChipEventPath(
        ChipPathId.forId(endpointId), ChipPathId.forId(clusterId), ChipPathId.forId(eventId));
  }
}
