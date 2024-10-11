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

import matter.controller.model.AttributePath
import matter.controller.model.Status

/** JNI wrapper callback class for [WriteAttributesCallback]. */
class WriteAttributesCallbackJni(
  private val wrappedWriteAttributesCallback: WriteAttributesCallback
) {
  private var callbackHandle: Long

  init {
    this.callbackHandle = newCallback()
  }

  private external fun newCallback(): Long

  private external fun deleteCallback(callbackHandle: Long)

  fun getCallbackHandle(): Long {
    return callbackHandle
  }

  // Called from native code only, which ignores access modifiers.
  private fun onError(
    isAttributePath: Boolean,
    endpointId: Int,
    clusterId: Long,
    attributeId: Long,
    e: Exception
  ) {
    wrappedWriteAttributesCallback.onError(
      if (isAttributePath)
        AttributePath(endpointId.toUShort(), clusterId.toUInt(), attributeId.toUInt())
      else null,
      e
    )
  }

  private fun onResponse(
    endpointId: Int,
    clusterId: Long,
    attributeId: Long,
    status: Int,
    clusterStatus: Int?
  ) {
    wrappedWriteAttributesCallback.onResponse(
      AttributePath(endpointId.toUShort(), clusterId.toUInt(), attributeId.toUInt()),
      Status(status, clusterStatus)
    )
  }

  private fun onDone() {
    wrappedWriteAttributesCallback.onDone()
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
