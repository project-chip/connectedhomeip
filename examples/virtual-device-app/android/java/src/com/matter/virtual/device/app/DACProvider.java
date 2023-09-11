/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package com.matter.virtual.device.app;

public interface DACProvider {
  byte[] GetCertificationDeclaration();

  byte[] GetFirmwareInformation();

  byte[] GetDeviceAttestationCert();

  byte[] GetProductAttestationIntermediateCert();

  byte[] GetDeviceAttestationCertPrivateKey();

  byte[] GetDeviceAttestationCertPublicKeyKey();
}
