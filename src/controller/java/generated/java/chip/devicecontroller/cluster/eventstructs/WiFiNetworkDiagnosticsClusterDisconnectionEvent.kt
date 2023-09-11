/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package chip.devicecontroller.cluster.eventstructs

import chip.devicecontroller.cluster.*
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

class WiFiNetworkDiagnosticsClusterDisconnectionEvent(val reasonCode: Int) {
  override fun toString(): String = buildString {
    append("WiFiNetworkDiagnosticsClusterDisconnectionEvent {\n")
    append("\treasonCode : $reasonCode\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_REASON_CODE), reasonCode)
      endStructure()
    }
  }

  companion object {
    private const val TAG_REASON_CODE = 0

    fun fromTlv(tag: Tag, tlvReader: TlvReader): WiFiNetworkDiagnosticsClusterDisconnectionEvent {
      tlvReader.enterStructure(tag)
      val reasonCode = tlvReader.getInt(ContextSpecificTag(TAG_REASON_CODE))

      tlvReader.exitContainer()

      return WiFiNetworkDiagnosticsClusterDisconnectionEvent(reasonCode)
    }
  }
}
