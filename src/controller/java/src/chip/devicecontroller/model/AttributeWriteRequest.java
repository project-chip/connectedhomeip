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
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.annotation.Nullable;
import org.json.JSONException;
import org.json.JSONObject;

/** An attribute write request that should be used for interaction model write interaction. */
public final class AttributeWriteRequest {
  private static final Logger logger = Logger.getLogger(AttributeWriteRequest.class.getName());
  @Nullable private final ChipPathId endpointId;
  private final ChipPathId clusterId, attributeId;
  private final Optional<Integer> dataVersion;
  @Nullable private final byte[] tlv;
  @Nullable private final JSONObject json;

  private AttributeWriteRequest(
      @Nullable ChipPathId endpointId,
      ChipPathId clusterId,
      ChipPathId attributeId,
      @Nullable byte[] tlv,
      @Nullable String jsonString,
      Optional<Integer> dataVersion) {
    this.endpointId = endpointId;
    this.clusterId = clusterId;
    this.attributeId = attributeId;

    if (tlv != null) {
      this.tlv = tlv.clone();
    } else {
      this.tlv = null;
    }

    JSONObject jsonObject = null;
    if (jsonString != null) {
      try {
        jsonObject = new JSONObject(jsonString);
      } catch (JSONException ex) {
        logger.log(Level.SEVERE, "Error parsing JSON string", ex);
      }
    }

    this.json = jsonObject;
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

  // For use in JNI.
  long getEndpointId(long wildcardValue) {
    return endpointId.getId(wildcardValue);
  }

  long getClusterId(long wildcardValue) {
    return clusterId.getId(wildcardValue);
  }

  long getAttributeId(long wildcardValue) {
    return attributeId.getId(wildcardValue);
  }

  public int getDataVersion() {
    return dataVersion.orElse(0);
  }

  public boolean hasDataVersion() {
    return dataVersion.isPresent();
  }

  @Nullable
  public byte[] getTlvByteArray() {
    if (tlv != null) {
      return tlv.clone();
    }
    return null;
  }

  @Nullable
  public JSONObject getJson() {
    return json;
  }

  @Nullable
  public String getJsonString() {
    if (json == null) return null;
    return json.toString();
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
    return new AttributeWriteRequest(
        endpointId, clusterId, attributeId, tlv, null, Optional.empty());
  }

  public static AttributeWriteRequest newInstance(
      ChipPathId endpointId,
      ChipPathId clusterId,
      ChipPathId attributeId,
      byte[] tlv,
      Optional<Integer> dataVersion) {
    return new AttributeWriteRequest(endpointId, clusterId, attributeId, tlv, null, dataVersion);
  }

  /** Create a new {@link AttributeWriteRequest} with only concrete ids. */
  public static AttributeWriteRequest newInstance(
      int endpointId, long clusterId, long attributeId, byte[] tlv) {
    return new AttributeWriteRequest(
        ChipPathId.forId(endpointId),
        ChipPathId.forId(clusterId),
        ChipPathId.forId(attributeId),
        tlv,
        null,
        Optional.empty());
  }

  /** Create a new {@link AttributeWriteRequest} with only concrete ids. */
  public static AttributeWriteRequest newInstance(
      int endpointId, long clusterId, long attributeId, byte[] tlv, Optional<Integer> dataVersion) {
    return new AttributeWriteRequest(
        ChipPathId.forId(endpointId),
        ChipPathId.forId(clusterId),
        ChipPathId.forId(attributeId),
        tlv,
        null,
        dataVersion);
  }

  public static AttributeWriteRequest newInstance(
      ChipPathId endpointId, ChipPathId clusterId, ChipPathId attributeId, String jsonString) {
    return new AttributeWriteRequest(
        endpointId, clusterId, attributeId, null, jsonString, Optional.empty());
  }

  /** Create a new {@link AttributeWriteRequest} with only concrete ids. */
  public static AttributeWriteRequest newInstance(
      int endpointId, long clusterId, long attributeId, String jsonString) {
    return new AttributeWriteRequest(
        ChipPathId.forId(endpointId),
        ChipPathId.forId(clusterId),
        ChipPathId.forId(attributeId),
        null,
        jsonString,
        Optional.empty());
  }

  public static AttributeWriteRequest newInstance(
      ChipPathId endpointId,
      ChipPathId clusterId,
      ChipPathId attributeId,
      String jsonString,
      Optional<Integer> dataVersion) {
    return new AttributeWriteRequest(
        endpointId, clusterId, attributeId, null, jsonString, dataVersion);
  }

  /** Create a new {@link AttributeWriteRequest} with only concrete ids. */
  public static AttributeWriteRequest newInstance(
      int endpointId,
      long clusterId,
      long attributeId,
      String jsonString,
      Optional<Integer> dataVersion) {
    return new AttributeWriteRequest(
        ChipPathId.forId(endpointId),
        ChipPathId.forId(clusterId),
        ChipPathId.forId(attributeId),
        null,
        jsonString,
        dataVersion);
  }
}
