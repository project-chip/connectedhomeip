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

class BasicInformationClusterCapabilityMinimaStruct(
  val caseSessionsPerFabric: UShort,
  val subscriptionsPerFabric: UShort
) {
  override fun toString(): String = buildString {
    append("BasicInformationClusterCapabilityMinimaStruct {\n")
    append("\tcaseSessionsPerFabric : $caseSessionsPerFabric\n")
    append("\tsubscriptionsPerFabric : $subscriptionsPerFabric\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_CASE_SESSIONS_PER_FABRIC), caseSessionsPerFabric)
      put(ContextSpecificTag(TAG_SUBSCRIPTIONS_PER_FABRIC), subscriptionsPerFabric)
      endStructure()
    }
  }

  companion object {
    private const val TAG_CASE_SESSIONS_PER_FABRIC = 0
    private const val TAG_SUBSCRIPTIONS_PER_FABRIC = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): BasicInformationClusterCapabilityMinimaStruct {
      tlvReader.enterStructure(tlvTag)
      val caseSessionsPerFabric =
        tlvReader.getUShort(ContextSpecificTag(TAG_CASE_SESSIONS_PER_FABRIC))
      val subscriptionsPerFabric =
        tlvReader.getUShort(ContextSpecificTag(TAG_SUBSCRIPTIONS_PER_FABRIC))

      tlvReader.exitContainer()

      return BasicInformationClusterCapabilityMinimaStruct(
        caseSessionsPerFabric,
        subscriptionsPerFabric
      )
    }
  }
}
