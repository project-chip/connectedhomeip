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

import chip.devicecontroller.KeypairDelegate

/**
 * Represents a configuration to use with CASE session establishment.
 *
 * @property keypairDelegate a delegate for signing operations
 * @property trustedRootCertificate the trusted root X.509 certificate in DER-encoded form
 * @property intermediateCertificate the optional intermediate X.509 certificate in DER-encoded form
 * @property operationalCertificate the node operational X.509 certificate in DER-encoded form
 * @property ipk the Identity Protection Key
 * @property fabricId the fabric ID to which these operational credentials are associated
 * @property nodeId the Admin Node ID to which these operational credentials are associated
 */
class OperationalKeyConfig(
  val keypairDelegate: KeypairDelegate,
  val trustedRootCertificate: ByteArray,
  val intermediateCertificate: ByteArray?,
  val operationalCertificate: ByteArray,
  val ipk: ByteArray,
  val fabricId: Long,
  val nodeId: Long,
)
