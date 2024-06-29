/*
 *   Copyright (c) 2024 Project CHIP Authors
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

public class Application {

  public int catalogVendorId;
  public String applicationId;

  public Application(int catalogVendorId, String applicationId) {
    this.catalogVendorId = catalogVendorId;
    this.applicationId = applicationId;
  }

  public Application createApplication(int catalogVendorId, String applicationId) {
    return new Application(catalogVendorId, applicationId);
  }

  @Override
  public String toString() {
    return "Application{" + "catalogVendorId=" + catalogVendorId + ", applicationId='" + '\'' + '}';
  }
}
