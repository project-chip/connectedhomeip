/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
