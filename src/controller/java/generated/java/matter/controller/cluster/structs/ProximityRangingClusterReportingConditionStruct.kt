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

class ProximityRangingClusterReportingConditionStruct(
  val minDistanceCondition: Optional<UShort>,
  val maxDistanceCondition: Optional<UShort>,
  val errorMarginCondition: Optional<UShort>,
) {
  override fun toString(): String = buildString {
    append("ProximityRangingClusterReportingConditionStruct {\n")
    append("\tminDistanceCondition : $minDistanceCondition\n")
    append("\tmaxDistanceCondition : $maxDistanceCondition\n")
    append("\terrorMarginCondition : $errorMarginCondition\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (minDistanceCondition.isPresent) {
        val optminDistanceCondition = minDistanceCondition.get()
        put(ContextSpecificTag(TAG_MIN_DISTANCE_CONDITION), optminDistanceCondition)
      }
      if (maxDistanceCondition.isPresent) {
        val optmaxDistanceCondition = maxDistanceCondition.get()
        put(ContextSpecificTag(TAG_MAX_DISTANCE_CONDITION), optmaxDistanceCondition)
      }
      if (errorMarginCondition.isPresent) {
        val opterrorMarginCondition = errorMarginCondition.get()
        put(ContextSpecificTag(TAG_ERROR_MARGIN_CONDITION), opterrorMarginCondition)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_MIN_DISTANCE_CONDITION = 0
    private const val TAG_MAX_DISTANCE_CONDITION = 1
    private const val TAG_ERROR_MARGIN_CONDITION = 2

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): ProximityRangingClusterReportingConditionStruct {
      tlvReader.enterStructure(tlvTag)
      val minDistanceCondition =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_MIN_DISTANCE_CONDITION))) {
          Optional.of(tlvReader.getUShort(ContextSpecificTag(TAG_MIN_DISTANCE_CONDITION)))
        } else {
          Optional.empty()
        }
      val maxDistanceCondition =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_MAX_DISTANCE_CONDITION))) {
          Optional.of(tlvReader.getUShort(ContextSpecificTag(TAG_MAX_DISTANCE_CONDITION)))
        } else {
          Optional.empty()
        }
      val errorMarginCondition =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_ERROR_MARGIN_CONDITION))) {
          Optional.of(tlvReader.getUShort(ContextSpecificTag(TAG_ERROR_MARGIN_CONDITION)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ProximityRangingClusterReportingConditionStruct(
        minDistanceCondition,
        maxDistanceCondition,
        errorMarginCondition,
      )
    }
  }
}
