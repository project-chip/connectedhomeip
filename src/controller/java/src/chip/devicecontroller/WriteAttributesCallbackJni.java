/*
 *   Copyright (c) 2023 Project CHIP Authors
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

/** JNI wrapper callback class for {@link WriteAttributesCallback}. */
public class WriteAttributesCallbackJni {
  private final WriteAttributesCallback wrappedWriteAttributesCallback;
  private long callbackHandle;

  public WriteAttributesCallbackJni(WriteAttributesCallback wrappedWriteAttributesCallback) {
    this.wrappedWriteAttributesCallback = wrappedWriteAttributesCallback;
    this.callbackHandle = newCallback(wrappedWriteAttributesCallback);
  }

  long getCallbackHandle() {
    return callbackHandle;
  }

  private native long newCallback(WriteAttributesCallback wrappedCallback);

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
