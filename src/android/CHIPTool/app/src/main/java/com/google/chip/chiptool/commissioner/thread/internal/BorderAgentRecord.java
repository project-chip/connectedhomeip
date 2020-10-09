/*
 *   Copyright (c) 2020 Project CHIP Authors
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

package com.google.chip.chiptool.commissioner.thread.internal;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.room.ColumnInfo;
import androidx.room.Entity;
import androidx.room.PrimaryKey;

@Entity(tableName = "border_agent_table")
public class BorderAgentRecord {

  @PrimaryKey
  @NonNull
  @ColumnInfo(name = "discriminator")
  private String discriminator;

  @NonNull
  @ColumnInfo(name = "network_name")
  private String networkName;

  @NonNull
  @ColumnInfo(name = "extended_pan_id", typeAffinity = ColumnInfo.BLOB)
  private byte[] extendedPanId;

  @NonNull
  @ColumnInfo(name = "pskc", typeAffinity = ColumnInfo.BLOB)
  private byte[] pskc;

  @Nullable
  @ColumnInfo(name = "active_operational_dataset", typeAffinity = ColumnInfo.BLOB)
  private byte[] activeOperationalDataset;

  public BorderAgentRecord(
      @NonNull String discriminator,
      @NonNull String networkName,
      @NonNull byte[] extendedPanId,
      @NonNull byte[] pskc,
      @Nullable byte[] activeOperationalDataset) {
    this.discriminator = discriminator;
    this.networkName = networkName;
    this.extendedPanId = extendedPanId;
    this.pskc = pskc;
    this.activeOperationalDataset = activeOperationalDataset;
  }

  @NonNull
  public String getDiscriminator() {
    return discriminator;
  }

  @NonNull
  public String getNetworkName() {
    return networkName;
  }

  @NonNull
  public byte[] getExtendedPanId() {
    return extendedPanId;
  }

  @NonNull
  public byte[] getPskc() {
    return pskc;
  }

  @Nullable
  public byte[] getActiveOperationalDataset() {
    return activeOperationalDataset;
  }
}
