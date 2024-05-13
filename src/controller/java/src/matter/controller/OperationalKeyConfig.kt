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
 * CertificateData Configuration data for X.509 certificates.
 *
 * @property trustedRootCertificate The trusted root X.509 certificate in DER-encoded form.
 * @property intermediateCertificate The optional intermediate X.509 certificate in DER-encoded
 *   form.
 * @property operationalCertificate The node operational X.509 certificate in DER-encoded form.
 */
class CertificateData(
  val trustedRootCertificate: ByteArray,
  val intermediateCertificate: ByteArray?,
  val operationalCertificate: ByteArray
)

/**
 * Configuration for use with CASE (Chip Authentication Session Establishment) session
 * establishment.
 *
 * @property keypairDelegate A delegate for signing operations.
 * @property certificateData Configuration data for X.509 certificates.
 * @property ipk The Identity Protection Key.
 * @property fabricId The fabric ID to which these operational credentials are associated.
 * @property nodeId The Admin Node ID to which these operational credentials are associated.
 */
class OperationalKeyConfig(
  val keypairDelegate: KeypairDelegate,
  val certificateData: CertificateData,
  val ipk: ByteArray,
  val fabricId: Long,
  val nodeId: Long
)
