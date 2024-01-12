/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    Copyright (c) 2019-2023 Google LLC.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

package matter.onboardingpayload

import com.google.common.truth.Truth.assertThat
import org.junit.Assert.assertEquals
import org.junit.Test
import org.junit.runner.RunWith
import org.junit.runners.JUnit4

@RunWith(JUnit4::class)
class QRCodeTest {
  private fun getDefaultPayload(): OnboardingPayload {
    val payload = OnboardingPayload()

    payload.version = 0
    payload.vendorId = 12
    payload.productId = 1
    payload.setupPinCode = 2048
    payload.commissioningFlow = CommissioningFlow.STANDARD.value
    payload.discoveryCapabilities.add(DiscoveryCapability.SOFT_AP)
    payload.setLongDiscriminatorValue(128)

    return payload
  }

  private fun checkWriteRead(
    inPayload: OnboardingPayload,
    allowInvalidPayload: Boolean = false
  ): Boolean {
    var generator = QRCodeOnboardingPayloadGenerator(inPayload)
    generator.setAllowInvalidPayload(allowInvalidPayload)
    var result = generator.payloadBase38Representation()

    var outPayload = QRCodeOnboardingPayloadParser(result).populatePayload()

    return inPayload == outPayload
  }

  private fun compareBinary(payload: OnboardingPayload, expectedBinary: String): Boolean {
    var generator = QRCodeOnboardingPayloadGenerator(payload)
    var result = generator.payloadBase38Representation()
    val resultBinary = toBinaryRepresentation(result)

    return (expectedBinary == resultBinary)
  }

  private fun toBinaryRepresentation(base38Result: String): String {
    // Remove the kQRCodePrefix
    val resultWithoutPrefix = base38Result.removePrefix(kQRCodePrefix)

    // Decode the base38 encoded String
    val buffer = base38Decode(resultWithoutPrefix)

    // Convert it to binary
    val binaryResult = StringBuilder()
    for (i in buffer.size downTo 1) {
      binaryResult.append(buffer[i - 1].toString(2).padStart(8, '0'))
    }

    // Insert spaces after each block
    var pos = binaryResult.length

    pos -= kVersionFieldLengthInBits
    binaryResult.insert(pos, " ")

    pos -= kVendorIDFieldLengthInBits
    binaryResult.insert(pos, " ")

    pos -= kProductIDFieldLengthInBits
    binaryResult.insert(pos, " ")

    pos -= kCommissioningFlowFieldLengthInBits
    binaryResult.insert(pos, " ")

    pos -= kRendezvousInfoFieldLengthInBits
    binaryResult.insert(pos, " ")

    pos -= kPayloadDiscriminatorFieldLengthInBits
    binaryResult.insert(pos, " ")

    pos -= kSetupPINCodeFieldLengthInBits
    binaryResult.insert(pos, " ")

    pos -= kPaddingFieldLengthInBits
    binaryResult.insert(pos, " ")

    return binaryResult.toString()
  }

  /*
   * Test Rendezvous Flags
   */
  @Test
  fun testRendezvousFlags() {
    val inPayload = getDefaultPayload()

    // Not having a value in rendezvousInformation is not allowed for a QR code.
    inPayload.setRendezvousInformation(0L)
    assertEquals(true, checkWriteRead(inPayload))

    inPayload.setRendezvousInformation(1L shl 0)
    assertEquals(true, checkWriteRead(inPayload))

    inPayload.setRendezvousInformation(1L shl 1)
    assertEquals(true, checkWriteRead(inPayload))

    inPayload.setRendezvousInformation(1L shl 2)
    assertEquals(true, checkWriteRead(inPayload))

    inPayload.setRendezvousInformation((1L shl 0) or (1L shl 2))
    assertEquals(true, checkWriteRead(inPayload))

    inPayload.setRendezvousInformation((1L shl 1) or (1L shl 2))
    assertEquals(true, checkWriteRead(inPayload))

    inPayload.setRendezvousInformation((1L shl 0) or (1L shl 1) or (1L shl 2))
    assertEquals(true, checkWriteRead(inPayload))
  }

  /*
   * Test Commissioning Flow
   */
  @Test
  fun testCommissioningFlow() {
    val inPayload = getDefaultPayload()

    inPayload.commissioningFlow = CommissioningFlow.STANDARD.value
    assertEquals(true, checkWriteRead(inPayload))

    inPayload.commissioningFlow = CommissioningFlow.USER_ACTION_REQUIRED.value
    assertEquals(true, checkWriteRead(inPayload))

    inPayload.commissioningFlow = CommissioningFlow.CUSTOM.value
    assertEquals(true, checkWriteRead(inPayload))
  }

  /*
   * Test Maximum Values
   */
  @Test
  fun testMaximumValues() {
    val inPayload = getDefaultPayload()

    inPayload.version = (1 shl kVersionFieldLengthInBits) - 1
    inPayload.vendorId = 0xFFFF
    inPayload.productId = 0xFFFF
    inPayload.commissioningFlow = CommissioningFlow.CUSTOM.value

    inPayload.setRendezvousInformation((1L shl 0) and (1L shl 1) and (1L shl 2))
    inPayload.setLongDiscriminatorValue((1 shl kVersionFieldLengthInBits) - 1)
    inPayload.setupPinCode = (1L shl kVersionFieldLengthInBits) - 1

    assertEquals(true, checkWriteRead(inPayload, allowInvalidPayload = true))
  }

  /*
   * Test Payload Byte Array Representation
   */
  @Test
  fun testPayloadByteArrayRep() {
    val payload = getDefaultPayload()

    val expected =
      " 0000 000000000000000100000000000 000010000000 00000001 00 0000000000000001 0000000000001100 000"
    assertEquals(true, compareBinary(payload, expected))
  }

  /*
   * Test Bitset Length
   */
  @Test
  fun testBitsetLen() {
    assertEquals(true, kTotalPayloadDataSizeInBits % 8 == 0)
  }

  /*
   * Test Setup Payload Verify
   */
  @Test
  fun testSetupPayloadVerify() {
    var payload = getDefaultPayload()
    assertEquals(true, payload.isValidQRCodePayload())

    // test invalid commissioning flow
    payload = getDefaultPayload()
    payload.commissioningFlow = CommissioningFlow.CUSTOM.value
    assertEquals(true, payload.isValidQRCodePayload())

    // test invalid version
    payload = getDefaultPayload()
    payload.version = (1 shl kVersionFieldLengthInBits)
    assertEquals(false, payload.isValidQRCodePayload())

    // test invalid setup PIN
    payload = getDefaultPayload()
    payload.setupPinCode = (1L shl kSetupPINCodeFieldLengthInBits)
    assertEquals(false, payload.isValidQRCodePayload())
  }

  /*
   * Test Invalid QR Code Payload - Wrong Character Set
   */
  @Test
  fun testInvalidQRCodePayload_wrongCharacterSet() {
    var invalidString = kDefaultPayloadQRCode
    invalidString = invalidString.dropLast(1) + " " // space is not contained in the base38 alphabet

    try {
      QRCodeOnboardingPayloadParser(invalidString).populatePayload()
      assertThat(false)
    } catch (e: Exception) {
      println("Expected exception occurred: ${e.message}")
    }
  }

  /*
   * Test Invalid QR Code Payload - Wrong  Length
   */
  @Test
  fun testInvalidQRCodePayload_wrongLength() {
    var invalidString = kDefaultPayloadQRCode
    invalidString = invalidString.dropLast(1)

    try {
      QRCodeOnboardingPayloadParser(invalidString).populatePayload()
      assertThat(false)
    } catch (e: Exception) {
      println("Expected exception occurred: ${e.message}")
    }
  }

  /*
   * Test Payload Equality
   */
  @Test
  fun testPayloadEquality() {
    val payload = getDefaultPayload()
    val equalPayload = getDefaultPayload()
    assertEquals(true, payload == equalPayload)
  }

  /*
   * Test Payload Inequality
   */
  @Test
  fun testPayloadInEquality() {
    val payload = getDefaultPayload()
    val unequalPayload = getDefaultPayload()

    unequalPayload.setLongDiscriminatorValue(28)
    unequalPayload.setupPinCode = 121233

    assertEquals(false, payload == unequalPayload)
  }

  /*
   * Test QRCode to Payload Generation
   */
  @Test
  fun testQRCodeToPayloadGeneration() {
    val payload = getDefaultPayload()
    var generator = QRCodeOnboardingPayloadGenerator(payload)
    var base38Rep = generator.payloadBase38Representation()

    var resultingPayload = QRCodeOnboardingPayloadParser(base38Rep).populatePayload()

    assertEquals(true, resultingPayload.isValidQRCodePayload())
    assertEquals(true, payload == resultingPayload)
  }

  /*
   * Test Extract Payload
   */
  @Test
  fun testExtractPayload() {
    assertEquals("ABC", QRCodeOnboardingPayloadParser.extractPayload("MT:ABC"))
    assertEquals("", QRCodeOnboardingPayloadParser.extractPayload("MT:"))
    assertEquals("", QRCodeOnboardingPayloadParser.extractPayload("H:"))
    assertEquals("", QRCodeOnboardingPayloadParser.extractPayload("ASMT:"))
    assertEquals("ABC", QRCodeOnboardingPayloadParser.extractPayload("Z%MT:ABC%"))
    assertEquals("ABC", QRCodeOnboardingPayloadParser.extractPayload("%Z%MT:ABC%"))
    assertEquals("ABC", QRCodeOnboardingPayloadParser.extractPayload("%Z%MT:ABC%DDD"))
    assertEquals("ABC", QRCodeOnboardingPayloadParser.extractPayload("MT:ABC%DDD"))
    assertEquals("ABC", QRCodeOnboardingPayloadParser.extractPayload("MT:ABC%"))
    assertEquals("", QRCodeOnboardingPayloadParser.extractPayload("ABC"))
    assertEquals("ABC", QRCodeOnboardingPayloadParser.extractPayload("Z%MT:ABC"))
    assertEquals("ABC", QRCodeOnboardingPayloadParser.extractPayload("%Z%MT:ABC"))
    assertEquals("", QRCodeOnboardingPayloadParser.extractPayload("%MT:"))
    assertEquals("", QRCodeOnboardingPayloadParser.extractPayload("%MT:%"))
    assertEquals("", QRCodeOnboardingPayloadParser.extractPayload("A%"))
    assertEquals("", QRCodeOnboardingPayloadParser.extractPayload("MT:%"))
    assertEquals("ABC", QRCodeOnboardingPayloadParser.extractPayload("%MT:ABC"))
  }

  /*
   * Test Parse QrCode to Expected Payload
   */
  @Test
  fun testParseQrCodeToExpectedPayload() {
    // Payload: MT:W0GU2OTB00KA0648G00
    // Vendor Id: 9050 (0x235A)
    // Product Id: 20043 (0x4E4B)
    // Setup Pin Code: 20202021
    // Setup Discriminator: 3840 (0xF00)

    val parser = OnboardingPayloadParser()
    assertThat(parser.parseQrCode("MT:W0GU2OTB00KA0648G00"))
      .isEqualTo(
        OnboardingPayload(
          discriminator = 0xF00,
          setupPinCode = 20202021,
          version = 0,
          vendorId = 0x235A,
          productId = 0x4E4B,
          commissioningFlow = CommissioningFlow.STANDARD.value,
          discoveryCapabilities = mutableSetOf(DiscoveryCapability.BLE),
        )
      )
  }

  /*
   * Test Generate QrCode from Expected Value
   */
  @Test
  fun testGenerateQrCodeFromExpectedValue() {
    // Payload: MT:W0GU2OTB00KA0648G00
    // Vendor Id: 9050 (0x235A)
    // Product Id: 20043 (0x4E4B)
    // Setup Pin Code: 20202021
    // Setup Discriminator: 3840 (0xF00)

    val parser = OnboardingPayloadParser()
    assertThat(
        parser.getQrCodeFromPayload(
          OnboardingPayload(
            discriminator = 0xF00,
            setupPinCode = 20202021,
            version = 0,
            vendorId = 0x235A,
            productId = 0x4E4B,
            commissioningFlow = CommissioningFlow.STANDARD.value,
            discoveryCapabilities = mutableSetOf(DiscoveryCapability.BLE),
          )
        )
      )
      .isEqualTo("MT:W0GU2OTB00KA0648G00")
  }

  /*
   * Test QRCode with optional data
   *
   * matches iOS test
   * https://github.com/project-chip/connectedhomeip/blob/927962863180270091c1694d4b1ce2e9ea16b8b5/src/darwin/Framework/CHIPTests/MTRSetupPayloadParserTests.m#L155
   */
  @Test
  fun testQRCodeWithOptionalData() {
    val payload =
      OnboardingPayload(
        discriminator = 128,
        setupPinCode = 2048,
        version = 0,
        vendorId = 12,
        productId = 1,
        commissioningFlow = CommissioningFlow.STANDARD.value,
        discoveryCapabilities = mutableSetOf(DiscoveryCapability.SOFT_AP),
      )
    val parsedQrCode =
      OnboardingPayloadParser()
        .parseQrCode("MT:M5L90MP500K64J0A33P0SET70" + ".QT52B.E23-WZE0WISA0DK5N1K8SQ1RYCU1O0")
    assertThat(parsedQrCode).isEqualTo(payload)

    var optionalQRCodeInfo = OptionalQRCodeInfoExtension()
    // Test 1st optional field
    optionalQRCodeInfo.tag = 0
    optionalQRCodeInfo.type = OptionalQRCodeInfoType.TYPE_STRING
    optionalQRCodeInfo.data = "123456789"

    assertThat(parsedQrCode.getAllOptionalExtensionData()[0]).isEqualTo(optionalQRCodeInfo)
    // verify we can grab just the serial number as well
    assertThat(parsedQrCode.getSerialNumber()).isEqualTo("123456789")

    // Test 2nd optional field
    optionalQRCodeInfo = OptionalQRCodeInfoExtension()
    optionalQRCodeInfo.tag = 130
    optionalQRCodeInfo.type = OptionalQRCodeInfoType.TYPE_STRING
    optionalQRCodeInfo.data = "myData"

    assertThat(parsedQrCode.getAllOptionalVendorData()[0]).isEqualTo(optionalQRCodeInfo)

    // Test 3rd optional field
    optionalQRCodeInfo = OptionalQRCodeInfoExtension()
    optionalQRCodeInfo.tag = 131
    optionalQRCodeInfo.type = OptionalQRCodeInfoType.TYPE_INT32
    optionalQRCodeInfo.int32 = 12

    assertThat(parsedQrCode.getAllOptionalVendorData()[1]).isEqualTo(optionalQRCodeInfo)
  }

  companion object {
    const val kDefaultPayloadQRCode: String = "MT:M5L90MP500K64J00000"
  }
}
