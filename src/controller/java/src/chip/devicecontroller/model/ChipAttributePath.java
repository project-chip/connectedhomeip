/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package chip.devicecontroller.model;

import java.util.Locale;
import java.util.Objects;

/** An attribute path that should be used for requests. */
public class ChipAttributePath {
  private ChipPathId endpointId, clusterId, attributeId;

  private ChipAttributePath(ChipPathId endpointId, ChipPathId clusterId, ChipPathId attributeId) {
    this.endpointId = endpointId;
    this.clusterId = clusterId;
    this.attributeId = attributeId;
  }

  public ChipPathId getEndpointId() {
    return endpointId;
  }

  public ChipPathId getClusterId() {
    return clusterId;
  }

  public ChipPathId getAttributeId() {
    return attributeId;
  }

  @Override
  public boolean equals(Object object) {
    if (object instanceof ChipAttributePath) {
      ChipAttributePath that = (ChipAttributePath) object;
      return Objects.equals(this.endpointId, that.endpointId)
          && Objects.equals(this.clusterId, that.clusterId)
          && Objects.equals(this.attributeId, that.attributeId);
    }
    return false;
  }

  @Override
  public int hashCode() {
    return Objects.hash(endpointId, clusterId, attributeId);
  }

  @Override
  public String toString() {
    return String.format(
        Locale.ENGLISH,
        "Endpoint %s, cluster %s, attribute %s",
        endpointId,
        clusterId,
        attributeId);
  }

  public static ChipAttributePath newInstance(
      ChipPathId endpointId, ChipPathId clusterId, ChipPathId attributeId) {
    return new ChipAttributePath(endpointId, clusterId, attributeId);
  }

  /** Create a new {@link ChipAttributePath} with only concrete ids. */
  public static ChipAttributePath newInstance(int endpointId, long clusterId, long attributeId) {
    return new ChipAttributePath(
        ChipPathId.forId(endpointId), ChipPathId.forId(clusterId), ChipPathId.forId(attributeId));
  }
}
