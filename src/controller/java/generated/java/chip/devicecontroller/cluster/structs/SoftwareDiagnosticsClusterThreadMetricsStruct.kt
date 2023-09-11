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
import java.util.Optional

class SoftwareDiagnosticsClusterThreadMetricsStruct(
  val id: Long,
  val name: Optional<String>,
  val stackFreeCurrent: Optional<Long>,
  val stackFreeMinimum: Optional<Long>,
  val stackSize: Optional<Long>
) {
  override fun toString(): String = buildString {
    append("SoftwareDiagnosticsClusterThreadMetricsStruct {\n")
    append("\tid : $id\n")
    append("\tname : $name\n")
    append("\tstackFreeCurrent : $stackFreeCurrent\n")
    append("\tstackFreeMinimum : $stackFreeMinimum\n")
    append("\tstackSize : $stackSize\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_ID), id)
      if (name.isPresent) {
        val optname = name.get()
        put(ContextSpecificTag(TAG_NAME), optname)
      }
      if (stackFreeCurrent.isPresent) {
        val optstackFreeCurrent = stackFreeCurrent.get()
        put(ContextSpecificTag(TAG_STACK_FREE_CURRENT), optstackFreeCurrent)
      }
      if (stackFreeMinimum.isPresent) {
        val optstackFreeMinimum = stackFreeMinimum.get()
        put(ContextSpecificTag(TAG_STACK_FREE_MINIMUM), optstackFreeMinimum)
      }
      if (stackSize.isPresent) {
        val optstackSize = stackSize.get()
        put(ContextSpecificTag(TAG_STACK_SIZE), optstackSize)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_ID = 0
    private const val TAG_NAME = 1
    private const val TAG_STACK_FREE_CURRENT = 2
    private const val TAG_STACK_FREE_MINIMUM = 3
    private const val TAG_STACK_SIZE = 4

    fun fromTlv(tag: Tag, tlvReader: TlvReader): SoftwareDiagnosticsClusterThreadMetricsStruct {
      tlvReader.enterStructure(tag)
      val id = tlvReader.getLong(ContextSpecificTag(TAG_ID))
      val name =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_NAME))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_NAME)))
        } else {
          Optional.empty()
        }
      val stackFreeCurrent =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_STACK_FREE_CURRENT))) {
          Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_STACK_FREE_CURRENT)))
        } else {
          Optional.empty()
        }
      val stackFreeMinimum =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_STACK_FREE_MINIMUM))) {
          Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_STACK_FREE_MINIMUM)))
        } else {
          Optional.empty()
        }
      val stackSize =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_STACK_SIZE))) {
          Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_STACK_SIZE)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return SoftwareDiagnosticsClusterThreadMetricsStruct(
        id,
        name,
        stackFreeCurrent,
        stackFreeMinimum,
        stackSize
      )
    }
  }
}
