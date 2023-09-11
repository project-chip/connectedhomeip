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

class WiFiNetworkDiagnosticsClusterConnectionStatusEvent(val connectionStatus: Int) {
  override fun toString(): String = buildString {
    append("WiFiNetworkDiagnosticsClusterConnectionStatusEvent {\n")
    append("\tconnectionStatus : $connectionStatus\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_CONNECTION_STATUS), connectionStatus)
      endStructure()
    }
  }

  companion object {
    private const val TAG_CONNECTION_STATUS = 0

    fun fromTlv(
      tag: Tag,
      tlvReader: TlvReader
    ): WiFiNetworkDiagnosticsClusterConnectionStatusEvent {
      tlvReader.enterStructure(tag)
      val connectionStatus = tlvReader.getInt(ContextSpecificTag(TAG_CONNECTION_STATUS))

      tlvReader.exitContainer()

      return WiFiNetworkDiagnosticsClusterConnectionStatusEvent(connectionStatus)
    }
  }
}
