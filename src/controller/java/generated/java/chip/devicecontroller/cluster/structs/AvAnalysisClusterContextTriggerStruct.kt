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
import java.util.Optional
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class AvAnalysisClusterContextTriggerStruct(
  val context: AvAnalysisClusterSemanticTagStruct,
  val zoneIDs: Optional<List<UInt>>?,
) {
  override fun toString(): String = buildString {
    append("AvAnalysisClusterContextTriggerStruct {\n")
    append("\tcontext : $context\n")
    append("\tzoneIDs : $zoneIDs\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      context.toTlv(ContextSpecificTag(TAG_CONTEXT), this)
      if (zoneIDs != null) {
        if (zoneIDs.isPresent) {
          val optzoneIDs = zoneIDs.get()
          startArray(ContextSpecificTag(TAG_ZONE_I_DS))
          for (item in optzoneIDs.iterator()) {
            put(AnonymousTag, item)
          }
          endArray()
        }
      } else {
        putNull(ContextSpecificTag(TAG_ZONE_I_DS))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_CONTEXT = 0
    private const val TAG_ZONE_I_DS = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): AvAnalysisClusterContextTriggerStruct {
      tlvReader.enterStructure(tlvTag)
      val context =
        AvAnalysisClusterSemanticTagStruct.fromTlv(ContextSpecificTag(TAG_CONTEXT), tlvReader)
      val zoneIDs =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_ZONE_I_DS))) {
            Optional.of(
              buildList<UInt> {
                tlvReader.enterArray(ContextSpecificTag(TAG_ZONE_I_DS))
                while (!tlvReader.isEndOfContainer()) {
                  add(tlvReader.getUInt(AnonymousTag))
                }
                tlvReader.exitContainer()
              }
            )
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_ZONE_I_DS))
          null
        }

      tlvReader.exitContainer()

      return AvAnalysisClusterContextTriggerStruct(context, zoneIDs)
    }
  }
}
