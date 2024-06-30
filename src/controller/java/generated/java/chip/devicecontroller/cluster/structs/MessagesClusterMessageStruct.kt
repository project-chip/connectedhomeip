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

class MessagesClusterMessageStruct(
  val messageID: ByteArray,
  val priority: UInt,
  val messageControl: UInt,
  val startTime: ULong?,
  val duration: ULong?,
  val messageText: String,
  val responses: Optional<List<MessagesClusterMessageResponseOptionStruct>>
) {
  override fun toString(): String = buildString {
    append("MessagesClusterMessageStruct {\n")
    append("\tmessageID : $messageID\n")
    append("\tpriority : $priority\n")
    append("\tmessageControl : $messageControl\n")
    append("\tstartTime : $startTime\n")
    append("\tduration : $duration\n")
    append("\tmessageText : $messageText\n")
    append("\tresponses : $responses\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_MESSAGE_I_D), messageID)
      put(ContextSpecificTag(TAG_PRIORITY), priority)
      put(ContextSpecificTag(TAG_MESSAGE_CONTROL), messageControl)
      if (startTime != null) {
        put(ContextSpecificTag(TAG_START_TIME), startTime)
      } else {
        putNull(ContextSpecificTag(TAG_START_TIME))
      }
      if (duration != null) {
        put(ContextSpecificTag(TAG_DURATION), duration)
      } else {
        putNull(ContextSpecificTag(TAG_DURATION))
      }
      put(ContextSpecificTag(TAG_MESSAGE_TEXT), messageText)
      if (responses.isPresent) {
        val optresponses = responses.get()
        startArray(ContextSpecificTag(TAG_RESPONSES))
        for (item in optresponses.iterator()) {
          item.toTlv(AnonymousTag, this)
        }
        endArray()
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_MESSAGE_I_D = 0
    private const val TAG_PRIORITY = 1
    private const val TAG_MESSAGE_CONTROL = 2
    private const val TAG_START_TIME = 3
    private const val TAG_DURATION = 4
    private const val TAG_MESSAGE_TEXT = 5
    private const val TAG_RESPONSES = 6

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): MessagesClusterMessageStruct {
      tlvReader.enterStructure(tlvTag)
      val messageID = tlvReader.getByteArray(ContextSpecificTag(TAG_MESSAGE_I_D))
      val priority = tlvReader.getUInt(ContextSpecificTag(TAG_PRIORITY))
      val messageControl = tlvReader.getUInt(ContextSpecificTag(TAG_MESSAGE_CONTROL))
      val startTime =
        if (!tlvReader.isNull()) {
          tlvReader.getULong(ContextSpecificTag(TAG_START_TIME))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_START_TIME))
          null
        }
      val duration =
        if (!tlvReader.isNull()) {
          tlvReader.getULong(ContextSpecificTag(TAG_DURATION))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_DURATION))
          null
        }
      val messageText = tlvReader.getString(ContextSpecificTag(TAG_MESSAGE_TEXT))
      val responses =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_RESPONSES))) {
          Optional.of(
            buildList<MessagesClusterMessageResponseOptionStruct> {
              tlvReader.enterArray(ContextSpecificTag(TAG_RESPONSES))
              while (!tlvReader.isEndOfContainer()) {
                add(MessagesClusterMessageResponseOptionStruct.fromTlv(AnonymousTag, tlvReader))
              }
              tlvReader.exitContainer()
            }
          )
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return MessagesClusterMessageStruct(
        messageID,
        priority,
        messageControl,
        startTime,
        duration,
        messageText,
        responses
      )
    }
  }
}
