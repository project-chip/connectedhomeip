/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019-2023 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package chip.tlv

import com.google.common.truth.Truth.assertThat
import java.math.BigInteger
import org.junit.Test
import org.junit.runner.RunWith
import org.junit.runners.JUnit4

// Extracted from a Newman device during a pairing flow. Represents a fabric
// ID and keys for the fabric 7885a14c693bf1cb.
private val fabricConfig =
  """
  D50000050001002701CBF13B694CA18578360
  21525010110240201300310149BF1430B26F5
  E4BBF380D3DB855BA1300414E0E8BAA1CAC6E
  8E7216D720BD13C61C5E0E7B9012405002406
  00181818
  """
    .trimIndent()
    .replace("\n", "")
    .chunked(2)
    .map { it.toInt(16) and 0xFF }
    .map { it.toByte() }
    .toByteArray()

@RunWith(JUnit4::class)
class TlvWriterTest {
  private fun String.octetsToByteArray(): ByteArray =
    replace(" ", "").chunked(2).map { it.toInt(16) and 0xFF }.map { it.toByte() }.toByteArray()

  @Test
  fun encodingFabricConfig_allElements() {
    val encodedTlv =
      TlvWriter(fabricConfig.size)
        .startStructure(FullyQualifiedTag(6, 0u, 5u, 1u))
        .put(ContextSpecificTag(1), BigInteger("7885a14c693bf1cb", 16).toLong().toULong())
        .startArray(ContextSpecificTag(2))
        .startStructure(AnonymousTag)
        .put(ContextSpecificTag(1), 0x1001u)
        .put(ContextSpecificTag(2), 0x01u)
        .put(ContextSpecificTag(3), "149BF1430B26F5E4BBF380D3DB855BA1".octetsToByteArray())
        .put(ContextSpecificTag(4), "E0E8BAA1CAC6E8E7216D720BD13C61C5E0E7B901".octetsToByteArray())
        .put(ContextSpecificTag(5), 0u)
        .put(ContextSpecificTag(6), 0u)
        .endStructure()
        .endArray()
        .endStructure()
        .validateTlv()
        .getEncoded()

    assertThat(encodedTlv).isEqualTo(fabricConfig)
  }
}
