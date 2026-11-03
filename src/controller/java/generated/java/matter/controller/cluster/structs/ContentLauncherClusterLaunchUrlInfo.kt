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

class ContentLauncherClusterLaunchUrlInfo(
  val url: String,
  val data: Optional<String>?,
  val contentType: Optional<String>?,
  val contentHeaders: Optional<List<String>>?,
  val offsetMillisecs: Optional<UInt>?,
  val queueType: Optional<UByte>?,
  val nextUrl: Optional<String>?,
) {
  override fun toString(): String = buildString {
    append("ContentLauncherClusterLaunchUrlInfo {\n")
    append("\turl : $url\n")
    append("\tdata : $data\n")
    append("\tcontentType : $contentType\n")
    append("\tcontentHeaders : $contentHeaders\n")
    append("\toffsetMillisecs : $offsetMillisecs\n")
    append("\tqueueType : $queueType\n")
    append("\tnextUrl : $nextUrl\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_URL), url)
      if (data != null) {
        if (data.isPresent) {
          val optdata = data.get()
          put(ContextSpecificTag(TAG_DATA), optdata)
        }
      } else {
        putNull(ContextSpecificTag(TAG_DATA))
      }
      if (contentType != null) {
        if (contentType.isPresent) {
          val optcontentType = contentType.get()
          put(ContextSpecificTag(TAG_CONTENT_TYPE), optcontentType)
        }
      } else {
        putNull(ContextSpecificTag(TAG_CONTENT_TYPE))
      }
      if (contentHeaders != null) {
        if (contentHeaders.isPresent) {
          val optcontentHeaders = contentHeaders.get()
          startArray(ContextSpecificTag(TAG_CONTENT_HEADERS))
          for (item in optcontentHeaders.iterator()) {
            put(AnonymousTag, item)
          }
          endArray()
        }
      } else {
        putNull(ContextSpecificTag(TAG_CONTENT_HEADERS))
      }
      if (offsetMillisecs != null) {
        if (offsetMillisecs.isPresent) {
          val optoffsetMillisecs = offsetMillisecs.get()
          put(ContextSpecificTag(TAG_OFFSET_MILLISECS), optoffsetMillisecs)
        }
      } else {
        putNull(ContextSpecificTag(TAG_OFFSET_MILLISECS))
      }
      if (queueType != null) {
        if (queueType.isPresent) {
          val optqueueType = queueType.get()
          put(ContextSpecificTag(TAG_QUEUE_TYPE), optqueueType)
        }
      } else {
        putNull(ContextSpecificTag(TAG_QUEUE_TYPE))
      }
      if (nextUrl != null) {
        if (nextUrl.isPresent) {
          val optnextUrl = nextUrl.get()
          put(ContextSpecificTag(TAG_NEXT_URL), optnextUrl)
        }
      } else {
        putNull(ContextSpecificTag(TAG_NEXT_URL))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_URL = 0
    private const val TAG_DATA = 1
    private const val TAG_CONTENT_TYPE = 2
    private const val TAG_CONTENT_HEADERS = 3
    private const val TAG_OFFSET_MILLISECS = 4
    private const val TAG_QUEUE_TYPE = 5
    private const val TAG_NEXT_URL = 6

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ContentLauncherClusterLaunchUrlInfo {
      tlvReader.enterStructure(tlvTag)
      val url = tlvReader.getString(ContextSpecificTag(TAG_URL))
      val data =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_DATA))) {
            Optional.of(tlvReader.getString(ContextSpecificTag(TAG_DATA)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_DATA))
          null
        }
      val contentType =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_CONTENT_TYPE))) {
            Optional.of(tlvReader.getString(ContextSpecificTag(TAG_CONTENT_TYPE)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_CONTENT_TYPE))
          null
        }
      val contentHeaders =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_CONTENT_HEADERS))) {
            Optional.of(
              buildList<String> {
                tlvReader.enterArray(ContextSpecificTag(TAG_CONTENT_HEADERS))
                while (!tlvReader.isEndOfContainer()) {
                  add(tlvReader.getString(AnonymousTag))
                }
                tlvReader.exitContainer()
              }
            )
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_CONTENT_HEADERS))
          null
        }
      val offsetMillisecs =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_OFFSET_MILLISECS))) {
            Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_OFFSET_MILLISECS)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_OFFSET_MILLISECS))
          null
        }
      val queueType =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_QUEUE_TYPE))) {
            Optional.of(tlvReader.getUByte(ContextSpecificTag(TAG_QUEUE_TYPE)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_QUEUE_TYPE))
          null
        }
      val nextUrl =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_NEXT_URL))) {
            Optional.of(tlvReader.getString(ContextSpecificTag(TAG_NEXT_URL)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_NEXT_URL))
          null
        }

      tlvReader.exitContainer()

      return ContentLauncherClusterLaunchUrlInfo(
        url,
        data,
        contentType,
        contentHeaders,
        offsetMillisecs,
        queueType,
        nextUrl,
      )
    }
  }
}
