/*
 *   Copyright (c) 2021 Project CHIP Authors
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
package com.matter.tv.server.tvapp;

public class AppPlatform {
  private static final String TAG = "AppPlatform";

  public AppPlatform(UserPrompter userPrompter, ContentAppEndpointManager manager) {
    nativeInit(userPrompter, manager);
  }

  public native void nativeInit(UserPrompter userPrompter, ContentAppEndpointManager manager);

  public native int addContentApp(
      String vendorName,
      int vendorId,
      String appName,
      int productId,
      String appVersion,
      ContentAppEndpointManager manager);

  public native int removeContentApp(int endpointId);

  static {
    System.loadLibrary("TvApp");
  }
}
