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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvParsingException
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

import java.util.Optional

class MessagesClusterPresentMessageRequestStruct (
    val messageID: ByteArray,
    val priority: UInt,
    val messageControl: UInt,
    val startTime: ULong,
    val duration: UInt,
    val messageText: String,
    val responses: List<MessagesClusterMessageResponseOptionStruct>) {
  override fun toString(): String  = buildString {
    append("MessagesClusterPresentMessageRequestStruct {\n")
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
      put(ContextSpecificTag(TAG_START_TIME), startTime)
      put(ContextSpecificTag(TAG_DURATION), duration)
      put(ContextSpecificTag(TAG_MESSAGE_TEXT), messageText)
      startArray(ContextSpecificTag(TAG_RESPONSES))
      for (item in responses.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endArray()
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

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : MessagesClusterPresentMessageRequestStruct {
      tlvReader.enterStructure(tlvTag)
      val messageID = tlvReader.getByteArray(ContextSpecificTag(TAG_MESSAGE_I_D))
      val priority = tlvReader.getUInt(ContextSpecificTag(TAG_PRIORITY))
      val messageControl = tlvReader.getUInt(ContextSpecificTag(TAG_MESSAGE_CONTROL))
      val startTime = tlvReader.getULong(ContextSpecificTag(TAG_START_TIME))
      val duration = tlvReader.getUInt(ContextSpecificTag(TAG_DURATION))
      val messageText = tlvReader.getString(ContextSpecificTag(TAG_MESSAGE_TEXT))
      val responses = buildList<MessagesClusterMessageResponseOptionStruct> {
      tlvReader.enterArray(ContextSpecificTag(TAG_RESPONSES))
      while(!tlvReader.isEndOfContainer()) {
        add(MessagesClusterMessageResponseOptionStruct.fromTlv(AnonymousTag, tlvReader))
      }
      tlvReader.exitContainer()
    }
      
      tlvReader.exitContainer()

      return MessagesClusterPresentMessageRequestStruct(messageID, priority, messageControl, startTime, duration, messageText, responses)
    }
  }
}
