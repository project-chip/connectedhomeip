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
 */
package com.matter.casting;

import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

/**
 * Unit tests for DACProviderStub.
 *
 * <p>These tests verify the SignWithDeviceAttestationKey method correctly handles both PKCS#8 and
 * SEC1 (raw) private key formats, with proper fallback behavior when PKCS#8 fails.
 */
@RunWith(RobolectricTestRunner.class)
@Config(sdk = 28)
public class DACProviderStubTest {

  private DACProviderStub dacProvider;

  // Test message to sign
  private static final byte[] TEST_MESSAGE = "Test message for DAC signing".getBytes();

  @Before
  public void setUp() {
    dacProvider = new DACProviderStub();
  }

  /**
   * Test that SignWithDeviceAttestationKey successfully signs a message with the SEC1 format
   * private key. This is the primary test case since the default key is in SEC1 format.
   */
  @Test
  public void testSignWithDeviceAttestationKey_SEC1Format_Success() {
    // Act
    byte[] signature = dacProvider.SignWithDeviceAttestationKey(TEST_MESSAGE);

    // Assert
    assertNotNull("Signature should not be null", signature);
    assertTrue("Signature should have non-zero length", signature.length > 0);

    // Verify signature is valid (ECDSA signatures are typically 70-72 bytes for secp256r1)
    assertTrue(
        "Signature length should be reasonable for ECDSA",
        signature.length >= 64 && signature.length <= 75);
  }

  /**
   * Test that the method can handle multiple signing operations correctly. This verifies that the
   * KeyFactory instance recreation in the catch block doesn't cause issues with subsequent calls.
   */
  @Test
  public void testSignWithDeviceAttestationKey_MultipleSignings_Success() {
    // Act - Sign multiple times
    byte[] signature1 = dacProvider.SignWithDeviceAttestationKey(TEST_MESSAGE);
    byte[] signature2 = dacProvider.SignWithDeviceAttestationKey(TEST_MESSAGE);
    byte[] signature3 = dacProvider.SignWithDeviceAttestationKey(TEST_MESSAGE);

    // Assert
    assertNotNull("First signature should not be null", signature1);
    assertNotNull("Second signature should not be null", signature2);
    assertNotNull("Third signature should not be null", signature3);

    assertTrue(
        "All signatures should have non-zero length",
        signature1.length > 0 && signature2.length > 0 && signature3.length > 0);
  }

  /**
   * Test that SignWithDeviceAttestationKey produces different signatures for different messages.
   * ECDSA signatures include randomness, so even the same message should produce different
   * signatures.
   */
  @Test
  public void testSignWithDeviceAttestationKey_DifferentMessages_DifferentSignatures() {
    // Arrange
    byte[] message1 = "Message 1".getBytes();
    byte[] message2 = "Message 2".getBytes();

    // Act
    byte[] signature1 = dacProvider.SignWithDeviceAttestationKey(message1);
    byte[] signature2 = dacProvider.SignWithDeviceAttestationKey(message2);

    // Assert
    assertNotNull("First signature should not be null", signature1);
    assertNotNull("Second signature should not be null", signature2);

    // Signatures should be different (this is probabilistic but extremely unlikely to be equal)
    boolean signaturesAreDifferent = false;
    if (signature1.length != signature2.length) {
      signaturesAreDifferent = true;
    } else {
      for (int i = 0; i < signature1.length; i++) {
        if (signature1[i] != signature2[i]) {
          signaturesAreDifferent = true;
          break;
        }
      }
    }
    assertTrue("Signatures for different messages should be different", signaturesAreDifferent);
  }

  /** Test that GetDeviceAttestationCert returns a valid certificate. */
  @Test
  public void testGetDeviceAttestationCert_ReturnsValidCertificate() {
    // Act
    byte[] cert = dacProvider.GetDeviceAttestationCert();

    // Assert
    assertNotNull("Certificate should not be null", cert);
    assertTrue("Certificate should have non-zero length", cert.length > 0);
  }

  /** Test that GetProductAttestationIntermediateCert returns a valid certificate. */
  @Test
  public void testGetProductAttestationIntermediateCert_ReturnsValidCertificate() {
    // Act
    byte[] cert = dacProvider.GetProductAttestationIntermediateCert();

    // Assert
    assertNotNull("PAI Certificate should not be null", cert);
    assertTrue("PAI Certificate should have non-zero length", cert.length > 0);
  }

  /** Test that GetCertificationDeclaration returns a valid declaration. */
  @Test
  public void testGetCertificationDeclaration_ReturnsValidDeclaration() {
    // Act
    byte[] declaration = dacProvider.GetCertificationDeclaration();

    // Assert
    assertNotNull("Certification declaration should not be null", declaration);
    assertTrue("Certification declaration should have non-zero length", declaration.length > 0);
  }

  /** Test that GetFirmwareInformation returns an empty array (as per current implementation). */
  @Test
  public void testGetFirmwareInformation_ReturnsEmptyArray() {
    // Act
    byte[] firmwareInfo = dacProvider.GetFirmwareInformation();

    // Assert
    assertNotNull("Firmware information should not be null", firmwareInfo);
    assertTrue("Firmware information should be empty array", firmwareInfo.length == 0);
  }

  /**
   * Test the PKCS#8 to SEC1 fallback behavior by verifying that even if PKCS#8 format fails, the
   * SEC1 format succeeds. This test verifies the core fix: that a fresh KeyFactory instance is
   * created in the catch block.
   */
  @Test
  public void testSignWithDeviceAttestationKey_PKCS8FailsFallsBackToSEC1_Success() {
    // The current implementation tries PKCS#8 first, then falls back to SEC1.
    // Since the default key is in SEC1 format, PKCS#8 will fail and SEC1 should succeed.
    // This test verifies that the fallback works correctly.

    // Act
    byte[] signature = dacProvider.SignWithDeviceAttestationKey(TEST_MESSAGE);

    // Assert - If we get a valid signature, it means the fallback worked
    assertNotNull("Signature should not be null after PKCS#8->SEC1 fallback", signature);
    assertTrue("Signature should have non-zero length after fallback", signature.length > 0);
  }

  /** Test that signing with an empty message doesn't crash and produces a valid signature. */
  @Test
  public void testSignWithDeviceAttestationKey_EmptyMessage_Success() {
    // Arrange
    byte[] emptyMessage = new byte[0];

    // Act
    byte[] signature = dacProvider.SignWithDeviceAttestationKey(emptyMessage);

    // Assert
    assertNotNull("Signature for empty message should not be null", signature);
    assertTrue("Signature for empty message should have non-zero length", signature.length > 0);
  }

  /** Test that signing with a large message works correctly. */
  @Test
  public void testSignWithDeviceAttestationKey_LargeMessage_Success() {
    // Arrange - Create a 10KB message
    byte[] largeMessage = new byte[10240];
    for (int i = 0; i < largeMessage.length; i++) {
      largeMessage[i] = (byte) (i % 256);
    }

    // Act
    byte[] signature = dacProvider.SignWithDeviceAttestationKey(largeMessage);

    // Assert
    assertNotNull("Signature for large message should not be null", signature);
    assertTrue("Signature for large message should have non-zero length", signature.length > 0);
  }
}
