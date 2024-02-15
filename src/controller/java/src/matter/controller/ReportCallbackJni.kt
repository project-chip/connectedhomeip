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
import matter.controller.model.EventPath
import matter.controller.model.NodeState

/** JNI wrapper callback class for [ReportCallback]. */
class ReportCallbackJni(
  subscriptionEstablishedCallback: SubscriptionEstablishedCallback?,
  reportCallback: ReportCallback,
  resubscriptionAttemptCallback: ResubscriptionAttemptCallback?
) {
  private val wrappedSubscriptionEstablishedCallback: SubscriptionEstablishedCallback? =
    subscriptionEstablishedCallback
  private val wrappedResubscriptionAttemptCallback: ResubscriptionAttemptCallback? =
    resubscriptionAttemptCallback

  private val wrappedReportCallback: ReportCallback = reportCallback
  private var callbackHandle: Long

  private var nodeState: NodeState? = null

  init {
    this.callbackHandle =
      newCallback(subscriptionEstablishedCallback, resubscriptionAttemptCallback)
  }

  private external fun newCallback(
    subscriptionEstablishedCallback: SubscriptionEstablishedCallback?,
    resubscriptionAttemptCallback: ResubscriptionAttemptCallback?
  ): Long

  private external fun deleteCallback(callbackHandle: Long)

  fun getJniHandle(): Long {
    return callbackHandle
  }

  // Called from native code only, which ignores access modifiers.
  private fun onReportBegin() {
    nodeState = NodeState()
  }

  private fun onReportEnd() {
    if (nodeState != null) {
      wrappedReportCallback.onReport(nodeState!!)
    }
    nodeState = null
  }

  private fun getNodeState(): NodeState? {
    return nodeState
  }

  private fun onError(
    isAttributePath: Boolean,
    attributeEndpointId: Int,
    attributeClusterId: Long,
    attributeId: Long,
    isEventPath: Boolean,
    eventEndpointId: Int,
    eventClusterId: Long,
    eventId: Long,
    e: Exception
  ) {
    wrappedReportCallback.onError(
      if (isAttributePath)
        AttributePath(
          attributeEndpointId.toUShort(),
          attributeClusterId.toUInt(),
          attributeId.toUInt()
        )
      else null,
      if (isEventPath)
        EventPath(eventEndpointId.toUShort(), eventClusterId.toUInt(), eventId.toUInt())
      else null,
      e
    )
  }

  private fun onDone() {
    wrappedReportCallback.onDone()
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
