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

class WebRTCTransportProviderClusterICEServerStruct(
  val urls: List<String>,
  val username: Optional<String>,
  val credential: Optional<String>,
  val caid: Optional<UShort>,
) {
  override fun toString(): String = buildString {
    append("WebRTCTransportProviderClusterICEServerStruct {\n")
    append("\turls : $urls\n")
    append("\tusername : $username\n")
    append("\tcredential : $credential\n")
    append("\tcaid : $caid\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      startArray(ContextSpecificTag(TAG_URLS))
      for (item in urls.iterator()) {
        put(AnonymousTag, item)
      }
      endArray()
      if (username.isPresent) {
        val optusername = username.get()
        put(ContextSpecificTag(TAG_USERNAME), optusername)
      }
      if (credential.isPresent) {
        val optcredential = credential.get()
        put(ContextSpecificTag(TAG_CREDENTIAL), optcredential)
      }
      if (caid.isPresent) {
        val optcaid = caid.get()
        put(ContextSpecificTag(TAG_CAID), optcaid)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_URLS = 1
    private const val TAG_USERNAME = 2
    private const val TAG_CREDENTIAL = 3
    private const val TAG_CAID = 4

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): WebRTCTransportProviderClusterICEServerStruct {
      tlvReader.enterStructure(tlvTag)
      val urls =
        buildList<String> {
          tlvReader.enterArray(ContextSpecificTag(TAG_URLS))
          while (!tlvReader.isEndOfContainer()) {
            add(tlvReader.getString(AnonymousTag))
          }
          tlvReader.exitContainer()
        }
      val username =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_USERNAME))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_USERNAME)))
        } else {
          Optional.empty()
        }
      val credential =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_CREDENTIAL))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_CREDENTIAL)))
        } else {
          Optional.empty()
        }
      val caid =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_CAID))) {
          Optional.of(tlvReader.getUShort(ContextSpecificTag(TAG_CAID)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return WebRTCTransportProviderClusterICEServerStruct(urls, username, credential, caid)
    }
  }
}
