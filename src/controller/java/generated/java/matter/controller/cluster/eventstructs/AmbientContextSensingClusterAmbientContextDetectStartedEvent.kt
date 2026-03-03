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
package matter.controller.cluster.eventstructs

import java.util.Optional
import matter.controller.cluster.*
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class AmbientContextSensingClusterAmbientContextDetectStartedEvent(
  val ambientContextType:
    Optional<
      List<matter.controller.cluster.structs.AmbientContextSensingClusterAmbientContextTypeStruct>
    >,
  val objectCount: Optional<UShort>,
) {
  override fun toString(): String = buildString {
    append("AmbientContextSensingClusterAmbientContextDetectStartedEvent {\n")
    append("\tambientContextType : $ambientContextType\n")
    append("\tobjectCount : $objectCount\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (ambientContextType.isPresent) {
        val optambientContextType = ambientContextType.get()
        startArray(ContextSpecificTag(TAG_AMBIENT_CONTEXT_TYPE))
        for (item in optambientContextType.iterator()) {
          item.toTlv(AnonymousTag, this)
        }
        endArray()
      }
      if (objectCount.isPresent) {
        val optobjectCount = objectCount.get()
        put(ContextSpecificTag(TAG_OBJECT_COUNT), optobjectCount)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_AMBIENT_CONTEXT_TYPE = 0
    private const val TAG_OBJECT_COUNT = 1

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): AmbientContextSensingClusterAmbientContextDetectStartedEvent {
      tlvReader.enterStructure(tlvTag)
      val ambientContextType =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_AMBIENT_CONTEXT_TYPE))) {
          Optional.of(
            buildList<
              matter.controller.cluster.structs.AmbientContextSensingClusterAmbientContextTypeStruct
            > {
              tlvReader.enterArray(ContextSpecificTag(TAG_AMBIENT_CONTEXT_TYPE))
              while (!tlvReader.isEndOfContainer()) {
                this.add(
                  matter.controller.cluster.structs
                    .AmbientContextSensingClusterAmbientContextTypeStruct
                    .fromTlv(AnonymousTag, tlvReader)
                )
              }
              tlvReader.exitContainer()
            }
          )
        } else {
          Optional.empty()
        }
      val objectCount =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_OBJECT_COUNT))) {
          Optional.of(tlvReader.getUShort(ContextSpecificTag(TAG_OBJECT_COUNT)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return AmbientContextSensingClusterAmbientContextDetectStartedEvent(
        ambientContextType,
        objectCount,
      )
    }
  }
}
