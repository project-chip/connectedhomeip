/*
 *   Copyright (c) 2026 Project CHIP Authors
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
package com.google.matter.alldevices;

public class App {
  private static final App INSTANCE = new App();

  public static App getInstance() {
    return INSTANCE;
  }

  private App() {}

  public native String[] getSupportedDeviceTypes();

  public native boolean startApp(String configurationJson);

  public native String[] getOnboardingCodes(int discriminator);

  public native boolean stopApp();

  public native void resetApp();

  static {
    System.loadLibrary("AllDevicesApp");
  }
}
