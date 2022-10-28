/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    Copyright (c) 2015-2017 Nest Labs, Inc.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
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
