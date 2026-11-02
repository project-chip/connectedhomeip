/*
 *   Copyright (c) 2026 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package com.google.chip.chiptool.devicemanagement

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertNull
import org.junit.Assert.assertTrue
import org.junit.Test

class MatterClusterMetaRegistryTest {

  @Test
  fun testRegistryContainsAllSevenClusterFamilies() {
    val lighting =
      MatterClusterMetaRegistry.getClustersByCategory(ClusterCategory.LIGHTING_AND_POWER)
    assertTrue("Lighting clusters should not be empty", lighting.isNotEmpty())
    assertNotNull(MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_ON_OFF))
    assertNotNull(
      MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_LEVEL_CONTROL)
    )
    assertNotNull(
      MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_COLOR_CONTROL)
    )

    val closures = MatterClusterMetaRegistry.getClustersByCategory(ClusterCategory.CLOSURES)
    assertTrue("Closure clusters should not be empty", closures.isNotEmpty())
    assertNotNull(MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_DOOR_LOCK))
    assertNotNull(
      MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_WINDOW_COVERING)
    )
    assertNotNull(
      MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_BARRIER_CONTROL)
    )

    val hvac = MatterClusterMetaRegistry.getClustersByCategory(ClusterCategory.HVAC)
    assertTrue("HVAC clusters should not be empty", hvac.isNotEmpty())
    assertNotNull(
      MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_THERMOSTAT)
    )
    assertNotNull(
      MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_FAN_CONTROL)
    )

    val media = MatterClusterMetaRegistry.getClustersByCategory(ClusterCategory.MEDIA)
    assertTrue("Media clusters should not be empty", media.isNotEmpty())
    assertNotNull(
      MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_MEDIA_PLAYBACK)
    )
    assertNotNull(
      MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_KEYPAD_INPUT)
    )
    assertNotNull(
      MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_AUDIO_OUTPUT)
    )

    val appliances =
      MatterClusterMetaRegistry.getClustersByCategory(ClusterCategory.APPLIANCES_AND_ROBOTICS)
    assertTrue("Appliance clusters should not be empty", appliances.isNotEmpty())
    assertNotNull(
      MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_RVC_RUN_MODE)
    )
    assertNotNull(
      MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_RVC_CLEAN_MODE)
    )
    assertNotNull(
      MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_OPERATIONAL_STATE)
    )
    assertNotNull(
      MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_MODE_SELECT)
    )

    val energy = MatterClusterMetaRegistry.getClustersByCategory(ClusterCategory.ENERGY_MANAGEMENT)
    assertTrue("Energy clusters should not be empty", energy.isNotEmpty())
    assertNotNull(
      MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_ENERGY_EVSE)
    )
    assertNotNull(
      MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_DEVICE_ENERGY_MGMT)
    )

    val sensors =
      MatterClusterMetaRegistry.getClustersByCategory(ClusterCategory.SENSORS_AND_ALARMS)
    assertTrue("Sensor clusters should not be empty", sensors.isNotEmpty())
    assertNotNull(
      MatterClusterMetaRegistry.getCluster(
        MatterClusterMetaRegistry.CLUSTER_TEMPERATURE_MEASUREMENT
      )
    )
    assertNotNull(
      MatterClusterMetaRegistry.getCluster(
        MatterClusterMetaRegistry.CLUSTER_RELATIVE_HUMIDITY_MEASUREMENT
      )
    )
    assertNotNull(
      MatterClusterMetaRegistry.getCluster(
        MatterClusterMetaRegistry.CLUSTER_ILLUMINANCE_MEASUREMENT
      )
    )
    assertNotNull(
      MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_OCCUPANCY_SENSING)
    )
    assertNotNull(
      MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_BOOLEAN_STATE)
    )
    assertNotNull(
      MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_SMOKE_CO_ALARM)
    )

    val levelControl =
      MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_LEVEL_CONTROL)!!
    assertNotNull("LevelControl should contain StepWithOnOff (0x05)", levelControl.commands[0x05L])
    assertEquals("StepWithOnOff", levelControl.commands[0x05L]?.name)
  }

  @Test
  fun testClusterLookupBySpecName() {
    val onOff = MatterClusterMetaRegistry.getClusterByName("OnOff")
    assertNotNull(onOff)
    assertEquals(MatterClusterMetaRegistry.CLUSTER_ON_OFF, onOff?.clusterId)

    val covering = MatterClusterMetaRegistry.getClusterByName("window_covering")
    assertNotNull(covering)
    assertEquals(MatterClusterMetaRegistry.CLUSTER_WINDOW_COVERING, covering?.clusterId)

    // Everyday words are not spec names; the registry does not guess.
    assertNull(MatterClusterMetaRegistry.getClusterByName("blinds"))
  }

  @Test
  fun testKelvinToMiredsConversions() {
    // 2700K Warm White -> ~370 Mireds
    assertEquals(370, MatterClusterMetaRegistry.kelvinToMireds(2700))
    // 4000K Neutral White -> 250 Mireds
    assertEquals(250, MatterClusterMetaRegistry.kelvinToMireds(4000))
    // 6500K Daylight -> ~154 Mireds
    assertEquals(154, MatterClusterMetaRegistry.kelvinToMireds(6500))

    // Inverse
    assertEquals(2703, MatterClusterMetaRegistry.miredsToKelvin(370))
    assertEquals(4000, MatterClusterMetaRegistry.miredsToKelvin(250))
    assertEquals(6494, MatterClusterMetaRegistry.miredsToKelvin(154))
  }

  @Test
  fun testCelsiusToCentidegreeConversions() {
    assertEquals(2150.toShort(), MatterClusterMetaRegistry.celsiusToCentidegrees(21.5))
    assertEquals((-500).toShort(), MatterClusterMetaRegistry.celsiusToCentidegrees(-5.0))
    assertEquals(21.5, MatterClusterMetaRegistry.centidegreesToCelsius(2150.toShort()), 0.001)
  }

  @Test
  fun testPercentageToLevelConversions() {
    assertEquals(0.toUByte(), MatterClusterMetaRegistry.percentageToLevel(0.0))
    assertEquals(127.toUByte(), MatterClusterMetaRegistry.percentageToLevel(50.0))
    assertEquals(254.toUByte(), MatterClusterMetaRegistry.percentageToLevel(100.0))

    assertEquals(0.0, MatterClusterMetaRegistry.levelToPercentage(0.toUByte()), 0.5)
    assertEquals(50.0, MatterClusterMetaRegistry.levelToPercentage(127.toUByte()), 0.5)
    assertEquals(100.0, MatterClusterMetaRegistry.levelToPercentage(254.toUByte()), 0.5)
  }

  @Test
  fun testSwitchAndPowerSourceClusterDefinitionsAndBatteryConversion() {
    val switchCluster =
      MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_SWITCH)
    assertNotNull("Switch cluster (0x003B) must be registered", switchCluster)
    assertEquals(0x003BL, switchCluster!!.clusterId)
    assertEquals("NumberOfPositions", switchCluster.attributes[0x0000L]?.name)
    assertEquals("CurrentPosition", switchCluster.attributes[0x0001L]?.name)
    assertEquals("MultiPressMax", switchCluster.attributes[0x0002L]?.name)
    assertEquals("SwitchLatched", switchCluster.events[0x00L]?.name)
    assertEquals("InitialPress", switchCluster.events[0x01L]?.name)
    assertEquals("LongPress", switchCluster.events[0x02L]?.name)
    assertEquals("ShortRelease", switchCluster.events[0x03L]?.name)
    assertEquals("LongRelease", switchCluster.events[0x04L]?.name)
    assertEquals("MultiPressOngoing", switchCluster.events[0x05L]?.name)
    assertEquals("MultiPressComplete", switchCluster.events[0x06L]?.name)

    val powerSource =
      MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_POWER_SOURCE)
    assertNotNull("Power Source cluster (0x002F) must be registered", powerSource)
    assertEquals(0x002FL, powerSource!!.clusterId)
    assertEquals("Status", powerSource.attributes[0x0000L]?.name)
    assertEquals("BatPercentRemaining", powerSource.attributes[0x000CL]?.name)
    assertEquals("BatChargeLevel", powerSource.attributes[0x000EL]?.name)

    // BatPercentRemaining is in half-percent units (0..200 -> 0..100%)
    assertEquals(87, MatterClusterMetaRegistry.batHalfPercentToPercent(174))
    assertEquals(100, MatterClusterMetaRegistry.batHalfPercentToPercent(200))
    assertEquals(0, MatterClusterMetaRegistry.batHalfPercentToPercent(0))
  }

  @Test
  fun testMeasurementMinMaxAreTypedAndScaledLikeMeasuredValue() {
    val temp =
      checkNotNull(
        MatterClusterMetaRegistry.getCluster(
          MatterClusterMetaRegistry.CLUSTER_TEMPERATURE_MEASUREMENT
        )
      )
    val tempMin = checkNotNull(temp.attributes[0x0001L])
    val tempMax = checkNotNull(temp.attributes[0x0002L])
    assertEquals(MatterDataType.INT16, tempMin.type)
    assertEquals(MatterDataType.INT16, tempMax.type)
    assertEquals("-40.0°C", UniversalTlvEncoder.formatDecodedValue(-4000, tempMin))
    assertEquals("125.0°C", UniversalTlvEncoder.formatDecodedValue(12500, tempMax))

    val humidity =
      checkNotNull(
        MatterClusterMetaRegistry.getCluster(
          MatterClusterMetaRegistry.CLUSTER_RELATIVE_HUMIDITY_MEASUREMENT
        )
      )
    val humMax = checkNotNull(humidity.attributes[0x0002L])
    assertEquals(MatterDataType.UINT16, humMax.type)
    assertEquals("100.0%", UniversalTlvEncoder.formatDecodedValue(10000, humMax))

    val lux =
      checkNotNull(
        MatterClusterMetaRegistry.getCluster(
          MatterClusterMetaRegistry.CLUSTER_ILLUMINANCE_MEASUREMENT
        )
      )
    val luxValue = checkNotNull(lux.attributes[0x0000L])
    assertEquals(MatterDataType.UINT16, luxValue.type)
    assertEquals("10 lx", UniversalTlvEncoder.formatDecodedValue(10001, luxValue))
    assertEquals("<1 lx", UniversalTlvEncoder.formatDecodedValue(0, luxValue))
  }

  @Test
  fun testOnOffAttributeIsReadOnly() {
    // onoff-cluster.xml does not mark OnOff (0x0006/0x0000) writable.
    val onOff = MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_ON_OFF)
    assertFalse(checkNotNull(onOff?.attributes?.get(0x0000L)).isWritable)
  }

  @Test
  fun testKeypadInputKeyCodesMatchCecKeyCodeEnum() {
    val keypad =
      MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_KEYPAD_INPUT)
    val keyCode = checkNotNull(keypad?.commands?.get(0x00L)).requestFields.first { it.tagId == 0 }
    assertEquals(9L, keyCode.enumValues["RootMenu"])
    assertEquals(10L, keyCode.enumValues["SetupMenu"])
    assertEquals(13L, keyCode.enumValues["Exit"])
    assertFalse(keyCode.enumValues.values.contains(24L))
  }

  @Test
  fun testClusterScopedAttributeTypeInferenceAndThermostatUnits() {
    val levelControl =
      checkNotNull(
        MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_LEVEL_CONTROL)
      )
    val startUpCurrentLevel = checkNotNull(levelControl.attributes[0x4000L])
    assertEquals(MatterDataType.UINT8, startUpCurrentLevel.type)

    val colorControl =
      checkNotNull(
        MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_COLOR_CONTROL)
      )
    val startUpMireds = checkNotNull(colorControl.attributes[0x4010L])
    assertEquals(MatterDataType.UINT16, startUpMireds.type)

    val thermostat =
      checkNotNull(
        MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_THERMOSTAT)
      )
    val minHeatLimit = checkNotNull(thermostat.attributes[0x0003L])
    assertEquals(MatterDataType.INT16, minHeatLimit.type)
    assertEquals("°C", minHeatLimit.unit)
    assertEquals("7.0°C", UniversalTlvEncoder.formatDecodedValue(700, minHeatLimit))

    val setpointChangeSource = thermostat.attributes[0x0030L]
    if (setpointChangeSource != null) {
      assertEquals(null, setpointChangeSource.unit)
    }
    val localTempCalibration = thermostat.attributes[0x0010L]
    if (localTempCalibration != null) {
      assertEquals(null, localTempCalibration.unit)
    }

    val descriptor =
      checkNotNull(
        MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_DESCRIPTOR)
      )
    assertEquals(MatterDataType.ARRAY, checkNotNull(descriptor.attributes[0x0000L]).type)
    assertEquals(MatterDataType.ARRAY, checkNotNull(descriptor.attributes[0x0001L]).type)
    assertEquals(MatterDataType.ARRAY, checkNotNull(descriptor.attributes[0x0002L]).type)
    assertEquals(MatterDataType.ARRAY, checkNotNull(descriptor.attributes[0x0003L]).type)
  }
}
