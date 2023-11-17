/*
 *   Copyright (c) 2021 Project CHIP Authors
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
package com.matter.tv.server.tvapp;

public interface DACProvider {
  byte[] GetCertificationDeclaration();

  byte[] GetFirmwareInformation();

  byte[] GetDeviceAttestationCert();

  byte[] GetProductAttestationIntermediateCert();

  /**
   * Sign a mesage with the device attestation key.
   *
   * <p>The signature should be a SHA256withECDSA Signature that's returned in the ECDSA X9.62 Asn1
   * format. This is the default behavior when using java.security.Signature with an EC P-256 curve.
   *
   * @param message The message to sign
   * @return The signature in ECDSA X9.62 Asn1 format.
   */
  byte[] SignWithDeviceAttestationKey(byte[] message);
}
