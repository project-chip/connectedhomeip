/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 * SPDX-FileCopyrightText: 2015-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * BluetoothGattCharacteristic.java Stub file to allow compiling standalone, without Android SDK.
 */
package android.bluetooth;

import java.util.UUID;

// Stub class to allow standalone compilation without Android
public class BluetoothGattCharacteristic {
  protected UUID mUuid;

  public BluetoothGattDescriptor getDescriptor(UUID uuid) {
    return null;
  }

  public BluetoothGattService getService() {
    return null;
  }

  public UUID getUuid() {
    return mUuid;
  }

  public byte[] getValue() {
    return null;
  }

  public boolean setValue(byte[] value) {
    return false;
  }
}
