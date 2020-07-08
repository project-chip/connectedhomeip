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

package com.google.chip.chiptool;

// Manages a single instance of CHIPDeviceController
// TODO: Rename this class to CHIPController when JNI ChipDeviceController API is available.
public class CHIPNativeBridge {

  // TODO: Add instance ref for CHIPDeviceController here

  // static {
  //  System.loadLibrary("native-lib");
  // }

  private static CHIPNativeBridge instance = null;

  private CHIPNativeBridge() {
    // TODO: Construct CHIPDeviceController here and initialize callbacks
  }

  public static CHIPNativeBridge getInstance() {
    if (instance == null) instance = new CHIPNativeBridge();
    return instance;
  }

  // TODO: Sample function to demonstrate end-to-end calls work. Remove this after JNI layer
  // becomes available.
  public native String base41Encode();

  public boolean isConnected() {
    return false;
  }

  // Human readable connection info for display purposes.
  public String getConnectionInfo() {
    if (!isConnected()) {
      return "NO CONNECTION";
    }

    // TODO: Get from managed ChipDeviceController
    return "127.0.0.1";
  }

  // TODO Add connection management methods here
}
