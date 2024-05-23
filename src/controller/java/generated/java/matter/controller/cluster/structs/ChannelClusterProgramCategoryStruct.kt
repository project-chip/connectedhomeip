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
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ChannelClusterProgramCategoryStruct(val category: String, val subCategory: Optional<String>) {
  override fun toString(): String = buildString {
    append("ChannelClusterProgramCategoryStruct {\n")
    append("\tcategory : $category\n")
    append("\tsubCategory : $subCategory\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_CATEGORY), category)
      if (subCategory.isPresent) {
        val optsubCategory = subCategory.get()
        put(ContextSpecificTag(TAG_SUB_CATEGORY), optsubCategory)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_CATEGORY = 0
    private const val TAG_SUB_CATEGORY = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ChannelClusterProgramCategoryStruct {
      tlvReader.enterStructure(tlvTag)
      val category = tlvReader.getString(ContextSpecificTag(TAG_CATEGORY))
      val subCategory =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_SUB_CATEGORY))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_SUB_CATEGORY)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ChannelClusterProgramCategoryStruct(category, subCategory)
    }
  }
}
