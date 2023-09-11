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

class WiFiNetworkDiagnosticsClusterAssociationFailureEvent(
  val associationFailure: Int,
  val status: Int
) {
  override fun toString(): String = buildString {
    append("WiFiNetworkDiagnosticsClusterAssociationFailureEvent {\n")
    append("\tassociationFailure : $associationFailure\n")
    append("\tstatus : $status\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_ASSOCIATION_FAILURE), associationFailure)
      put(ContextSpecificTag(TAG_STATUS), status)
      endStructure()
    }
  }

  companion object {
    private const val TAG_ASSOCIATION_FAILURE = 0
    private const val TAG_STATUS = 1

    fun fromTlv(
      tag: Tag,
      tlvReader: TlvReader
    ): WiFiNetworkDiagnosticsClusterAssociationFailureEvent {
      tlvReader.enterStructure(tag)
      val associationFailure = tlvReader.getInt(ContextSpecificTag(TAG_ASSOCIATION_FAILURE))
      val status = tlvReader.getInt(ContextSpecificTag(TAG_STATUS))

      tlvReader.exitContainer()

      return WiFiNetworkDiagnosticsClusterAssociationFailureEvent(associationFailure, status)
    }
  }
}
