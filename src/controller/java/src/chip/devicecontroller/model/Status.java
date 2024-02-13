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

public final class Status {
  private Integer status;
  private Optional<Integer> clusterStatus;

  private Status(int status, Optional<Integer> clusterStatus) {
    this.status = status;
    this.clusterStatus = clusterStatus;
  }

  // Getters
  public Integer getStatus() {
    return status;
  }

  public Optional<Integer> getClusterStatus() {
    return clusterStatus;
  }

  public String toString() {
    return String.format(
        Locale.ENGLISH,
        "status %s, clusterStatus %s",
        String.valueOf(status),
        clusterStatus.isPresent() ? String.valueOf(clusterStatus.get()) : "None");
  }

  public static Status newInstance(int status, int clusterStatus) {
    return new Status(status, Optional.of(clusterStatus));
  }

  public static Status newInstance(int status) {
    return new Status(status, Optional.empty());
  }
}
