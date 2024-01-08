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

class ThreadNetworkDiagnosticsClusterOperationalDatasetComponents(
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
  val channelMaskPresent: Boolean
) {
  override fun toString(): String = buildString {
    append("ThreadNetworkDiagnosticsClusterOperationalDatasetComponents {\n")
    append("\tactiveTimestampPresent : $activeTimestampPresent\n")
    append("\tpendingTimestampPresent : $pendingTimestampPresent\n")
    append("\tmasterKeyPresent : $masterKeyPresent\n")
    append("\tnetworkNamePresent : $networkNamePresent\n")
    append("\textendedPanIdPresent : $extendedPanIdPresent\n")
    append("\tmeshLocalPrefixPresent : $meshLocalPrefixPresent\n")
    append("\tdelayPresent : $delayPresent\n")
    append("\tpanIdPresent : $panIdPresent\n")
    append("\tchannelPresent : $channelPresent\n")
    append("\tpskcPresent : $pskcPresent\n")
    append("\tsecurityPolicyPresent : $securityPolicyPresent\n")
    append("\tchannelMaskPresent : $channelMaskPresent\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_ACTIVE_TIMESTAMP_PRESENT), activeTimestampPresent)
      put(ContextSpecificTag(TAG_PENDING_TIMESTAMP_PRESENT), pendingTimestampPresent)
      put(ContextSpecificTag(TAG_MASTER_KEY_PRESENT), masterKeyPresent)
      put(ContextSpecificTag(TAG_NETWORK_NAME_PRESENT), networkNamePresent)
      put(ContextSpecificTag(TAG_EXTENDED_PAN_ID_PRESENT), extendedPanIdPresent)
      put(ContextSpecificTag(TAG_MESH_LOCAL_PREFIX_PRESENT), meshLocalPrefixPresent)
      put(ContextSpecificTag(TAG_DELAY_PRESENT), delayPresent)
      put(ContextSpecificTag(TAG_PAN_ID_PRESENT), panIdPresent)
      put(ContextSpecificTag(TAG_CHANNEL_PRESENT), channelPresent)
      put(ContextSpecificTag(TAG_PSKC_PRESENT), pskcPresent)
      put(ContextSpecificTag(TAG_SECURITY_POLICY_PRESENT), securityPolicyPresent)
      put(ContextSpecificTag(TAG_CHANNEL_MASK_PRESENT), channelMaskPresent)
      endStructure()
    }
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

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader
    ): ThreadNetworkDiagnosticsClusterOperationalDatasetComponents {
      tlvReader.enterStructure(tlvTag)
      val activeTimestampPresent =
        tlvReader.getBoolean(ContextSpecificTag(TAG_ACTIVE_TIMESTAMP_PRESENT))
      val pendingTimestampPresent =
        tlvReader.getBoolean(ContextSpecificTag(TAG_PENDING_TIMESTAMP_PRESENT))
      val masterKeyPresent = tlvReader.getBoolean(ContextSpecificTag(TAG_MASTER_KEY_PRESENT))
      val networkNamePresent = tlvReader.getBoolean(ContextSpecificTag(TAG_NETWORK_NAME_PRESENT))
      val extendedPanIdPresent =
        tlvReader.getBoolean(ContextSpecificTag(TAG_EXTENDED_PAN_ID_PRESENT))
      val meshLocalPrefixPresent =
        tlvReader.getBoolean(ContextSpecificTag(TAG_MESH_LOCAL_PREFIX_PRESENT))
      val delayPresent = tlvReader.getBoolean(ContextSpecificTag(TAG_DELAY_PRESENT))
      val panIdPresent = tlvReader.getBoolean(ContextSpecificTag(TAG_PAN_ID_PRESENT))
      val channelPresent = tlvReader.getBoolean(ContextSpecificTag(TAG_CHANNEL_PRESENT))
      val pskcPresent = tlvReader.getBoolean(ContextSpecificTag(TAG_PSKC_PRESENT))
      val securityPolicyPresent =
        tlvReader.getBoolean(ContextSpecificTag(TAG_SECURITY_POLICY_PRESENT))
      val channelMaskPresent = tlvReader.getBoolean(ContextSpecificTag(TAG_CHANNEL_MASK_PRESENT))

      tlvReader.exitContainer()

      return ThreadNetworkDiagnosticsClusterOperationalDatasetComponents(
        activeTimestampPresent,
        pendingTimestampPresent,
        masterKeyPresent,
        networkNamePresent,
        extendedPanIdPresent,
        meshLocalPrefixPresent,
        delayPresent,
        panIdPresent,
        channelPresent,
        pskcPresent,
        securityPolicyPresent,
        channelMaskPresent
      )
    }
  }
}
