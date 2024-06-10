/**
 * Copyright (c) 2024 Project CHIP Authors All rights reserved.
 *
 * <p>Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of the License at
 *
 * <p>http://www.apache.org/licenses/LICENSE-2.0
 *
 * <p>Unless required by applicable law or agreed to in writing, software distributed under the
 * License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.matter.casting.core;

import chip.devicecontroller.ChipClusters;
import com.matter.casting.support.DeviceTypeStruct;
import java.util.List;

/** This represents an Endpoint on a CastingPlayer e.g. a Speaker or a Matter Content App */
public interface Endpoint {
  int getId();

  int getVendorId();

  int getProductId();

  List<DeviceTypeStruct> getDeviceTypeList();

  /** Get an instance of a cluster based on its Class */
  <T extends ChipClusters.BaseChipCluster> T getCluster(Class<T> clusterClass);

  /** Get the CastingPlayer that this Endpoint is a part of. */
  CastingPlayer getCastingPlayer();
}
