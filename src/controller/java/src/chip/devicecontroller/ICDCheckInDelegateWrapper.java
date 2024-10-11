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

package chip.devicecontroller;

class ICDCheckInDelegateWrapper {
  private ICDCheckInDelegate delegate;

  ICDCheckInDelegateWrapper(ICDCheckInDelegate delegate) {
    this.delegate = delegate;
  }

  // For JNI call
  @SuppressWarnings("unused")
  private void onCheckInComplete(
      long peerNodeId,
      long checkInNodeId,
      long startCounter,
      long offset,
      long monitoredSubject,
      byte[] icdAesKey,
      byte[] icdHmacKey) {
    delegate.onCheckInComplete(
        new ICDClientInfo(
            peerNodeId,
            checkInNodeId,
            startCounter,
            offset,
            monitoredSubject,
            icdAesKey,
            icdHmacKey));
  }

  @SuppressWarnings("unused")
  private byte[] onKeyRefreshNeeded(
      long peerNodeId,
      long checkInNodeId,
      long startCounter,
      long offset,
      long monitoredSubject,
      byte[] icdAesKey,
      byte[] icdHmacKey) {
    return delegate.onKeyRefreshNeeded(
        new ICDClientInfo(
            peerNodeId,
            checkInNodeId,
            startCounter,
            offset,
            monitoredSubject,
            icdAesKey,
            icdHmacKey));
  }

  @SuppressWarnings("unused")
  private void onKeyRefreshDone(long errorCode) {
    delegate.onKeyRefreshDone(errorCode);
  }
}
