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
  val contributorNodeID: ULong?,
  val contributorEndpointID: UShort?,
  val contributorName: String?,
  val previousContributorStatus: UByte,
  val currentContributorStatus: UByte,
  val fabricIndex: UByte,
) {
  override fun toString(): String = buildString {
    append("AmbientSensingUnionClusterContributorStatusChangeStruct {\n")
    append("\tcontributorNodeID : $contributorNodeID\n")
    append("\tcontributorEndpointID : $contributorEndpointID\n")
    append("\tcontributorName : $contributorName\n")
    append("\tpreviousContributorStatus : $previousContributorStatus\n")
    append("\tcurrentContributorStatus : $currentContributorStatus\n")
    append("\tfabricIndex : $fabricIndex\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (contributorNodeID != null) {
        put(ContextSpecificTag(TAG_CONTRIBUTOR_NODE_ID), contributorNodeID)
      } else {
        putNull(ContextSpecificTag(TAG_CONTRIBUTOR_NODE_ID))
      }
      if (contributorEndpointID != null) {
        put(ContextSpecificTag(TAG_CONTRIBUTOR_ENDPOINT_ID), contributorEndpointID)
      } else {
        putNull(ContextSpecificTag(TAG_CONTRIBUTOR_ENDPOINT_ID))
      }
      if (contributorName != null) {
        put(ContextSpecificTag(TAG_CONTRIBUTOR_NAME), contributorName)
      } else {
        putNull(ContextSpecificTag(TAG_CONTRIBUTOR_NAME))
      }
      put(ContextSpecificTag(TAG_PREVIOUS_CONTRIBUTOR_STATUS), previousContributorStatus)
      put(ContextSpecificTag(TAG_CURRENT_CONTRIBUTOR_STATUS), currentContributorStatus)
      put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
      endStructure()
    }
  }

  companion object {
    private const val TAG_CONTRIBUTOR_NODE_ID = 0
    private const val TAG_CONTRIBUTOR_ENDPOINT_ID = 1
    private const val TAG_CONTRIBUTOR_NAME = 2
    private const val TAG_PREVIOUS_CONTRIBUTOR_STATUS = 3
    private const val TAG_CURRENT_CONTRIBUTOR_STATUS = 4
    private const val TAG_FABRIC_INDEX = 254

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): AmbientSensingUnionClusterContributorStatusChangeStruct {
      tlvReader.enterStructure(tlvTag)
      val contributorNodeID =
        if (!tlvReader.isNull()) {
          tlvReader.getULong(ContextSpecificTag(TAG_CONTRIBUTOR_NODE_ID))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_CONTRIBUTOR_NODE_ID))
          null
        }
      val contributorEndpointID =
        if (!tlvReader.isNull()) {
          tlvReader.getUShort(ContextSpecificTag(TAG_CONTRIBUTOR_ENDPOINT_ID))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_CONTRIBUTOR_ENDPOINT_ID))
          null
        }
      val contributorName =
        if (!tlvReader.isNull()) {
          tlvReader.getString(ContextSpecificTag(TAG_CONTRIBUTOR_NAME))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_CONTRIBUTOR_NAME))
          null
        }
      val previousContributorStatus =
        tlvReader.getUByte(ContextSpecificTag(TAG_PREVIOUS_CONTRIBUTOR_STATUS))
      val currentContributorStatus =
        tlvReader.getUByte(ContextSpecificTag(TAG_CURRENT_CONTRIBUTOR_STATUS))
      val fabricIndex = tlvReader.getUByte(ContextSpecificTag(TAG_FABRIC_INDEX))

      tlvReader.exitContainer()

      return AmbientSensingUnionClusterContributorStatusChangeStruct(
        contributorNodeID,
        contributorEndpointID,
        contributorName,
        previousContributorStatus,
        currentContributorStatus,
        fabricIndex,
      )
    }
  }
}
