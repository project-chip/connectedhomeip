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
package chip.devicecontroller.cluster.eventstructs

import chip.devicecontroller.cluster.*
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvParsingException
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

import java.util.Optional

class MessagesClusterMessageCompleteEvent (
    val messageID: ByteArray,
    val timestamp: ULong,
    val responseID: ULong?,
    val reply: String?,
    val futureMessagesPref: UInt?) {
  override fun toString(): String  = buildString {
    append("MessagesClusterMessageCompleteEvent {\n")
    append("\tmessageID : $messageID\n")
    append("\ttimestamp : $timestamp\n")
    append("\tresponseID : $responseID\n")
    append("\treply : $reply\n")
    append("\tfutureMessagesPref : $futureMessagesPref\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_MESSAGE_I_D), messageID)
      put(ContextSpecificTag(TAG_TIMESTAMP), timestamp)
      if (responseID != null) {
      put(ContextSpecificTag(TAG_RESPONSE_I_D), responseID)
    } else {
      putNull(ContextSpecificTag(TAG_RESPONSE_I_D))
    }
      if (reply != null) {
      put(ContextSpecificTag(TAG_REPLY), reply)
    } else {
      putNull(ContextSpecificTag(TAG_REPLY))
    }
      if (futureMessagesPref != null) {
      put(ContextSpecificTag(TAG_FUTURE_MESSAGES_PREF), futureMessagesPref)
    } else {
      putNull(ContextSpecificTag(TAG_FUTURE_MESSAGES_PREF))
    }
      endStructure()
    }
  }

  companion object {
    private const val TAG_MESSAGE_I_D = 0
    private const val TAG_TIMESTAMP = 2
    private const val TAG_RESPONSE_I_D = 3
    private const val TAG_REPLY = 4
    private const val TAG_FUTURE_MESSAGES_PREF = 5

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : MessagesClusterMessageCompleteEvent {
      tlvReader.enterStructure(tlvTag)
      val messageID = tlvReader.getByteArray(ContextSpecificTag(TAG_MESSAGE_I_D))
      val timestamp = tlvReader.getULong(ContextSpecificTag(TAG_TIMESTAMP))
      val responseID = if (!tlvReader.isNull()) {
      tlvReader.getULong(ContextSpecificTag(TAG_RESPONSE_I_D))
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_RESPONSE_I_D))
      null
    }
      val reply = if (!tlvReader.isNull()) {
      tlvReader.getString(ContextSpecificTag(TAG_REPLY))
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_REPLY))
      null
    }
      val futureMessagesPref = if (!tlvReader.isNull()) {
      tlvReader.getUInt(ContextSpecificTag(TAG_FUTURE_MESSAGES_PREF))
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_FUTURE_MESSAGES_PREF))
      null
    }
      
      tlvReader.exitContainer()

      return MessagesClusterMessageCompleteEvent(messageID, timestamp, responseID, reply, futureMessagesPref)
    }
  }
}
