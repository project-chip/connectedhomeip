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

import javax.annotation.Nonnull;
import javax.annotation.Nullable;

/**
 * Delegate for ICD check in.
 *
 * <p>See detailed in {@link ChipDeviceController#setICDCheckInDelegate(ICDCheckInDelegate)}
 */
public interface ICDCheckInDelegate {
  /**
   * Callback used to let the application know that a check-in message was received and validated.
   *
   * @param info ICDClientInfo object representing the state associated with the node that sent the
   *     check-in message.
   */
  void onCheckInComplete(@Nonnull ICDClientInfo info);

  /**
   * Callback used to let the application know that a key refresh is needed to avoid counter
   * rollover problems.
   *
   * @param info ICDClientInfo object representing the state associated with the node that sent the
   *     check-in message.
   * @return refreshed key
   */
  @Nullable
  byte[] onKeyRefreshNeeded(@Nonnull ICDClientInfo info);

  /**
   * Callback used to let the application know that the re-registration process is done.
   *
   * @param errorCode to check for success and failure
   */
  void onKeyRefreshDone(long errorCode);
}
