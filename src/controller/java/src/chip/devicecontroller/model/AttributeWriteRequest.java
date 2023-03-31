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
import java.util.Optional;

/** An attribute write request that should be used for interaction model write interaction. */
public final class AttributeWriteRequest {
  private final ChipPathId endpointId, clusterId, attributeId;
  private final Optional<Integer> dataVersion;
  private final byte[] tlv;

  private AttributeWriteRequest(
      ChipPathId endpointId,
      ChipPathId clusterId,
      ChipPathId attributeId,
      byte[] tlv,
      Optional<Integer> dataVersion) {
    this.endpointId = endpointId;
    this.clusterId = clusterId;
    this.attributeId = attributeId;
    this.tlv = tlv.clone();
    this.dataVersion = dataVersion;
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

  public int getDataVersion() {
    return dataVersion.orElse(0);
  }

  public boolean hasDataVersion() {
    return dataVersion.isPresent();
  }

  public byte[] getTlvByteArray() {
    return tlv.clone();
  }

  // check whether the current AttributeWriteRequest has same path as others.
  @Override
  public boolean equals(Object object) {
    if (object instanceof AttributeWriteRequest) {
      AttributeWriteRequest that = (AttributeWriteRequest) object;
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

  public static AttributeWriteRequest newInstance(
      ChipPathId endpointId, ChipPathId clusterId, ChipPathId attributeId, byte[] tlv) {
    return new AttributeWriteRequest(endpointId, clusterId, attributeId, tlv, Optional.empty());
  }

  public static AttributeWriteRequest newInstance(
      ChipPathId endpointId,
      ChipPathId clusterId,
      ChipPathId attributeId,
      byte[] tlv,
      Optional<Integer> dataVersion) {
    return new AttributeWriteRequest(endpointId, clusterId, attributeId, tlv, dataVersion);
  }

  /** Create a new {@link AttributeWriteRequest} with only concrete ids. */
  public static AttributeWriteRequest newInstance(
      long endpointId, long clusterId, long attributeId, byte[] tlv) {
    return new AttributeWriteRequest(
        ChipPathId.forId(endpointId),
        ChipPathId.forId(clusterId),
        ChipPathId.forId(attributeId),
        tlv,
        Optional.empty());
  }

  public static AttributeWriteRequest newInstance(
      long endpointId,
      long clusterId,
      long attributeId,
      byte[] tlv,
      Optional<Integer> dataVersion) {
    return new AttributeWriteRequest(
        ChipPathId.forId(endpointId),
        ChipPathId.forId(clusterId),
        ChipPathId.forId(attributeId),
        tlv,
        dataVersion);
  }
}
