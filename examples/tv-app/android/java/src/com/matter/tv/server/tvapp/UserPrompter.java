/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
