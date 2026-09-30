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
package chip.devicecontroller.cluster.eventstructs

import chip.devicecontroller.cluster.*
import java.util.Optional
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class AvAnalysisClusterPerceivedContextEvent(
  val sessionID: UInt,
  val sourceNodeId: Optional<ULong>,
  val sourceStartTimestamp: Optional<ULong>,
  val newIdentifiedContexts:
    Optional<List<chip.devicecontroller.cluster.structs.AvAnalysisClusterTrackedContext>>,
  val currentIdentifiedContexts:
    Optional<List<chip.devicecontroller.cluster.structs.AvAnalysisClusterTrackedContext>>,
  val expiredContexts:
    Optional<List<chip.devicecontroller.cluster.structs.AvAnalysisClusterTrackedContext>>,
) {
  override fun toString(): String = buildString {
    append("AvAnalysisClusterPerceivedContextEvent {\n")
    append("\tsessionID : $sessionID\n")
    append("\tsourceNodeId : $sourceNodeId\n")
    append("\tsourceStartTimestamp : $sourceStartTimestamp\n")
    append("\tnewIdentifiedContexts : $newIdentifiedContexts\n")
    append("\tcurrentIdentifiedContexts : $currentIdentifiedContexts\n")
    append("\texpiredContexts : $expiredContexts\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_SESSION_ID), sessionID)
      if (sourceNodeId.isPresent) {
        val optsourceNodeId = sourceNodeId.get()
        put(ContextSpecificTag(TAG_SOURCE_NODE_ID), optsourceNodeId)
      }
      if (sourceStartTimestamp.isPresent) {
        val optsourceStartTimestamp = sourceStartTimestamp.get()
        put(ContextSpecificTag(TAG_SOURCE_START_TIMESTAMP), optsourceStartTimestamp)
      }
      if (newIdentifiedContexts.isPresent) {
        val optnewIdentifiedContexts = newIdentifiedContexts.get()
        startArray(ContextSpecificTag(TAG_NEW_IDENTIFIED_CONTEXTS))
        for (item in optnewIdentifiedContexts.iterator()) {
          item.toTlv(AnonymousTag, this)
        }
        endArray()
      }
      if (currentIdentifiedContexts.isPresent) {
        val optcurrentIdentifiedContexts = currentIdentifiedContexts.get()
        startArray(ContextSpecificTag(TAG_CURRENT_IDENTIFIED_CONTEXTS))
        for (item in optcurrentIdentifiedContexts.iterator()) {
          item.toTlv(AnonymousTag, this)
        }
        endArray()
      }
      if (expiredContexts.isPresent) {
        val optexpiredContexts = expiredContexts.get()
        startArray(ContextSpecificTag(TAG_EXPIRED_CONTEXTS))
        for (item in optexpiredContexts.iterator()) {
          item.toTlv(AnonymousTag, this)
        }
        endArray()
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_SESSION_ID = 0
    private const val TAG_SOURCE_NODE_ID = 1
    private const val TAG_SOURCE_START_TIMESTAMP = 2
    private const val TAG_NEW_IDENTIFIED_CONTEXTS = 3
    private const val TAG_CURRENT_IDENTIFIED_CONTEXTS = 4
    private const val TAG_EXPIRED_CONTEXTS = 5

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): AvAnalysisClusterPerceivedContextEvent {
      tlvReader.enterStructure(tlvTag)
      val sessionID = tlvReader.getUInt(ContextSpecificTag(TAG_SESSION_ID))
      val sourceNodeId =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_SOURCE_NODE_ID))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_SOURCE_NODE_ID)))
        } else {
          Optional.empty()
        }
      val sourceStartTimestamp =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_SOURCE_START_TIMESTAMP))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_SOURCE_START_TIMESTAMP)))
        } else {
          Optional.empty()
        }
      val newIdentifiedContexts =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_NEW_IDENTIFIED_CONTEXTS))) {
          Optional.of(
            buildList<chip.devicecontroller.cluster.structs.AvAnalysisClusterTrackedContext> {
              tlvReader.enterArray(ContextSpecificTag(TAG_NEW_IDENTIFIED_CONTEXTS))
              while (!tlvReader.isEndOfContainer()) {
                this.add(
                  chip.devicecontroller.cluster.structs.AvAnalysisClusterTrackedContext.fromTlv(
                    AnonymousTag,
                    tlvReader,
                  )
                )
              }
              tlvReader.exitContainer()
            }
          )
        } else {
          Optional.empty()
        }
      val currentIdentifiedContexts =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_CURRENT_IDENTIFIED_CONTEXTS))) {
          Optional.of(
            buildList<chip.devicecontroller.cluster.structs.AvAnalysisClusterTrackedContext> {
              tlvReader.enterArray(ContextSpecificTag(TAG_CURRENT_IDENTIFIED_CONTEXTS))
              while (!tlvReader.isEndOfContainer()) {
                this.add(
                  chip.devicecontroller.cluster.structs.AvAnalysisClusterTrackedContext.fromTlv(
                    AnonymousTag,
                    tlvReader,
                  )
                )
              }
              tlvReader.exitContainer()
            }
          )
        } else {
          Optional.empty()
        }
      val expiredContexts =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_EXPIRED_CONTEXTS))) {
          Optional.of(
            buildList<chip.devicecontroller.cluster.structs.AvAnalysisClusterTrackedContext> {
              tlvReader.enterArray(ContextSpecificTag(TAG_EXPIRED_CONTEXTS))
              while (!tlvReader.isEndOfContainer()) {
                this.add(
                  chip.devicecontroller.cluster.structs.AvAnalysisClusterTrackedContext.fromTlv(
                    AnonymousTag,
                    tlvReader,
                  )
                )
              }
              tlvReader.exitContainer()
            }
          )
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return AvAnalysisClusterPerceivedContextEvent(
        sessionID,
        sourceNodeId,
        sourceStartTimestamp,
        newIdentifiedContexts,
        currentIdentifiedContexts,
        expiredContexts,
      )
    }
  }
}
