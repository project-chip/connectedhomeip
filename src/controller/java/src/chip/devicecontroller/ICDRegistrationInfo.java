/*
 *   Copyright (c) 2020-2023 Project CHIP Authors
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
package chip.devicecontroller;

import javax.annotation.Nullable;

/** Class for holding ICD registration infomation. */
public class ICDRegistrationInfo {
  @Nullable private final Long checkInNodeId;
  @Nullable private final Long monitoredSubject;
  @Nullable private final byte[] symmetricKey;
  @Nullable private final Integer clientType;
  @Nullable private final Long stayActiveDurationMsec;

  private ICDRegistrationInfo(Builder builder) {
    this.checkInNodeId = builder.checkInNodeId;
    this.monitoredSubject = builder.monitoredSubject;
    this.symmetricKey = builder.symmetricKey;
    this.clientType = builder.clientType;
    this.stayActiveDurationMsec = builder.stayActiveDurationMsec;
  }

  /** Returns the duration period to stay active. */
  public Long getICDStayActiveDurationMsec() {
    return stayActiveDurationMsec;
  }

  /** Returns the check in node ID. */
  public Long getCheckInNodeId() {
    return checkInNodeId;
  }

  /** Returns the monitored subject of the ICD. */
  public Long getMonitoredSubject() {
    return monitoredSubject;
  }

  /** Returns the 16 bytes ICD symmetric key. */
  public byte[] getSymmetricKey() {
    return symmetricKey;
  }

  public Integer getClientType() {
    return clientType;
  }

  public static Builder newBuilder() {
    return new Builder();
  }

  /** Builder for {@link ICDRegistrationInfo}. */
  public static class Builder {
    @Nullable private Long checkInNodeId = null;
    @Nullable private Long monitoredSubject = null;
    @Nullable private byte[] symmetricKey = null;
    @Nullable private Integer clientType = null;
    @Nullable private Long stayActiveDurationMsec = null;

    private Builder() {}

    /** The check-in node id for the ICD. If not set this value, node id of the commissioner. */
    public Builder setCheckInNodeId(long checkInNodeId) {
      this.checkInNodeId = checkInNodeId;
      return this;
    }

    /**
     * The monitored subject of the ICD. If not set this value, the node id used for
     * icd-check-in-nodeid
     */
    public Builder setMonitoredSubject(long monitoredSubject) {
      this.monitoredSubject = monitoredSubject;
      return this;
    }

    /**
     * The 16 bytes ICD symmetric key, If not set this value, this value will be randomly generated.
     */
    public Builder setSymmetricKey(byte[] symmetricKey) {
      this.symmetricKey = symmetricKey;
      return this;
    }

    public Builder setClientType(Integer clientType) {
      this.clientType = clientType;
      return this;
    }

    /**
     * Request LIT device to stay active for specific duration after commission completes, the upper
     * bound is 30 seconds.
     */
    public Builder setICDStayActiveDurationMsec(Long stayActiveDurationMsec) {
      this.stayActiveDurationMsec = stayActiveDurationMsec;
      return this;
    }

    public ICDRegistrationInfo build() {
      return new ICDRegistrationInfo(this);
    }
  }
}
