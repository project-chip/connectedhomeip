/*
 *   Copyright (c) 2023 Project CHIP Authors
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
import java.util.logging.Logger;

/** A data version filter that should be used for interaction model write interaction. */
public final class DataVersionFilter {
  private static final Logger logger = Logger.getLogger(DataVersionFilter.class.getName());
  private final ChipPathId endpointId;
  private final ChipPathId clusterId;
  private final long dataVersion;

  private DataVersionFilter(ChipPathId endpointId, ChipPathId clusterId, long dataVersion) {
    this.endpointId = endpointId;
    this.clusterId = clusterId;
    this.dataVersion = dataVersion;
  }

  public ChipPathId getEndpointId() {
    return endpointId;
  }

  public ChipPathId getClusterId() {
    return clusterId;
  }

  public long getDataVersion() {
    return dataVersion;
  }

  // For use in JNI.
  private long getEndpointId(long wildcardValue) {
    return endpointId.getId(wildcardValue);
  }

  private long getClusterId(long wildcardValue) {
    return clusterId.getId(wildcardValue);
  }

  // check whether the current DataVersionFilter has same path as others.
  @Override
  public boolean equals(Object object) {
    if (object instanceof DataVersionFilter) {
      DataVersionFilter that = (DataVersionFilter) object;
      return Objects.equals(this.endpointId, that.endpointId)
          && Objects.equals(this.clusterId, that.clusterId);
    }
    return false;
  }

  @Override
  public int hashCode() {
    return Objects.hash(endpointId, clusterId);
  }

  @Override
  public String toString() {
    return String.format(Locale.ENGLISH, "Endpoint %s, cluster %s", endpointId, clusterId);
  }

  public static DataVersionFilter newInstance(
      ChipPathId endpointId, ChipPathId clusterId, long dataVersion) {
    return new DataVersionFilter(endpointId, clusterId, dataVersion);
  }
}
