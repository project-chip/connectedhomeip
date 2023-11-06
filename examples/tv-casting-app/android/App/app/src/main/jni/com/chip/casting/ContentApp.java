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
