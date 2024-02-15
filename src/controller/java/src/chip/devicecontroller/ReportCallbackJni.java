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
package chip.devicecontroller;

import chip.devicecontroller.model.ChipAttributePath;
import chip.devicecontroller.model.ChipEventPath;
import chip.devicecontroller.model.NodeState;
import javax.annotation.Nullable;

/** JNI wrapper callback class for {@link ReportCallback}. */
public class ReportCallbackJni {
  @Nullable private SubscriptionEstablishedCallback wrappedSubscriptionEstablishedCallback;
  @Nullable private ResubscriptionAttemptCallback wrappedResubscriptionAttemptCallback;
  private ReportCallback wrappedReportCallback;
  private long callbackHandle;
  @Nullable private NodeState nodeState;

  public ReportCallbackJni(
      @Nullable SubscriptionEstablishedCallback subscriptionEstablishedCallback,
      ReportCallback reportCallback,
      ResubscriptionAttemptCallback resubscriptionAttemptCallback) {
    this.wrappedSubscriptionEstablishedCallback = subscriptionEstablishedCallback;
    this.wrappedReportCallback = reportCallback;
    this.wrappedResubscriptionAttemptCallback = resubscriptionAttemptCallback;
    this.callbackHandle =
        newCallback(subscriptionEstablishedCallback, resubscriptionAttemptCallback);
  }

  long getCallbackHandle() {
    return callbackHandle;
  }

  private native long newCallback(
      @Nullable SubscriptionEstablishedCallback subscriptionEstablishedCallback,
      @Nullable ResubscriptionAttemptCallback resubscriptionAttemptCallback);

  private native void deleteCallback(long callbackHandle);

  // Called from native code only, which ignores access modifiers.
  private void onReportBegin() {
    nodeState = new NodeState();
  }

  private void onReportEnd() {
    if (nodeState != null) {
      wrappedReportCallback.onReport(nodeState);
    }
    nodeState = null;
  }

  private NodeState getNodeState() {
    return nodeState;
  }

  private void onError(
      boolean isAttributePath,
      int attributeEndpointId,
      long attributeClusterId,
      long attributeId,
      boolean isEventPath,
      int eventEndpointId,
      long eventClusterId,
      long eventId,
      Exception e) {
    wrappedReportCallback.onError(
        isAttributePath
            ? ChipAttributePath.newInstance(attributeEndpointId, attributeClusterId, attributeId)
            : null,
        isEventPath ? ChipEventPath.newInstance(eventEndpointId, eventClusterId, eventId) : null,
        e);
  }

  private void onDone() {
    wrappedReportCallback.onDone();
  }

  // TODO(#8578): Replace finalizer with PhantomReference.
  @SuppressWarnings("deprecation")
  protected void finalize() throws Throwable {
    super.finalize();

    if (callbackHandle != 0) {
      deleteCallback(callbackHandle);
      callbackHandle = 0;
    }
  }
}
