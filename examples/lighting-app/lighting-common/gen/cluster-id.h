
/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

// Prevent multiple inclusion
#pragma once

// Definitions for cluster: Basic
#define ZCL_BASIC_CLUSTER_ID (0x0000)

// Definitions for cluster: Power Configuration
#define ZCL_POWER_CONFIG_CLUSTER_ID (0x0001)

// Definitions for cluster: Device Temperature Configuration
#define ZCL_DEVICE_TEMP_CLUSTER_ID (0x0002)

// Definitions for cluster: Identify
#define ZCL_IDENTIFY_CLUSTER_ID (0x0003)

// Definitions for cluster: Groups
#define ZCL_GROUPS_CLUSTER_ID (0x0004)

// Definitions for cluster: Scenes
#define ZCL_SCENES_CLUSTER_ID (0x0005)

// Definitions for cluster: On/off
#define ZCL_ON_OFF_CLUSTER_ID (0x0006)

// Definitions for cluster: On/off Switch Configuration
#define ZCL_ON_OFF_SWITCH_CONFIG_CLUSTER_ID (0x0007)

// Definitions for cluster: Level Control
#define ZCL_LEVEL_CONTROL_CLUSTER_ID (0x0008)

// Definitions for cluster: Alarms
#define ZCL_ALARM_CLUSTER_ID (0x0009)

// Definitions for cluster: Time
#define ZCL_TIME_CLUSTER_ID (0x000A)

// Definitions for cluster: RSSI Location
#define ZCL_RSSI_LOCATION_CLUSTER_ID (0x000B)

// Definitions for cluster: Binary Input (Basic)
#define ZCL_BINARY_INPUT_BASIC_CLUSTER_ID (0x000F)

// Definitions for cluster: Commissioning
#define ZCL_COMMISSIONING_CLUSTER_ID (0x0015)

// Definitions for cluster: Partition
#define ZCL_PARTITION_CLUSTER_ID (0x0016)

// Definitions for cluster: Over the Air Bootloading
#define ZCL_OTA_BOOTLOAD_CLUSTER_ID (0x0019)

// Definitions for cluster: Power Profile
#define ZCL_POWER_PROFILE_CLUSTER_ID (0x001A)

// Definitions for cluster: Appliance Control
#define ZCL_APPLIANCE_CONTROL_CLUSTER_ID (0x001B)

// Definitions for cluster: Poll Control
#define ZCL_POLL_CONTROL_CLUSTER_ID (0x0020)

// Definitions for cluster: Green Power
#define ZCL_GREEN_POWER_CLUSTER_ID (0x0021)

// Definitions for cluster: Keep-Alive
#define ZCL_KEEPALIVE_CLUSTER_ID (0x0025)

// Definitions for cluster: Shade Configuration
#define ZCL_SHADE_CONFIG_CLUSTER_ID (0x0100)

// Definitions for cluster: Door Lock
#define ZCL_DOOR_LOCK_CLUSTER_ID (0x0101)

// Definitions for cluster: Window Covering
#define ZCL_WINDOW_COVERING_CLUSTER_ID (0x0102)

// Definitions for cluster: Barrier Control
#define ZCL_BARRIER_CONTROL_CLUSTER_ID (0x0103)

// Definitions for cluster: Pump Configuration and Control
#define ZCL_PUMP_CONFIG_CONTROL_CLUSTER_ID (0x0200)

// Definitions for cluster: Thermostat
#define ZCL_THERMOSTAT_CLUSTER_ID (0x0201)

// Definitions for cluster: Fan Control
#define ZCL_FAN_CONTROL_CLUSTER_ID (0x0202)

// Definitions for cluster: Dehumidification Control
#define ZCL_DEHUMID_CONTROL_CLUSTER_ID (0x0203)

// Definitions for cluster: Thermostat User Interface Configuration
#define ZCL_THERMOSTAT_UI_CONFIG_CLUSTER_ID (0x0204)

// Definitions for cluster: Color Control
#define ZCL_COLOR_CONTROL_CLUSTER_ID (0x0300)

// Definitions for cluster: Ballast Configuration
#define ZCL_BALLAST_CONFIGURATION_CLUSTER_ID (0x0301)

// Definitions for cluster: Illuminance Measurement
#define ZCL_ILLUM_MEASUREMENT_CLUSTER_ID (0x0400)

// Definitions for cluster: Illuminance Level Sensing
#define ZCL_ILLUM_LEVEL_SENSING_CLUSTER_ID (0x0401)

// Definitions for cluster: Temperature Measurement
#define ZCL_TEMP_MEASUREMENT_CLUSTER_ID (0x0402)

// Definitions for cluster: Pressure Measurement
#define ZCL_PRESSURE_MEASUREMENT_CLUSTER_ID (0x0403)

// Definitions for cluster: Flow Measurement
#define ZCL_FLOW_MEASUREMENT_CLUSTER_ID (0x0404)

// Definitions for cluster: Relative Humidity Measurement
#define ZCL_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID (0x0405)

// Definitions for cluster: Occupancy Sensing
#define ZCL_OCCUPANCY_SENSING_CLUSTER_ID (0x0406)

// Definitions for cluster: Carbon Monoxide Concentration Measurement
#define ZCL_CARBON_MONOXIDE_CONCENTRATION_MEASUREMENT_CLUSTER_ID (0x040C)

// Definitions for cluster: Carbon Dioxide Concentration Measurement
#define ZCL_CARBON_DIOXIDE_CONCENTRATION_MEASUREMENT_CLUSTER_ID (0x040D)

// Definitions for cluster: Ethylene Concentration Measurement
#define ZCL_ETHYLENE_CONCENTRATION_MEASUREMENT_CLUSTER_ID (0x040E)

// Definitions for cluster: Ethylene Oxide Concentration Measurement
#define ZCL_ETHYLENE_OXIDE_CONCENTRATION_MEASUREMENT_CLUSTER_ID (0x040F)

// Definitions for cluster: Hydrogen Concentration Measurement
#define ZCL_HYDROGEN_CONCENTRATION_MEASUREMENT_CLUSTER_ID (0x0410)

// Definitions for cluster: Hydrogen Sulphide Concentration Measurement
#define ZCL_HYDROGEN_SULPHIDE_CONCENTRATION_MEASUREMENT_CLUSTER_ID (0x0411)

// Definitions for cluster: Nitric Oxide Concentration Measurement
#define ZCL_NITRIC_OXIDE_CONCENTRATION_MEASUREMENT_CLUSTER_ID (0x0412)

// Definitions for cluster: Nitrogen Dioxide Concentration Measurement
#define ZCL_NITROGEN_DIOXIDE_CONCENTRATION_MEASUREMENT_CLUSTER_ID (0x0413)

// Definitions for cluster: Oxygen Concentration Measurement
#define ZCL_OXYGEN_CONCENTRATION_MEASUREMENT_CLUSTER_ID (0x0414)

// Definitions for cluster: Ozone Concentration Measurement
#define ZCL_OZONE_CONCENTRATION_MEASUREMENT_CLUSTER_ID (0x0415)

// Definitions for cluster: Sulfur Dioxide Concentration Measurement
#define ZCL_SULFUR_DIOXIDE_CONCENTRATION_MEASUREMENT_CLUSTER_ID (0x0416)

// Definitions for cluster: Dissolved Oxygen Concentration Measurement
#define ZCL_DISSOLVED_OXYGEN_CONCENTRATION_MEASUREMENT_CLUSTER_ID (0x0417)

// Definitions for cluster: Bromate Concentration Measurement
#define ZCL_BROMATE_CONCENTRATION_MEASUREMENT_CLUSTER_ID (0x0418)

// Definitions for cluster: Chloramines Concentration Measurement
#define ZCL_CHLORAMINES_CONCENTRATION_MEASUREMENT_CLUSTER_ID (0x0419)

// Definitions for cluster: Chlorine Concentration Measurement
#define ZCL_CHLORINE_CONCENTRATION_MEASUREMENT_CLUSTER_ID (0x041A)

// Definitions for cluster: Fecal coliform and E. Coli Concentration Measurement
#define ZCL_FECAL_COLIFORM_AND_E_COLI_CONCENTRATION_MEASUREMENT_CLUSTER_ID (0x041B)

// Definitions for cluster: Fluoride Concentration Measurement
#define ZCL_FLUORIDE_CONCENTRATION_MEASUREMENT_CLUSTER_ID (0x041C)

// Definitions for cluster: Haloacetic Acids Concentration Measurement
#define ZCL_HALOACETIC_ACIDS_CONCENTRATION_MEASUREMENT_CLUSTER_ID (0x041D)

// Definitions for cluster: Total Trihalomethanes Concentration Measurement
#define ZCL_TOTAL_TRIHALOMETHANES_CONCENTRATION_MEASUREMENT_CLUSTER_ID (0x041E)

// Definitions for cluster: Total Coliform Bacteria Concentration Measurement
#define ZCL_TOTAL_COLIFORM_BACTERIA_CONCENTRATION_MEASUREMENT_CLUSTER_ID (0x041F)

// Definitions for cluster: Turbidity Concentration Measurement
#define ZCL_TURBIDITY_CONCENTRATION_MEASUREMENT_CLUSTER_ID (0x0420)

// Definitions for cluster: Copper Concentration Measurement
#define ZCL_COPPER_CONCENTRATION_MEASUREMENT_CLUSTER_ID (0x0421)

// Definitions for cluster: Lead Concentration Measurement
#define ZCL_LEAD_CONCENTRATION_MEASUREMENT_CLUSTER_ID (0x0422)

// Definitions for cluster: Manganese Concentration Measurement
#define ZCL_MANGANESE_CONCENTRATION_MEASUREMENT_CLUSTER_ID (0x0423)

// Definitions for cluster: Sulfate Concentration Measurement
#define ZCL_SULFATE_CONCENTRATION_MEASUREMENT_CLUSTER_ID (0x0424)

// Definitions for cluster: Bromodichloromethane Concentration Measurement
#define ZCL_BROMODICHLOROMETHANE_CONCENTRATION_MEASUREMENT_CLUSTER_ID (0x0425)

// Definitions for cluster: Bromoform Concentration Measurement
#define ZCL_BROMOFORM_CONCENTRATION_MEASUREMENT_CLUSTER_ID (0x0426)

// Definitions for cluster: Chlorodibromomethane Concentration Measurement
#define ZCL_CHLORODIBROMOMETHANE_CONCENTRATION_MEASUREMENT_CLUSTER_ID (0x0427)

// Definitions for cluster: Chloroform Concentration Measurement
#define ZCL_CHLOROFORM_CONCENTRATION_MEASUREMENT_CLUSTER_ID (0x0428)

// Definitions for cluster: Sodium Concentration Measurement
#define ZCL_SODIUM_CONCENTRATION_MEASUREMENT_CLUSTER_ID (0x0429)

// Definitions for cluster: IAS Zone
#define ZCL_IAS_ZONE_CLUSTER_ID (0x0500)

// Definitions for cluster: IAS ACE
#define ZCL_IAS_ACE_CLUSTER_ID (0x0501)

// Definitions for cluster: IAS WD
#define ZCL_IAS_WD_CLUSTER_ID (0x0502)

// Definitions for cluster: Generic Tunnel
#define ZCL_GENERIC_TUNNEL_CLUSTER_ID (0x0600)

// Definitions for cluster: BACnet Protocol Tunnel
#define ZCL_BACNET_PROTOCOL_TUNNEL_CLUSTER_ID (0x0601)

// Definitions for cluster: 11073 Protocol Tunnel
#define ZCL_11073_PROTOCOL_TUNNEL_CLUSTER_ID (0x0614)

// Definitions for cluster: ISO 7816 Protocol Tunnel
#define ZCL_ISO7816_PROTOCOL_TUNNEL_CLUSTER_ID (0x0615)

// Definitions for cluster: Price
#define ZCL_PRICE_CLUSTER_ID (0x0700)

// Definitions for cluster: Demand Response and Load Control
#define ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID (0x0701)

// Definitions for cluster: Simple Metering
#define ZCL_SIMPLE_METERING_CLUSTER_ID (0x0702)

// Definitions for cluster: Messaging
#define ZCL_MESSAGING_CLUSTER_ID (0x0703)

// Definitions for cluster: Tunneling
#define ZCL_TUNNELING_CLUSTER_ID (0x0704)

// Definitions for cluster: Prepayment
#define ZCL_PREPAYMENT_CLUSTER_ID (0x0705)

// Definitions for cluster: Energy Management
#define ZCL_ENERGY_MANAGEMENT_CLUSTER_ID (0x0706)

// Definitions for cluster: Calendar
#define ZCL_CALENDAR_CLUSTER_ID (0x0707)

// Definitions for cluster: Device Management
#define ZCL_DEVICE_MANAGEMENT_CLUSTER_ID (0x0708)

// Definitions for cluster: Events
#define ZCL_EVENTS_CLUSTER_ID (0x0709)

// Definitions for cluster: MDU Pairing
#define ZCL_MDU_PAIRING_CLUSTER_ID (0x070A)

// Definitions for cluster: Sub-GHz
#define ZCL_SUB_GHZ_CLUSTER_ID (0x070B)

// Definitions for cluster: Key Establishment
#define ZCL_KEY_ESTABLISHMENT_CLUSTER_ID (0x0800)

// Definitions for cluster: Information
#define ZCL_INFORMATION_CLUSTER_ID (0x0900)

// Definitions for cluster: Data Sharing
#define ZCL_DATA_SHARING_CLUSTER_ID (0x0901)

// Definitions for cluster: Gaming
#define ZCL_GAMING_CLUSTER_ID (0x0902)

// Definitions for cluster: Data Rate Control
#define ZCL_DATA_RATE_CONTROL_CLUSTER_ID (0x0903)

// Definitions for cluster: Voice over ZigBee
#define ZCL_VOICE_OVER_ZIGBEE_CLUSTER_ID (0x0904)

// Definitions for cluster: Chatting
#define ZCL_CHATTING_CLUSTER_ID (0x0905)

// Definitions for cluster: Payment
#define ZCL_PAYMENT_CLUSTER_ID (0x0A01)

// Definitions for cluster: Billing
#define ZCL_BILLING_CLUSTER_ID (0x0A02)

// Definitions for cluster: Appliance Identification
#define ZCL_APPLIANCE_IDENTIFICATION_CLUSTER_ID (0x0B00)

// Definitions for cluster: Meter Identification
#define ZCL_METER_IDENTIFICATION_CLUSTER_ID (0x0B01)

// Definitions for cluster: Appliance Events and Alert
#define ZCL_APPLIANCE_EVENTS_AND_ALERT_CLUSTER_ID (0x0B02)

// Definitions for cluster: Appliance Statistics
#define ZCL_APPLIANCE_STATISTICS_CLUSTER_ID (0x0B03)

// Definitions for cluster: Electrical Measurement
#define ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID (0x0B04)

// Definitions for cluster: Diagnostics
#define ZCL_DIAGNOSTICS_CLUSTER_ID (0x0B05)

// Definitions for cluster: ZLL Commissioning
#define ZCL_ZLL_COMMISSIONING_CLUSTER_ID (0x1000)

// Definitions for cluster: Sample Mfg Specific Cluster
#define ZCL_SAMPLE_MFG_SPECIFIC_CLUSTER_ID (0xFC00)

// Definitions for cluster: Sample Mfg Specific Cluster 2
#define ZCL_SAMPLE_MFG_SPECIFIC_CLUSTER_2_ID (0xFC00)

// Definitions for cluster: Configuration Cluster
#define ZCL_OTA_CONFIGURATION_CLUSTER_ID (0xFC01)

// Definitions for cluster: MFGLIB Cluster
#define ZCL_MFGLIB_CLUSTER_ID (0xFC02)

// Definitions for cluster: SL Works With All Hubs
#define ZCL_SL_WWAH_CLUSTER_ID (0xFC57)
