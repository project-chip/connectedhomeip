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
import java.util.concurrent.atomic.AtomicInteger
import kotlin.math.ceil
import kotlin.math.log10
import kotlin.math.pow
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test
import org.junit.runner.RunWith
import org.junit.runners.JUnit4

@RunWith(JUnit4::class)
class ManualCodeTest {
  private fun checkGenerator(
    payload: OnboardingPayload,
    expectedResult: String,
    skipPayloadValidation: Boolean = false
  ): Boolean {
    val generator = ManualOnboardingPayloadGenerator(payload)
    generator.setSkipPayloadValidation(skipPayloadValidation)
    val result = generator.payloadDecimalStringRepresentation()
    var expectedResultWithCheckChar = expectedResult

    if (expectedResult.isNotEmpty()) {
      val expectedCheckChar = Verhoeff10.computeCheckChar(expectedResult)
      expectedResultWithCheckChar += expectedCheckChar
    }

    val same = result == expectedResultWithCheckChar
    if (!same) {
      println("Actual result: $result")
      println("Expected result: $expectedResultWithCheckChar")
    }

    return same
  }

  private fun getDefaultPayload(): OnboardingPayload {
    val payload = OnboardingPayload()
    payload.setupPinCode = 12345679
    payload.discriminator = 2560
    return payload
  }

  private fun assertPayloadValues(
    payload: OnboardingPayload,
    pinCode: Long,
    discriminator: Int,
    vendorId: Int,
    productId: Int
  ) {
    assertEquals(payload.setupPinCode, pinCode)
    assertEquals(payload.discriminator, discriminator)
    assertEquals(payload.vendorId, vendorId)
    assertEquals(payload.productId, productId)
  }

  private fun assertEmptyPayloadWithError(payload: OnboardingPayload) {
    assertEquals(payload.setupPinCode, 0)
    assertEquals(payload.discriminator, 0)
    assertEquals(payload.vendorId, 0)
    assertEquals(payload.productId, 0)
  }

  private fun computeCheckChar(str: String): Char {
    // Strip out dashes, if any, from the string before computing the checksum.
    val newStr = str.replace("-", "")
    return Verhoeff10.computeCheckChar(newStr)
  }

  /*
   * Generate Decimal Representation from Partial Payload
   */
  @Test
  fun testDecimalRepresentation_partialPayload() {
    val payload = getDefaultPayload()
    val expectedResult = "2412950753"
    val result = checkGenerator(payload, expectedResult)
    assertEquals(true, result)
  }

  /*
   * Generate Decimal Representation from Partial Payload (Custom Flow)
   */
  @Test
  fun testDecimalRepresentation_partialPayload_requiresCustomFlow() {
    val payload = getDefaultPayload()
    payload.commissioningFlow = CommissioningFlow.CUSTOM.value
    val expectedResult = "64129507530000000000"
    val result = checkGenerator(payload, expectedResult)
    assertEquals(true, result)
  }

  /*
   * Generate Decimal Representation from Full Payload with Zeros
   */
  @Test
  fun testDecimalRepresentation_fullPayloadWithZeros() {
    val payload = getDefaultPayload()
    payload.commissioningFlow = CommissioningFlow.CUSTOM.value
    payload.vendorId = 1
    payload.productId = 1

    val expectedResult = "64129507530000100001"
    val result = checkGenerator(payload, expectedResult)
    assertEquals(true, result)
  }

  /*
   * Decimal Representation from Full Payload without Zeros
   */
  @Test
  fun testDecimalRepresentation_fullPayloadWithoutZeros_doesNotRequireCustomFlow() {
    val payload = getDefaultPayload()
    payload.vendorId = 45367
    payload.productId = 14526

    val expectedResult = "2412950753"
    val result = checkGenerator(payload, expectedResult)
    assertEquals(true, result)
  }

  /*
   * Decimal Representation from Full Payload without Zeros (Custom Flow)
   */
  @Test
  fun testDecimalRepresentation_fullPayloadWithoutZeros() {
    val payload = getDefaultPayload()
    payload.commissioningFlow = CommissioningFlow.CUSTOM.value
    payload.vendorId = 45367
    payload.productId = 14526

    val expectedResult = "64129507534536714526"
    val result = checkGenerator(payload, expectedResult)
    assertEquals(true, result)
  }

  /*
   * Test 12 bit discriminator for manual setup code
   */
  @Test
  fun testGenerateAndParser_manualSetupCodeWithLongDiscriminator() {
    val payload = getDefaultPayload()
    payload.setLongDiscriminatorValue(0xa1f)

    // Test short 11 digit code
    var generator = ManualOnboardingPayloadGenerator(payload)
    var result = generator.payloadDecimalStringRepresentation()
    var outPayload = ManualOnboardingPayloadParser(result).populatePayload()
    assertPayloadValues(
      outPayload,
      payload.setupPinCode,
      discriminator = 0xa,
      payload.vendorId,
      payload.productId
    )

    payload.vendorId = 1
    payload.productId = 1
    payload.commissioningFlow = CommissioningFlow.CUSTOM.value
    payload.setLongDiscriminatorValue(0xb1f)

    // Test long 21 digit code
    generator = ManualOnboardingPayloadGenerator(payload)
    result = generator.payloadDecimalStringRepresentation()
    outPayload = ManualOnboardingPayloadParser(result).populatePayload()
    assertPayloadValues(
      outPayload,
      payload.setupPinCode,
      discriminator = 0xb,
      payload.vendorId,
      payload.productId
    )
  }

  /*
   * Test Decimal Representation - All Ones
   */
  @Test
  fun testDecimalRepresentation_allOnes() {
    val payload = getDefaultPayload()
    payload.setupPinCode = 0x7FFFFFF
    payload.setLongDiscriminatorValue(0xFFF)
    payload.commissioningFlow = CommissioningFlow.CUSTOM.value
    payload.vendorId = 65535
    payload.productId = 65535

    val expectedResult = "76553581916553565535"
    val result = checkGenerator(payload, expectedResult, true)
    assertEquals(true, result)
  }

  /*
   * Parse from Partial Payload
   */
  @Test
  fun testPayloadParser_partialPayload() {
    var decimalString = "2361087535"

    decimalString += Verhoeff10.computeCheckChar(decimalString)
    assertEquals(11, decimalString.length)
    var payload = ManualOnboardingPayloadParser(decimalString).populatePayload()
    assertPayloadValues(
      payload,
      pinCode = 123456780,
      discriminator = 0xa,
      vendorId = 0,
      productId = 0
    )

    // The same thing, but with dashes separating digit groups.
    decimalString = "236-108753-5"
    decimalString += computeCheckChar(decimalString)
    assertEquals(13, decimalString.length)
    payload = ManualOnboardingPayloadParser(decimalString).populatePayload()
    assertPayloadValues(
      payload,
      pinCode = 123456780,
      discriminator = 0xa,
      vendorId = 0,
      productId = 0
    )

    decimalString = "0000010000"
    decimalString += Verhoeff10.computeCheckChar(decimalString)
    assertEquals(11, decimalString.length)
    payload = ManualOnboardingPayloadParser(decimalString).populatePayload()
    assertPayloadValues(payload, pinCode = 1, discriminator = 0, vendorId = 0, productId = 0)

    decimalString = "63610875350000000000"
    decimalString += Verhoeff10.computeCheckChar(decimalString)
    assertEquals(21, decimalString.length)
    payload = ManualOnboardingPayloadParser(decimalString).populatePayload()
    assertPayloadValues(
      payload,
      pinCode = 123456780,
      discriminator = 0xa,
      vendorId = 0,
      productId = 0
    )

    // no discriminator (= 0)
    decimalString = "0033407535"
    decimalString += Verhoeff10.computeCheckChar(decimalString)
    assertEquals(11, decimalString.length)
    payload = ManualOnboardingPayloadParser(decimalString).populatePayload()
    assertPayloadValues(
      payload,
      pinCode = 123456780,
      discriminator = 0,
      vendorId = 0,
      productId = 0
    )

    // no vid (= 0)
    decimalString = "63610875350000014526"
    decimalString += Verhoeff10.computeCheckChar(decimalString)
    assertEquals(21, decimalString.length)
    payload = ManualOnboardingPayloadParser(decimalString).populatePayload()
    assertPayloadValues(
      payload,
      pinCode = 123456780,
      discriminator = 0xa,
      vendorId = 0,
      productId = 14526
    )

    // no pid (= 0)
    decimalString = "63610875354536700000"
    decimalString += Verhoeff10.computeCheckChar(decimalString)
    assertEquals(21, decimalString.length)
    payload = ManualOnboardingPayloadParser(decimalString).populatePayload()
    assertPayloadValues(
      payload,
      pinCode = 123456780,
      discriminator = 0xa,
      vendorId = 45367,
      productId = 0
    )
  }

  /*
   * Parse from Full Payload
   */
  @Test
  fun testPayloadParser_fullPayload() {
    var decimalString = "63610875354536714526"

    decimalString += Verhoeff10.computeCheckChar(decimalString)
    var payload = ManualOnboardingPayloadParser(decimalString).populatePayload()
    assertPayloadValues(
      payload,
      pinCode = 123456780,
      discriminator = 0xa,
      vendorId = 45367,
      productId = 14526
    )

    // The same thing, but with dashes separating digit groups.
    decimalString = "6361-0875-3545-3671-4526"
    decimalString += computeCheckChar(decimalString)
    payload = ManualOnboardingPayloadParser(decimalString).populatePayload()
    assertPayloadValues(
      payload,
      pinCode = 123456780,
      discriminator = 0xa,
      vendorId = 45367,
      productId = 14526
    )

    decimalString = "52927623630456200032"
    decimalString += Verhoeff10.computeCheckChar(decimalString)
    payload = ManualOnboardingPayloadParser(decimalString).populatePayload()
    assertPayloadValues(
      payload,
      pinCode = 38728284,
      discriminator = 0x5,
      vendorId = 4562,
      productId = 32
    )

    decimalString = "40000100000000100001"
    decimalString += Verhoeff10.computeCheckChar(decimalString)
    payload = ManualOnboardingPayloadParser(decimalString).populatePayload()
    assertPayloadValues(payload, pinCode = 1, discriminator = 0, vendorId = 1, productId = 1)
  }

  /*
   * Test Invalid Entry To QR Code Parser
   */
  @Test
  fun testPayloadParser_invalidEntry() {
    var payload = OnboardingPayload()

    // Empty input
    var decimalString = ""
    decimalString += Verhoeff10.computeCheckChar(decimalString)
    try {
      payload = ManualOnboardingPayloadParser(decimalString).populatePayload()
    } catch (e: Exception) {
      println("Expected exception occurred: ${e.message}")
    }
    assertEmptyPayloadWithError(payload)

    // Invalid character
    decimalString = "24184.2196"
    try {
      decimalString += Verhoeff10.computeCheckChar(decimalString)
      payload = ManualOnboardingPayloadParser(decimalString).populatePayload()
    } catch (e: Exception) {
      println("Expected exception occurred: ${e.message}")
    }
    assertEmptyPayloadWithError(payload)

    // too short
    decimalString = "2456"
    try {
      decimalString += Verhoeff10.computeCheckChar(decimalString)
      payload = ManualOnboardingPayloadParser(decimalString).populatePayload()
    } catch (e: Exception) {
      println("Expected exception occurred: ${e.message}")
    }
    assertEmptyPayloadWithError(payload)

    // too long for long code
    decimalString = "123456789123456785671"
    try {
      decimalString += Verhoeff10.computeCheckChar(decimalString)
      payload = ManualOnboardingPayloadParser(decimalString).populatePayload()
    } catch (e: Exception) {
      println("Expected exception occurred: ${e.message}")
    }
    assertEmptyPayloadWithError(payload)

    // too long for short code
    decimalString = "12749875380"
    try {
      decimalString += Verhoeff10.computeCheckChar(decimalString)
      payload = ManualOnboardingPayloadParser(decimalString).populatePayload()
    } catch (e: Exception) {
      println("Expected exception occurred: ${e.message}")
    }
    assertEmptyPayloadWithError(payload)

    // bit to indicate short code but long code length
    decimalString = "23456789123456785610"
    try {
      decimalString += Verhoeff10.computeCheckChar(decimalString)
      payload = ManualOnboardingPayloadParser(decimalString).populatePayload()
    } catch (e: Exception) {
      println("Expected exception occurred: ${e.message}")
    }
    assertEmptyPayloadWithError(payload)

    // no pin code (= 0)
    decimalString = "2327680000"
    try {
      decimalString += Verhoeff10.computeCheckChar(decimalString)
      payload = ManualOnboardingPayloadParser(decimalString).populatePayload()
    } catch (e: Exception) {
      println("Expected exception occurred: ${e.message}")
    }
    assertEmptyPayloadWithError(payload)

    // wrong check digit
    decimalString = "02684354589"
    try {
      payload = ManualOnboardingPayloadParser(decimalString).populatePayload()
    } catch (e: Exception) {
      println("Expected exception occurred: ${e.message}")
    }
    assertEmptyPayloadWithError(payload)
  }

  /*
   * Test Short Read Write
   */
  @Test
  fun testShortCodeReadWrite() {
    val inPayload = getDefaultPayload()
    var generator = ManualOnboardingPayloadGenerator(inPayload)
    var result = generator.payloadDecimalStringRepresentation()
    val outPayload = ManualOnboardingPayloadParser(result).populatePayload()

    // Override the discriminator in the input payload with the short version,
    // since that's what we will produce.
    inPayload.setShortDiscriminatorValue(inPayload.getShortDiscriminatorValue())
    assertEquals(inPayload, outPayload)
  }

  /*
   * Test Long Read Write
   */
  @Test
  fun testLongCodeReadWrite() {
    val inPayload = getDefaultPayload()
    inPayload.commissioningFlow = CommissioningFlow.CUSTOM.value
    inPayload.vendorId = 1
    inPayload.productId = 1

    var generator = ManualOnboardingPayloadGenerator(inPayload)
    var result = generator.payloadDecimalStringRepresentation()
    val outPayload = ManualOnboardingPayloadParser(result).populatePayload()

    // Override the discriminator in the input payload with the short version,
    // since that's what we will produce.
    inPayload.setShortDiscriminatorValue(inPayload.getShortDiscriminatorValue())
    assertEquals(inPayload, outPayload)
  }

  /*
   * Check Decimal String Validity
   */
  @Test
  fun testCheckDecimalStringValidity() {
    var outReprensation: String
    var checkDigit: Char
    var decimalString: String
    var representationWithoutCheckDigit: String = ""

    try {
      ManualOnboardingPayloadParser.checkDecimalStringValidity(representationWithoutCheckDigit)
      assertTrue(false)
    } catch (e: Exception) {
      println("Expected exception occurred: ${e.message}")
    }

    representationWithoutCheckDigit = "1"
    try {
      ManualOnboardingPayloadParser.checkDecimalStringValidity(representationWithoutCheckDigit)
      assertTrue(false)
    } catch (e: Exception) {
      println("Expected exception occurred: ${e.message}")
    }

    representationWithoutCheckDigit = "10109"
    checkDigit = Verhoeff10.computeCheckChar(representationWithoutCheckDigit)
    decimalString = representationWithoutCheckDigit + checkDigit

    outReprensation = ManualOnboardingPayloadParser.checkDecimalStringValidity(decimalString)
    assertThat(outReprensation).isEqualTo(representationWithoutCheckDigit)

    representationWithoutCheckDigit = "0000"
    checkDigit = Verhoeff10.computeCheckChar(representationWithoutCheckDigit)
    decimalString = representationWithoutCheckDigit + checkDigit
    outReprensation = ManualOnboardingPayloadParser.checkDecimalStringValidity(decimalString)
    assertThat(outReprensation).isEqualTo(representationWithoutCheckDigit)
  }

  /*
   * Check QR Code Length Validity
   */
  @Test
  fun testCheckCodeLengthValidity() {
    ManualOnboardingPayloadParser.checkCodeLengthValidity("01234567890123456789", true)
    ManualOnboardingPayloadParser.checkCodeLengthValidity("0123456789", false)

    try {
      ManualOnboardingPayloadParser.checkCodeLengthValidity("01234567891", false)
      assertTrue(false)
    } catch (e: Exception) {
      println("Expected exception occurred: ${e.message}")
    }

    try {
      ManualOnboardingPayloadParser.checkCodeLengthValidity("012345678", false)
      assertTrue(false)
    } catch (e: Exception) {
      println("Expected exception occurred: ${e.message}")
    }

    try {
      ManualOnboardingPayloadParser.checkCodeLengthValidity("012345678901234567891", true)
      assertTrue(false)
    } catch (e: Exception) {
      println("Expected exception occurred: ${e.message}")
    }

    try {
      ManualOnboardingPayloadParser.checkCodeLengthValidity("0123456789012345678", true)
      assertTrue(false)
    } catch (e: Exception) {
      println("Expected exception occurred: ${e.message}")
    }
  }

  /*
   * Test Decimal String to Number
   */
  @Test
  fun testDecimalStringToNumber() {
    var number = ManualOnboardingPayloadParser.toNumber("12345")
    assertEquals(12345u, number)

    number = ManualOnboardingPayloadParser.toNumber("01234567890")
    assertEquals(1234567890u, number)

    number = ManualOnboardingPayloadParser.toNumber("00000001")
    assertEquals(1u, number)

    number = ManualOnboardingPayloadParser.toNumber("0")
    assertEquals(0u, number)

    try {
      ManualOnboardingPayloadParser.toNumber("012345.123456789")
      assertTrue(false)
    } catch (e: Exception) {
      println("Expected exception occurred: ${e.message}")
    }

    try {
      ManualOnboardingPayloadParser.toNumber("/")
      assertTrue(false)
    } catch (e: Exception) {
      println("Expected exception occurred: ${e.message}")
    }
  }

  /*
   * Test Short Code Character Lengths
   */
  @Test
  fun testShortCodeCharLengths() {
    val numBits = 1 + kSetupPINCodeFieldLengthInBits + kManualSetupDiscriminatorFieldLengthInBits
    val manualSetupShortCodeCharLength = ceil(log10(2.0.pow(numBits.toDouble()))).toInt()
    assertEquals(manualSetupShortCodeCharLength, kManualSetupShortCodeCharLength)

    val manualSetupVendorIdCharLength =
      ceil(log10(2.0.pow(kVendorIDFieldLengthInBits.toDouble()))).toInt()
    assertEquals(manualSetupVendorIdCharLength, kManualSetupVendorIdCharLength)

    val manualSetupProductIdCharLength =
      ceil(log10(2.0.pow(kProductIDFieldLengthInBits.toDouble()))).toInt()
    assertEquals(manualSetupProductIdCharLength, kManualSetupProductIdCharLength)

    val manualSetupLongCodeCharLength =
      kManualSetupShortCodeCharLength +
        kManualSetupVendorIdCharLength +
        kManualSetupProductIdCharLength
    assertEquals(manualSetupLongCodeCharLength, kManualSetupLongCodeCharLength)
  }

  /*
   * Test Read Characters from Decimal String
   */
  @Test
  fun testReadCharsFromDecimalString() {
    val index = AtomicInteger(3)
    var number = ManualOnboardingPayloadParser.readDigitsFromDecimalString("12345", index, 2)
    assertEquals(45u, number)

    index.set(2)
    number = ManualOnboardingPayloadParser.readDigitsFromDecimalString("6256276377282", index, 7)
    assertEquals(5627637u, number)

    index.set(0)
    number = ManualOnboardingPayloadParser.readDigitsFromDecimalString("10", index, 2)
    assertEquals(10u, number)

    index.set(0)
    number = ManualOnboardingPayloadParser.readDigitsFromDecimalString("10", index, 2)
    assertEquals(10u, number)

    index.set(1)
    number = ManualOnboardingPayloadParser.readDigitsFromDecimalString("11", index, 1)
    assertEquals(1u, number)

    index.set(2)
    number = ManualOnboardingPayloadParser.readDigitsFromDecimalString("100001", index, 3)
    assertEquals(0u, number)

    try {
      index.set(1)
      ManualOnboardingPayloadParser.readDigitsFromDecimalString("12345", index, 5)
      assertTrue(false)
    } catch (e: Exception) {
      println("Expected exception occurred: ${e.message}")
    }

    try {
      ManualOnboardingPayloadParser.readDigitsFromDecimalString("12", index, 5)
      assertTrue(false)
    } catch (e: Exception) {
      println("Expected exception occurred: ${e.message}")
    }

    try {
      index.set(200)
      ManualOnboardingPayloadParser.readDigitsFromDecimalString("6256276377282", index, 1)
      assertTrue(false)
    } catch (e: Exception) {
      println("Expected exception occurred: ${e.message}")
    }
  }

  /*
   * Generate Full Payload and Parse it
   */
  @Test
  fun testGenerateAndParser_fullPayload() {
    val payload = getDefaultPayload()
    payload.commissioningFlow = CommissioningFlow.CUSTOM.value
    payload.vendorId = 1
    payload.productId = 1

    val generator = ManualOnboardingPayloadGenerator(payload)
    val result = generator.payloadDecimalStringRepresentation()

    val outPayload = ManualOnboardingPayloadParser(result).populatePayload()

    assertPayloadValues(
      outPayload,
      pinCode = payload.setupPinCode,
      discriminator = 0xa,
      vendorId = payload.vendorId,
      productId = payload.productId
    )
  }

  /*
   * Generate Partial Payload and Parse it
   */
  @Test
  fun testGenerateAndParser_partialPayload() {
    val payload = getDefaultPayload()
    val generator = ManualOnboardingPayloadGenerator(payload)
    val result = generator.payloadDecimalStringRepresentation()

    val outPayload = ManualOnboardingPayloadParser(result).populatePayload()

    assertPayloadValues(
      outPayload,
      pinCode = payload.setupPinCode,
      discriminator = 0xa,
      vendorId = payload.vendorId,
      productId = payload.productId
    )
  }

  /*
   * Test Parse Short Manual PairingCode to Expected Payload
   */
  @Test
  fun testParseShortManualPairingCodeToExpectedPayload() {
    // Payload: MT:W0GU2OTB00KA0648G00
    // Setup Pin Code: 20202021
    // Setup Discriminator: 15

    val parser = OnboardingPayloadParser()
    assertThat(parser.parseManualPairingCode("34970112332"))
      .isEqualTo(
        OnboardingPayload(
          discriminator = 15,
          setupPinCode = 20202021,
          version = 0,
          vendorId = 0,
          productId = 0,
          hasShortDiscriminator = true,
          commissioningFlow = CommissioningFlow.STANDARD.value,
          discoveryCapabilities = mutableSetOf(),
        )
      )
  }

  /*
   * Test Parse Long Manual PairingCode to Expected Payload
   */
  @Test
  fun testParseLongManualPairingCodeToExpectedPayload() {
    // Payload: MT:W0GU2OTB00KA0648G00
    // Vendor Id: 9050 (0x235A)
    // Product Id: 17729 (0x4541)
    // Setup Pin Code: 20202021
    // Setup Discriminator: 15

    val parser = OnboardingPayloadParser()
    assertThat(parser.parseManualPairingCode("749701123309050177298"))
      .isEqualTo(
        OnboardingPayload(
          discriminator = 15,
          setupPinCode = 20202021,
          version = 0,
          vendorId = 0x235A,
          productId = 0x4541,
          hasShortDiscriminator = true,
          commissioningFlow = CommissioningFlow.CUSTOM.value,
          discoveryCapabilities = mutableSetOf(),
        )
      )
  }

  /*
   * Test Generate Manual PairingCode to Expected Payload
   */
  @Test
  fun testGenerateManualPairingCodetoExpectedPayload() {
    val parser = OnboardingPayloadParser()
    assertThat(
        parser.getManualPairingCodeFromPayload(
          OnboardingPayload(
            discriminator = 15,
            setupPinCode = 20202021,
            version = 0,
            vendorId = 0x235A,
            productId = 0x4541,
            hasShortDiscriminator = true,
            commissioningFlow = CommissioningFlow.CUSTOM.value,
            discoveryCapabilities = mutableSetOf(),
          )
        )
      )
      .isEqualTo("749701123309050177298")
  }
}
