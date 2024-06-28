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
package matter.controller.cluster.structs

import java.util.Optional
import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class SoftwareDiagnosticsClusterThreadMetricsStruct(
  val id: ULong,
  val name: Optional<String>,
  val stackFreeCurrent: Optional<UInt>,
  val stackFreeMinimum: Optional<UInt>,
  val stackSize: Optional<UInt>,
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

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
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

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): SoftwareDiagnosticsClusterThreadMetricsStruct {
      tlvReader.enterStructure(tlvTag)
      val id = tlvReader.getULong(ContextSpecificTag(TAG_ID))
      val name =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_NAME))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_NAME)))
        } else {
          Optional.empty()
        }
      val stackFreeCurrent =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_STACK_FREE_CURRENT))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_STACK_FREE_CURRENT)))
        } else {
          Optional.empty()
        }
      val stackFreeMinimum =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_STACK_FREE_MINIMUM))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_STACK_FREE_MINIMUM)))
        } else {
          Optional.empty()
        }
      val stackSize =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_STACK_SIZE))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_STACK_SIZE)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return SoftwareDiagnosticsClusterThreadMetricsStruct(
        id,
        name,
        stackFreeCurrent,
        stackFreeMinimum,
        stackSize,
      )
    }
  }
}
