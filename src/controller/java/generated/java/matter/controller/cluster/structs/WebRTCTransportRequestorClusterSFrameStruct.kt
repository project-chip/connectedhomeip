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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class WebRTCTransportRequestorClusterSFrameStruct(
  val audioCipherSuite: UShort,
  val videoCipherSuite: UShort,
  val senderKey: WebRTCTransportRequestorClusterSFrameKeyStruct,
  val receiveKeys: List<WebRTCTransportRequestorClusterSFrameKeyStruct>,
  val ratchetBits: UByte,
  val ratchetTime: Optional<UByte>
) {
  override fun toString(): String = buildString {
    append("WebRTCTransportRequestorClusterSFrameStruct {\n")
    append("\taudioCipherSuite : $audioCipherSuite\n")
    append("\tvideoCipherSuite : $videoCipherSuite\n")
    append("\tsenderKey : $senderKey\n")
    append("\treceiveKeys : $receiveKeys\n")
    append("\tratchetBits : $ratchetBits\n")
    append("\tratchetTime : $ratchetTime\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_AUDIO_CIPHER_SUITE), audioCipherSuite)
      put(ContextSpecificTag(TAG_VIDEO_CIPHER_SUITE), videoCipherSuite)
      senderKey.toTlv(ContextSpecificTag(TAG_SENDER_KEY), this)
      startArray(ContextSpecificTag(TAG_RECEIVE_KEYS))
      for (item in receiveKeys.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endArray()
      put(ContextSpecificTag(TAG_RATCHET_BITS), ratchetBits)
      if (ratchetTime.isPresent) {
        val optratchetTime = ratchetTime.get()
        put(ContextSpecificTag(TAG_RATCHET_TIME), optratchetTime)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_AUDIO_CIPHER_SUITE = 0
    private const val TAG_VIDEO_CIPHER_SUITE = 1
    private const val TAG_SENDER_KEY = 2
    private const val TAG_RECEIVE_KEYS = 3
    private const val TAG_RATCHET_BITS = 4
    private const val TAG_RATCHET_TIME = 5

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): WebRTCTransportRequestorClusterSFrameStruct {
      tlvReader.enterStructure(tlvTag)
      val audioCipherSuite = tlvReader.getUShort(ContextSpecificTag(TAG_AUDIO_CIPHER_SUITE))
      val videoCipherSuite = tlvReader.getUShort(ContextSpecificTag(TAG_VIDEO_CIPHER_SUITE))
      val senderKey = WebRTCTransportRequestorClusterSFrameKeyStruct.fromTlv(ContextSpecificTag(TAG_SENDER_KEY), tlvReader)
      val receiveKeys = buildList<WebRTCTransportRequestorClusterSFrameKeyStruct> {
      tlvReader.enterArray(ContextSpecificTag(TAG_RECEIVE_KEYS))
      while(!tlvReader.isEndOfContainer()) {
        add(WebRTCTransportRequestorClusterSFrameKeyStruct.fromTlv(AnonymousTag, tlvReader))
      }
      tlvReader.exitContainer()
    }
      val ratchetBits = tlvReader.getUByte(ContextSpecificTag(TAG_RATCHET_BITS))
      val ratchetTime = if (tlvReader.isNextTag(ContextSpecificTag(TAG_RATCHET_TIME))) {
      Optional.of(tlvReader.getUByte(ContextSpecificTag(TAG_RATCHET_TIME)))
    } else {
      Optional.empty()
    }
      
      tlvReader.exitContainer()

      return WebRTCTransportRequestorClusterSFrameStruct(audioCipherSuite, videoCipherSuite, senderKey, receiveKeys, ratchetBits, ratchetTime)
    }
  }
}
