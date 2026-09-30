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
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class AmbientContextSensingClusterAmbientContextDetectEndedEvent(
  val eventStartTimePos: Optional<ULong>,
  val eventStartTimeSys: Optional<ULong>,
) {
  override fun toString(): String = buildString {
    append("AmbientContextSensingClusterAmbientContextDetectEndedEvent {\n")
    append("\teventStartTimePos : $eventStartTimePos\n")
    append("\teventStartTimeSys : $eventStartTimeSys\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (eventStartTimePos.isPresent) {
        val opteventStartTimePos = eventStartTimePos.get()
        put(ContextSpecificTag(TAG_EVENT_START_TIME_POS), opteventStartTimePos)
      }
      if (eventStartTimeSys.isPresent) {
        val opteventStartTimeSys = eventStartTimeSys.get()
        put(ContextSpecificTag(TAG_EVENT_START_TIME_SYS), opteventStartTimeSys)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_EVENT_START_TIME_POS = 0
    private const val TAG_EVENT_START_TIME_SYS = 1

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): AmbientContextSensingClusterAmbientContextDetectEndedEvent {
      tlvReader.enterStructure(tlvTag)
      val eventStartTimePos =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_EVENT_START_TIME_POS))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_EVENT_START_TIME_POS)))
        } else {
          Optional.empty()
        }
      val eventStartTimeSys =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_EVENT_START_TIME_SYS))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_EVENT_START_TIME_SYS)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return AmbientContextSensingClusterAmbientContextDetectEndedEvent(
        eventStartTimePos,
        eventStartTimeSys,
      )
    }
  }
}
