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

/** An invoke element that should be used for interaction model invoke request and response. */
public final class InvokeElement {
  private static final Logger logger = Logger.getLogger(InvokeElement.class.getName());
  @Nullable private final ChipPathId endpointId;
  private final ChipPathId clusterId, commandId;
  private final Optional<Integer> groupId;
  @Nullable private final byte[] tlv;
  @Nullable private final JSONObject json;

  private InvokeElement(
      ChipPathId endpointId,
      Optional<Integer> groupId,
      ChipPathId clusterId,
      ChipPathId commandId,
      @Nullable byte[] tlv,
      @Nullable String jsonString) {
    this.endpointId = endpointId;
    this.clusterId = clusterId;
    this.commandId = commandId;
    this.groupId = groupId;

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

  public Optional<Integer> getGroupId() {
    return groupId;
  }

  public boolean isEndpointIdValid() {
    return endpointId != null;
  }

  public boolean isGroupIdValid() {
    return groupId.isPresent();
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

  // check whether the current InvokeElement has same path as others.
  @Override
  public boolean equals(Object object) {
    if (object instanceof InvokeElement) {
      InvokeElement that = (InvokeElement) object;
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
        Locale.ENGLISH, "Endpoint %s, cluster %s, command %s", endpointId, clusterId, commandId);
  }

  public static InvokeElement newInstance(
      ChipPathId endpointId,
      ChipPathId clusterId,
      ChipPathId commandId,
      @Nullable byte[] tlv,
      @Nullable String jsonString) {
    return new InvokeElement(endpointId, Optional.empty(), clusterId, commandId, tlv, jsonString);
  }

  /** Create a new {@link InvokeElement} with only concrete ids. */
  public static InvokeElement newInstance(
      int endpointId,
      long clusterId,
      long commandId,
      @Nullable byte[] tlv,
      @Nullable String jsonString) {
    return new InvokeElement(
        ChipPathId.forId(endpointId),
        Optional.empty(),
        ChipPathId.forId(clusterId),
        ChipPathId.forId(commandId),
        tlv,
        jsonString);
  }

  public static InvokeElement newGroupInstance(
      int groupId,
      long clusterId,
      long commandId,
      @Nullable byte[] tlv,
      @Nullable String jsonString) {
    return new InvokeElement(
        null,
        Optional.of(groupId),
        ChipPathId.forId(clusterId),
        ChipPathId.forId(commandId),
        tlv,
        jsonString);
  }

  public static InvokeElement newGroupInstance(
      int groupId,
      ChipPathId clusterId,
      ChipPathId commandId,
      @Nullable byte[] tlv,
      @Nullable String jsonString) {
    return new InvokeElement(null, Optional.of(groupId), clusterId, commandId, tlv, jsonString);
  }
}
