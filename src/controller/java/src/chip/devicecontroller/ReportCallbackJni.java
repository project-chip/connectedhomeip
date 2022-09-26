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

import androidx.annotation.Nullable;

/** JNI wrapper callback class for {@link ReportCallback}. */
public class ReportCallbackJni {
  @Nullable private SubscriptionEstablishedCallback wrappedSubscriptionEstablishedCallback;
  @Nullable private ResubscriptionAttemptCallback wrappedResubscriptionAttemptCallback;
  private ReportCallback wrappedReportCallback;
  private long callbackHandle;

  public ReportCallbackJni(
      @Nullable SubscriptionEstablishedCallback subscriptionEstablishedCallback,
      ReportCallback reportCallback,
      ResubscriptionAttemptCallback resubscriptionAttemptCallback) {
    this.wrappedSubscriptionEstablishedCallback = subscriptionEstablishedCallback;
    this.wrappedReportCallback = reportCallback;
    this.wrappedResubscriptionAttemptCallback = resubscriptionAttemptCallback;
    this.callbackHandle =
        newCallback(subscriptionEstablishedCallback, reportCallback, resubscriptionAttemptCallback);
  }

  long getCallbackHandle() {
    return callbackHandle;
  }

  private native long newCallback(
      @Nullable SubscriptionEstablishedCallback subscriptionEstablishedCallback,
      ReportCallback wrappedCallback,
      @Nullable ResubscriptionAttemptCallback resubscriptionAttemptCallback);

  private native void deleteCallback(long callbackHandle);

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
