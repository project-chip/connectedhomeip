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

import matter.controller.cluster.*
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class AmbientSensingUnionClusterUnionContributorListChangeEvent(
  val unionContributorList:
    List<matter.controller.cluster.structs.AmbientSensingUnionClusterUnionMemberStruct>
) {
  override fun toString(): String = buildString {
    append("AmbientSensingUnionClusterUnionContributorListChangeEvent {\n")
    append("\tunionContributorList : $unionContributorList\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      startArray(ContextSpecificTag(TAG_UNION_CONTRIBUTOR_LIST))
      for (item in unionContributorList.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endArray()
      endStructure()
    }
  }

  companion object {
    private const val TAG_UNION_CONTRIBUTOR_LIST = 0

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): AmbientSensingUnionClusterUnionContributorListChangeEvent {
      tlvReader.enterStructure(tlvTag)
      val unionContributorList =
        buildList<matter.controller.cluster.structs.AmbientSensingUnionClusterUnionMemberStruct> {
          tlvReader.enterArray(ContextSpecificTag(TAG_UNION_CONTRIBUTOR_LIST))
          while (!tlvReader.isEndOfContainer()) {
            this.add(
              matter.controller.cluster.structs.AmbientSensingUnionClusterUnionMemberStruct.fromTlv(
                AnonymousTag,
                tlvReader,
              )
            )
          }
          tlvReader.exitContainer()
        }

      tlvReader.exitContainer()

      return AmbientSensingUnionClusterUnionContributorListChangeEvent(unionContributorList)
    }
  }
}
