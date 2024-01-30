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
import java.util.Optional
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class MessagesClusterMessageCompleteEvent(
  val messageID: ByteArray,
  val responseID: Optional<ULong>?,
  val reply: Optional<String>?,
  val futureMessagesPreference: UInt?
) {
  override fun toString(): String = buildString {
    append("MessagesClusterMessageCompleteEvent {\n")
    append("\tmessageID : $messageID\n")
    append("\tresponseID : $responseID\n")
    append("\treply : $reply\n")
    append("\tfutureMessagesPreference : $futureMessagesPreference\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_MESSAGE_I_D), messageID)
      if (responseID != null) {
        if (responseID.isPresent) {
          val optresponseID = responseID.get()
          put(ContextSpecificTag(TAG_RESPONSE_I_D), optresponseID)
        }
      } else {
        putNull(ContextSpecificTag(TAG_RESPONSE_I_D))
      }
      if (reply != null) {
        if (reply.isPresent) {
          val optreply = reply.get()
          put(ContextSpecificTag(TAG_REPLY), optreply)
        }
      } else {
        putNull(ContextSpecificTag(TAG_REPLY))
      }
      if (futureMessagesPreference != null) {
        put(ContextSpecificTag(TAG_FUTURE_MESSAGES_PREFERENCE), futureMessagesPreference)
      } else {
        putNull(ContextSpecificTag(TAG_FUTURE_MESSAGES_PREFERENCE))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_MESSAGE_I_D = 0
    private const val TAG_RESPONSE_I_D = 1
    private const val TAG_REPLY = 2
    private const val TAG_FUTURE_MESSAGES_PREFERENCE = 3

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): MessagesClusterMessageCompleteEvent {
      tlvReader.enterStructure(tlvTag)
      val messageID = tlvReader.getByteArray(ContextSpecificTag(TAG_MESSAGE_I_D))
      val responseID =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_RESPONSE_I_D))) {
            Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_RESPONSE_I_D)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_RESPONSE_I_D))
          null
        }
      val reply =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_REPLY))) {
            Optional.of(tlvReader.getString(ContextSpecificTag(TAG_REPLY)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_REPLY))
          null
        }
      val futureMessagesPreference =
        if (!tlvReader.isNull()) {
          tlvReader.getUInt(ContextSpecificTag(TAG_FUTURE_MESSAGES_PREFERENCE))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_FUTURE_MESSAGES_PREFERENCE))
          null
        }

      tlvReader.exitContainer()

      return MessagesClusterMessageCompleteEvent(
        messageID,
        responseID,
        reply,
        futureMessagesPreference
      )
    }
  }
}
