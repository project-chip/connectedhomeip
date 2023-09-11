/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 * SPDX-FileCopyrightText: 2015-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/** BluetoothGattService.java Stub file to allow compiling standalone, without Android SDK. */
package android.bluetooth;

import java.util.List;
import java.util.UUID;

// Stub class to allow standalone compilation without Android
public class BluetoothGattService {
  protected UUID mUuid;

  public BluetoothGattCharacteristic getCharacteristic(UUID uuid) {
    return null;
  }

  public List<BluetoothGattCharacteristic> getCharacteristics() {
    return null;
  }

  public UUID getUuid() {
    return mUuid;
  }
}
