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

/** An invoke element that should be used for interaction model invoke request and response. */
public final class InvokeElement {
  private final ChipPathId endpointId, clusterId, commandId;
  private final byte[] tlv;

  private InvokeElement(
      ChipPathId endpointId, ChipPathId clusterId, ChipPathId commandId, byte[] tlv) {
    this.endpointId = endpointId;
    this.clusterId = clusterId;
    this.commandId = commandId;
    this.tlv = tlv.clone();
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

  public byte[] getTlvByteArray() {
    return tlv.clone();
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
      ChipPathId endpointId, ChipPathId clusterId, ChipPathId commandId, byte[] tlv) {
    return new InvokeElement(endpointId, clusterId, commandId, tlv);
  }

  /** Create a new {@link InvokeElement} with only concrete ids. */
  public static InvokeElement newInstance(
      long endpointId, long clusterId, long commandId, byte[] tlv) {
    return new InvokeElement(
        ChipPathId.forId(endpointId),
        ChipPathId.forId(clusterId),
        ChipPathId.forId(commandId),
        tlv);
  }
}
