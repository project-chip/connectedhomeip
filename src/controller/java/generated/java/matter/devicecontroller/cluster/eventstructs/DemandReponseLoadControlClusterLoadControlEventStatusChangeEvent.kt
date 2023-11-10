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
package matter.devicecontroller.cluster.eventstructs

import java.util.Optional
import matter.devicecontroller.cluster.*
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class DemandReponseLoadControlClusterLoadControlEventStatusChangeEvent(
  val eventID: ByteArray,
  val transitionIndex: UByte?,
  val status: UInt,
  val criticality: UInt,
  val control: UInt,
  val temperatureControl: matter.devicecontroller.cluster.structs.DemandReponseLoadControlClusterTemperatureControlStruct,
  val averageLoadControl: matter.devicecontroller.cluster.structs.DemandReponseLoadControlClusterAverageLoadControlStruct,
  val dutyCycleControl: matter.devicecontroller.cluster.structs.DemandReponseLoadControlClusterDutyCycleControlStruct,
  val powerSavingsControl: matter.devicecontroller.cluster.structs.DemandReponseLoadControlClusterPowerSavingsControlStruct,
  val heatingSourceControl: matter.devicecontroller.cluster.structs.DemandReponseLoadControlClusterHeatingSourceControlStruct,
  val signature: ByteArray
) {
  override fun toString(): String = buildString {
    append("DemandReponseLoadControlClusterLoadControlEventStatusChangeEvent {\n")
    append("\teventID : $eventID\n")
    append("\ttransitionIndex : $transitionIndex\n")
    append("\tstatus : $status\n")
    append("\tcriticality : $criticality\n")
    append("\tcontrol : $control\n")
    append("\ttemperatureControl : $temperatureControl\n")
    append("\taverageLoadControl : $averageLoadControl\n")
    append("\tdutyCycleControl : $dutyCycleControl\n")
    append("\tpowerSavingsControl : $powerSavingsControl\n")
    append("\theatingSourceControl : $heatingSourceControl\n")
    append("\tsignature : $signature\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_EVENT_I_D), eventID)
      if (transitionIndex != null) {
        put(ContextSpecificTag(TAG_TRANSITION_INDEX), transitionIndex)
      } else {
        putNull(ContextSpecificTag(TAG_TRANSITION_INDEX))
      }
      put(ContextSpecificTag(TAG_STATUS), status)
      put(ContextSpecificTag(TAG_CRITICALITY), criticality)
      put(ContextSpecificTag(TAG_CONTROL), control)
      temperatureControl.toTlv(ContextSpecificTag(TAG_TEMPERATURE_CONTROL), this)
      averageLoadControl.toTlv(ContextSpecificTag(TAG_AVERAGE_LOAD_CONTROL), this)
      dutyCycleControl.toTlv(ContextSpecificTag(TAG_DUTY_CYCLE_CONTROL), this)
      powerSavingsControl.toTlv(ContextSpecificTag(TAG_POWER_SAVINGS_CONTROL), this)
      heatingSourceControl.toTlv(ContextSpecificTag(TAG_HEATING_SOURCE_CONTROL), this)
      put(ContextSpecificTag(TAG_SIGNATURE), signature)
      endStructure()
    }
  }

  companion object {
    private const val TAG_EVENT_I_D = 0
    private const val TAG_TRANSITION_INDEX = 1
    private const val TAG_STATUS = 2
    private const val TAG_CRITICALITY = 3
    private const val TAG_CONTROL = 4
    private const val TAG_TEMPERATURE_CONTROL = 5
    private const val TAG_AVERAGE_LOAD_CONTROL = 6
    private const val TAG_DUTY_CYCLE_CONTROL = 7
    private const val TAG_POWER_SAVINGS_CONTROL = 8
    private const val TAG_HEATING_SOURCE_CONTROL = 9
    private const val TAG_SIGNATURE = 255

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : DemandReponseLoadControlClusterLoadControlEventStatusChangeEvent {
      tlvReader.enterStructure(tlvTag)
      val eventID = tlvReader.getByteArray(ContextSpecificTag(TAG_EVENT_I_D))
      val transitionIndex = if (!tlvReader.isNull()) {
        tlvReader.getUByte(ContextSpecificTag(TAG_TRANSITION_INDEX))
      } else {
        tlvReader.getNull(ContextSpecificTag(TAG_TRANSITION_INDEX))
        null
      }
      val status = tlvReader.getUInt(ContextSpecificTag(TAG_STATUS))
      val criticality = tlvReader.getUInt(ContextSpecificTag(TAG_CRITICALITY))
      val control = tlvReader.getUInt(ContextSpecificTag(TAG_CONTROL))
      val temperatureControl = matter.devicecontroller.cluster.structs.DemandReponseLoadControlClusterTemperatureControlStruct.fromTlv(ContextSpecificTag(TAG_TEMPERATURE_CONTROL), tlvReader)
      val averageLoadControl = matter.devicecontroller.cluster.structs.DemandReponseLoadControlClusterAverageLoadControlStruct.fromTlv(ContextSpecificTag(TAG_AVERAGE_LOAD_CONTROL), tlvReader)
      val dutyCycleControl = matter.devicecontroller.cluster.structs.DemandReponseLoadControlClusterDutyCycleControlStruct.fromTlv(ContextSpecificTag(TAG_DUTY_CYCLE_CONTROL), tlvReader)
      val powerSavingsControl = matter.devicecontroller.cluster.structs.DemandReponseLoadControlClusterPowerSavingsControlStruct.fromTlv(ContextSpecificTag(TAG_POWER_SAVINGS_CONTROL), tlvReader)
      val heatingSourceControl = matter.devicecontroller.cluster.structs.DemandReponseLoadControlClusterHeatingSourceControlStruct.fromTlv(ContextSpecificTag(TAG_HEATING_SOURCE_CONTROL), tlvReader)
      val signature = tlvReader.getByteArray(ContextSpecificTag(TAG_SIGNATURE))
      
      tlvReader.exitContainer()

      return DemandReponseLoadControlClusterLoadControlEventStatusChangeEvent(eventID, transitionIndex, status, criticality, control, temperatureControl, averageLoadControl, dutyCycleControl, powerSavingsControl, heatingSourceControl, signature)
    }
  }
}
