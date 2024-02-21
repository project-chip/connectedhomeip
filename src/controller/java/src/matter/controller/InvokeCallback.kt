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
package matter.controller

import java.lang.Exception

/** An interface for receiving invoke response. */
interface InvokeCallback {
  /**
   * OnError will be called when an error occurs after failing to call
   *
   * @param Exception The IllegalStateException which encapsulated the error message, the possible
   *   chip error could be - CHIP_ERROR_TIMEOUT: A response was not received within the expected
   *   response timeout. - CHIP_ERROR_*TLV*: A malformed, non-compliant response was received from
   *   the server. - CHIP_ERROR encapsulating the converted error from the StatusIB: If we got a
   *   non-path-specific status response from the server. - CHIP_ERROR*: All other cases.
   */
  fun onError(ex: Exception)

  /**
   * OnResponse will be called when a invoke response has been received and processed for the given
   * path.
   *
   * @param invokeElement The invoke element in invoke response that could have null or nonNull tlv
   *   data
   * @param successCode If data in InvokeElment is null, successCode can be any success status,
   *   including possibly a cluster-specific one. If data in InvokeElement is not null, successCode
   *   will always be a generic SUCCESS(0) with no-cluster specific information
   */
  fun onResponse(invokeResponse: InvokeResponse?, successCode: Long)

  fun onDone() {}
}
