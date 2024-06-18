/**
 * Copyright (c) 2024 Project CHIP Authors All rights reserved.
 *
 * <p>Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of the License at
 *
 * <p>http://www.apache.org/licenses/LICENSE-2.0
 *
 * <p>Unless required by applicable law or agreed to in writing, software distributed under the
 * License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.matter.casting.support;

/**
 * Feature: Target Content Application - An entry in the IdentificationDeclarationOptions.java
 * TargetAppList which contains a TargetVendorId and an optional TargetProductId.
 */
public class TargetAppInfo {
  /** Target Target Content Application Vendor ID, null means unspecified */
  public Integer vendorId;
  /** Target Target Content Application Product ID, null means unspecified */
  public Integer productId;
}
