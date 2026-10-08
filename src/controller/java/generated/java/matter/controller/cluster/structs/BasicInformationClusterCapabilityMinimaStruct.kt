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

class BasicInformationClusterCapabilityMinimaStruct(
  val caseSessionsPerFabric: UShort,
  val subscriptionsPerFabric: UShort,
  val simultaneousInvocationsSupported: Optional<UShort>,
  val simultaneousWritesSupported: Optional<UShort>,
  val readPathsSupported: Optional<UShort>,
  val subscribePathsSupported: Optional<UShort>,
) {
  override fun toString(): String = buildString {
    append("BasicInformationClusterCapabilityMinimaStruct {\n")
    append("\tcaseSessionsPerFabric : $caseSessionsPerFabric\n")
    append("\tsubscriptionsPerFabric : $subscriptionsPerFabric\n")
    append("\tsimultaneousInvocationsSupported : $simultaneousInvocationsSupported\n")
    append("\tsimultaneousWritesSupported : $simultaneousWritesSupported\n")
    append("\treadPathsSupported : $readPathsSupported\n")
    append("\tsubscribePathsSupported : $subscribePathsSupported\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_CASE_SESSIONS_PER_FABRIC), caseSessionsPerFabric)
      put(ContextSpecificTag(TAG_SUBSCRIPTIONS_PER_FABRIC), subscriptionsPerFabric)
      if (simultaneousInvocationsSupported.isPresent) {
        val optsimultaneousInvocationsSupported = simultaneousInvocationsSupported.get()
        put(
          ContextSpecificTag(TAG_SIMULTANEOUS_INVOCATIONS_SUPPORTED),
          optsimultaneousInvocationsSupported,
        )
      }
      if (simultaneousWritesSupported.isPresent) {
        val optsimultaneousWritesSupported = simultaneousWritesSupported.get()
        put(ContextSpecificTag(TAG_SIMULTANEOUS_WRITES_SUPPORTED), optsimultaneousWritesSupported)
      }
      if (readPathsSupported.isPresent) {
        val optreadPathsSupported = readPathsSupported.get()
        put(ContextSpecificTag(TAG_READ_PATHS_SUPPORTED), optreadPathsSupported)
      }
      if (subscribePathsSupported.isPresent) {
        val optsubscribePathsSupported = subscribePathsSupported.get()
        put(ContextSpecificTag(TAG_SUBSCRIBE_PATHS_SUPPORTED), optsubscribePathsSupported)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_CASE_SESSIONS_PER_FABRIC = 0
    private const val TAG_SUBSCRIPTIONS_PER_FABRIC = 1
    private const val TAG_SIMULTANEOUS_INVOCATIONS_SUPPORTED = 2
    private const val TAG_SIMULTANEOUS_WRITES_SUPPORTED = 3
    private const val TAG_READ_PATHS_SUPPORTED = 4
    private const val TAG_SUBSCRIBE_PATHS_SUPPORTED = 5

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): BasicInformationClusterCapabilityMinimaStruct {
      tlvReader.enterStructure(tlvTag)
      val caseSessionsPerFabric =
        tlvReader.getUShort(ContextSpecificTag(TAG_CASE_SESSIONS_PER_FABRIC))
      val subscriptionsPerFabric =
        tlvReader.getUShort(ContextSpecificTag(TAG_SUBSCRIPTIONS_PER_FABRIC))
      val simultaneousInvocationsSupported =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_SIMULTANEOUS_INVOCATIONS_SUPPORTED))) {
          Optional.of(
            tlvReader.getUShort(ContextSpecificTag(TAG_SIMULTANEOUS_INVOCATIONS_SUPPORTED))
          )
        } else {
          Optional.empty()
        }
      val simultaneousWritesSupported =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_SIMULTANEOUS_WRITES_SUPPORTED))) {
          Optional.of(tlvReader.getUShort(ContextSpecificTag(TAG_SIMULTANEOUS_WRITES_SUPPORTED)))
        } else {
          Optional.empty()
        }
      val readPathsSupported =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_READ_PATHS_SUPPORTED))) {
          Optional.of(tlvReader.getUShort(ContextSpecificTag(TAG_READ_PATHS_SUPPORTED)))
        } else {
          Optional.empty()
        }
      val subscribePathsSupported =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_SUBSCRIBE_PATHS_SUPPORTED))) {
          Optional.of(tlvReader.getUShort(ContextSpecificTag(TAG_SUBSCRIBE_PATHS_SUPPORTED)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return BasicInformationClusterCapabilityMinimaStruct(
        caseSessionsPerFabric,
        subscriptionsPerFabric,
        simultaneousInvocationsSupported,
        simultaneousWritesSupported,
        readPathsSupported,
        subscribePathsSupported,
      )
    }
  }
}
