/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
package chip.devicecontroller.cluster.structs

import chip.devicecontroller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class OperationalCredentialsClusterCryptoProfileChainStruct(
  val PAACrypto: UInt,
  val PAICrypto: UInt,
  val DACCrypto: UInt,
) {
  override fun toString(): String = buildString {
    append("OperationalCredentialsClusterCryptoProfileChainStruct {\n")
    append("\tPAACrypto : $PAACrypto\n")
    append("\tPAICrypto : $PAICrypto\n")
    append("\tDACCrypto : $DACCrypto\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_PAA_CRYPTO), PAACrypto)
      put(ContextSpecificTag(TAG_PAI_CRYPTO), PAICrypto)
      put(ContextSpecificTag(TAG_DAC_CRYPTO), DACCrypto)
      endStructure()
    }
  }

  companion object {
    private const val TAG_PAA_CRYPTO = 0
    private const val TAG_PAI_CRYPTO = 1
    private const val TAG_DAC_CRYPTO = 2

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): OperationalCredentialsClusterCryptoProfileChainStruct {
      tlvReader.enterStructure(tlvTag)
      val PAACrypto = tlvReader.getUInt(ContextSpecificTag(TAG_PAA_CRYPTO))
      val PAICrypto = tlvReader.getUInt(ContextSpecificTag(TAG_PAI_CRYPTO))
      val DACCrypto = tlvReader.getUInt(ContextSpecificTag(TAG_DAC_CRYPTO))

      tlvReader.exitContainer()

      return OperationalCredentialsClusterCryptoProfileChainStruct(PAACrypto, PAICrypto, DACCrypto)
    }
  }
}
