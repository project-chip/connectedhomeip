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

class AmbientSensingUnionClusterContributorStatusChangeStruct(
  val contributorIndex: UByte,
  val previousContributorStatus: UByte,
  val currentContributorStatus: UByte,
) {
  override fun toString(): String = buildString {
    append("AmbientSensingUnionClusterContributorStatusChangeStruct {\n")
    append("\tcontributorIndex : $contributorIndex\n")
    append("\tpreviousContributorStatus : $previousContributorStatus\n")
    append("\tcurrentContributorStatus : $currentContributorStatus\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_CONTRIBUTOR_INDEX), contributorIndex)
      put(ContextSpecificTag(TAG_PREVIOUS_CONTRIBUTOR_STATUS), previousContributorStatus)
      put(ContextSpecificTag(TAG_CURRENT_CONTRIBUTOR_STATUS), currentContributorStatus)
      endStructure()
    }
  }

  companion object {
    private const val TAG_CONTRIBUTOR_INDEX = 0
    private const val TAG_PREVIOUS_CONTRIBUTOR_STATUS = 1
    private const val TAG_CURRENT_CONTRIBUTOR_STATUS = 2

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): AmbientSensingUnionClusterContributorStatusChangeStruct {
      tlvReader.enterStructure(tlvTag)
      val contributorIndex = tlvReader.getUByte(ContextSpecificTag(TAG_CONTRIBUTOR_INDEX))
      val previousContributorStatus =
        tlvReader.getUByte(ContextSpecificTag(TAG_PREVIOUS_CONTRIBUTOR_STATUS))
      val currentContributorStatus =
        tlvReader.getUByte(ContextSpecificTag(TAG_CURRENT_CONTRIBUTOR_STATUS))

      tlvReader.exitContainer()

      return AmbientSensingUnionClusterContributorStatusChangeStruct(
        contributorIndex,
        previousContributorStatus,
        currentContributorStatus,
      )
    }
  }
}
