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
 */
package com.matter.casting.core;

import com.matter.casting.support.DeviceTypeStruct;
import java.util.List;
import java.util.Objects;

public class MatterEndpoint implements Endpoint {
  private static final String TAG = MatterEndpoint.class.getSimpleName();
  protected long _cppEndpoint;

  @Override
  public native int getId();

  @Override
  public native int getVendorId();

  @Override
  public native int getProductId();

  @Override
  public native List<DeviceTypeStruct> getDeviceTypeList();

  @Override
  public native CastingPlayer getCastingPlayer();

  @Override
  public String toString() {
    return "MatterEndpoint{" + "id=" + getId() + '}';
  }

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;
    MatterEndpoint that = (MatterEndpoint) o;
    return getId() == that.getId();
  }

  @Override
  public int hashCode() {
    return Objects.hash(getId());
  }
}
