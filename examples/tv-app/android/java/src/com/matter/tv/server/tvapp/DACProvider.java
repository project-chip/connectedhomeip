/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
