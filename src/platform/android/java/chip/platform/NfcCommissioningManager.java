/*
 *   Copyright (c) 2025 Project CHIP Authors
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
package chip.platform;

public interface NfcCommissioningManager {

  /**
   * @brief Set the NfcCallback (for app). NB: This NfcCallback interface is currently NOT USED.
   * @return None
   */
  void setNfcCallback(NfcCallback nfcCallback);

  /**
   * @brief Get the NfcCallback. NB: This NfcCallback interface is currently NOT USED.
   * @return NfcCallback
   */
  NfcCallback getCallback();

  /**
   * @brief Set the AndroidChipPlatform.
   * @param platform contains the AndroidChipPlatform to use.
   * @return None
   */
  void setAndroidChipPlatform(AndroidChipPlatform platform);

  /**
   * @brief NFCCommissioningManager init function.
   * @return int (currently not used)
   */
  int init();

  /**
   * @brief NFCCommissioningManager shutdown function.
   * @return None
   */
  void shutdown();

  /**
   * @brief Function to send data to an NFC Tag. See NfcApplicationDelegate.
   * @param buf contains the byte array to send.
   * @return None
   */
  void sendToNfcTag(byte[] buf);
}
