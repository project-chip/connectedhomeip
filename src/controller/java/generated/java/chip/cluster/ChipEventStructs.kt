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
package chip.devicecontroller.cluster

import chip.tlv.Tag
import chip.tlv.AnonymousTag
import chip.tlv.ContextSpecificTag
import chip.tlv.TlvParsingException
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

import java.util.Optional

class ChipEventStructs {
    class AccessControlClusterAccessControlEntryChangedEvent (
        val adminNodeID: Long?,
        val adminPasscodeID: Int?,
        val changeType: Int,
        val latestValue: ChipStructs.AccessControlClusterAccessControlEntryStruct?,
        val fabricIndex: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("AccessControlClusterAccessControlEntryChangedEvent {\n")
        builder.append("\tadminNodeID : $adminNodeID\n")
        builder.append("\tadminPasscodeID : $adminPasscodeID\n")
        builder.append("\tchangeType : $changeType\n")
        builder.append("\tlatestValue : $latestValue\n")
        builder.append("\tfabricIndex : $fabricIndex\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        if (adminNodeID == null) { tlvWriter.putNull(ContextSpecificTag(1)) }
    else {
      tlvWriter.put(ContextSpecificTag(1), adminNodeID)
    }
        if (adminPasscodeID == null) { tlvWriter.putNull(ContextSpecificTag(2)) }
    else {
      tlvWriter.put(ContextSpecificTag(2), adminPasscodeID)
    }
        tlvWriter.put(ContextSpecificTag(3), changeType)
        if (latestValue == null) { tlvWriter.putNull(ContextSpecificTag(4)) }
    else {
      latestValue.toTlv(ContextSpecificTag(4), tlvWriter)
    }
        tlvWriter.put(ContextSpecificTag(254), fabricIndex)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : AccessControlClusterAccessControlEntryChangedEvent {
          tlvReader.enterStructure(tag)
          val adminNodeID: Long? = try {
      tlvReader.getLong(ContextSpecificTag(1))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(1))
      null
    }
          val adminPasscodeID: Int? = try {
      tlvReader.getInt(ContextSpecificTag(2))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(2))
      null
    }
          val changeType: Int = tlvReader.getInt(ContextSpecificTag(3))
          val latestValue: ChipStructs.AccessControlClusterAccessControlEntryStruct? = try {
      ChipStructs.AccessControlClusterAccessControlEntryStruct.fromTlv(ContextSpecificTag(4), tlvReader)
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(4))
      null
    }
          val fabricIndex: Int = tlvReader.getInt(ContextSpecificTag(254))
          
          tlvReader.exitContainer()

          return AccessControlClusterAccessControlEntryChangedEvent(adminNodeID, adminPasscodeID, changeType, latestValue, fabricIndex)
        }
      }
    }
    
    class AccessControlClusterAccessControlExtensionChangedEvent (
        val adminNodeID: Long?,
        val adminPasscodeID: Int?,
        val changeType: Int,
        val latestValue: ChipStructs.AccessControlClusterAccessControlExtensionStruct?,
        val fabricIndex: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("AccessControlClusterAccessControlExtensionChangedEvent {\n")
        builder.append("\tadminNodeID : $adminNodeID\n")
        builder.append("\tadminPasscodeID : $adminPasscodeID\n")
        builder.append("\tchangeType : $changeType\n")
        builder.append("\tlatestValue : $latestValue\n")
        builder.append("\tfabricIndex : $fabricIndex\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        if (adminNodeID == null) { tlvWriter.putNull(ContextSpecificTag(1)) }
    else {
      tlvWriter.put(ContextSpecificTag(1), adminNodeID)
    }
        if (adminPasscodeID == null) { tlvWriter.putNull(ContextSpecificTag(2)) }
    else {
      tlvWriter.put(ContextSpecificTag(2), adminPasscodeID)
    }
        tlvWriter.put(ContextSpecificTag(3), changeType)
        if (latestValue == null) { tlvWriter.putNull(ContextSpecificTag(4)) }
    else {
      latestValue.toTlv(ContextSpecificTag(4), tlvWriter)
    }
        tlvWriter.put(ContextSpecificTag(254), fabricIndex)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : AccessControlClusterAccessControlExtensionChangedEvent {
          tlvReader.enterStructure(tag)
          val adminNodeID: Long? = try {
      tlvReader.getLong(ContextSpecificTag(1))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(1))
      null
    }
          val adminPasscodeID: Int? = try {
      tlvReader.getInt(ContextSpecificTag(2))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(2))
      null
    }
          val changeType: Int = tlvReader.getInt(ContextSpecificTag(3))
          val latestValue: ChipStructs.AccessControlClusterAccessControlExtensionStruct? = try {
      ChipStructs.AccessControlClusterAccessControlExtensionStruct.fromTlv(ContextSpecificTag(4), tlvReader)
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(4))
      null
    }
          val fabricIndex: Int = tlvReader.getInt(ContextSpecificTag(254))
          
          tlvReader.exitContainer()

          return AccessControlClusterAccessControlExtensionChangedEvent(adminNodeID, adminPasscodeID, changeType, latestValue, fabricIndex)
        }
      }
    }
    
    class ActionsClusterStateChangedEvent (
        val actionID: Int,
        val invokeID: Long,
        val newState: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("ActionsClusterStateChangedEvent {\n")
        builder.append("\tactionID : $actionID\n")
        builder.append("\tinvokeID : $invokeID\n")
        builder.append("\tnewState : $newState\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), actionID)
        tlvWriter.put(ContextSpecificTag(1), invokeID)
        tlvWriter.put(ContextSpecificTag(2), newState)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : ActionsClusterStateChangedEvent {
          tlvReader.enterStructure(tag)
          val actionID: Int = tlvReader.getInt(ContextSpecificTag(0))
          val invokeID: Long = tlvReader.getLong(ContextSpecificTag(1))
          val newState: Int = tlvReader.getInt(ContextSpecificTag(2))
          
          tlvReader.exitContainer()

          return ActionsClusterStateChangedEvent(actionID, invokeID, newState)
        }
      }
    }
    
    class ActionsClusterActionFailedEvent (
        val actionID: Int,
        val invokeID: Long,
        val newState: Int,
        val error: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("ActionsClusterActionFailedEvent {\n")
        builder.append("\tactionID : $actionID\n")
        builder.append("\tinvokeID : $invokeID\n")
        builder.append("\tnewState : $newState\n")
        builder.append("\terror : $error\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), actionID)
        tlvWriter.put(ContextSpecificTag(1), invokeID)
        tlvWriter.put(ContextSpecificTag(2), newState)
        tlvWriter.put(ContextSpecificTag(3), error)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : ActionsClusterActionFailedEvent {
          tlvReader.enterStructure(tag)
          val actionID: Int = tlvReader.getInt(ContextSpecificTag(0))
          val invokeID: Long = tlvReader.getLong(ContextSpecificTag(1))
          val newState: Int = tlvReader.getInt(ContextSpecificTag(2))
          val error: Int = tlvReader.getInt(ContextSpecificTag(3))
          
          tlvReader.exitContainer()

          return ActionsClusterActionFailedEvent(actionID, invokeID, newState, error)
        }
      }
    }
    
    class BasicInformationClusterStartUpEvent (
        val softwareVersion: Long) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("BasicInformationClusterStartUpEvent {\n")
        builder.append("\tsoftwareVersion : $softwareVersion\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), softwareVersion)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : BasicInformationClusterStartUpEvent {
          tlvReader.enterStructure(tag)
          val softwareVersion: Long = tlvReader.getLong(ContextSpecificTag(0))
          
          tlvReader.exitContainer()

          return BasicInformationClusterStartUpEvent(softwareVersion)
        }
      }
    }
    
    class BasicInformationClusterShutDownEvent () {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("BasicInformationClusterShutDownEvent {\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : BasicInformationClusterShutDownEvent {
          tlvReader.enterStructure(tag)
          
          tlvReader.exitContainer()

          return BasicInformationClusterShutDownEvent()
        }
      }
    }
    
    class BasicInformationClusterLeaveEvent (
        val fabricIndex: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("BasicInformationClusterLeaveEvent {\n")
        builder.append("\tfabricIndex : $fabricIndex\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), fabricIndex)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : BasicInformationClusterLeaveEvent {
          tlvReader.enterStructure(tag)
          val fabricIndex: Int = tlvReader.getInt(ContextSpecificTag(0))
          
          tlvReader.exitContainer()

          return BasicInformationClusterLeaveEvent(fabricIndex)
        }
      }
    }
    
    class BasicInformationClusterReachableChangedEvent (
        val reachableNewValue: Boolean) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("BasicInformationClusterReachableChangedEvent {\n")
        builder.append("\treachableNewValue : $reachableNewValue\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), reachableNewValue)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : BasicInformationClusterReachableChangedEvent {
          tlvReader.enterStructure(tag)
          val reachableNewValue: Boolean = tlvReader.getBoolean(ContextSpecificTag(0))
          
          tlvReader.exitContainer()

          return BasicInformationClusterReachableChangedEvent(reachableNewValue)
        }
      }
    }
    
    class OtaSoftwareUpdateRequestorClusterStateTransitionEvent (
        val previousState: Int,
        val newState: Int,
        val reason: Int,
        val targetSoftwareVersion: Long?) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("OtaSoftwareUpdateRequestorClusterStateTransitionEvent {\n")
        builder.append("\tpreviousState : $previousState\n")
        builder.append("\tnewState : $newState\n")
        builder.append("\treason : $reason\n")
        builder.append("\ttargetSoftwareVersion : $targetSoftwareVersion\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), previousState)
        tlvWriter.put(ContextSpecificTag(1), newState)
        tlvWriter.put(ContextSpecificTag(2), reason)
        if (targetSoftwareVersion == null) { tlvWriter.putNull(ContextSpecificTag(3)) }
    else {
      tlvWriter.put(ContextSpecificTag(3), targetSoftwareVersion)
    }
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : OtaSoftwareUpdateRequestorClusterStateTransitionEvent {
          tlvReader.enterStructure(tag)
          val previousState: Int = tlvReader.getInt(ContextSpecificTag(0))
          val newState: Int = tlvReader.getInt(ContextSpecificTag(1))
          val reason: Int = tlvReader.getInt(ContextSpecificTag(2))
          val targetSoftwareVersion: Long? = try {
      tlvReader.getLong(ContextSpecificTag(3))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(3))
      null
    }
          
          tlvReader.exitContainer()

          return OtaSoftwareUpdateRequestorClusterStateTransitionEvent(previousState, newState, reason, targetSoftwareVersion)
        }
      }
    }
    
    class OtaSoftwareUpdateRequestorClusterVersionAppliedEvent (
        val softwareVersion: Long,
        val productID: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("OtaSoftwareUpdateRequestorClusterVersionAppliedEvent {\n")
        builder.append("\tsoftwareVersion : $softwareVersion\n")
        builder.append("\tproductID : $productID\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), softwareVersion)
        tlvWriter.put(ContextSpecificTag(1), productID)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : OtaSoftwareUpdateRequestorClusterVersionAppliedEvent {
          tlvReader.enterStructure(tag)
          val softwareVersion: Long = tlvReader.getLong(ContextSpecificTag(0))
          val productID: Int = tlvReader.getInt(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return OtaSoftwareUpdateRequestorClusterVersionAppliedEvent(softwareVersion, productID)
        }
      }
    }
    
    class OtaSoftwareUpdateRequestorClusterDownloadErrorEvent (
        val softwareVersion: Long,
        val bytesDownloaded: Long,
        val progressPercent: Int?,
        val platformCode: Long?) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("OtaSoftwareUpdateRequestorClusterDownloadErrorEvent {\n")
        builder.append("\tsoftwareVersion : $softwareVersion\n")
        builder.append("\tbytesDownloaded : $bytesDownloaded\n")
        builder.append("\tprogressPercent : $progressPercent\n")
        builder.append("\tplatformCode : $platformCode\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), softwareVersion)
        tlvWriter.put(ContextSpecificTag(1), bytesDownloaded)
        if (progressPercent == null) { tlvWriter.putNull(ContextSpecificTag(2)) }
    else {
      tlvWriter.put(ContextSpecificTag(2), progressPercent)
    }
        if (platformCode == null) { tlvWriter.putNull(ContextSpecificTag(3)) }
    else {
      tlvWriter.put(ContextSpecificTag(3), platformCode)
    }
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : OtaSoftwareUpdateRequestorClusterDownloadErrorEvent {
          tlvReader.enterStructure(tag)
          val softwareVersion: Long = tlvReader.getLong(ContextSpecificTag(0))
          val bytesDownloaded: Long = tlvReader.getLong(ContextSpecificTag(1))
          val progressPercent: Int? = try {
      tlvReader.getInt(ContextSpecificTag(2))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(2))
      null
    }
          val platformCode: Long? = try {
      tlvReader.getLong(ContextSpecificTag(3))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(3))
      null
    }
          
          tlvReader.exitContainer()

          return OtaSoftwareUpdateRequestorClusterDownloadErrorEvent(softwareVersion, bytesDownloaded, progressPercent, platformCode)
        }
      }
    }
    
    class PowerSourceClusterWiredFaultChangeEvent (
        val current: List<Int>,
        val previous: List<Int>) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("PowerSourceClusterWiredFaultChangeEvent {\n")
        builder.append("\tcurrent : $current\n")
        builder.append("\tprevious : $previous\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.startList(ContextSpecificTag(0))
      val iter_current = current.iterator()
      while(iter_current.hasNext()) {
        val next = iter_current.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
        tlvWriter.startList(ContextSpecificTag(1))
      val iter_previous = previous.iterator()
      while(iter_previous.hasNext()) {
        val next = iter_previous.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : PowerSourceClusterWiredFaultChangeEvent {
          tlvReader.enterStructure(tag)
          val current: List<Int> = mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(0))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          val previous: List<Int> = mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(1))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          
          tlvReader.exitContainer()

          return PowerSourceClusterWiredFaultChangeEvent(current, previous)
        }
      }
    }
    
    class PowerSourceClusterBatFaultChangeEvent (
        val current: List<Int>,
        val previous: List<Int>) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("PowerSourceClusterBatFaultChangeEvent {\n")
        builder.append("\tcurrent : $current\n")
        builder.append("\tprevious : $previous\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.startList(ContextSpecificTag(0))
      val iter_current = current.iterator()
      while(iter_current.hasNext()) {
        val next = iter_current.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
        tlvWriter.startList(ContextSpecificTag(1))
      val iter_previous = previous.iterator()
      while(iter_previous.hasNext()) {
        val next = iter_previous.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : PowerSourceClusterBatFaultChangeEvent {
          tlvReader.enterStructure(tag)
          val current: List<Int> = mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(0))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          val previous: List<Int> = mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(1))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          
          tlvReader.exitContainer()

          return PowerSourceClusterBatFaultChangeEvent(current, previous)
        }
      }
    }
    
    class PowerSourceClusterBatChargeFaultChangeEvent (
        val current: List<Int>,
        val previous: List<Int>) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("PowerSourceClusterBatChargeFaultChangeEvent {\n")
        builder.append("\tcurrent : $current\n")
        builder.append("\tprevious : $previous\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.startList(ContextSpecificTag(0))
      val iter_current = current.iterator()
      while(iter_current.hasNext()) {
        val next = iter_current.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
        tlvWriter.startList(ContextSpecificTag(1))
      val iter_previous = previous.iterator()
      while(iter_previous.hasNext()) {
        val next = iter_previous.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : PowerSourceClusterBatChargeFaultChangeEvent {
          tlvReader.enterStructure(tag)
          val current: List<Int> = mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(0))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          val previous: List<Int> = mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(1))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          
          tlvReader.exitContainer()

          return PowerSourceClusterBatChargeFaultChangeEvent(current, previous)
        }
      }
    }
    
    class GeneralDiagnosticsClusterHardwareFaultChangeEvent (
        val current: List<Int>,
        val previous: List<Int>) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("GeneralDiagnosticsClusterHardwareFaultChangeEvent {\n")
        builder.append("\tcurrent : $current\n")
        builder.append("\tprevious : $previous\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.startList(ContextSpecificTag(0))
      val iter_current = current.iterator()
      while(iter_current.hasNext()) {
        val next = iter_current.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
        tlvWriter.startList(ContextSpecificTag(1))
      val iter_previous = previous.iterator()
      while(iter_previous.hasNext()) {
        val next = iter_previous.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : GeneralDiagnosticsClusterHardwareFaultChangeEvent {
          tlvReader.enterStructure(tag)
          val current: List<Int> = mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(0))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          val previous: List<Int> = mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(1))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          
          tlvReader.exitContainer()

          return GeneralDiagnosticsClusterHardwareFaultChangeEvent(current, previous)
        }
      }
    }
    
    class GeneralDiagnosticsClusterRadioFaultChangeEvent (
        val current: List<Int>,
        val previous: List<Int>) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("GeneralDiagnosticsClusterRadioFaultChangeEvent {\n")
        builder.append("\tcurrent : $current\n")
        builder.append("\tprevious : $previous\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.startList(ContextSpecificTag(0))
      val iter_current = current.iterator()
      while(iter_current.hasNext()) {
        val next = iter_current.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
        tlvWriter.startList(ContextSpecificTag(1))
      val iter_previous = previous.iterator()
      while(iter_previous.hasNext()) {
        val next = iter_previous.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : GeneralDiagnosticsClusterRadioFaultChangeEvent {
          tlvReader.enterStructure(tag)
          val current: List<Int> = mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(0))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          val previous: List<Int> = mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(1))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          
          tlvReader.exitContainer()

          return GeneralDiagnosticsClusterRadioFaultChangeEvent(current, previous)
        }
      }
    }
    
    class GeneralDiagnosticsClusterNetworkFaultChangeEvent (
        val current: List<Int>,
        val previous: List<Int>) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("GeneralDiagnosticsClusterNetworkFaultChangeEvent {\n")
        builder.append("\tcurrent : $current\n")
        builder.append("\tprevious : $previous\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.startList(ContextSpecificTag(0))
      val iter_current = current.iterator()
      while(iter_current.hasNext()) {
        val next = iter_current.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
        tlvWriter.startList(ContextSpecificTag(1))
      val iter_previous = previous.iterator()
      while(iter_previous.hasNext()) {
        val next = iter_previous.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : GeneralDiagnosticsClusterNetworkFaultChangeEvent {
          tlvReader.enterStructure(tag)
          val current: List<Int> = mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(0))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          val previous: List<Int> = mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(1))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          
          tlvReader.exitContainer()

          return GeneralDiagnosticsClusterNetworkFaultChangeEvent(current, previous)
        }
      }
    }
    
    class GeneralDiagnosticsClusterBootReasonEvent (
        val bootReason: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("GeneralDiagnosticsClusterBootReasonEvent {\n")
        builder.append("\tbootReason : $bootReason\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), bootReason)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : GeneralDiagnosticsClusterBootReasonEvent {
          tlvReader.enterStructure(tag)
          val bootReason: Int = tlvReader.getInt(ContextSpecificTag(0))
          
          tlvReader.exitContainer()

          return GeneralDiagnosticsClusterBootReasonEvent(bootReason)
        }
      }
    }
    
    class SoftwareDiagnosticsClusterSoftwareFaultEvent (
        val id: Long,
        val name: Optional<String>,
        val faultRecording: Optional<ByteArray>) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("SoftwareDiagnosticsClusterSoftwareFaultEvent {\n")
        builder.append("\tid : $id\n")
        builder.append("\tname : $name\n")
        builder.append("\tfaultRecording : $faultRecording\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), id)
        if (name.isPresent) {
      val opt_name = name.get()
      tlvWriter.put(ContextSpecificTag(1), opt_name)
    }
        if (faultRecording.isPresent) {
      val opt_faultRecording = faultRecording.get()
      tlvWriter.put(ContextSpecificTag(2), opt_faultRecording)
    }
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : SoftwareDiagnosticsClusterSoftwareFaultEvent {
          tlvReader.enterStructure(tag)
          val id: Long = tlvReader.getLong(ContextSpecificTag(0))
          val name: Optional<String> = try {
      Optional.of(tlvReader.getString(ContextSpecificTag(1)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          val faultRecording: Optional<ByteArray> = try {
      Optional.of(tlvReader.getByteArray(ContextSpecificTag(2)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          
          tlvReader.exitContainer()

          return SoftwareDiagnosticsClusterSoftwareFaultEvent(id, name, faultRecording)
        }
      }
    }
    
    class ThreadNetworkDiagnosticsClusterConnectionStatusEvent (
        val connectionStatus: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("ThreadNetworkDiagnosticsClusterConnectionStatusEvent {\n")
        builder.append("\tconnectionStatus : $connectionStatus\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), connectionStatus)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : ThreadNetworkDiagnosticsClusterConnectionStatusEvent {
          tlvReader.enterStructure(tag)
          val connectionStatus: Int = tlvReader.getInt(ContextSpecificTag(0))
          
          tlvReader.exitContainer()

          return ThreadNetworkDiagnosticsClusterConnectionStatusEvent(connectionStatus)
        }
      }
    }
    
    class ThreadNetworkDiagnosticsClusterNetworkFaultChangeEvent (
        val current: List<Int>,
        val previous: List<Int>) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("ThreadNetworkDiagnosticsClusterNetworkFaultChangeEvent {\n")
        builder.append("\tcurrent : $current\n")
        builder.append("\tprevious : $previous\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.startList(ContextSpecificTag(0))
      val iter_current = current.iterator()
      while(iter_current.hasNext()) {
        val next = iter_current.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
        tlvWriter.startList(ContextSpecificTag(1))
      val iter_previous = previous.iterator()
      while(iter_previous.hasNext()) {
        val next = iter_previous.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : ThreadNetworkDiagnosticsClusterNetworkFaultChangeEvent {
          tlvReader.enterStructure(tag)
          val current: List<Int> = mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(0))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          val previous: List<Int> = mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(1))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          
          tlvReader.exitContainer()

          return ThreadNetworkDiagnosticsClusterNetworkFaultChangeEvent(current, previous)
        }
      }
    }
    
    class WiFiNetworkDiagnosticsClusterDisconnectionEvent (
        val reasonCode: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("WiFiNetworkDiagnosticsClusterDisconnectionEvent {\n")
        builder.append("\treasonCode : $reasonCode\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), reasonCode)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : WiFiNetworkDiagnosticsClusterDisconnectionEvent {
          tlvReader.enterStructure(tag)
          val reasonCode: Int = tlvReader.getInt(ContextSpecificTag(0))
          
          tlvReader.exitContainer()

          return WiFiNetworkDiagnosticsClusterDisconnectionEvent(reasonCode)
        }
      }
    }
    
    class WiFiNetworkDiagnosticsClusterAssociationFailureEvent (
        val associationFailure: Int,
        val status: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("WiFiNetworkDiagnosticsClusterAssociationFailureEvent {\n")
        builder.append("\tassociationFailure : $associationFailure\n")
        builder.append("\tstatus : $status\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), associationFailure)
        tlvWriter.put(ContextSpecificTag(1), status)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : WiFiNetworkDiagnosticsClusterAssociationFailureEvent {
          tlvReader.enterStructure(tag)
          val associationFailure: Int = tlvReader.getInt(ContextSpecificTag(0))
          val status: Int = tlvReader.getInt(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return WiFiNetworkDiagnosticsClusterAssociationFailureEvent(associationFailure, status)
        }
      }
    }
    
    class WiFiNetworkDiagnosticsClusterConnectionStatusEvent (
        val connectionStatus: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("WiFiNetworkDiagnosticsClusterConnectionStatusEvent {\n")
        builder.append("\tconnectionStatus : $connectionStatus\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), connectionStatus)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : WiFiNetworkDiagnosticsClusterConnectionStatusEvent {
          tlvReader.enterStructure(tag)
          val connectionStatus: Int = tlvReader.getInt(ContextSpecificTag(0))
          
          tlvReader.exitContainer()

          return WiFiNetworkDiagnosticsClusterConnectionStatusEvent(connectionStatus)
        }
      }
    }
    
    class TimeSynchronizationClusterDSTTableEmptyEvent () {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("TimeSynchronizationClusterDSTTableEmptyEvent {\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : TimeSynchronizationClusterDSTTableEmptyEvent {
          tlvReader.enterStructure(tag)
          
          tlvReader.exitContainer()

          return TimeSynchronizationClusterDSTTableEmptyEvent()
        }
      }
    }
    
    class TimeSynchronizationClusterDSTStatusEvent (
        val DSTOffsetActive: Boolean) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("TimeSynchronizationClusterDSTStatusEvent {\n")
        builder.append("\tDSTOffsetActive : $DSTOffsetActive\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), DSTOffsetActive)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : TimeSynchronizationClusterDSTStatusEvent {
          tlvReader.enterStructure(tag)
          val DSTOffsetActive: Boolean = tlvReader.getBoolean(ContextSpecificTag(0))
          
          tlvReader.exitContainer()

          return TimeSynchronizationClusterDSTStatusEvent(DSTOffsetActive)
        }
      }
    }
    
    class TimeSynchronizationClusterTimeZoneStatusEvent (
        val offset: Long,
        val name: Optional<String>) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("TimeSynchronizationClusterTimeZoneStatusEvent {\n")
        builder.append("\toffset : $offset\n")
        builder.append("\tname : $name\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), offset)
        if (name.isPresent) {
      val opt_name = name.get()
      tlvWriter.put(ContextSpecificTag(1), opt_name)
    }
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : TimeSynchronizationClusterTimeZoneStatusEvent {
          tlvReader.enterStructure(tag)
          val offset: Long = tlvReader.getLong(ContextSpecificTag(0))
          val name: Optional<String> = try {
      Optional.of(tlvReader.getString(ContextSpecificTag(1)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          
          tlvReader.exitContainer()

          return TimeSynchronizationClusterTimeZoneStatusEvent(offset, name)
        }
      }
    }
    
    class TimeSynchronizationClusterTimeFailureEvent () {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("TimeSynchronizationClusterTimeFailureEvent {\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : TimeSynchronizationClusterTimeFailureEvent {
          tlvReader.enterStructure(tag)
          
          tlvReader.exitContainer()

          return TimeSynchronizationClusterTimeFailureEvent()
        }
      }
    }
    
    class TimeSynchronizationClusterMissingTrustedTimeSourceEvent () {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("TimeSynchronizationClusterMissingTrustedTimeSourceEvent {\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : TimeSynchronizationClusterMissingTrustedTimeSourceEvent {
          tlvReader.enterStructure(tag)
          
          tlvReader.exitContainer()

          return TimeSynchronizationClusterMissingTrustedTimeSourceEvent()
        }
      }
    }
    
    class BridgedDeviceBasicInformationClusterStartUpEvent (
        val softwareVersion: Long) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("BridgedDeviceBasicInformationClusterStartUpEvent {\n")
        builder.append("\tsoftwareVersion : $softwareVersion\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), softwareVersion)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : BridgedDeviceBasicInformationClusterStartUpEvent {
          tlvReader.enterStructure(tag)
          val softwareVersion: Long = tlvReader.getLong(ContextSpecificTag(0))
          
          tlvReader.exitContainer()

          return BridgedDeviceBasicInformationClusterStartUpEvent(softwareVersion)
        }
      }
    }
    
    class BridgedDeviceBasicInformationClusterShutDownEvent () {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("BridgedDeviceBasicInformationClusterShutDownEvent {\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : BridgedDeviceBasicInformationClusterShutDownEvent {
          tlvReader.enterStructure(tag)
          
          tlvReader.exitContainer()

          return BridgedDeviceBasicInformationClusterShutDownEvent()
        }
      }
    }
    
    class BridgedDeviceBasicInformationClusterLeaveEvent () {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("BridgedDeviceBasicInformationClusterLeaveEvent {\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : BridgedDeviceBasicInformationClusterLeaveEvent {
          tlvReader.enterStructure(tag)
          
          tlvReader.exitContainer()

          return BridgedDeviceBasicInformationClusterLeaveEvent()
        }
      }
    }
    
    class BridgedDeviceBasicInformationClusterReachableChangedEvent (
        val reachableNewValue: Boolean) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("BridgedDeviceBasicInformationClusterReachableChangedEvent {\n")
        builder.append("\treachableNewValue : $reachableNewValue\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), reachableNewValue)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : BridgedDeviceBasicInformationClusterReachableChangedEvent {
          tlvReader.enterStructure(tag)
          val reachableNewValue: Boolean = tlvReader.getBoolean(ContextSpecificTag(0))
          
          tlvReader.exitContainer()

          return BridgedDeviceBasicInformationClusterReachableChangedEvent(reachableNewValue)
        }
      }
    }
    
    class SwitchClusterSwitchLatchedEvent (
        val newPosition: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("SwitchClusterSwitchLatchedEvent {\n")
        builder.append("\tnewPosition : $newPosition\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), newPosition)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : SwitchClusterSwitchLatchedEvent {
          tlvReader.enterStructure(tag)
          val newPosition: Int = tlvReader.getInt(ContextSpecificTag(0))
          
          tlvReader.exitContainer()

          return SwitchClusterSwitchLatchedEvent(newPosition)
        }
      }
    }
    
    class SwitchClusterInitialPressEvent (
        val newPosition: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("SwitchClusterInitialPressEvent {\n")
        builder.append("\tnewPosition : $newPosition\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), newPosition)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : SwitchClusterInitialPressEvent {
          tlvReader.enterStructure(tag)
          val newPosition: Int = tlvReader.getInt(ContextSpecificTag(0))
          
          tlvReader.exitContainer()

          return SwitchClusterInitialPressEvent(newPosition)
        }
      }
    }
    
    class SwitchClusterLongPressEvent (
        val newPosition: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("SwitchClusterLongPressEvent {\n")
        builder.append("\tnewPosition : $newPosition\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), newPosition)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : SwitchClusterLongPressEvent {
          tlvReader.enterStructure(tag)
          val newPosition: Int = tlvReader.getInt(ContextSpecificTag(0))
          
          tlvReader.exitContainer()

          return SwitchClusterLongPressEvent(newPosition)
        }
      }
    }
    
    class SwitchClusterShortReleaseEvent (
        val previousPosition: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("SwitchClusterShortReleaseEvent {\n")
        builder.append("\tpreviousPosition : $previousPosition\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), previousPosition)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : SwitchClusterShortReleaseEvent {
          tlvReader.enterStructure(tag)
          val previousPosition: Int = tlvReader.getInt(ContextSpecificTag(0))
          
          tlvReader.exitContainer()

          return SwitchClusterShortReleaseEvent(previousPosition)
        }
      }
    }
    
    class SwitchClusterLongReleaseEvent (
        val previousPosition: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("SwitchClusterLongReleaseEvent {\n")
        builder.append("\tpreviousPosition : $previousPosition\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), previousPosition)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : SwitchClusterLongReleaseEvent {
          tlvReader.enterStructure(tag)
          val previousPosition: Int = tlvReader.getInt(ContextSpecificTag(0))
          
          tlvReader.exitContainer()

          return SwitchClusterLongReleaseEvent(previousPosition)
        }
      }
    }
    
    class SwitchClusterMultiPressOngoingEvent (
        val newPosition: Int,
        val currentNumberOfPressesCounted: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("SwitchClusterMultiPressOngoingEvent {\n")
        builder.append("\tnewPosition : $newPosition\n")
        builder.append("\tcurrentNumberOfPressesCounted : $currentNumberOfPressesCounted\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), newPosition)
        tlvWriter.put(ContextSpecificTag(1), currentNumberOfPressesCounted)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : SwitchClusterMultiPressOngoingEvent {
          tlvReader.enterStructure(tag)
          val newPosition: Int = tlvReader.getInt(ContextSpecificTag(0))
          val currentNumberOfPressesCounted: Int = tlvReader.getInt(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return SwitchClusterMultiPressOngoingEvent(newPosition, currentNumberOfPressesCounted)
        }
      }
    }
    
    class SwitchClusterMultiPressCompleteEvent (
        val previousPosition: Int,
        val totalNumberOfPressesCounted: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("SwitchClusterMultiPressCompleteEvent {\n")
        builder.append("\tpreviousPosition : $previousPosition\n")
        builder.append("\ttotalNumberOfPressesCounted : $totalNumberOfPressesCounted\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), previousPosition)
        tlvWriter.put(ContextSpecificTag(1), totalNumberOfPressesCounted)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : SwitchClusterMultiPressCompleteEvent {
          tlvReader.enterStructure(tag)
          val previousPosition: Int = tlvReader.getInt(ContextSpecificTag(0))
          val totalNumberOfPressesCounted: Int = tlvReader.getInt(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return SwitchClusterMultiPressCompleteEvent(previousPosition, totalNumberOfPressesCounted)
        }
      }
    }
    
    class BooleanStateClusterStateChangeEvent (
        val stateValue: Boolean) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("BooleanStateClusterStateChangeEvent {\n")
        builder.append("\tstateValue : $stateValue\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), stateValue)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : BooleanStateClusterStateChangeEvent {
          tlvReader.enterStructure(tag)
          val stateValue: Boolean = tlvReader.getBoolean(ContextSpecificTag(0))
          
          tlvReader.exitContainer()

          return BooleanStateClusterStateChangeEvent(stateValue)
        }
      }
    }
    
    class RefrigeratorAlarmClusterNotifyEvent (
        val active: Long,
        val inactive: Long,
        val state: Long,
        val mask: Long) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("RefrigeratorAlarmClusterNotifyEvent {\n")
        builder.append("\tactive : $active\n")
        builder.append("\tinactive : $inactive\n")
        builder.append("\tstate : $state\n")
        builder.append("\tmask : $mask\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), active)
        tlvWriter.put(ContextSpecificTag(1), inactive)
        tlvWriter.put(ContextSpecificTag(2), state)
        tlvWriter.put(ContextSpecificTag(3), mask)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : RefrigeratorAlarmClusterNotifyEvent {
          tlvReader.enterStructure(tag)
          val active: Long = tlvReader.getLong(ContextSpecificTag(0))
          val inactive: Long = tlvReader.getLong(ContextSpecificTag(1))
          val state: Long = tlvReader.getLong(ContextSpecificTag(2))
          val mask: Long = tlvReader.getLong(ContextSpecificTag(3))
          
          tlvReader.exitContainer()

          return RefrigeratorAlarmClusterNotifyEvent(active, inactive, state, mask)
        }
      }
    }
    
    class SmokeCoAlarmClusterSmokeAlarmEvent (
        val alarmSeverityLevel: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("SmokeCoAlarmClusterSmokeAlarmEvent {\n")
        builder.append("\talarmSeverityLevel : $alarmSeverityLevel\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), alarmSeverityLevel)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : SmokeCoAlarmClusterSmokeAlarmEvent {
          tlvReader.enterStructure(tag)
          val alarmSeverityLevel: Int = tlvReader.getInt(ContextSpecificTag(0))
          
          tlvReader.exitContainer()

          return SmokeCoAlarmClusterSmokeAlarmEvent(alarmSeverityLevel)
        }
      }
    }
    
    class SmokeCoAlarmClusterCOAlarmEvent (
        val alarmSeverityLevel: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("SmokeCoAlarmClusterCOAlarmEvent {\n")
        builder.append("\talarmSeverityLevel : $alarmSeverityLevel\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), alarmSeverityLevel)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : SmokeCoAlarmClusterCOAlarmEvent {
          tlvReader.enterStructure(tag)
          val alarmSeverityLevel: Int = tlvReader.getInt(ContextSpecificTag(0))
          
          tlvReader.exitContainer()

          return SmokeCoAlarmClusterCOAlarmEvent(alarmSeverityLevel)
        }
      }
    }
    
    class SmokeCoAlarmClusterLowBatteryEvent (
        val alarmSeverityLevel: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("SmokeCoAlarmClusterLowBatteryEvent {\n")
        builder.append("\talarmSeverityLevel : $alarmSeverityLevel\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), alarmSeverityLevel)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : SmokeCoAlarmClusterLowBatteryEvent {
          tlvReader.enterStructure(tag)
          val alarmSeverityLevel: Int = tlvReader.getInt(ContextSpecificTag(0))
          
          tlvReader.exitContainer()

          return SmokeCoAlarmClusterLowBatteryEvent(alarmSeverityLevel)
        }
      }
    }
    
    class SmokeCoAlarmClusterHardwareFaultEvent () {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("SmokeCoAlarmClusterHardwareFaultEvent {\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : SmokeCoAlarmClusterHardwareFaultEvent {
          tlvReader.enterStructure(tag)
          
          tlvReader.exitContainer()

          return SmokeCoAlarmClusterHardwareFaultEvent()
        }
      }
    }
    
    class SmokeCoAlarmClusterEndOfServiceEvent () {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("SmokeCoAlarmClusterEndOfServiceEvent {\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : SmokeCoAlarmClusterEndOfServiceEvent {
          tlvReader.enterStructure(tag)
          
          tlvReader.exitContainer()

          return SmokeCoAlarmClusterEndOfServiceEvent()
        }
      }
    }
    
    class SmokeCoAlarmClusterSelfTestCompleteEvent () {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("SmokeCoAlarmClusterSelfTestCompleteEvent {\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : SmokeCoAlarmClusterSelfTestCompleteEvent {
          tlvReader.enterStructure(tag)
          
          tlvReader.exitContainer()

          return SmokeCoAlarmClusterSelfTestCompleteEvent()
        }
      }
    }
    
    class SmokeCoAlarmClusterAlarmMutedEvent () {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("SmokeCoAlarmClusterAlarmMutedEvent {\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : SmokeCoAlarmClusterAlarmMutedEvent {
          tlvReader.enterStructure(tag)
          
          tlvReader.exitContainer()

          return SmokeCoAlarmClusterAlarmMutedEvent()
        }
      }
    }
    
    class SmokeCoAlarmClusterMuteEndedEvent () {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("SmokeCoAlarmClusterMuteEndedEvent {\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : SmokeCoAlarmClusterMuteEndedEvent {
          tlvReader.enterStructure(tag)
          
          tlvReader.exitContainer()

          return SmokeCoAlarmClusterMuteEndedEvent()
        }
      }
    }
    
    class SmokeCoAlarmClusterInterconnectSmokeAlarmEvent (
        val alarmSeverityLevel: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("SmokeCoAlarmClusterInterconnectSmokeAlarmEvent {\n")
        builder.append("\talarmSeverityLevel : $alarmSeverityLevel\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), alarmSeverityLevel)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : SmokeCoAlarmClusterInterconnectSmokeAlarmEvent {
          tlvReader.enterStructure(tag)
          val alarmSeverityLevel: Int = tlvReader.getInt(ContextSpecificTag(0))
          
          tlvReader.exitContainer()

          return SmokeCoAlarmClusterInterconnectSmokeAlarmEvent(alarmSeverityLevel)
        }
      }
    }
    
    class SmokeCoAlarmClusterInterconnectCOAlarmEvent (
        val alarmSeverityLevel: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("SmokeCoAlarmClusterInterconnectCOAlarmEvent {\n")
        builder.append("\talarmSeverityLevel : $alarmSeverityLevel\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), alarmSeverityLevel)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : SmokeCoAlarmClusterInterconnectCOAlarmEvent {
          tlvReader.enterStructure(tag)
          val alarmSeverityLevel: Int = tlvReader.getInt(ContextSpecificTag(0))
          
          tlvReader.exitContainer()

          return SmokeCoAlarmClusterInterconnectCOAlarmEvent(alarmSeverityLevel)
        }
      }
    }
    
    class SmokeCoAlarmClusterAllClearEvent () {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("SmokeCoAlarmClusterAllClearEvent {\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : SmokeCoAlarmClusterAllClearEvent {
          tlvReader.enterStructure(tag)
          
          tlvReader.exitContainer()

          return SmokeCoAlarmClusterAllClearEvent()
        }
      }
    }
    
    class DishwasherAlarmClusterNotifyEvent (
        val active: Long,
        val inactive: Long,
        val state: Long,
        val mask: Long) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("DishwasherAlarmClusterNotifyEvent {\n")
        builder.append("\tactive : $active\n")
        builder.append("\tinactive : $inactive\n")
        builder.append("\tstate : $state\n")
        builder.append("\tmask : $mask\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), active)
        tlvWriter.put(ContextSpecificTag(1), inactive)
        tlvWriter.put(ContextSpecificTag(2), state)
        tlvWriter.put(ContextSpecificTag(3), mask)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : DishwasherAlarmClusterNotifyEvent {
          tlvReader.enterStructure(tag)
          val active: Long = tlvReader.getLong(ContextSpecificTag(0))
          val inactive: Long = tlvReader.getLong(ContextSpecificTag(1))
          val state: Long = tlvReader.getLong(ContextSpecificTag(2))
          val mask: Long = tlvReader.getLong(ContextSpecificTag(3))
          
          tlvReader.exitContainer()

          return DishwasherAlarmClusterNotifyEvent(active, inactive, state, mask)
        }
      }
    }
    
    class OperationalStateClusterOperationalErrorEvent (
        val errorState: ChipStructs.OperationalStateClusterErrorStateStruct) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("OperationalStateClusterOperationalErrorEvent {\n")
        builder.append("\terrorState : $errorState\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        errorState.toTlv(ContextSpecificTag(0), tlvWriter)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : OperationalStateClusterOperationalErrorEvent {
          tlvReader.enterStructure(tag)
          val errorState: ChipStructs.OperationalStateClusterErrorStateStruct = ChipStructs.OperationalStateClusterErrorStateStruct.fromTlv(ContextSpecificTag(0), tlvReader)
          
          tlvReader.exitContainer()

          return OperationalStateClusterOperationalErrorEvent(errorState)
        }
      }
    }
    
    class OperationalStateClusterOperationCompletionEvent (
        val completionErrorCode: Int,
        val totalOperationalTime: Optional<Long>?,
        val pausedTime: Optional<Long>?) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("OperationalStateClusterOperationCompletionEvent {\n")
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
      val opt_totalOperationalTime = totalOperationalTime.get()
      tlvWriter.put(ContextSpecificTag(1), opt_totalOperationalTime)
    }
    }
        if (pausedTime == null) { tlvWriter.putNull(ContextSpecificTag(2)) }
    else {
      if (pausedTime.isPresent) {
      val opt_pausedTime = pausedTime.get()
      tlvWriter.put(ContextSpecificTag(2), opt_pausedTime)
    }
    }
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : OperationalStateClusterOperationCompletionEvent {
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

          return OperationalStateClusterOperationCompletionEvent(completionErrorCode, totalOperationalTime, pausedTime)
        }
      }
    }
    
    class RvcOperationalStateClusterOperationalErrorEvent (
        val errorState: ChipStructs.RvcOperationalStateClusterErrorStateStruct) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("RvcOperationalStateClusterOperationalErrorEvent {\n")
        builder.append("\terrorState : $errorState\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        errorState.toTlv(ContextSpecificTag(0), tlvWriter)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : RvcOperationalStateClusterOperationalErrorEvent {
          tlvReader.enterStructure(tag)
          val errorState: ChipStructs.RvcOperationalStateClusterErrorStateStruct = ChipStructs.RvcOperationalStateClusterErrorStateStruct.fromTlv(ContextSpecificTag(0), tlvReader)
          
          tlvReader.exitContainer()

          return RvcOperationalStateClusterOperationalErrorEvent(errorState)
        }
      }
    }
    
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
      val opt_totalOperationalTime = totalOperationalTime.get()
      tlvWriter.put(ContextSpecificTag(1), opt_totalOperationalTime)
    }
    }
        if (pausedTime == null) { tlvWriter.putNull(ContextSpecificTag(2)) }
    else {
      if (pausedTime.isPresent) {
      val opt_pausedTime = pausedTime.get()
      tlvWriter.put(ContextSpecificTag(2), opt_pausedTime)
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
    
    class DoorLockClusterDoorLockAlarmEvent (
        val alarmCode: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("DoorLockClusterDoorLockAlarmEvent {\n")
        builder.append("\talarmCode : $alarmCode\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), alarmCode)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : DoorLockClusterDoorLockAlarmEvent {
          tlvReader.enterStructure(tag)
          val alarmCode: Int = tlvReader.getInt(ContextSpecificTag(0))
          
          tlvReader.exitContainer()

          return DoorLockClusterDoorLockAlarmEvent(alarmCode)
        }
      }
    }
    
    class DoorLockClusterDoorStateChangeEvent (
        val doorState: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("DoorLockClusterDoorStateChangeEvent {\n")
        builder.append("\tdoorState : $doorState\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), doorState)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : DoorLockClusterDoorStateChangeEvent {
          tlvReader.enterStructure(tag)
          val doorState: Int = tlvReader.getInt(ContextSpecificTag(0))
          
          tlvReader.exitContainer()

          return DoorLockClusterDoorStateChangeEvent(doorState)
        }
      }
    }
    
    class DoorLockClusterLockOperationEvent (
        val lockOperationType: Int,
        val operationSource: Int,
        val userIndex: Int?,
        val fabricIndex: Int?,
        val sourceNode: Long?,
        val credentials: Optional<List<ChipStructs.DoorLockClusterCredentialStruct>>?) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("DoorLockClusterLockOperationEvent {\n")
        builder.append("\tlockOperationType : $lockOperationType\n")
        builder.append("\toperationSource : $operationSource\n")
        builder.append("\tuserIndex : $userIndex\n")
        builder.append("\tfabricIndex : $fabricIndex\n")
        builder.append("\tsourceNode : $sourceNode\n")
        builder.append("\tcredentials : $credentials\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), lockOperationType)
        tlvWriter.put(ContextSpecificTag(1), operationSource)
        if (userIndex == null) { tlvWriter.putNull(ContextSpecificTag(2)) }
    else {
      tlvWriter.put(ContextSpecificTag(2), userIndex)
    }
        if (fabricIndex == null) { tlvWriter.putNull(ContextSpecificTag(3)) }
    else {
      tlvWriter.put(ContextSpecificTag(3), fabricIndex)
    }
        if (sourceNode == null) { tlvWriter.putNull(ContextSpecificTag(4)) }
    else {
      tlvWriter.put(ContextSpecificTag(4), sourceNode)
    }
        if (credentials == null) { tlvWriter.putNull(ContextSpecificTag(5)) }
    else {
      if (credentials.isPresent) {
      val opt_credentials = credentials.get()
      tlvWriter.startList(ContextSpecificTag(5))
      val iter_opt_credentials = opt_credentials.iterator()
      while(iter_opt_credentials.hasNext()) {
        val next = iter_opt_credentials.next()
        next.toTlv(AnonymousTag, tlvWriter)
      }
      tlvWriter.endList()
    }
    }
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : DoorLockClusterLockOperationEvent {
          tlvReader.enterStructure(tag)
          val lockOperationType: Int = tlvReader.getInt(ContextSpecificTag(0))
          val operationSource: Int = tlvReader.getInt(ContextSpecificTag(1))
          val userIndex: Int? = try {
      tlvReader.getInt(ContextSpecificTag(2))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(2))
      null
    }
          val fabricIndex: Int? = try {
      tlvReader.getInt(ContextSpecificTag(3))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(3))
      null
    }
          val sourceNode: Long? = try {
      tlvReader.getLong(ContextSpecificTag(4))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(4))
      null
    }
          val credentials: Optional<List<ChipStructs.DoorLockClusterCredentialStruct>>? = try {
      try {
      Optional.of(mutableListOf<ChipStructs.DoorLockClusterCredentialStruct>().apply {
      tlvReader.enterList(ContextSpecificTag(5))
      while(true) {
        try {
          this.add(ChipStructs.DoorLockClusterCredentialStruct.fromTlv(AnonymousTag, tlvReader))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    })
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(5))
      null
    }
          
          tlvReader.exitContainer()

          return DoorLockClusterLockOperationEvent(lockOperationType, operationSource, userIndex, fabricIndex, sourceNode, credentials)
        }
      }
    }
    
    class DoorLockClusterLockOperationErrorEvent (
        val lockOperationType: Int,
        val operationSource: Int,
        val operationError: Int,
        val userIndex: Int?,
        val fabricIndex: Int?,
        val sourceNode: Long?,
        val credentials: Optional<List<ChipStructs.DoorLockClusterCredentialStruct>>?) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("DoorLockClusterLockOperationErrorEvent {\n")
        builder.append("\tlockOperationType : $lockOperationType\n")
        builder.append("\toperationSource : $operationSource\n")
        builder.append("\toperationError : $operationError\n")
        builder.append("\tuserIndex : $userIndex\n")
        builder.append("\tfabricIndex : $fabricIndex\n")
        builder.append("\tsourceNode : $sourceNode\n")
        builder.append("\tcredentials : $credentials\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), lockOperationType)
        tlvWriter.put(ContextSpecificTag(1), operationSource)
        tlvWriter.put(ContextSpecificTag(2), operationError)
        if (userIndex == null) { tlvWriter.putNull(ContextSpecificTag(3)) }
    else {
      tlvWriter.put(ContextSpecificTag(3), userIndex)
    }
        if (fabricIndex == null) { tlvWriter.putNull(ContextSpecificTag(4)) }
    else {
      tlvWriter.put(ContextSpecificTag(4), fabricIndex)
    }
        if (sourceNode == null) { tlvWriter.putNull(ContextSpecificTag(5)) }
    else {
      tlvWriter.put(ContextSpecificTag(5), sourceNode)
    }
        if (credentials == null) { tlvWriter.putNull(ContextSpecificTag(6)) }
    else {
      if (credentials.isPresent) {
      val opt_credentials = credentials.get()
      tlvWriter.startList(ContextSpecificTag(6))
      val iter_opt_credentials = opt_credentials.iterator()
      while(iter_opt_credentials.hasNext()) {
        val next = iter_opt_credentials.next()
        next.toTlv(AnonymousTag, tlvWriter)
      }
      tlvWriter.endList()
    }
    }
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : DoorLockClusterLockOperationErrorEvent {
          tlvReader.enterStructure(tag)
          val lockOperationType: Int = tlvReader.getInt(ContextSpecificTag(0))
          val operationSource: Int = tlvReader.getInt(ContextSpecificTag(1))
          val operationError: Int = tlvReader.getInt(ContextSpecificTag(2))
          val userIndex: Int? = try {
      tlvReader.getInt(ContextSpecificTag(3))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(3))
      null
    }
          val fabricIndex: Int? = try {
      tlvReader.getInt(ContextSpecificTag(4))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(4))
      null
    }
          val sourceNode: Long? = try {
      tlvReader.getLong(ContextSpecificTag(5))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(5))
      null
    }
          val credentials: Optional<List<ChipStructs.DoorLockClusterCredentialStruct>>? = try {
      try {
      Optional.of(mutableListOf<ChipStructs.DoorLockClusterCredentialStruct>().apply {
      tlvReader.enterList(ContextSpecificTag(6))
      while(true) {
        try {
          this.add(ChipStructs.DoorLockClusterCredentialStruct.fromTlv(AnonymousTag, tlvReader))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    })
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(6))
      null
    }
          
          tlvReader.exitContainer()

          return DoorLockClusterLockOperationErrorEvent(lockOperationType, operationSource, operationError, userIndex, fabricIndex, sourceNode, credentials)
        }
      }
    }
    
    class DoorLockClusterLockUserChangeEvent (
        val lockDataType: Int,
        val dataOperationType: Int,
        val operationSource: Int,
        val userIndex: Int?,
        val fabricIndex: Int?,
        val sourceNode: Long?,
        val dataIndex: Int?) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("DoorLockClusterLockUserChangeEvent {\n")
        builder.append("\tlockDataType : $lockDataType\n")
        builder.append("\tdataOperationType : $dataOperationType\n")
        builder.append("\toperationSource : $operationSource\n")
        builder.append("\tuserIndex : $userIndex\n")
        builder.append("\tfabricIndex : $fabricIndex\n")
        builder.append("\tsourceNode : $sourceNode\n")
        builder.append("\tdataIndex : $dataIndex\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), lockDataType)
        tlvWriter.put(ContextSpecificTag(1), dataOperationType)
        tlvWriter.put(ContextSpecificTag(2), operationSource)
        if (userIndex == null) { tlvWriter.putNull(ContextSpecificTag(3)) }
    else {
      tlvWriter.put(ContextSpecificTag(3), userIndex)
    }
        if (fabricIndex == null) { tlvWriter.putNull(ContextSpecificTag(4)) }
    else {
      tlvWriter.put(ContextSpecificTag(4), fabricIndex)
    }
        if (sourceNode == null) { tlvWriter.putNull(ContextSpecificTag(5)) }
    else {
      tlvWriter.put(ContextSpecificTag(5), sourceNode)
    }
        if (dataIndex == null) { tlvWriter.putNull(ContextSpecificTag(6)) }
    else {
      tlvWriter.put(ContextSpecificTag(6), dataIndex)
    }
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : DoorLockClusterLockUserChangeEvent {
          tlvReader.enterStructure(tag)
          val lockDataType: Int = tlvReader.getInt(ContextSpecificTag(0))
          val dataOperationType: Int = tlvReader.getInt(ContextSpecificTag(1))
          val operationSource: Int = tlvReader.getInt(ContextSpecificTag(2))
          val userIndex: Int? = try {
      tlvReader.getInt(ContextSpecificTag(3))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(3))
      null
    }
          val fabricIndex: Int? = try {
      tlvReader.getInt(ContextSpecificTag(4))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(4))
      null
    }
          val sourceNode: Long? = try {
      tlvReader.getLong(ContextSpecificTag(5))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(5))
      null
    }
          val dataIndex: Int? = try {
      tlvReader.getInt(ContextSpecificTag(6))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(6))
      null
    }
          
          tlvReader.exitContainer()

          return DoorLockClusterLockUserChangeEvent(lockDataType, dataOperationType, operationSource, userIndex, fabricIndex, sourceNode, dataIndex)
        }
      }
    }
    
    class PumpConfigurationAndControlClusterSupplyVoltageLowEvent () {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("PumpConfigurationAndControlClusterSupplyVoltageLowEvent {\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : PumpConfigurationAndControlClusterSupplyVoltageLowEvent {
          tlvReader.enterStructure(tag)
          
          tlvReader.exitContainer()

          return PumpConfigurationAndControlClusterSupplyVoltageLowEvent()
        }
      }
    }
    
    class PumpConfigurationAndControlClusterSupplyVoltageHighEvent () {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("PumpConfigurationAndControlClusterSupplyVoltageHighEvent {\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : PumpConfigurationAndControlClusterSupplyVoltageHighEvent {
          tlvReader.enterStructure(tag)
          
          tlvReader.exitContainer()

          return PumpConfigurationAndControlClusterSupplyVoltageHighEvent()
        }
      }
    }
    
    class PumpConfigurationAndControlClusterPowerMissingPhaseEvent () {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("PumpConfigurationAndControlClusterPowerMissingPhaseEvent {\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : PumpConfigurationAndControlClusterPowerMissingPhaseEvent {
          tlvReader.enterStructure(tag)
          
          tlvReader.exitContainer()

          return PumpConfigurationAndControlClusterPowerMissingPhaseEvent()
        }
      }
    }
    
    class PumpConfigurationAndControlClusterSystemPressureLowEvent () {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("PumpConfigurationAndControlClusterSystemPressureLowEvent {\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : PumpConfigurationAndControlClusterSystemPressureLowEvent {
          tlvReader.enterStructure(tag)
          
          tlvReader.exitContainer()

          return PumpConfigurationAndControlClusterSystemPressureLowEvent()
        }
      }
    }
    
    class PumpConfigurationAndControlClusterSystemPressureHighEvent () {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("PumpConfigurationAndControlClusterSystemPressureHighEvent {\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : PumpConfigurationAndControlClusterSystemPressureHighEvent {
          tlvReader.enterStructure(tag)
          
          tlvReader.exitContainer()

          return PumpConfigurationAndControlClusterSystemPressureHighEvent()
        }
      }
    }
    
    class PumpConfigurationAndControlClusterDryRunningEvent () {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("PumpConfigurationAndControlClusterDryRunningEvent {\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : PumpConfigurationAndControlClusterDryRunningEvent {
          tlvReader.enterStructure(tag)
          
          tlvReader.exitContainer()

          return PumpConfigurationAndControlClusterDryRunningEvent()
        }
      }
    }
    
    class PumpConfigurationAndControlClusterMotorTemperatureHighEvent () {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("PumpConfigurationAndControlClusterMotorTemperatureHighEvent {\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : PumpConfigurationAndControlClusterMotorTemperatureHighEvent {
          tlvReader.enterStructure(tag)
          
          tlvReader.exitContainer()

          return PumpConfigurationAndControlClusterMotorTemperatureHighEvent()
        }
      }
    }
    
    class PumpConfigurationAndControlClusterPumpMotorFatalFailureEvent () {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("PumpConfigurationAndControlClusterPumpMotorFatalFailureEvent {\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : PumpConfigurationAndControlClusterPumpMotorFatalFailureEvent {
          tlvReader.enterStructure(tag)
          
          tlvReader.exitContainer()

          return PumpConfigurationAndControlClusterPumpMotorFatalFailureEvent()
        }
      }
    }
    
    class PumpConfigurationAndControlClusterElectronicTemperatureHighEvent () {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("PumpConfigurationAndControlClusterElectronicTemperatureHighEvent {\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : PumpConfigurationAndControlClusterElectronicTemperatureHighEvent {
          tlvReader.enterStructure(tag)
          
          tlvReader.exitContainer()

          return PumpConfigurationAndControlClusterElectronicTemperatureHighEvent()
        }
      }
    }
    
    class PumpConfigurationAndControlClusterPumpBlockedEvent () {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("PumpConfigurationAndControlClusterPumpBlockedEvent {\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : PumpConfigurationAndControlClusterPumpBlockedEvent {
          tlvReader.enterStructure(tag)
          
          tlvReader.exitContainer()

          return PumpConfigurationAndControlClusterPumpBlockedEvent()
        }
      }
    }
    
    class PumpConfigurationAndControlClusterSensorFailureEvent () {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("PumpConfigurationAndControlClusterSensorFailureEvent {\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : PumpConfigurationAndControlClusterSensorFailureEvent {
          tlvReader.enterStructure(tag)
          
          tlvReader.exitContainer()

          return PumpConfigurationAndControlClusterSensorFailureEvent()
        }
      }
    }
    
    class PumpConfigurationAndControlClusterElectronicNonFatalFailureEvent () {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("PumpConfigurationAndControlClusterElectronicNonFatalFailureEvent {\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : PumpConfigurationAndControlClusterElectronicNonFatalFailureEvent {
          tlvReader.enterStructure(tag)
          
          tlvReader.exitContainer()

          return PumpConfigurationAndControlClusterElectronicNonFatalFailureEvent()
        }
      }
    }
    
    class PumpConfigurationAndControlClusterElectronicFatalFailureEvent () {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("PumpConfigurationAndControlClusterElectronicFatalFailureEvent {\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : PumpConfigurationAndControlClusterElectronicFatalFailureEvent {
          tlvReader.enterStructure(tag)
          
          tlvReader.exitContainer()

          return PumpConfigurationAndControlClusterElectronicFatalFailureEvent()
        }
      }
    }
    
    class PumpConfigurationAndControlClusterGeneralFaultEvent () {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("PumpConfigurationAndControlClusterGeneralFaultEvent {\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : PumpConfigurationAndControlClusterGeneralFaultEvent {
          tlvReader.enterStructure(tag)
          
          tlvReader.exitContainer()

          return PumpConfigurationAndControlClusterGeneralFaultEvent()
        }
      }
    }
    
    class PumpConfigurationAndControlClusterLeakageEvent () {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("PumpConfigurationAndControlClusterLeakageEvent {\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : PumpConfigurationAndControlClusterLeakageEvent {
          tlvReader.enterStructure(tag)
          
          tlvReader.exitContainer()

          return PumpConfigurationAndControlClusterLeakageEvent()
        }
      }
    }
    
    class PumpConfigurationAndControlClusterAirDetectionEvent () {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("PumpConfigurationAndControlClusterAirDetectionEvent {\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : PumpConfigurationAndControlClusterAirDetectionEvent {
          tlvReader.enterStructure(tag)
          
          tlvReader.exitContainer()

          return PumpConfigurationAndControlClusterAirDetectionEvent()
        }
      }
    }
    
    class PumpConfigurationAndControlClusterTurbineOperationEvent () {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("PumpConfigurationAndControlClusterTurbineOperationEvent {\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : PumpConfigurationAndControlClusterTurbineOperationEvent {
          tlvReader.enterStructure(tag)
          
          tlvReader.exitContainer()

          return PumpConfigurationAndControlClusterTurbineOperationEvent()
        }
      }
    }
    
    class UnitTestingClusterTestEventEvent (
        val arg1: Int,
        val arg2: Int,
        val arg3: Boolean,
        val arg4: ChipStructs.UnitTestingClusterSimpleStruct,
        val arg5: List<ChipStructs.UnitTestingClusterSimpleStruct>,
        val arg6: List<Int>) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("UnitTestingClusterTestEventEvent {\n")
        builder.append("\targ1 : $arg1\n")
        builder.append("\targ2 : $arg2\n")
        builder.append("\targ3 : $arg3\n")
        builder.append("\targ4 : $arg4\n")
        builder.append("\targ5 : $arg5\n")
        builder.append("\targ6 : $arg6\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(1), arg1)
        tlvWriter.put(ContextSpecificTag(2), arg2)
        tlvWriter.put(ContextSpecificTag(3), arg3)
        arg4.toTlv(ContextSpecificTag(4), tlvWriter)
        tlvWriter.startList(ContextSpecificTag(5))
      val iter_arg5 = arg5.iterator()
      while(iter_arg5.hasNext()) {
        val next = iter_arg5.next()
        next.toTlv(AnonymousTag, tlvWriter)
      }
      tlvWriter.endList()
        tlvWriter.startList(ContextSpecificTag(6))
      val iter_arg6 = arg6.iterator()
      while(iter_arg6.hasNext()) {
        val next = iter_arg6.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : UnitTestingClusterTestEventEvent {
          tlvReader.enterStructure(tag)
          val arg1: Int = tlvReader.getInt(ContextSpecificTag(1))
          val arg2: Int = tlvReader.getInt(ContextSpecificTag(2))
          val arg3: Boolean = tlvReader.getBoolean(ContextSpecificTag(3))
          val arg4: ChipStructs.UnitTestingClusterSimpleStruct = ChipStructs.UnitTestingClusterSimpleStruct.fromTlv(ContextSpecificTag(4), tlvReader)
          val arg5: List<ChipStructs.UnitTestingClusterSimpleStruct> = mutableListOf<ChipStructs.UnitTestingClusterSimpleStruct>().apply {
      tlvReader.enterList(ContextSpecificTag(5))
      while(true) {
        try {
          this.add(ChipStructs.UnitTestingClusterSimpleStruct.fromTlv(AnonymousTag, tlvReader))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          val arg6: List<Int> = mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(6))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          
          tlvReader.exitContainer()

          return UnitTestingClusterTestEventEvent(arg1, arg2, arg3, arg4, arg5, arg6)
        }
      }
    }
    
    class UnitTestingClusterTestFabricScopedEventEvent (
        val fabricIndex: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("UnitTestingClusterTestFabricScopedEventEvent {\n")
        builder.append("\tfabricIndex : $fabricIndex\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(254), fabricIndex)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : UnitTestingClusterTestFabricScopedEventEvent {
          tlvReader.enterStructure(tag)
          val fabricIndex: Int = tlvReader.getInt(ContextSpecificTag(254))
          
          tlvReader.exitContainer()

          return UnitTestingClusterTestFabricScopedEventEvent(fabricIndex)
        }
      }
    }
    }
