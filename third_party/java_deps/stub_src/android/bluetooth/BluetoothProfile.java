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

/** BluetoothProfile.java Stub file to allow compiling standalone, without Android SDK. */
package android.bluetooth;

// Stub class to allow standalone compilation without Android
public final class BluetoothProfile {

  /** The profile is in disconnected state */
  public static final int STATE_DISCONNECTED = 0;
  /** The profile is in connecting state */
  public static final int STATE_CONNECTING = 1;
  /** The profile is in connected state */
  public static final int STATE_CONNECTED = 2;
  /** The profile is in disconnecting state */
  public static final int STATE_DISCONNECTING = 3;
}
