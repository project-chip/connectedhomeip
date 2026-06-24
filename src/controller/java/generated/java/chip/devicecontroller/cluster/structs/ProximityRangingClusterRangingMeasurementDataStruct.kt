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
package chip.devicecontroller.cluster.structs

import chip.devicecontroller.cluster.*
import java.util.Optional
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ProximityRangingClusterRangingMeasurementDataStruct(
  val wiFiDevIK: Optional<ByteArray>,
  val BLEDeviceID: Optional<ULong>,
  val BLTDevIK: Optional<ByteArray>,
  val timeOfMeasurement: Optional<ULong>,
  val timeOfMeasurementOffset: Optional<ULong>,
  val distance: UInt?,
  val errorMargin: Optional<UInt>,
  val rdr: Optional<ProximityRangingClusterRDRStruct>,
  val detectedAttackLevel: Optional<UInt>,
  val rssi: Optional<Int>?,
  val txPower: Optional<Int>?,
) {
  override fun toString(): String = buildString {
    append("ProximityRangingClusterRangingMeasurementDataStruct {\n")
    append("\twiFiDevIK : $wiFiDevIK\n")
    append("\tBLEDeviceID : $BLEDeviceID\n")
    append("\tBLTDevIK : $BLTDevIK\n")
    append("\ttimeOfMeasurement : $timeOfMeasurement\n")
    append("\ttimeOfMeasurementOffset : $timeOfMeasurementOffset\n")
    append("\tdistance : $distance\n")
    append("\terrorMargin : $errorMargin\n")
    append("\trdr : $rdr\n")
    append("\tdetectedAttackLevel : $detectedAttackLevel\n")
    append("\trssi : $rssi\n")
    append("\ttxPower : $txPower\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (wiFiDevIK.isPresent) {
        val optwiFiDevIK = wiFiDevIK.get()
        put(ContextSpecificTag(TAG_WI_FI_DEV_IK), optwiFiDevIK)
      }
      if (BLEDeviceID.isPresent) {
        val optBLEDeviceID = BLEDeviceID.get()
        put(ContextSpecificTag(TAG_BLE_DEVICE_ID), optBLEDeviceID)
      }
      if (BLTDevIK.isPresent) {
        val optBLTDevIK = BLTDevIK.get()
        put(ContextSpecificTag(TAG_BLT_DEV_IK), optBLTDevIK)
      }
      if (timeOfMeasurement.isPresent) {
        val opttimeOfMeasurement = timeOfMeasurement.get()
        put(ContextSpecificTag(TAG_TIME_OF_MEASUREMENT), opttimeOfMeasurement)
      }
      if (timeOfMeasurementOffset.isPresent) {
        val opttimeOfMeasurementOffset = timeOfMeasurementOffset.get()
        put(ContextSpecificTag(TAG_TIME_OF_MEASUREMENT_OFFSET), opttimeOfMeasurementOffset)
      }
      if (distance != null) {
        put(ContextSpecificTag(TAG_DISTANCE), distance)
      } else {
        putNull(ContextSpecificTag(TAG_DISTANCE))
      }
      if (errorMargin.isPresent) {
        val opterrorMargin = errorMargin.get()
        put(ContextSpecificTag(TAG_ERROR_MARGIN), opterrorMargin)
      }
      if (rdr.isPresent) {
        val optrdr = rdr.get()
        optrdr.toTlv(ContextSpecificTag(TAG_RDR), this)
      }
      if (detectedAttackLevel.isPresent) {
        val optdetectedAttackLevel = detectedAttackLevel.get()
        put(ContextSpecificTag(TAG_DETECTED_ATTACK_LEVEL), optdetectedAttackLevel)
      }
      if (rssi != null) {
        if (rssi.isPresent) {
          val optrssi = rssi.get()
          put(ContextSpecificTag(TAG_RSSI), optrssi)
        }
      } else {
        putNull(ContextSpecificTag(TAG_RSSI))
      }
      if (txPower != null) {
        if (txPower.isPresent) {
          val opttxPower = txPower.get()
          put(ContextSpecificTag(TAG_TX_POWER), opttxPower)
        }
      } else {
        putNull(ContextSpecificTag(TAG_TX_POWER))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_WI_FI_DEV_IK = 0
    private const val TAG_BLE_DEVICE_ID = 1
    private const val TAG_BLT_DEV_IK = 2
    private const val TAG_TIME_OF_MEASUREMENT = 3
    private const val TAG_TIME_OF_MEASUREMENT_OFFSET = 4
    private const val TAG_DISTANCE = 5
    private const val TAG_ERROR_MARGIN = 6
    private const val TAG_RDR = 7
    private const val TAG_DETECTED_ATTACK_LEVEL = 8
    private const val TAG_RSSI = 9
    private const val TAG_TX_POWER = 10

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): ProximityRangingClusterRangingMeasurementDataStruct {
      tlvReader.enterStructure(tlvTag)
      val wiFiDevIK =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_WI_FI_DEV_IK))) {
          Optional.of(tlvReader.getByteArray(ContextSpecificTag(TAG_WI_FI_DEV_IK)))
        } else {
          Optional.empty()
        }
      val BLEDeviceID =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_BLE_DEVICE_ID))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_BLE_DEVICE_ID)))
        } else {
          Optional.empty()
        }
      val BLTDevIK =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_BLT_DEV_IK))) {
          Optional.of(tlvReader.getByteArray(ContextSpecificTag(TAG_BLT_DEV_IK)))
        } else {
          Optional.empty()
        }
      val timeOfMeasurement =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_TIME_OF_MEASUREMENT))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_TIME_OF_MEASUREMENT)))
        } else {
          Optional.empty()
        }
      val timeOfMeasurementOffset =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_TIME_OF_MEASUREMENT_OFFSET))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_TIME_OF_MEASUREMENT_OFFSET)))
        } else {
          Optional.empty()
        }
      val distance =
        if (!tlvReader.isNull()) {
          tlvReader.getUInt(ContextSpecificTag(TAG_DISTANCE))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_DISTANCE))
          null
        }
      val errorMargin =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_ERROR_MARGIN))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_ERROR_MARGIN)))
        } else {
          Optional.empty()
        }
      val rdr =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_RDR))) {
          Optional.of(
            ProximityRangingClusterRDRStruct.fromTlv(ContextSpecificTag(TAG_RDR), tlvReader)
          )
        } else {
          Optional.empty()
        }
      val detectedAttackLevel =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_DETECTED_ATTACK_LEVEL))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_DETECTED_ATTACK_LEVEL)))
        } else {
          Optional.empty()
        }
      val rssi =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_RSSI))) {
            Optional.of(tlvReader.getInt(ContextSpecificTag(TAG_RSSI)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_RSSI))
          null
        }
      val txPower =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_TX_POWER))) {
            Optional.of(tlvReader.getInt(ContextSpecificTag(TAG_TX_POWER)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_TX_POWER))
          null
        }

      tlvReader.exitContainer()

      return ProximityRangingClusterRangingMeasurementDataStruct(
        wiFiDevIK,
        BLEDeviceID,
        BLTDevIK,
        timeOfMeasurement,
        timeOfMeasurementOffset,
        distance,
        errorMargin,
        rdr,
        detectedAttackLevel,
        rssi,
        txPower,
      )
    }
  }
}
