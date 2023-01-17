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

/*
 *   This class is provides the JNI interface to the linux layer of the ContentAppPlatform
 */
public class AppPlatform {
  private static final String TAG = "AppPlatform";

  public AppPlatform(ContentAppEndpointManager manager) {
    nativeInit(manager);
  }

  // Initializes the ContentAppPlatform on startup
  public native void nativeInit(ContentAppEndpointManager manager);

  // Method to add a content app as a new endpoint
  public native int addContentApp(
      String vendorName,
      int vendorId,
      String appName,
      int productId,
      String appVersion,
      ContentAppEndpointManager manager);

  // Method to add a content app at an existing endpoint after restart of the matter server
  public native int addContentAppAtEndpoint(
      String vendorName,
      int vendorId,
      String appName,
      int productId,
      String appVersion,
      int endpointId,
      ContentAppEndpointManager manager);

  // Method to remove content app as endpoint (happens when the app is uninstalled)
  public native int removeContentApp(int endpointId);

  // Method to report attribute change for content app endpoints to the SDK
  public native void reportAttributeChange(int endpointId, int clusterId, int attributeId);

  // Method to add the current vendorId of the node as an admin to enable clients from same vendor
  // to be admins
  public native void addSelfVendorAsAdmin();

  static {
    System.loadLibrary("TvApp");
  }
}
