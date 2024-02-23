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

public class DeviceAttestation {
  /**
   * Extract skid from paa cert.
   *
   * @param paaCert The product attestation authority (PAA) cert
   * @return The subject key identifier (SKID)
   */
  public static native byte[] extractSkidFromPaaCert(byte[] paaCert);

  /**
   * Extract akid from pai cert.
   *
   * @param paiCert The product attestation intermediate (PAI) cert
   * @return The authority key identifier (AKID)
   */
  public static native byte[] extractAkidFromPaiCert(byte[] paiCert);

  /**
   * Validate device Attestation. If device attestation fails, {@link ChipDeviceControllerException}
   * exception occurs.
   *
   * @param vendorId Vendor ID from the node
   * @param productId Product ID from the node
   * @param paaCert The product attestation authority (PAA) cert
   * @param paiCert The product attestation intermediate (PAI) cert
   * @param dacCert The device attestation certificate (DAC) cert
   * @param attestationElements Attestation elements from the node
   */
  public static native void validateAttestationInfo(
      int vendorId,
      int productId,
      byte[] paaCert,
      byte[] paiCert,
      byte[] dacCert,
      byte[] attestationElements)
      throws ChipDeviceControllerException;
}
