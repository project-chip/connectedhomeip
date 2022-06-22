/*
 *   Copyright (c) 2022 Project CHIP Authors
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

public interface UserPrompter {

  /*
   *  Called to prompt the user for consent to allow the given commissioneeName/vendorId/productId to be commissioned.
   * For example "[commissioneeName] is requesting permission to cast to this TV, approve?"
   *
   * If user responds with OK then implementor calls UserPrompterResolver.OnPromptAccepted;
   * If user responds with Cancel then implementor calls calls UserPrompterResolver.OnPromptDeclined();
   *
   */
  void promptForCommissionOkPermission(int vendorId, int productId, String commissioneeName);

  /*
   *  Called to prompt the user to enter the setup pincode displayed by the given commissioneeName/vendorId/productId to be
   * commissioned. For example "Please enter pin displayed in casting app."
   *
   * If user responds with OK then implementor calls UserPrompterResolver.OnPinCodeEntered();
   * If user responds with Cancel then implementor calls UserPrompterResolver.OnPinCodeDeclined();
   *
   */
  void promptForCommissionPinCode(int vendorId, int productId, String commissioneeName);

  /*
   *   Called to notify the user that commissioning succeeded. It can be in form of UI Notification.
   */
  void promptCommissioningSucceeded(int vendorId, int productId, String commissioneeName);

  /*
   *   Called to notify the user that commissioning succeeded. It can be in form of UI Notification.
   */
  void promptCommissioningFailed(String commissioneeName, String error);
}
