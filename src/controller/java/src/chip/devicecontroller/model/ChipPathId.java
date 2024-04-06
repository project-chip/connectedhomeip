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
package chip.devicecontroller.model;

import java.util.Objects;

/** Class representing a potentially wildcard identifier for endpoints/clusters/attributes. */
public class ChipPathId {
  private final long id;
  private final IdType type;

  public enum IdType {
    // Enum names are read by JNI code.
    CONCRETE,
    WILDCARD;
  }

  private ChipPathId(long id, IdType type) {
    this.id = id;
    this.type = type;
  }

  public long getId() {
    return id;
  }

  public IdType getType() {
    return type;
  }

  // For use in JNI.
  long getId(long wildCardValue) {
    return type == IdType.WILDCARD ? wildCardValue : id;
  }

  @Override
  public boolean equals(Object object) {
    if (object instanceof ChipPathId) {
      ChipPathId that = (ChipPathId) object;
      return this.type == that.type && this.id == that.id;
    }
    return false;
  }

  @Override
  public int hashCode() {
    return Objects.hash(id, type);
  }

  @Override
  public String toString() {
    return type == IdType.WILDCARD ? "WILDCARD" : String.valueOf(id);
  }

  public static ChipPathId forId(long id) {
    return new ChipPathId(id, IdType.CONCRETE);
  }

  public static ChipPathId forWildcard() {
    return new ChipPathId(-1, IdType.WILDCARD);
  }
}
