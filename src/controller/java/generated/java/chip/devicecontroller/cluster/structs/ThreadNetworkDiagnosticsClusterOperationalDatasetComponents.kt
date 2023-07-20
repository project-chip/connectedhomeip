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
    tlvWriter.put(ContextSpecificTag(TAG_ACTIVE_TIMESTAMP_PRESENT), activeTimestampPresent)
    tlvWriter.put(ContextSpecificTag(TAG_PENDING_TIMESTAMP_PRESENT), pendingTimestampPresent)
    tlvWriter.put(ContextSpecificTag(TAG_MASTER_KEY_PRESENT), masterKeyPresent)
    tlvWriter.put(ContextSpecificTag(TAG_NETWORK_NAME_PRESENT), networkNamePresent)
    tlvWriter.put(ContextSpecificTag(TAG_EXTENDED_PAN_ID_PRESENT), extendedPanIdPresent)
    tlvWriter.put(ContextSpecificTag(TAG_MESH_LOCAL_PREFIX_PRESENT), meshLocalPrefixPresent)
    tlvWriter.put(ContextSpecificTag(TAG_DELAY_PRESENT), delayPresent)
    tlvWriter.put(ContextSpecificTag(TAG_PAN_ID_PRESENT), panIdPresent)
    tlvWriter.put(ContextSpecificTag(TAG_CHANNEL_PRESENT), channelPresent)
    tlvWriter.put(ContextSpecificTag(TAG_PSKC_PRESENT), pskcPresent)
    tlvWriter.put(ContextSpecificTag(TAG_SECURITY_POLICY_PRESENT), securityPolicyPresent)
    tlvWriter.put(ContextSpecificTag(TAG_CHANNEL_MASK_PRESENT), channelMaskPresent)
    tlvWriter.endStructure()
  }

  companion object {
    private const val TAG_ACTIVE_TIMESTAMP_PRESENT = 0
    private const val TAG_PENDING_TIMESTAMP_PRESENT = 1
    private const val TAG_MASTER_KEY_PRESENT = 2
    private const val TAG_NETWORK_NAME_PRESENT = 3
    private const val TAG_EXTENDED_PAN_ID_PRESENT = 4
    private const val TAG_MESH_LOCAL_PREFIX_PRESENT = 5
    private const val TAG_DELAY_PRESENT = 6
    private const val TAG_PAN_ID_PRESENT = 7
    private const val TAG_CHANNEL_PRESENT = 8
    private const val TAG_PSKC_PRESENT = 9
    private const val TAG_SECURITY_POLICY_PRESENT = 10
    private const val TAG_CHANNEL_MASK_PRESENT = 11

    fun fromTlv(tag: Tag, tlvReader: TlvReader) : ThreadNetworkDiagnosticsClusterOperationalDatasetComponents {
      tlvReader.enterStructure(tag)
      val activeTimestampPresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(TAG_ACTIVE_TIMESTAMP_PRESENT))
      val pendingTimestampPresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(TAG_PENDING_TIMESTAMP_PRESENT))
      val masterKeyPresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(TAG_MASTER_KEY_PRESENT))
      val networkNamePresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(TAG_NETWORK_NAME_PRESENT))
      val extendedPanIdPresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(TAG_EXTENDED_PAN_ID_PRESENT))
      val meshLocalPrefixPresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(TAG_MESH_LOCAL_PREFIX_PRESENT))
      val delayPresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(TAG_DELAY_PRESENT))
      val panIdPresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(TAG_PAN_ID_PRESENT))
      val channelPresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(TAG_CHANNEL_PRESENT))
      val pskcPresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(TAG_PSKC_PRESENT))
      val securityPolicyPresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(TAG_SECURITY_POLICY_PRESENT))
      val channelMaskPresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(TAG_CHANNEL_MASK_PRESENT))
      
      tlvReader.exitContainer()

      return ThreadNetworkDiagnosticsClusterOperationalDatasetComponents(activeTimestampPresent, pendingTimestampPresent, masterKeyPresent, networkNamePresent, extendedPanIdPresent, meshLocalPrefixPresent, delayPresent, panIdPresent, channelPresent, pskcPresent, securityPolicyPresent, channelMaskPresent)
    }
  }
}
