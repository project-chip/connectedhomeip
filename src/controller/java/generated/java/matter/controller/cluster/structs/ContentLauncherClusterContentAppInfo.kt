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

import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ContentLauncherClusterContentAppInfo(
  val contentAppVendorID: UShort,
  val contentAppProductID: UShort,
  val data: String,
) {
  override fun toString(): String = buildString {
    append("ContentLauncherClusterContentAppInfo {\n")
    append("\tcontentAppVendorID : $contentAppVendorID\n")
    append("\tcontentAppProductID : $contentAppProductID\n")
    append("\tdata : $data\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_CONTENT_APP_VENDOR_ID), contentAppVendorID)
      put(ContextSpecificTag(TAG_CONTENT_APP_PRODUCT_ID), contentAppProductID)
      put(ContextSpecificTag(TAG_DATA), data)
      endStructure()
    }
  }

  companion object {
    private const val TAG_CONTENT_APP_VENDOR_ID = 0
    private const val TAG_CONTENT_APP_PRODUCT_ID = 1
    private const val TAG_DATA = 2

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ContentLauncherClusterContentAppInfo {
      tlvReader.enterStructure(tlvTag)
      val contentAppVendorID = tlvReader.getUShort(ContextSpecificTag(TAG_CONTENT_APP_VENDOR_ID))
      val contentAppProductID = tlvReader.getUShort(ContextSpecificTag(TAG_CONTENT_APP_PRODUCT_ID))
      val data = tlvReader.getString(ContextSpecificTag(TAG_DATA))

      tlvReader.exitContainer()

      return ContentLauncherClusterContentAppInfo(contentAppVendorID, contentAppProductID, data)
    }
  }
}
