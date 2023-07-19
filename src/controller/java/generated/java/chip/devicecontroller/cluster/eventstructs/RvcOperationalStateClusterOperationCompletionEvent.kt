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
import chip.tlv.AnonymousTag
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvParsingException
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

import java.util.Optional

class RvcOperationalStateClusterOperationCompletionEvent (
    val completionErrorCode: Int,
    val totalOperationalTime: Optional<Long>?,
    val pausedTime: Optional<Long>?) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("RvcOperationalStateClusterOperationCompletionEvent {\n")
    builder.append("\tcompletionErrorCode : $completionErrorCode\n")
    builder.append("\ttotalOperationalTime : $totalOperationalTime\n")
    builder.append("\tpausedTime : $pausedTime\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(0), completionErrorCode)
    if (totalOperationalTime == null) { tlvWriter.putNull(ContextSpecificTag(1)) }
    else {
      if (totalOperationalTime.isPresent) {
      val opttotalOperationalTime = totalOperationalTime.get()
      tlvWriter.put(ContextSpecificTag(1), opttotalOperationalTime)
    }
    }
    if (pausedTime == null) { tlvWriter.putNull(ContextSpecificTag(2)) }
    else {
      if (pausedTime.isPresent) {
      val optpausedTime = pausedTime.get()
      tlvWriter.put(ContextSpecificTag(2), optpausedTime)
    }
    }
    tlvWriter.endStructure()
  }

  companion object {
    fun fromTlv(tag: Tag, tlvReader: TlvReader) : RvcOperationalStateClusterOperationCompletionEvent {
      tlvReader.enterStructure(tag)
      val completionErrorCode: Int = tlvReader.getInt(ContextSpecificTag(0))
      val totalOperationalTime: Optional<Long>? = try {
      try {
      Optional.of(tlvReader.getLong(ContextSpecificTag(1)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(1))
      null
    }
      val pausedTime: Optional<Long>? = try {
      try {
      Optional.of(tlvReader.getLong(ContextSpecificTag(2)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(2))
      null
    }
      
      tlvReader.exitContainer()

      return RvcOperationalStateClusterOperationCompletionEvent(completionErrorCode, totalOperationalTime, pausedTime)
    }
  }
}
