/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package chip.devicecontroller.cluster.structs

import chip.devicecontroller.cluster.*
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

class BasicInformationClusterCapabilityMinimaStruct(
  val caseSessionsPerFabric: Int,
  val subscriptionsPerFabric: Int
) {
  override fun toString(): String = buildString {
    append("BasicInformationClusterCapabilityMinimaStruct {\n")
    append("\tcaseSessionsPerFabric : $caseSessionsPerFabric\n")
    append("\tsubscriptionsPerFabric : $subscriptionsPerFabric\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_CASE_SESSIONS_PER_FABRIC), caseSessionsPerFabric)
      put(ContextSpecificTag(TAG_SUBSCRIPTIONS_PER_FABRIC), subscriptionsPerFabric)
      endStructure()
    }
  }

  companion object {
    private const val TAG_CASE_SESSIONS_PER_FABRIC = 0
    private const val TAG_SUBSCRIPTIONS_PER_FABRIC = 1

    fun fromTlv(tag: Tag, tlvReader: TlvReader): BasicInformationClusterCapabilityMinimaStruct {
      tlvReader.enterStructure(tag)
      val caseSessionsPerFabric = tlvReader.getInt(ContextSpecificTag(TAG_CASE_SESSIONS_PER_FABRIC))
      val subscriptionsPerFabric =
        tlvReader.getInt(ContextSpecificTag(TAG_SUBSCRIPTIONS_PER_FABRIC))

      tlvReader.exitContainer()

      return BasicInformationClusterCapabilityMinimaStruct(
        caseSessionsPerFabric,
        subscriptionsPerFabric
      )
    }
  }
}
