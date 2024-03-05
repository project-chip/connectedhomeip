/*
 *   Copyright (c) 2024 Project CHIP Authors
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
import java.util.Optional;

/** Class for Interaction Model Status * */
public final class Status {
  public enum Code {
    Success(0x0),
    Failure(0x01),
    InvalidSusbscription(0x7d),
    UnsupportedAccess(0x7e),
    UnsupportedEndPoint(0x7f),
    InvalidAction(0x80),
    UnsupportedCommand(0x81),
    Deprecated82(0x82),
    Deprecated83(0x83),
    Deprecated84(0x84),
    InvalidCommand(0x85),
    UnsupportedAttribute(0x86),
    ConstraintError(0x87),
    UnsupportedWrite(0x88),
    ResourceExhausted(0x89),
    Deprecated8a(0x8a),
    NotFound(0x8b),
    UnreportableAttribute(0x8c),
    InvalidDataType(0x8d),
    Deprecated8e(0x8e),
    UnsupportedRead(0x8f),
    Deprecated90(0x90),
    Deprecated91(0x91),
    DataVersionMismatch(0x92),
    Deprecated93(0x93),
    Timeout(0x94),
    Reserved95(0x95),
    Reserved96(0x96),
    Reserved97(0x97),
    Reserved98(0x98),
    Reserved99(0x99),
    Reserved9a(0x9a),
    Busy(0x9c),
    Deprecatedc0(0xc0),
    Deprecatedc1(0xc1),
    Deprecatedc2(0xc2),
    UnsupportedCluster(0xc3),
    Deprecatedc4(0xc4),
    NoUpstreamSubsricption(0xc5),
    NeedTimedInteraction(0xc6),
    UnsupportedEvent(0xc7),
    PathExhausted(0xc8),
    TimedRequestMismatch(0xc9),
    FailsafeRequired(0xca),
    InvalidInState(0xcb),
    NoCommandResponse(0xcc),
    WriteIgnored(0xf0);

    private int id = 0;

    Code(int id) {
      this.id = id;
    }

    public int getId() {
      return id;
    }

    public static Code fromId(int id) {
      for (Code type : values()) {
        if (type.getId() == id) {
          return type;
        }
      }
      return null;
    }
  }

  private Code status = Code.Success;
  private Optional<Integer> clusterStatus;

  private Status(int status, Optional<Integer> clusterStatus) {
    this.status = Code.fromId(status);
    this.clusterStatus = clusterStatus;
  }

  // Getters
  public Code getStatus() {
    return status;
  }

  public Optional<Integer> getClusterStatus() {
    return clusterStatus;
  }

  public String toString() {
    return String.format(
        Locale.ENGLISH,
        "status %s, clusterStatus %s",
        status.name(),
        clusterStatus.isPresent() ? String.valueOf(clusterStatus.get()) : "None");
  }

  public static Status newInstance(int status) {
    return new Status(status, Optional.empty());
  }

  public static Status newInstance(int status, Integer clusterStatus) {
    return new Status(status, Optional.ofNullable(clusterStatus));
  }

  public static Status newInstance(int status, Optional<Integer> clusterStatus) {
    return new Status(status, clusterStatus);
  }
}
