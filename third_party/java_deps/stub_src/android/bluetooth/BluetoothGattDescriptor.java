/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 * SPDX-FileCopyrightText: 2015-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/** BluetoothGattDescriptor.java Stub file to allow compiling standalone, without Android SDK. */
package android.bluetooth;

import java.util.UUID;

// Stub class to allow standalone compilation without Android
public class BluetoothGattDescriptor {
  public static final byte[] ENABLE_NOTIFICATION_VALUE = {0x01, 0x00};

  public static final byte[] ENABLE_INDICATION_VALUE = {0x02, 0x00};

  public static final byte[] DISABLE_NOTIFICATION_VALUE = {0x00, 0x00};

  protected UUID mUuid;

  public BluetoothGattCharacteristic getCharacteristic() {
    return null;
  }

  public UUID getUuid() {
    return mUuid;
  }

  public byte[] getValue() {
    return null;
  }

  public boolean setValue(byte[] value) {
    return true;
  }
}
