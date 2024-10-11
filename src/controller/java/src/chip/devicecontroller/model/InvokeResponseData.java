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

/** Class for tracking invoke response data with either data or status */
public final class InvokeResponseData {
  private static final Logger logger = Logger.getLogger(InvokeResponseData.class.getName());
  @Nullable private final ChipPathId endpointId;
  private final ChipPathId clusterId, commandId;
  private final Optional<Integer> commandRef;
  @Nullable private final byte[] tlv;
  @Nullable private final JSONObject json;
  @Nullable private final Status status;

  private InvokeResponseData(
      ChipPathId endpointId,
      ChipPathId clusterId,
      ChipPathId commandId,
      Optional<Integer> commandRef,
      @Nullable byte[] tlv,
      @Nullable String jsonString) {
    this.endpointId = endpointId;
    this.clusterId = clusterId;
    this.commandId = commandId;
    this.commandRef = commandRef;

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
    this.status = null;
  }

  private InvokeResponseData(
      ChipPathId endpointId,
      ChipPathId clusterId,
      ChipPathId commandId,
      Optional<Integer> commandRef,
      int status,
      Optional<Integer> clusterStatus) {
    this.endpointId = endpointId;
    this.clusterId = clusterId;
    this.commandId = commandId;
    this.commandRef = commandRef;
    this.status = Status.newInstance(status, clusterStatus);
    this.tlv = null;
    this.json = null;
  }

  public ChipPathId getEndpointId() {
    return endpointId;
  }

  public ChipPathId getClusterId() {
    return clusterId;
  }

  public ChipPathId getCommandId() {
    return commandId;
  }

  public Optional<Integer> getCommandRef() {
    return commandRef;
  }

  @Nullable
  public Status getStatus() {
    return status;
  }

  // For use in JNI.
  private long getEndpointId(long wildcardValue) {
    return endpointId.getId(wildcardValue);
  }

  private long getClusterId(long wildcardValue) {
    return clusterId.getId(wildcardValue);
  }

  private long getCommandId(long wildcardValue) {
    return commandId.getId(wildcardValue);
  }

  public boolean isEndpointIdValid() {
    return endpointId != null;
  }

  @Nullable
  public byte[] getTlvByteArray() {
    if (tlv != null) {
      return tlv.clone();
    }
    return null;
  }

  @Nullable
  public JSONObject getJsonObject() {
    return json;
  }

  @Nullable
  public String getJsonString() {
    if (json == null) return null;
    return json.toString();
  }

  // check whether the current InvokeResponseData has same path as others.
  @Override
  public boolean equals(Object object) {
    if (object instanceof InvokeResponseData) {
      InvokeResponseData that = (InvokeResponseData) object;
      return Objects.equals(this.endpointId, that.endpointId)
          && Objects.equals(this.clusterId, that.clusterId)
          && Objects.equals(this.commandId, that.commandId);
    }
    return false;
  }

  @Override
  public int hashCode() {
    return Objects.hash(endpointId, clusterId, commandId);
  }

  @Override
  public String toString() {
    return String.format(
        Locale.ENGLISH,
        "Endpoint %s, cluster %s, command %s, payload: %s, status: %s",
        endpointId,
        clusterId,
        commandId,
        json == null ? "null" : getJsonString(),
        status == null ? "null" : status.toString());
  }

  public static InvokeResponseData newInstance(
      ChipPathId endpointId,
      ChipPathId clusterId,
      ChipPathId commandId,
      Optional<Integer> commandRef,
      @Nullable byte[] tlv,
      @Nullable String jsonString) {
    return new InvokeResponseData(endpointId, clusterId, commandId, commandRef, tlv, jsonString);
  }

  public static InvokeResponseData newInstance(
      int endpointId,
      long clusterId,
      long commandId,
      Optional<Integer> commandRef,
      @Nullable byte[] tlv,
      @Nullable String jsonString) {
    return new InvokeResponseData(
        ChipPathId.forId(endpointId),
        ChipPathId.forId(clusterId),
        ChipPathId.forId(commandId),
        commandRef,
        tlv,
        jsonString);
  }

  public static InvokeResponseData newInstance(
      ChipPathId endpointId,
      ChipPathId clusterId,
      ChipPathId commandId,
      Optional<Integer> commandRef,
      int status,
      Optional<Integer> clusterStatus) {
    return new InvokeResponseData(
        endpointId, clusterId, commandId, commandRef, status, clusterStatus);
  }

  public static InvokeResponseData newInstance(
      int endpointId,
      long clusterId,
      long commandId,
      Optional<Integer> commandRef,
      int status,
      Optional<Integer> clusterStatus) {
    return new InvokeResponseData(
        ChipPathId.forId(endpointId),
        ChipPathId.forId(clusterId),
        ChipPathId.forId(commandId),
        commandRef,
        status,
        clusterStatus);
  }
}
