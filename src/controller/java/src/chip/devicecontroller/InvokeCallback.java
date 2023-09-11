/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package chip.devicecontroller;

import chip.devicecontroller.model.InvokeElement;

/** An interface for receiving invoke response. */
public interface InvokeCallback {

  /**
   * OnError will be called when an error occurs after failing to call
   *
   * @param Exception The IllegalStateException which encapsulated the error message, the possible
   *     chip error could be - CHIP_ERROR_TIMEOUT: A response was not received within the expected
   *     response timeout. - CHIP_ERROR_*TLV*: A malformed, non-compliant response was received from
   *     the server. - CHIP_ERROR encapsulating the converted error from the StatusIB: If we got a
   *     non-path-specific status response from the server. - CHIP_ERROR*: All other cases.
   */
  void onError(Exception e);

  /**
   * OnResponse will be called when a invoke response has been received and processed for the given
   * path.
   *
   * @param invokeElement The invoke element in invoke response that could have null or nonNull tlv
   *     data
   * @param successCode If data in InvokeElment is null, successCode can be any success status,
   *     including possibly a cluster-specific one. If data in InvokeElement is not null,
   *     successCode will always be a generic SUCCESS(0) with no-cluster specific information
   */
  void onResponse(InvokeElement invokeElement, long successCode);

  default void onDone() {}
}
