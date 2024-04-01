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

  /**
   * Called to when CancelCommissioning is received via UDC. Indicates that commissioner can stop
   * showing the passcode entry or display dialog. For example, can show text such as "Commissioning
   * cancelled by client" before hiding dialog.
   */
  void hidePromptsOnCancel(int vendorId, int productId, String commissioneeName);

  /**
   * Called to display the given setup passcode to the user, for commissioning the given
   * commissioneeName with the given vendorId and productId, and provide instructions for where to
   * enter it in the commissionee (when pairingHint and pairingInstruction are provided). For
   * example "Casting Passcode: [passcode]. For more instructions, click here."
   */
  void promptWithCommissionerPasscode(
      int vendorId,
      int productId,
      String commissioneeName,
      long passcode,
      int pairingHint,
      String pairingInstruction);

  /*
   *   Called to notify the user that commissioning succeeded. It can be in form of UI Notification.
   */
  void promptCommissioningStarted(int vendorId, int productId, String commissioneeName);

  /*
   *   Called to notify the user that commissioning succeeded. It can be in form of UI Notification.
   */
  void promptCommissioningSucceeded(int vendorId, int productId, String commissioneeName);

  /*
   *   Called to notify the user that commissioning succeeded. It can be in form of UI Notification.
   */
  void promptCommissioningFailed(String commissioneeName, String error);

  /*
   * Called to display a message on the screen
   */
  void promptWithMessage(Message message);
}
