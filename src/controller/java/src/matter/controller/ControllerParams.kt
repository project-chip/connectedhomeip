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
package matter.controller

/**
 * Parameters representing initialization arguments for [MatterController].
 *
 * @param operationalKeyConfig An optional signing configuration for operational control of devices.
 * @param udpListenPort A port for UDP communications, or [UDP_PORT_AUTO] to select automatically.
 * @param vendorId The identifier for the vendor using this controller.
 * @param countryCode The Regulatory Location country code.
 */
class ControllerParams
@JvmOverloads
constructor(
  val operationalKeyConfig: OperationalKeyConfig? = null,
  val udpListenPort: Int = UDP_PORT_AUTO,
  val vendorId: Int = VENDOR_ID_TEST,
  val countryCode: String? = null,
  val enableServerInteractions: Boolean = false,
) {
  companion object {
    /** Matter assigned vendor ID for Google. */
    const val VENDOR_ID_TEST = 0xFFF1
    /** Indicates that the UDP listen port should be chosen automatically. */
    const val UDP_PORT_AUTO = 0
  }
}
