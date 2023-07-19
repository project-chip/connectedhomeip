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
import chip.tlv.AnonymousTag
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvParsingException
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

import java.util.Optional

class ThreadNetworkDiagnosticsClusterOperationalDatasetComponents (
    val activeTimestampPresent: Boolean,
    val pendingTimestampPresent: Boolean,
    val masterKeyPresent: Boolean,
    val networkNamePresent: Boolean,
    val extendedPanIdPresent: Boolean,
    val meshLocalPrefixPresent: Boolean,
    val delayPresent: Boolean,
    val panIdPresent: Boolean,
    val channelPresent: Boolean,
    val pskcPresent: Boolean,
    val securityPolicyPresent: Boolean,
    val channelMaskPresent: Boolean) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("ThreadNetworkDiagnosticsClusterOperationalDatasetComponents {\n")
    builder.append("\tactiveTimestampPresent : $activeTimestampPresent\n")
    builder.append("\tpendingTimestampPresent : $pendingTimestampPresent\n")
    builder.append("\tmasterKeyPresent : $masterKeyPresent\n")
    builder.append("\tnetworkNamePresent : $networkNamePresent\n")
    builder.append("\textendedPanIdPresent : $extendedPanIdPresent\n")
    builder.append("\tmeshLocalPrefixPresent : $meshLocalPrefixPresent\n")
    builder.append("\tdelayPresent : $delayPresent\n")
    builder.append("\tpanIdPresent : $panIdPresent\n")
    builder.append("\tchannelPresent : $channelPresent\n")
    builder.append("\tpskcPresent : $pskcPresent\n")
    builder.append("\tsecurityPolicyPresent : $securityPolicyPresent\n")
    builder.append("\tchannelMaskPresent : $channelMaskPresent\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(0), activeTimestampPresent)
    tlvWriter.put(ContextSpecificTag(1), pendingTimestampPresent)
    tlvWriter.put(ContextSpecificTag(2), masterKeyPresent)
    tlvWriter.put(ContextSpecificTag(3), networkNamePresent)
    tlvWriter.put(ContextSpecificTag(4), extendedPanIdPresent)
    tlvWriter.put(ContextSpecificTag(5), meshLocalPrefixPresent)
    tlvWriter.put(ContextSpecificTag(6), delayPresent)
    tlvWriter.put(ContextSpecificTag(7), panIdPresent)
    tlvWriter.put(ContextSpecificTag(8), channelPresent)
    tlvWriter.put(ContextSpecificTag(9), pskcPresent)
    tlvWriter.put(ContextSpecificTag(10), securityPolicyPresent)
    tlvWriter.put(ContextSpecificTag(11), channelMaskPresent)
    tlvWriter.endStructure()
  }

  companion object {
    fun fromTlv(tag: Tag, tlvReader: TlvReader) : ThreadNetworkDiagnosticsClusterOperationalDatasetComponents {
      tlvReader.enterStructure(tag)
      val activeTimestampPresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(0))
      val pendingTimestampPresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(1))
      val masterKeyPresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(2))
      val networkNamePresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(3))
      val extendedPanIdPresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(4))
      val meshLocalPrefixPresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(5))
      val delayPresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(6))
      val panIdPresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(7))
      val channelPresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(8))
      val pskcPresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(9))
      val securityPolicyPresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(10))
      val channelMaskPresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(11))
      
      tlvReader.exitContainer()

      return ThreadNetworkDiagnosticsClusterOperationalDatasetComponents(activeTimestampPresent, pendingTimestampPresent, masterKeyPresent, networkNamePresent, extendedPanIdPresent, meshLocalPrefixPresent, delayPresent, panIdPresent, channelPresent, pskcPresent, securityPolicyPresent, channelMaskPresent)
    }
  }
}
