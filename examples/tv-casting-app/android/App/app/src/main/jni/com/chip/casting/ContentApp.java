/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package com.chip.casting;

import java.util.List;
import java.util.Objects;

public class ContentApp {
  private short endpointId;
  private List<Integer> clusterIds;

  private boolean isInitialized = false;

  public ContentApp(short endpointId, List<Integer> clusterIds) {
    this.endpointId = endpointId;
    this.clusterIds = clusterIds;
    this.isInitialized = true;
  }

  public boolean equals(Object object) {
    if (this == object) return true;
    if (object == null || getClass() != object.getClass()) return false;
    if (!super.equals(object)) return false;
    ContentApp that = (ContentApp) object;
    return endpointId == that.endpointId;
  }

  public int hashCode() {
    return Objects.hash(super.hashCode(), endpointId);
  }

  @java.lang.Override
  public java.lang.String toString() {
    return "endpointId=" + endpointId;
  }

  public short getEndpointId() {
    return endpointId;
  }

  public List<Integer> getClusterIds() {
    return clusterIds;
  }

  public boolean isInitialized() {
    return isInitialized;
  }
}
