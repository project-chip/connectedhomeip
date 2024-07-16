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

import matter.controller.model.CommandPath

/** JNI wrapper callback class for [InvokeCallback]. */
class InvokeCallbackJni(val wrappedInvokeCallback: InvokeCallback) {
  private var callbackHandle: Long

  init {
    this.callbackHandle = newCallback()
  }

  private external fun newCallback(): Long

  private external fun deleteCallback(callbackHandle: Long)

  fun getJniHandle(): Long {
    return callbackHandle
  }

  private fun onError(e: Exception) {
    wrappedInvokeCallback.onError(e)
  }

  private fun onResponse(
    endpointId: Int,
    clusterId: Long,
    commandId: Long,
    tlv: ByteArray,
    jsonString: String,
    successCode: Long
  ) {
    wrappedInvokeCallback.onResponse(
      InvokeResponse(
        tlv,
        CommandPath(endpointId.toUShort(), clusterId.toUInt(), commandId.toUInt()),
        jsonString
      ),
      successCode
    )
  }

  private fun onDone() {
    wrappedInvokeCallback.onDone()
  }

  // TODO(#8578): Replace finalizer with PhantomReference.
  @Suppress("deprecation")
  @Throws(Throwable::class)
  protected fun finalize() {
    if (callbackHandle != 0L) {
      deleteCallback(callbackHandle)
      callbackHandle = 0
    }
  }
}
