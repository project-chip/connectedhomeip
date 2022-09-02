/***************************************************************************//**
 * @file  zap-command-structs.h
 * @brief This file contains a set of generated command structs for all incoming
 commands to handle incomming zcl messages.
 * This file is generated. Do not update file manually.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#ifndef __ZAP_COMMAND_STRUCTS_H__
#define __ZAP_COMMAND_STRUCTS_H__ 1

/**
 * @defgroup zcl_command_structs ZCL Command Structures
 * @ingroup command
 * @brief Application Framework command structs reference
 * 
 * This document describes the generated command structs that 
 * are used to hold data when over-the-air command payload is parsed.
 */

/**
 *
 * @addtogroup zcl_command_structs
 * @{
 */

#include "sl_component_catalog.h"
#ifdef SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT

#include "sl_signature_decode.h"

#define ZAP_SIGNATURE_TYPE_INT8U        (SL_SIGNATURE_FIELD_1_BYTE)
#define ZAP_SIGNATURE_TYPE_INT8S        (SL_SIGNATURE_FIELD_1_BYTE)
#define ZAP_SIGNATURE_TYPE_ENUM8        (SL_SIGNATURE_FIELD_1_BYTE)
#define ZAP_SIGNATURE_TYPE_BITMAP8      (SL_SIGNATURE_FIELD_1_BYTE)
#define ZAP_SIGNATURE_TYPE_BOOLEAN      (SL_SIGNATURE_FIELD_1_BYTE)

#define ZAP_SIGNATURE_TYPE_INT16U       (SL_SIGNATURE_FIELD_2_BYTES)
#define ZAP_SIGNATURE_TYPE_DATA16       (SL_SIGNATURE_FIELD_2_BYTES)
#define ZAP_SIGNATURE_TYPE_INT16S       (SL_SIGNATURE_FIELD_2_BYTES)
#define ZAP_SIGNATURE_TYPE_ENUM16       (SL_SIGNATURE_FIELD_2_BYTES)
#define ZAP_SIGNATURE_TYPE_BITMAP16     (SL_SIGNATURE_FIELD_2_BYTES)
#define ZAP_SIGNATURE_TYPE_CLUSTER_ID   (SL_SIGNATURE_FIELD_2_BYTES)

#define ZAP_SIGNATURE_TYPE_INT24U    (SL_SIGNATURE_FIELD_3_BYTES)
#define ZAP_SIGNATURE_TYPE_ENUM24    (SL_SIGNATURE_FIELD_3_BYTES)
#define ZAP_SIGNATURE_TYPE_BITMAP24  (SL_SIGNATURE_FIELD_3_BYTES)

#define ZAP_SIGNATURE_TYPE_INT32U       (SL_SIGNATURE_FIELD_4_BYTES)
#define ZAP_SIGNATURE_TYPE_INT32S       (SL_SIGNATURE_FIELD_4_BYTES)
#define ZAP_SIGNATURE_TYPE_ENUM32       (SL_SIGNATURE_FIELD_4_BYTES)
#define ZAP_SIGNATURE_TYPE_BITMAP32     (SL_SIGNATURE_FIELD_4_BYTES)
#define ZAP_SIGNATURE_TYPE_UTC_TIME     (SL_SIGNATURE_FIELD_4_BYTES)
#define ZAP_SIGNATURE_TYPE_TIME_OF_DAY  (SL_SIGNATURE_FIELD_4_BYTES)

#define ZAP_SIGNATURE_TYPE_OCTET_STRING  (SL_SIGNATURE_FIELD_STRING)
#define ZAP_SIGNATURE_TYPE_CHAR_STRING   (SL_SIGNATURE_FIELD_STRING)
#define ZAP_SIGNATURE_TYPE_LONG_STRING   (SL_SIGNATURE_FIELD_LONG_STRING)
#define ZAP_SIGNATURE_TYPE_ARRAY         (SL_SIGNATURE_FIELD_POINTER)
#define ZAP_SIGNATURE_TYPE_POINTER       (SL_SIGNATURE_FIELD_POINTER)

// TODO: These need to be validated. Not sure we copy them out.
#define ZAP_SIGNATURE_TYPE_IEEE_ADDRESS (SL_SIGNATURE_FIELD_MASK_BLOB | 0x08)
#define ZAP_SIGNATURE_TYPE_SECURITY_KEY (SL_SIGNATURE_FIELD_MASK_BLOB | 0x10)


/** @brief Structure for ZCL command "GetZoneIdMapResponse" from "IAS ACE" 
 */
typedef struct __zcl_ias_ace_cluster_get_zone_id_map_response_command {
  uint16_t section0;
  uint16_t section1;
  uint16_t section2;
  uint16_t section3;
  uint16_t section4;
  uint16_t section5;
  uint16_t section6;
  uint16_t section7;
  uint16_t section8;
  uint16_t section9;
  uint16_t section10;
  uint16_t section11;
  uint16_t section12;
  uint16_t section13;
  uint16_t section14;
  uint16_t section15;
} sl_zcl_ias_ace_cluster_get_zone_id_map_response_command_t;

#define sl_zcl_ias_ace_cluster_get_zone_id_map_response_command_signature  { \
  16, \
  (ZAP_SIGNATURE_TYPE_BITMAP16), offsetof(sl_zcl_ias_ace_cluster_get_zone_id_map_response_command_t, section0), \
  (ZAP_SIGNATURE_TYPE_BITMAP16), offsetof(sl_zcl_ias_ace_cluster_get_zone_id_map_response_command_t, section1), \
  (ZAP_SIGNATURE_TYPE_BITMAP16), offsetof(sl_zcl_ias_ace_cluster_get_zone_id_map_response_command_t, section2), \
  (ZAP_SIGNATURE_TYPE_BITMAP16), offsetof(sl_zcl_ias_ace_cluster_get_zone_id_map_response_command_t, section3), \
  (ZAP_SIGNATURE_TYPE_BITMAP16), offsetof(sl_zcl_ias_ace_cluster_get_zone_id_map_response_command_t, section4), \
  (ZAP_SIGNATURE_TYPE_BITMAP16), offsetof(sl_zcl_ias_ace_cluster_get_zone_id_map_response_command_t, section5), \
  (ZAP_SIGNATURE_TYPE_BITMAP16), offsetof(sl_zcl_ias_ace_cluster_get_zone_id_map_response_command_t, section6), \
  (ZAP_SIGNATURE_TYPE_BITMAP16), offsetof(sl_zcl_ias_ace_cluster_get_zone_id_map_response_command_t, section7), \
  (ZAP_SIGNATURE_TYPE_BITMAP16), offsetof(sl_zcl_ias_ace_cluster_get_zone_id_map_response_command_t, section8), \
  (ZAP_SIGNATURE_TYPE_BITMAP16), offsetof(sl_zcl_ias_ace_cluster_get_zone_id_map_response_command_t, section9), \
  (ZAP_SIGNATURE_TYPE_BITMAP16), offsetof(sl_zcl_ias_ace_cluster_get_zone_id_map_response_command_t, section10), \
  (ZAP_SIGNATURE_TYPE_BITMAP16), offsetof(sl_zcl_ias_ace_cluster_get_zone_id_map_response_command_t, section11), \
  (ZAP_SIGNATURE_TYPE_BITMAP16), offsetof(sl_zcl_ias_ace_cluster_get_zone_id_map_response_command_t, section12), \
  (ZAP_SIGNATURE_TYPE_BITMAP16), offsetof(sl_zcl_ias_ace_cluster_get_zone_id_map_response_command_t, section13), \
  (ZAP_SIGNATURE_TYPE_BITMAP16), offsetof(sl_zcl_ias_ace_cluster_get_zone_id_map_response_command_t, section14), \
  (ZAP_SIGNATURE_TYPE_BITMAP16), offsetof(sl_zcl_ias_ace_cluster_get_zone_id_map_response_command_t, section15), \
}


/** @brief Structure for ZCL command "ZoneStatusChangeNotification" from "IAS Zone" 
 */
typedef struct __zcl_ias_zone_cluster_zone_status_change_notification_command {
  uint16_t zoneStatus;
  uint8_t extendedStatus;
  uint8_t zoneId;
  uint16_t delay;
} sl_zcl_ias_zone_cluster_zone_status_change_notification_command_t;

#define sl_zcl_ias_zone_cluster_zone_status_change_notification_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_BITMAP16), offsetof(sl_zcl_ias_zone_cluster_zone_status_change_notification_command_t, zoneStatus), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_ias_zone_cluster_zone_status_change_notification_command_t, extendedStatus), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_ias_zone_cluster_zone_status_change_notification_command_t, zoneId), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_ias_zone_cluster_zone_status_change_notification_command_t, delay), \
}


/** @brief Structure for ZCL command "InitiateKeyEstablishmentRequest" from "Key Establishment" 
 */
typedef struct __zcl_key_establishment_cluster_initiate_key_establishment_request_command {
  uint16_t keyEstablishmentSuite;
  uint8_t ephemeralDataGenerateTime;
  uint8_t confirmKeyGenerateTime;
  /* TYPE WARNING: Identity defaults to */ uint8_t * identity;
} sl_zcl_key_establishment_cluster_initiate_key_establishment_request_command_t;

#define sl_zcl_key_establishment_cluster_initiate_key_establishment_request_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_BITMAP16), offsetof(sl_zcl_key_establishment_cluster_initiate_key_establishment_request_command_t, keyEstablishmentSuite), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_key_establishment_cluster_initiate_key_establishment_request_command_t, ephemeralDataGenerateTime), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_key_establishment_cluster_initiate_key_establishment_request_command_t, confirmKeyGenerateTime), \
  (ZAP_SIGNATURE_TYPE_POINTER), offsetof(sl_zcl_key_establishment_cluster_initiate_key_establishment_request_command_t, identity), \
}


/** @brief Structure for ZCL command "InitiateKeyEstablishmentResponse" from "Key Establishment" 
 */
typedef struct __zcl_key_establishment_cluster_initiate_key_establishment_response_command {
  uint16_t requestedKeyEstablishmentSuite;
  uint8_t ephemeralDataGenerateTime;
  uint8_t confirmKeyGenerateTime;
  /* TYPE WARNING: Identity defaults to */ uint8_t * identity;
} sl_zcl_key_establishment_cluster_initiate_key_establishment_response_command_t;

#define sl_zcl_key_establishment_cluster_initiate_key_establishment_response_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_BITMAP16), offsetof(sl_zcl_key_establishment_cluster_initiate_key_establishment_response_command_t, requestedKeyEstablishmentSuite), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_key_establishment_cluster_initiate_key_establishment_response_command_t, ephemeralDataGenerateTime), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_key_establishment_cluster_initiate_key_establishment_response_command_t, confirmKeyGenerateTime), \
  (ZAP_SIGNATURE_TYPE_POINTER), offsetof(sl_zcl_key_establishment_cluster_initiate_key_establishment_response_command_t, identity), \
}


/** @brief Structure for ZCL command "TakeSnapshot" from "Simple Metering" 
 */
typedef struct __zcl_simple_metering_cluster_take_snapshot_command {
  uint32_t snapshotCause;
} sl_zcl_simple_metering_cluster_take_snapshot_command_t;

#define sl_zcl_simple_metering_cluster_take_snapshot_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_BITMAP32), offsetof(sl_zcl_simple_metering_cluster_take_snapshot_command_t, snapshotCause), \
}


/** @brief Structure for ZCL command "ActionControl" from "Gaming" 
 */
typedef struct __zcl_gaming_cluster_action_control_command {
  uint32_t actions;
} sl_zcl_gaming_cluster_action_control_command_t;

#define sl_zcl_gaming_cluster_action_control_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_BITMAP32), offsetof(sl_zcl_gaming_cluster_action_control_command_t, actions), \
}


/** @brief Structure for ZCL command "EZModeInvoke" from "Identify" 
 */
typedef struct __zcl_identify_cluster_ez_mode_invoke_command {
  uint8_t action;
} sl_zcl_identify_cluster_ez_mode_invoke_command_t;

#define sl_zcl_identify_cluster_ez_mode_invoke_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_identify_cluster_ez_mode_invoke_command_t, action), \
}


/** @brief Structure for ZCL command "RssiPing" from "RSSI Location" 
 */
typedef struct __zcl_rssi_location_cluster_rssi_ping_command {
  uint8_t locationType;
} sl_zcl_rssi_location_cluster_rssi_ping_command_t;

#define sl_zcl_rssi_location_cluster_rssi_ping_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_rssi_location_cluster_rssi_ping_command_t, locationType), \
}


/** @brief Structure for ZCL command "Squawk" from "IAS WD" 
 */
typedef struct __zcl_ias_wd_cluster_squawk_command {
  uint8_t squawkInfo;
} sl_zcl_ias_wd_cluster_squawk_command_t;

#define sl_zcl_ias_wd_cluster_squawk_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_ias_wd_cluster_squawk_command_t, squawkInfo), \
}


/** @brief Structure for ZCL command "GetCurrentPrice" from "Price" 
 */
typedef struct __zcl_price_cluster_get_current_price_command {
  uint8_t commandOptions;
} sl_zcl_price_cluster_get_current_price_command_t;

#define sl_zcl_price_cluster_get_current_price_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_price_cluster_get_current_price_command_t, commandOptions), \
}


/** @brief Structure for ZCL command "CancelAllLoadControlEvents" from "Demand Response and Load Control" 
 */
typedef struct __zcl_demand_response_and_load_control_cluster_cancel_all_load_control_events_command {
  uint8_t cancelControl;
} sl_zcl_demand_response_and_load_control_cluster_cancel_all_load_control_events_command_t;

#define sl_zcl_demand_response_and_load_control_cluster_cancel_all_load_control_events_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_demand_response_and_load_control_cluster_cancel_all_load_control_events_command_t, cancelControl), \
}


/** @brief Structure for ZCL command "ClearEventLogResponse" from "Events" 
 */
typedef struct __zcl_events_cluster_clear_event_log_response_command {
  uint8_t clearedEventsLogs;
} sl_zcl_events_cluster_clear_event_log_response_command_t;

#define sl_zcl_events_cluster_clear_event_log_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_events_cluster_clear_event_log_response_command_t, clearedEventsLogs), \
}


/** @brief Structure for ZCL command "GetAlertsResponse" from "Appliance Events and Alert" 
 */
typedef struct __zcl_appliance_events_and_alert_cluster_get_alerts_response_command {
  uint8_t alertsCount;
  /* TYPE WARNING: AlertStructure array defaults to */ uint8_t * alertStructures;
} sl_zcl_appliance_events_and_alert_cluster_get_alerts_response_command_t;

#define sl_zcl_appliance_events_and_alert_cluster_get_alerts_response_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_appliance_events_and_alert_cluster_get_alerts_response_command_t, alertsCount), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_appliance_events_and_alert_cluster_get_alerts_response_command_t, alertStructures), \
}


/** @brief Structure for ZCL command "AlertsNotification" from "Appliance Events and Alert" 
 */
typedef struct __zcl_appliance_events_and_alert_cluster_alerts_notification_command {
  uint8_t alertsCount;
  /* TYPE WARNING: AlertStructure array defaults to */ uint8_t * alertStructures;
} sl_zcl_appliance_events_and_alert_cluster_alerts_notification_command_t;

#define sl_zcl_appliance_events_and_alert_cluster_alerts_notification_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_appliance_events_and_alert_cluster_alerts_notification_command_t, alertsCount), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_appliance_events_and_alert_cluster_alerts_notification_command_t, alertStructures), \
}


/** @brief Structure for ZCL command "GetWeeklySchedule" from "Thermostat" 
 */
typedef struct __zcl_thermostat_cluster_get_weekly_schedule_command {
  uint8_t daysToReturn;
  uint8_t modeToReturn;
} sl_zcl_thermostat_cluster_get_weekly_schedule_command_t;

#define sl_zcl_thermostat_cluster_get_weekly_schedule_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_thermostat_cluster_get_weekly_schedule_command_t, daysToReturn), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_thermostat_cluster_get_weekly_schedule_command_t, modeToReturn), \
}


/** @brief Structure for ZCL command "EstablishmentRequest" from "Voice over ZigBee" 
 */
typedef struct __zcl_voice_over_zig_bee_cluster_establishment_request_command {
  uint8_t flag;
  uint8_t codecType;
  uint8_t sampFreq;
  uint8_t codecRate;
  uint8_t serviceType;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * buffer;
} sl_zcl_voice_over_zig_bee_cluster_establishment_request_command_t;

#define sl_zcl_voice_over_zig_bee_cluster_establishment_request_command_signature  { \
  6, \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_voice_over_zig_bee_cluster_establishment_request_command_t, flag), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_voice_over_zig_bee_cluster_establishment_request_command_t, codecType), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_voice_over_zig_bee_cluster_establishment_request_command_t, sampFreq), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_voice_over_zig_bee_cluster_establishment_request_command_t, codecRate), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_voice_over_zig_bee_cluster_establishment_request_command_t, serviceType), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_voice_over_zig_bee_cluster_establishment_request_command_t, buffer), \
}


/** @brief Structure for ZCL command "ColorLoopSet" from "Color Control" 
 */
typedef struct __zcl_color_control_cluster_color_loop_set_command {
  uint8_t updateFlags;
  uint8_t action;
  uint8_t direction;
  uint16_t time;
  uint16_t startHue;
  uint8_t optionsMask;
  uint8_t optionsOverride;
} sl_zcl_color_control_cluster_color_loop_set_command_t;

#define sl_zcl_color_control_cluster_color_loop_set_command_signature  { \
  7, \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_color_loop_set_command_t, updateFlags), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_color_control_cluster_color_loop_set_command_t, action), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_color_control_cluster_color_loop_set_command_t, direction), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_color_control_cluster_color_loop_set_command_t, time), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_color_control_cluster_color_loop_set_command_t, startHue), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_color_loop_set_command_t, optionsMask), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_color_loop_set_command_t, optionsOverride), \
}


/** @brief Structure for ZCL command "LocationDataNotification" from "RSSI Location" 
 */
typedef struct __zcl_rssi_location_cluster_location_data_notification_command {
  uint8_t locationType;
  int16_t coordinate1;
  int16_t coordinate2;
  int16_t coordinate3;
  int16_t power;
  uint16_t pathLossExponent;
  uint8_t locationMethod;
  uint8_t qualityMeasure;
  uint16_t locationAge;
} sl_zcl_rssi_location_cluster_location_data_notification_command_t;

#define sl_zcl_rssi_location_cluster_location_data_notification_command_signature  { \
  9, \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_rssi_location_cluster_location_data_notification_command_t, locationType), \
  (ZAP_SIGNATURE_TYPE_INT16S), offsetof(sl_zcl_rssi_location_cluster_location_data_notification_command_t, coordinate1), \
  (ZAP_SIGNATURE_TYPE_INT16S), offsetof(sl_zcl_rssi_location_cluster_location_data_notification_command_t, coordinate2), \
  (ZAP_SIGNATURE_TYPE_INT16S), offsetof(sl_zcl_rssi_location_cluster_location_data_notification_command_t, coordinate3), \
  (ZAP_SIGNATURE_TYPE_INT16S), offsetof(sl_zcl_rssi_location_cluster_location_data_notification_command_t, power), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_rssi_location_cluster_location_data_notification_command_t, pathLossExponent), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_rssi_location_cluster_location_data_notification_command_t, locationMethod), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_rssi_location_cluster_location_data_notification_command_t, qualityMeasure), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_rssi_location_cluster_location_data_notification_command_t, locationAge), \
}


/** @brief Structure for ZCL command "CompactLocationDataNotification" from "RSSI Location" 
 */
typedef struct __zcl_rssi_location_cluster_compact_location_data_notification_command {
  uint8_t locationType;
  int16_t coordinate1;
  int16_t coordinate2;
  int16_t coordinate3;
  uint8_t qualityMeasure;
  uint16_t locationAge;
} sl_zcl_rssi_location_cluster_compact_location_data_notification_command_t;

#define sl_zcl_rssi_location_cluster_compact_location_data_notification_command_signature  { \
  6, \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_rssi_location_cluster_compact_location_data_notification_command_t, locationType), \
  (ZAP_SIGNATURE_TYPE_INT16S), offsetof(sl_zcl_rssi_location_cluster_compact_location_data_notification_command_t, coordinate1), \
  (ZAP_SIGNATURE_TYPE_INT16S), offsetof(sl_zcl_rssi_location_cluster_compact_location_data_notification_command_t, coordinate2), \
  (ZAP_SIGNATURE_TYPE_INT16S), offsetof(sl_zcl_rssi_location_cluster_compact_location_data_notification_command_t, coordinate3), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_rssi_location_cluster_compact_location_data_notification_command_t, qualityMeasure), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_rssi_location_cluster_compact_location_data_notification_command_t, locationAge), \
}


/** @brief Structure for ZCL command "OnWithTimedOff" from "On/off" 
 */
typedef struct __zcl_on_off_cluster_on_with_timed_off_command {
  uint8_t onOffControl;
  uint16_t onTime;
  uint16_t offWaitTime;
} sl_zcl_on_off_cluster_on_with_timed_off_command_t;

#define sl_zcl_on_off_cluster_on_with_timed_off_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_on_off_cluster_on_with_timed_off_command_t, onOffControl), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_on_off_cluster_on_with_timed_off_command_t, onTime), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_on_off_cluster_on_with_timed_off_command_t, offWaitTime), \
}


/** @brief Structure for ZCL command "GpSinkCommissioningMode" from "Green Power" 
 */
typedef struct __zcl_green_power_cluster_gp_sink_commissioning_mode_command {
  uint8_t options;
  uint16_t gpmAddrForSecurity;
  uint16_t gpmAddrForPairing;
  uint8_t sinkEndpoint;
} sl_zcl_green_power_cluster_gp_sink_commissioning_mode_command_t;

#define sl_zcl_green_power_cluster_gp_sink_commissioning_mode_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_green_power_cluster_gp_sink_commissioning_mode_command_t, options), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_green_power_cluster_gp_sink_commissioning_mode_command_t, gpmAddrForSecurity), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_green_power_cluster_gp_sink_commissioning_mode_command_t, gpmAddrForPairing), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_green_power_cluster_gp_sink_commissioning_mode_command_t, sinkEndpoint), \
}


/** @brief Structure for ZCL command "CopyScene" from "Scenes" 
 */
typedef struct __zcl_scenes_cluster_copy_scene_command {
  uint8_t mode;
  uint16_t groupIdFrom;
  uint8_t sceneIdFrom;
  uint16_t groupIdTo;
  uint8_t sceneIdTo;
} sl_zcl_scenes_cluster_copy_scene_command_t;

#define sl_zcl_scenes_cluster_copy_scene_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_scenes_cluster_copy_scene_command_t, mode), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_scenes_cluster_copy_scene_command_t, groupIdFrom), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_scenes_cluster_copy_scene_command_t, sceneIdFrom), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_scenes_cluster_copy_scene_command_t, groupIdTo), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_scenes_cluster_copy_scene_command_t, sceneIdTo), \
}


/** @brief Structure for ZCL command "StartWarning" from "IAS WD" 
 */
typedef struct __zcl_ias_wd_cluster_start_warning_command {
  uint8_t warningInfo;
  uint16_t warningDuration;
  uint8_t strobeDutyCycle;
  uint8_t strobeLevel;
} sl_zcl_ias_wd_cluster_start_warning_command_t;

#define sl_zcl_ias_wd_cluster_start_warning_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_ias_wd_cluster_start_warning_command_t, warningInfo), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_ias_wd_cluster_start_warning_command_t, warningDuration), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_ias_wd_cluster_start_warning_command_t, strobeDutyCycle), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_ias_wd_cluster_start_warning_command_t, strobeLevel), \
}


/** @brief Structure for ZCL command "GetEventLog" from "Events" 
 */
typedef struct __zcl_events_cluster_get_event_log_command {
  uint8_t eventControlLogId;
  uint16_t eventId;
  uint32_t startTime;
  uint32_t endTime;
  uint8_t numberOfEvents;
  uint16_t eventOffset;
} sl_zcl_events_cluster_get_event_log_command_t;

#define sl_zcl_events_cluster_get_event_log_command_signature  { \
  6, \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_events_cluster_get_event_log_command_t, eventControlLogId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_events_cluster_get_event_log_command_t, eventId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_events_cluster_get_event_log_command_t, startTime), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_events_cluster_get_event_log_command_t, endTime), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_events_cluster_get_event_log_command_t, numberOfEvents), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_events_cluster_get_event_log_command_t, eventOffset), \
}


/** @brief Structure for ZCL command "Delete" from "Information" 
 */
typedef struct __zcl_information_cluster_delete_command {
  uint8_t deletionOptions;
  /* TYPE WARNING: INT16U array defaults to */ uint8_t * contentIds;
} sl_zcl_information_cluster_delete_command_t;

#define sl_zcl_information_cluster_delete_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_information_cluster_delete_command_t, deletionOptions), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_information_cluster_delete_command_t, contentIds), \
}


/** @brief Structure for ZCL command "ChangePaymentModeResponse" from "Prepayment" 
 */
typedef struct __zcl_prepayment_cluster_change_payment_mode_response_command {
  uint8_t friendlyCredit;
  uint32_t friendlyCreditCalendarId;
  uint32_t emergencyCreditLimit;
  uint32_t emergencyCreditThreshold;
} sl_zcl_prepayment_cluster_change_payment_mode_response_command_t;

#define sl_zcl_prepayment_cluster_change_payment_mode_response_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_prepayment_cluster_change_payment_mode_response_command_t, friendlyCredit), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_prepayment_cluster_change_payment_mode_response_command_t, friendlyCreditCalendarId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_prepayment_cluster_change_payment_mode_response_command_t, emergencyCreditLimit), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_prepayment_cluster_change_payment_mode_response_command_t, emergencyCreditThreshold), \
}


/** @brief Structure for ZCL command "SaveStartupParameters" from "Commissioning" 
 */
typedef struct __zcl_commissioning_cluster_save_startup_parameters_command {
  uint8_t options;
  uint8_t index;
} sl_zcl_commissioning_cluster_save_startup_parameters_command_t;

#define sl_zcl_commissioning_cluster_save_startup_parameters_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_commissioning_cluster_save_startup_parameters_command_t, options), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_commissioning_cluster_save_startup_parameters_command_t, index), \
}


/** @brief Structure for ZCL command "RestoreStartupParameters" from "Commissioning" 
 */
typedef struct __zcl_commissioning_cluster_restore_startup_parameters_command {
  uint8_t options;
  uint8_t index;
} sl_zcl_commissioning_cluster_restore_startup_parameters_command_t;

#define sl_zcl_commissioning_cluster_restore_startup_parameters_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_commissioning_cluster_restore_startup_parameters_command_t, options), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_commissioning_cluster_restore_startup_parameters_command_t, index), \
}


/** @brief Structure for ZCL command "ResetStartupParameters" from "Commissioning" 
 */
typedef struct __zcl_commissioning_cluster_reset_startup_parameters_command {
  uint8_t options;
  uint8_t index;
} sl_zcl_commissioning_cluster_reset_startup_parameters_command_t;

#define sl_zcl_commissioning_cluster_reset_startup_parameters_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_commissioning_cluster_reset_startup_parameters_command_t, options), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_commissioning_cluster_reset_startup_parameters_command_t, index), \
}


/** @brief Structure for ZCL command "GetLocationData" from "RSSI Location" 
 */
typedef struct __zcl_rssi_location_cluster_get_location_data_command {
  uint8_t flags;
  uint8_t numberResponses;
  uint8_t * targetAddress;
} sl_zcl_rssi_location_cluster_get_location_data_command_t;

#define sl_zcl_rssi_location_cluster_get_location_data_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_rssi_location_cluster_get_location_data_command_t, flags), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_rssi_location_cluster_get_location_data_command_t, numberResponses), \
  (ZAP_SIGNATURE_TYPE_IEEE_ADDRESS), offsetof(sl_zcl_rssi_location_cluster_get_location_data_command_t, targetAddress), \
}


/** @brief Structure for ZCL command "GetPowerProfilePriceExtended" from "Power Profile" 
 */
typedef struct __zcl_power_profile_cluster_get_power_profile_price_extended_command {
  uint8_t options;
  uint8_t powerProfileId;
  uint16_t powerProfileStartTime;
} sl_zcl_power_profile_cluster_get_power_profile_price_extended_command_t;

#define sl_zcl_power_profile_cluster_get_power_profile_price_extended_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_power_profile_cluster_get_power_profile_price_extended_command_t, options), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_power_profile_cluster_get_power_profile_price_extended_command_t, powerProfileId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_power_profile_cluster_get_power_profile_price_extended_command_t, powerProfileStartTime), \
}


/** @brief Structure for ZCL command "RestartDevice" from "Commissioning" 
 */
typedef struct __zcl_commissioning_cluster_restart_device_command {
  uint8_t options;
  uint8_t delay;
  uint8_t jitter;
} sl_zcl_commissioning_cluster_restart_device_command_t;

#define sl_zcl_commissioning_cluster_restart_device_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_commissioning_cluster_restart_device_command_t, options), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_commissioning_cluster_restart_device_command_t, delay), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_commissioning_cluster_restart_device_command_t, jitter), \
}


/** @brief Structure for ZCL command "TransferPartitionedFrame" from "Partition" 
 */
typedef struct __zcl_partition_cluster_transfer_partitioned_frame_command {
  uint8_t fragmentationOptions;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * partitionedIndicatorAndFrame;
} sl_zcl_partition_cluster_transfer_partitioned_frame_command_t;

#define sl_zcl_partition_cluster_transfer_partitioned_frame_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_partition_cluster_transfer_partitioned_frame_command_t, fragmentationOptions), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_partition_cluster_transfer_partitioned_frame_command_t, partitionedIndicatorAndFrame), \
}


/** @brief Structure for ZCL command "MultipleAck" from "Partition" 
 */
typedef struct __zcl_partition_cluster_multiple_ack_command {
  uint8_t ackOptions;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * firstFrameIdAndNackList;
} sl_zcl_partition_cluster_multiple_ack_command_t;

#define sl_zcl_partition_cluster_multiple_ack_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_partition_cluster_multiple_ack_command_t, ackOptions), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_partition_cluster_multiple_ack_command_t, firstFrameIdAndNackList), \
}


/** @brief Structure for ZCL command "WriteFileRequest" from "Data Sharing" 
 */
typedef struct __zcl_data_sharing_cluster_write_file_request_command {
  uint8_t writeOptions;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * fileSize;
} sl_zcl_data_sharing_cluster_write_file_request_command_t;

#define sl_zcl_data_sharing_cluster_write_file_request_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_data_sharing_cluster_write_file_request_command_t, writeOptions), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_data_sharing_cluster_write_file_request_command_t, fileSize), \
}


/** @brief Structure for ZCL command "FileTransmission" from "Data Sharing" 
 */
typedef struct __zcl_data_sharing_cluster_file_transmission_command {
  uint8_t transmitOptions;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * buffer;
} sl_zcl_data_sharing_cluster_file_transmission_command_t;

#define sl_zcl_data_sharing_cluster_file_transmission_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_data_sharing_cluster_file_transmission_command_t, transmitOptions), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_data_sharing_cluster_file_transmission_command_t, buffer), \
}


/** @brief Structure for ZCL command "RecordTransmission" from "Data Sharing" 
 */
typedef struct __zcl_data_sharing_cluster_record_transmission_command {
  uint8_t transmitOptions;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * buffer;
} sl_zcl_data_sharing_cluster_record_transmission_command_t;

#define sl_zcl_data_sharing_cluster_record_transmission_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_data_sharing_cluster_record_transmission_command_t, transmitOptions), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_data_sharing_cluster_record_transmission_command_t, buffer), \
}


/** @brief Structure for ZCL command "SearchChatResponse" from "Chatting" 
 */
typedef struct __zcl_chatting_cluster_search_chat_response_command {
  uint8_t options;
  /* TYPE WARNING: ChatRoom array defaults to */ uint8_t * chatRoomList;
} sl_zcl_chatting_cluster_search_chat_response_command_t;

#define sl_zcl_chatting_cluster_search_chat_response_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_chatting_cluster_search_chat_response_command_t, options), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_chatting_cluster_search_chat_response_command_t, chatRoomList), \
}


/** @brief Structure for ZCL command "Stop" from "Level Control" 
 */
typedef struct __zcl_level_control_cluster_stop_command {
  uint8_t optionMask;
  uint8_t optionOverride;
} sl_zcl_level_control_cluster_stop_command_t;

#define sl_zcl_level_control_cluster_stop_command_signature  { \
  2, \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_level_control_cluster_stop_command_t, optionMask), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_level_control_cluster_stop_command_t, optionOverride), \
}


/** @brief Structure for ZCL command "StopMoveStep" from "Color Control" 
 */
typedef struct __zcl_color_control_cluster_stop_move_step_command {
  uint8_t optionsMask;
  uint8_t optionsOverride;
} sl_zcl_color_control_cluster_stop_move_step_command_t;

#define sl_zcl_color_control_cluster_stop_move_step_command_signature  { \
  2, \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_stop_move_step_command_t, optionsMask), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_stop_move_step_command_t, optionsOverride), \
}


/** @brief Structure for ZCL command "ConfigureDeliveryEnable" from "Information" 
 */
typedef struct __zcl_information_cluster_configure_delivery_enable_command {
  uint8_t enable;
} sl_zcl_information_cluster_configure_delivery_enable_command_t;

#define sl_zcl_information_cluster_configure_delivery_enable_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_BOOLEAN), offsetof(sl_zcl_information_cluster_configure_delivery_enable_command_t, enable), \
}


/** @brief Structure for ZCL command "SurveyBeacons" from "SL Works With All Hubs" 
 */
typedef struct __zcl_sl_works_with_all_hubs_cluster_survey_beacons_command {
  uint8_t standardBeacons;
} sl_zcl_sl_works_with_all_hubs_cluster_survey_beacons_command_t;

#define sl_zcl_sl_works_with_all_hubs_cluster_survey_beacons_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_BOOLEAN), offsetof(sl_zcl_sl_works_with_all_hubs_cluster_survey_beacons_command_t, standardBeacons), \
}


/** @brief Structure for ZCL command "CheckInResponse" from "Poll Control" 
 */
typedef struct __zcl_poll_control_cluster_check_in_response_command {
  uint8_t startFastPolling;
  uint16_t fastPollTimeout;
} sl_zcl_poll_control_cluster_check_in_response_command_t;

#define sl_zcl_poll_control_cluster_check_in_response_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_BOOLEAN), offsetof(sl_zcl_poll_control_cluster_check_in_response_command_t, startFastPolling), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_poll_control_cluster_check_in_response_command_t, fastPollTimeout), \
}


/** @brief Structure for ZCL command "GetZoneStatusResponse" from "IAS ACE" 
 */
typedef struct __zcl_ias_ace_cluster_get_zone_status_response_command {
  uint8_t zoneStatusComplete;
  uint8_t numberOfZones;
  /* TYPE WARNING: IasAceZoneStatusResult array defaults to */ uint8_t * zoneStatusResult;
} sl_zcl_ias_ace_cluster_get_zone_status_response_command_t;

#define sl_zcl_ias_ace_cluster_get_zone_status_response_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_BOOLEAN), offsetof(sl_zcl_ias_ace_cluster_get_zone_status_response_command_t, zoneStatusComplete), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_ias_ace_cluster_get_zone_status_response_command_t, numberOfZones), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_ias_ace_cluster_get_zone_status_response_command_t, zoneStatusResult), \
}


/** @brief Structure for ZCL command "SupportedTunnelProtocolsResponse" from "Tunneling" 
 */
typedef struct __zcl_tunneling_cluster_supported_tunnel_protocols_response_command {
  uint8_t protocolListComplete;
  uint8_t protocolCount;
  /* TYPE WARNING: Protocol array defaults to */ uint8_t * protocolList;
} sl_zcl_tunneling_cluster_supported_tunnel_protocols_response_command_t;

#define sl_zcl_tunneling_cluster_supported_tunnel_protocols_response_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_BOOLEAN), offsetof(sl_zcl_tunneling_cluster_supported_tunnel_protocols_response_command_t, protocolListComplete), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_tunneling_cluster_supported_tunnel_protocols_response_command_t, protocolCount), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_tunneling_cluster_supported_tunnel_protocols_response_command_t, protocolList), \
}


/** @brief Structure for ZCL command "ConfigureNodeDescription" from "Information" 
 */
typedef struct __zcl_information_cluster_configure_node_description_command {
  uint8_t * description;
} sl_zcl_information_cluster_configure_node_description_command_t;

#define sl_zcl_information_cluster_configure_node_description_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_information_cluster_configure_node_description_command_t, description), \
}


/** @brief Structure for ZCL command "StartChatRequest" from "Chatting" 
 */
typedef struct __zcl_chatting_cluster_start_chat_request_command {
  uint8_t * name;
  uint16_t uid;
  uint8_t * nickname;
} sl_zcl_chatting_cluster_start_chat_request_command_t;

#define sl_zcl_chatting_cluster_start_chat_request_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_chatting_cluster_start_chat_request_command_t, name), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_chatting_cluster_start_chat_request_command_t, uid), \
  (ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_chatting_cluster_start_chat_request_command_t, nickname), \
}


/** @brief Structure for ZCL command "GetLocalesSupported" from "Basic" 
 */
typedef struct __zcl_basic_cluster_get_locales_supported_command {
  uint8_t * startLocale;
  uint8_t maxLocalesRequested;
} sl_zcl_basic_cluster_get_locales_supported_command_t;

#define sl_zcl_basic_cluster_get_locales_supported_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_basic_cluster_get_locales_supported_command_t, startLocale), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_basic_cluster_get_locales_supported_command_t, maxLocalesRequested), \
}


/** @brief Structure for ZCL command "LockDoor" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_lock_door_command {
  uint8_t * PIN;
} sl_zcl_door_lock_cluster_lock_door_command_t;

#define sl_zcl_door_lock_cluster_lock_door_command_signature  { \
  1, \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_door_lock_cluster_lock_door_command_t, PIN), \
}


/** @brief Structure for ZCL command "UnlockDoor" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_unlock_door_command {
  uint8_t * PIN;
} sl_zcl_door_lock_cluster_unlock_door_command_t;

#define sl_zcl_door_lock_cluster_unlock_door_command_signature  { \
  1, \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_door_lock_cluster_unlock_door_command_t, PIN), \
}


/** @brief Structure for ZCL command "Toggle" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_toggle_command {
  uint8_t * pin;
} sl_zcl_door_lock_cluster_toggle_command_t;

#define sl_zcl_door_lock_cluster_toggle_command_signature  { \
  1, \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_door_lock_cluster_toggle_command_t, pin), \
}


/** @brief Structure for ZCL command "ApsLinkKeyAuthorizationQuery" from "SL Works With All Hubs" 
 */
typedef struct __zcl_sl_works_with_all_hubs_cluster_aps_link_key_authorization_query_command {
  uint16_t clusterId;
} sl_zcl_sl_works_with_all_hubs_cluster_aps_link_key_authorization_query_command_t;

#define sl_zcl_sl_works_with_all_hubs_cluster_aps_link_key_authorization_query_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_CLUSTER_ID), offsetof(sl_zcl_sl_works_with_all_hubs_cluster_aps_link_key_authorization_query_command_t, clusterId), \
}


/** @brief Structure for ZCL command "ReadHandshakeParam" from "Partition" 
 */
typedef struct __zcl_partition_cluster_read_handshake_param_command {
  uint16_t partitionedClusterId;
  /* TYPE WARNING: ATTRIBUTE_ID array defaults to */ uint8_t * attributeList;
} sl_zcl_partition_cluster_read_handshake_param_command_t;

#define sl_zcl_partition_cluster_read_handshake_param_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_CLUSTER_ID), offsetof(sl_zcl_partition_cluster_read_handshake_param_command_t, partitionedClusterId), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_partition_cluster_read_handshake_param_command_t, attributeList), \
}


/** @brief Structure for ZCL command "ApsLinkKeyAuthorizationQueryResponse" from "SL Works With All Hubs" 
 */
typedef struct __zcl_sl_works_with_all_hubs_cluster_aps_link_key_authorization_query_response_command {
  uint16_t clusterId;
  uint8_t apsLinkKeyAuthStatus;
} sl_zcl_sl_works_with_all_hubs_cluster_aps_link_key_authorization_query_response_command_t;

#define sl_zcl_sl_works_with_all_hubs_cluster_aps_link_key_authorization_query_response_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_CLUSTER_ID), offsetof(sl_zcl_sl_works_with_all_hubs_cluster_aps_link_key_authorization_query_response_command_t, clusterId), \
  (ZAP_SIGNATURE_TYPE_BOOLEAN), offsetof(sl_zcl_sl_works_with_all_hubs_cluster_aps_link_key_authorization_query_response_command_t, apsLinkKeyAuthStatus), \
}


/** @brief Structure for ZCL command "WriteHandshakeParam" from "Partition" 
 */
typedef struct __zcl_partition_cluster_write_handshake_param_command {
  uint16_t partitionedClusterId;
  /* TYPE WARNING: WriteAttributeRecord array defaults to */ uint8_t * writeAttributeRecords;
} sl_zcl_partition_cluster_write_handshake_param_command_t;

#define sl_zcl_partition_cluster_write_handshake_param_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_CLUSTER_ID), offsetof(sl_zcl_partition_cluster_write_handshake_param_command_t, partitionedClusterId), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_partition_cluster_write_handshake_param_command_t, writeAttributeRecords), \
}


/** @brief Structure for ZCL command "ReadHandshakeParamResponse" from "Partition" 
 */
typedef struct __zcl_partition_cluster_read_handshake_param_response_command {
  uint16_t partitionedClusterId;
  /* TYPE WARNING: ReadAttributeStatusRecord array defaults to */ uint8_t * readAttributeStatusRecords;
} sl_zcl_partition_cluster_read_handshake_param_response_command_t;

#define sl_zcl_partition_cluster_read_handshake_param_response_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_CLUSTER_ID), offsetof(sl_zcl_partition_cluster_read_handshake_param_response_command_t, partitionedClusterId), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_partition_cluster_read_handshake_param_response_command_t, readAttributeStatusRecords), \
}


/** @brief Structure for ZCL command "PathDeletion" from "Data Rate Control" 
 */
typedef struct __zcl_data_rate_control_cluster_path_deletion_command {
  uint16_t originatorAddress;
  uint16_t destinationAddress;
} sl_zcl_data_rate_control_cluster_path_deletion_command_t;

#define sl_zcl_data_rate_control_cluster_path_deletion_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_DATA16), offsetof(sl_zcl_data_rate_control_cluster_path_deletion_command_t, originatorAddress), \
  (ZAP_SIGNATURE_TYPE_DATA16), offsetof(sl_zcl_data_rate_control_cluster_path_deletion_command_t, destinationAddress), \
}


/** @brief Structure for ZCL command "PathCreation" from "Data Rate Control" 
 */
typedef struct __zcl_data_rate_control_cluster_path_creation_command {
  uint16_t originatorAddress;
  uint16_t destinationAddress;
  uint8_t dataRate;
} sl_zcl_data_rate_control_cluster_path_creation_command_t;

#define sl_zcl_data_rate_control_cluster_path_creation_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_DATA16), offsetof(sl_zcl_data_rate_control_cluster_path_creation_command_t, originatorAddress), \
  (ZAP_SIGNATURE_TYPE_DATA16), offsetof(sl_zcl_data_rate_control_cluster_path_creation_command_t, destinationAddress), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_data_rate_control_cluster_path_creation_command_t, dataRate), \
}


/** @brief Structure for ZCL command "DataRateNotification" from "Data Rate Control" 
 */
typedef struct __zcl_data_rate_control_cluster_data_rate_notification_command {
  uint16_t originatorAddress;
  uint16_t destinationAddress;
  uint8_t dataRate;
} sl_zcl_data_rate_control_cluster_data_rate_notification_command_t;

#define sl_zcl_data_rate_control_cluster_data_rate_notification_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_DATA16), offsetof(sl_zcl_data_rate_control_cluster_data_rate_notification_command_t, originatorAddress), \
  (ZAP_SIGNATURE_TYPE_DATA16), offsetof(sl_zcl_data_rate_control_cluster_data_rate_notification_command_t, destinationAddress), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_data_rate_control_cluster_data_rate_notification_command_t, dataRate), \
}


/** @brief Structure for ZCL command "DataRateControl" from "Data Rate Control" 
 */
typedef struct __zcl_data_rate_control_cluster_data_rate_control_command {
  uint16_t originatorAddress;
  uint16_t destinationAddress;
  uint8_t dataRate;
} sl_zcl_data_rate_control_cluster_data_rate_control_command_t;

#define sl_zcl_data_rate_control_cluster_data_rate_control_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_DATA16), offsetof(sl_zcl_data_rate_control_cluster_data_rate_control_command_t, originatorAddress), \
  (ZAP_SIGNATURE_TYPE_DATA16), offsetof(sl_zcl_data_rate_control_cluster_data_rate_control_command_t, destinationAddress), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_data_rate_control_cluster_data_rate_control_command_t, dataRate), \
}


/** @brief Structure for ZCL command "TransferNpdu" from "BACnet Protocol Tunnel" 
 */
typedef struct __zcl_ba_cnet_protocol_tunnel_cluster_transfer_npdu_command {
  /* TYPE WARNING: DATA8 array defaults to */ uint8_t * npdu;
} sl_zcl_ba_cnet_protocol_tunnel_cluster_transfer_npdu_command_t;

#define sl_zcl_ba_cnet_protocol_tunnel_cluster_transfer_npdu_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_ba_cnet_protocol_tunnel_cluster_transfer_npdu_command_t, npdu), \
}


/** @brief Structure for ZCL command "ZoneEnrollRequest" from "IAS Zone" 
 */
typedef struct __zcl_ias_zone_cluster_zone_enroll_request_command {
  uint16_t zoneType;
  uint16_t manufacturerCode;
} sl_zcl_ias_zone_cluster_zone_enroll_request_command_t;

#define sl_zcl_ias_zone_cluster_zone_enroll_request_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_ENUM16), offsetof(sl_zcl_ias_zone_cluster_zone_enroll_request_command_t, zoneType), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_ias_zone_cluster_zone_enroll_request_command_t, manufacturerCode), \
}


/** @brief Structure for ZCL command "RestartDeviceResponse" from "Commissioning" 
 */
typedef struct __zcl_commissioning_cluster_restart_device_response_command {
  uint8_t status;
} sl_zcl_commissioning_cluster_restart_device_response_command_t;

#define sl_zcl_commissioning_cluster_restart_device_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_commissioning_cluster_restart_device_response_command_t, status), \
}


/** @brief Structure for ZCL command "SaveStartupParametersResponse" from "Commissioning" 
 */
typedef struct __zcl_commissioning_cluster_save_startup_parameters_response_command {
  uint8_t status;
} sl_zcl_commissioning_cluster_save_startup_parameters_response_command_t;

#define sl_zcl_commissioning_cluster_save_startup_parameters_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_commissioning_cluster_save_startup_parameters_response_command_t, status), \
}


/** @brief Structure for ZCL command "RestoreStartupParametersResponse" from "Commissioning" 
 */
typedef struct __zcl_commissioning_cluster_restore_startup_parameters_response_command {
  uint8_t status;
} sl_zcl_commissioning_cluster_restore_startup_parameters_response_command_t;

#define sl_zcl_commissioning_cluster_restore_startup_parameters_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_commissioning_cluster_restore_startup_parameters_response_command_t, status), \
}


/** @brief Structure for ZCL command "ResetStartupParametersResponse" from "Commissioning" 
 */
typedef struct __zcl_commissioning_cluster_reset_startup_parameters_response_command {
  uint8_t status;
} sl_zcl_commissioning_cluster_reset_startup_parameters_response_command_t;

#define sl_zcl_commissioning_cluster_reset_startup_parameters_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_commissioning_cluster_reset_startup_parameters_response_command_t, status), \
}


/** @brief Structure for ZCL command "ExecutionOfACommand" from "Appliance Control" 
 */
typedef struct __zcl_appliance_control_cluster_execution_of_a_command_command {
  uint8_t commandId;
} sl_zcl_appliance_control_cluster_execution_of_a_command_command_t;

#define sl_zcl_appliance_control_cluster_execution_of_a_command_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_appliance_control_cluster_execution_of_a_command_command_t, commandId), \
}


/** @brief Structure for ZCL command "OverloadWarning" from "Appliance Control" 
 */
typedef struct __zcl_appliance_control_cluster_overload_warning_command {
  uint8_t warningEvent;
} sl_zcl_appliance_control_cluster_overload_warning_command_t;

#define sl_zcl_appliance_control_cluster_overload_warning_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_appliance_control_cluster_overload_warning_command_t, warningEvent), \
}


/** @brief Structure for ZCL command "SetPinResponse" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_set_pin_response_command {
  uint8_t status;
} sl_zcl_door_lock_cluster_set_pin_response_command_t;

#define sl_zcl_door_lock_cluster_set_pin_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_door_lock_cluster_set_pin_response_command_t, status), \
}


/** @brief Structure for ZCL command "SetRfidResponse" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_set_rfid_response_command {
  uint8_t status;
} sl_zcl_door_lock_cluster_set_rfid_response_command_t;

#define sl_zcl_door_lock_cluster_set_rfid_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_door_lock_cluster_set_rfid_response_command_t, status), \
}


/** @brief Structure for ZCL command "ArmResponse" from "IAS ACE" 
 */
typedef struct __zcl_ias_ace_cluster_arm_response_command {
  uint8_t armNotification;
} sl_zcl_ias_ace_cluster_arm_response_command_t;

#define sl_zcl_ias_ace_cluster_arm_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_ias_ace_cluster_arm_response_command_t, armNotification), \
}


/** @brief Structure for ZCL command "ConnectStatusNotification" from "11073 Protocol Tunnel" 
 */
typedef struct __zcl_11073_protocol_tunnel_cluster_connect_status_notification_command {
  uint8_t connectStatus;
} sl_zcl_11073_protocol_tunnel_cluster_connect_status_notification_command_t;

#define sl_zcl_11073_protocol_tunnel_cluster_connect_status_notification_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_11073_protocol_tunnel_cluster_connect_status_notification_command_t, connectStatus), \
}


/** @brief Structure for ZCL command "LocalChangeSupply" from "Simple Metering" 
 */
typedef struct __zcl_simple_metering_cluster_local_change_supply_command {
  uint8_t proposedSupplyStatus;
} sl_zcl_simple_metering_cluster_local_change_supply_command_t;

#define sl_zcl_simple_metering_cluster_local_change_supply_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_simple_metering_cluster_local_change_supply_command_t, proposedSupplyStatus), \
}


/** @brief Structure for ZCL command "RequestNewPassword" from "Device Management" 
 */
typedef struct __zcl_device_management_cluster_request_new_password_command {
  uint8_t passwordType;
} sl_zcl_device_management_cluster_request_new_password_command_t;

#define sl_zcl_device_management_cluster_request_new_password_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_device_management_cluster_request_new_password_command_t, passwordType), \
}


/** @brief Structure for ZCL command "ClearEventLogRequest" from "Events" 
 */
typedef struct __zcl_events_cluster_clear_event_log_request_command {
  uint8_t logId;
} sl_zcl_events_cluster_clear_event_log_request_command_t;

#define sl_zcl_events_cluster_clear_event_log_request_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_events_cluster_clear_event_log_request_command_t, logId), \
}


/** @brief Structure for ZCL command "ControlResponse" from "Voice over ZigBee" 
 */
typedef struct __zcl_voice_over_zig_bee_cluster_control_response_command {
  uint8_t ackNack;
} sl_zcl_voice_over_zig_bee_cluster_control_response_command_t;

#define sl_zcl_voice_over_zig_bee_cluster_control_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_voice_over_zig_bee_cluster_control_response_command_t, ackNack), \
}


/** @brief Structure for ZCL command "Control" from "Voice over ZigBee" 
 */
typedef struct __zcl_voice_over_zig_bee_cluster_control_command {
  uint8_t controlType;
} sl_zcl_voice_over_zig_bee_cluster_control_command_t;

#define sl_zcl_voice_over_zig_bee_cluster_control_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_voice_over_zig_bee_cluster_control_command_t, controlType), \
}


/** @brief Structure for ZCL command "SetIasZoneEnrollmentMethod" from "SL Works With All Hubs" 
 */
typedef struct __zcl_sl_works_with_all_hubs_cluster_set_ias_zone_enrollment_method_command {
  uint8_t enrollmentMode;
} sl_zcl_sl_works_with_all_hubs_cluster_set_ias_zone_enrollment_method_command_t;

#define sl_zcl_sl_works_with_all_hubs_cluster_set_ias_zone_enrollment_method_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_sl_works_with_all_hubs_cluster_set_ias_zone_enrollment_method_command_t, enrollmentMode), \
}


/** @brief Structure for ZCL command "UpdateCommissionState" from "Identify" 
 */
typedef struct __zcl_identify_cluster_update_commission_state_command {
  uint8_t action;
  uint8_t commissionStateMask;
} sl_zcl_identify_cluster_update_commission_state_command_t;

#define sl_zcl_identify_cluster_update_commission_state_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_identify_cluster_update_commission_state_command_t, action), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_identify_cluster_update_commission_state_command_t, commissionStateMask), \
}


/** @brief Structure for ZCL command "SetWeeklySchedule" from "Thermostat" 
 */
typedef struct __zcl_thermostat_cluster_set_weekly_schedule_command {
  uint8_t numberOfTransitionsForSequence;
  uint8_t dayOfWeekForSequence;
  uint8_t modeForSequence;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * payload;
} sl_zcl_thermostat_cluster_set_weekly_schedule_command_t;

#define sl_zcl_thermostat_cluster_set_weekly_schedule_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_thermostat_cluster_set_weekly_schedule_command_t, numberOfTransitionsForSequence), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_thermostat_cluster_set_weekly_schedule_command_t, dayOfWeekForSequence), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_thermostat_cluster_set_weekly_schedule_command_t, modeForSequence), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_thermostat_cluster_set_weekly_schedule_command_t, payload), \
}


/** @brief Structure for ZCL command "CurrentWeeklySchedule" from "Thermostat" 
 */
typedef struct __zcl_thermostat_cluster_current_weekly_schedule_command {
  uint8_t numberOfTransitionsForSequence;
  uint8_t dayOfWeekForSequence;
  uint8_t modeForSequence;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * payload;
} sl_zcl_thermostat_cluster_current_weekly_schedule_command_t;

#define sl_zcl_thermostat_cluster_current_weekly_schedule_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_thermostat_cluster_current_weekly_schedule_command_t, numberOfTransitionsForSequence), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_thermostat_cluster_current_weekly_schedule_command_t, dayOfWeekForSequence), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_thermostat_cluster_current_weekly_schedule_command_t, modeForSequence), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_thermostat_cluster_current_weekly_schedule_command_t, payload), \
}


/** @brief Structure for ZCL command "SignalStateResponse" from "Appliance Control" 
 */
typedef struct __zcl_appliance_control_cluster_signal_state_response_command {
  uint8_t applianceStatus;
  uint8_t remoteEnableFlagsAndDeviceStatus2;
  uint32_t applianceStatus2;
} sl_zcl_appliance_control_cluster_signal_state_response_command_t;

#define sl_zcl_appliance_control_cluster_signal_state_response_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_appliance_control_cluster_signal_state_response_command_t, applianceStatus), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_appliance_control_cluster_signal_state_response_command_t, remoteEnableFlagsAndDeviceStatus2), \
  (ZAP_SIGNATURE_TYPE_INT24U), offsetof(sl_zcl_appliance_control_cluster_signal_state_response_command_t, applianceStatus2), \
}


/** @brief Structure for ZCL command "SignalStateNotification" from "Appliance Control" 
 */
typedef struct __zcl_appliance_control_cluster_signal_state_notification_command {
  uint8_t applianceStatus;
  uint8_t remoteEnableFlagsAndDeviceStatus2;
  uint32_t applianceStatus2;
} sl_zcl_appliance_control_cluster_signal_state_notification_command_t;

#define sl_zcl_appliance_control_cluster_signal_state_notification_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_appliance_control_cluster_signal_state_notification_command_t, applianceStatus), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_appliance_control_cluster_signal_state_notification_command_t, remoteEnableFlagsAndDeviceStatus2), \
  (ZAP_SIGNATURE_TYPE_INT24U), offsetof(sl_zcl_appliance_control_cluster_signal_state_notification_command_t, applianceStatus2), \
}


/** @brief Structure for ZCL command "RequestInformation" from "Information" 
 */
typedef struct __zcl_information_cluster_request_information_command {
  uint8_t inquiryId;
  uint8_t dataTypeId;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * requestInformationPayload;
} sl_zcl_information_cluster_request_information_command_t;

#define sl_zcl_information_cluster_request_information_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_information_cluster_request_information_command_t, inquiryId), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_information_cluster_request_information_command_t, dataTypeId), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_information_cluster_request_information_command_t, requestInformationPayload), \
}


/** @brief Structure for ZCL command "Update" from "Information" 
 */
typedef struct __zcl_information_cluster_update_command {
  uint8_t accessControl;
  uint8_t option;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * contents;
} sl_zcl_information_cluster_update_command_t;

#define sl_zcl_information_cluster_update_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_information_cluster_update_command_t, accessControl), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_information_cluster_update_command_t, option), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_information_cluster_update_command_t, contents), \
}


/** @brief Structure for ZCL command "Arm" from "IAS ACE" 
 */
typedef struct __zcl_ias_ace_cluster_arm_command {
  uint8_t armMode;
  uint8_t * armDisarmCode;
  uint8_t zoneId;
} sl_zcl_ias_ace_cluster_arm_command_t;

#define sl_zcl_ias_ace_cluster_arm_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_ias_ace_cluster_arm_command_t, armMode), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_ias_ace_cluster_arm_command_t, armDisarmCode), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_ias_ace_cluster_arm_command_t, zoneId), \
}


/** @brief Structure for ZCL command "ResetAlarm" from "Alarms" 
 */
typedef struct __zcl_alarms_cluster_reset_alarm_command {
  uint8_t alarmCode;
  uint16_t clusterId;
} sl_zcl_alarms_cluster_reset_alarm_command_t;

#define sl_zcl_alarms_cluster_reset_alarm_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_alarms_cluster_reset_alarm_command_t, alarmCode), \
  (ZAP_SIGNATURE_TYPE_CLUSTER_ID), offsetof(sl_zcl_alarms_cluster_reset_alarm_command_t, clusterId), \
}


/** @brief Structure for ZCL command "Alarm" from "Alarms" 
 */
typedef struct __zcl_alarms_cluster_alarm_command {
  uint8_t alarmCode;
  uint16_t clusterId;
} sl_zcl_alarms_cluster_alarm_command_t;

#define sl_zcl_alarms_cluster_alarm_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_alarms_cluster_alarm_command_t, alarmCode), \
  (ZAP_SIGNATURE_TYPE_CLUSTER_ID), offsetof(sl_zcl_alarms_cluster_alarm_command_t, clusterId), \
}


/** @brief Structure for ZCL command "TriggerEffect" from "Identify" 
 */
typedef struct __zcl_identify_cluster_trigger_effect_command {
  uint8_t effectId;
  uint8_t effectVariant;
} sl_zcl_identify_cluster_trigger_effect_command_t;

#define sl_zcl_identify_cluster_trigger_effect_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_identify_cluster_trigger_effect_command_t, effectId), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_identify_cluster_trigger_effect_command_t, effectVariant), \
}


/** @brief Structure for ZCL command "OffWithEffect" from "On/off" 
 */
typedef struct __zcl_on_off_cluster_off_with_effect_command {
  uint8_t effectId;
  uint8_t effectVariant;
} sl_zcl_on_off_cluster_off_with_effect_command_t;

#define sl_zcl_on_off_cluster_off_with_effect_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_on_off_cluster_off_with_effect_command_t, effectId), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_on_off_cluster_off_with_effect_command_t, effectVariant), \
}


/** @brief Structure for ZCL command "EstablishmentResponse" from "Voice over ZigBee" 
 */
typedef struct __zcl_voice_over_zig_bee_cluster_establishment_response_command {
  uint8_t ackNack;
  uint8_t codecType;
} sl_zcl_voice_over_zig_bee_cluster_establishment_response_command_t;

#define sl_zcl_voice_over_zig_bee_cluster_establishment_response_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_voice_over_zig_bee_cluster_establishment_response_command_t, ackNack), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_voice_over_zig_bee_cluster_establishment_response_command_t, codecType), \
}


/** @brief Structure for ZCL command "SearchGame" from "Gaming" 
 */
typedef struct __zcl_gaming_cluster_search_game_command {
  uint8_t specificGame;
  uint16_t gameId;
} sl_zcl_gaming_cluster_search_game_command_t;

#define sl_zcl_gaming_cluster_search_game_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_gaming_cluster_search_game_command_t, specificGame), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_gaming_cluster_search_game_command_t, gameId), \
}


/** @brief Structure for ZCL command "StartChatResponse" from "Chatting" 
 */
typedef struct __zcl_chatting_cluster_start_chat_response_command {
  uint8_t status;
  uint16_t cid;
} sl_zcl_chatting_cluster_start_chat_response_command_t;

#define sl_zcl_chatting_cluster_start_chat_response_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_chatting_cluster_start_chat_response_command_t, status), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_chatting_cluster_start_chat_response_command_t, cid), \
}


/** @brief Structure for ZCL command "EnhancedMoveHue" from "Color Control" 
 */
typedef struct __zcl_color_control_cluster_enhanced_move_hue_command {
  uint8_t moveMode;
  uint16_t rate;
  uint8_t optionsMask;
  uint8_t optionsOverride;
} sl_zcl_color_control_cluster_enhanced_move_hue_command_t;

#define sl_zcl_color_control_cluster_enhanced_move_hue_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_color_control_cluster_enhanced_move_hue_command_t, moveMode), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_color_control_cluster_enhanced_move_hue_command_t, rate), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_enhanced_move_hue_command_t, optionsMask), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_enhanced_move_hue_command_t, optionsOverride), \
}


/** @brief Structure for ZCL command "ConnectRequest" from "11073 Protocol Tunnel" 
 */
typedef struct __zcl_11073_protocol_tunnel_cluster_connect_request_command {
  uint8_t connectControl;
  uint16_t idleTimeout;
  uint8_t * managerTarget;
  uint8_t managerEndpoint;
} sl_zcl_11073_protocol_tunnel_cluster_connect_request_command_t;

#define sl_zcl_11073_protocol_tunnel_cluster_connect_request_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_11073_protocol_tunnel_cluster_connect_request_command_t, connectControl), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_11073_protocol_tunnel_cluster_connect_request_command_t, idleTimeout), \
  (ZAP_SIGNATURE_TYPE_IEEE_ADDRESS), offsetof(sl_zcl_11073_protocol_tunnel_cluster_connect_request_command_t, managerTarget), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_11073_protocol_tunnel_cluster_connect_request_command_t, managerEndpoint), \
}


/** @brief Structure for ZCL command "EnhancedStepHue" from "Color Control" 
 */
typedef struct __zcl_color_control_cluster_enhanced_step_hue_command {
  uint8_t stepMode;
  uint16_t stepSize;
  uint16_t transitionTime;
  uint8_t optionsMask;
  uint8_t optionsOverride;
} sl_zcl_color_control_cluster_enhanced_step_hue_command_t;

#define sl_zcl_color_control_cluster_enhanced_step_hue_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_color_control_cluster_enhanced_step_hue_command_t, stepMode), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_color_control_cluster_enhanced_step_hue_command_t, stepSize), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_color_control_cluster_enhanced_step_hue_command_t, transitionTime), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_enhanced_step_hue_command_t, optionsMask), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_enhanced_step_hue_command_t, optionsOverride), \
}


/** @brief Structure for ZCL command "MoveColorTemperature" from "Color Control" 
 */
typedef struct __zcl_color_control_cluster_move_color_temperature_command {
  uint8_t moveMode;
  uint16_t rate;
  uint16_t colorTemperatureMinimum;
  uint16_t colorTemperatureMaximum;
  uint8_t optionsMask;
  uint8_t optionsOverride;
} sl_zcl_color_control_cluster_move_color_temperature_command_t;

#define sl_zcl_color_control_cluster_move_color_temperature_command_signature  { \
  6, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_color_control_cluster_move_color_temperature_command_t, moveMode), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_color_control_cluster_move_color_temperature_command_t, rate), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_color_control_cluster_move_color_temperature_command_t, colorTemperatureMinimum), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_color_control_cluster_move_color_temperature_command_t, colorTemperatureMaximum), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_move_color_temperature_command_t, optionsMask), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_move_color_temperature_command_t, optionsOverride), \
}


/** @brief Structure for ZCL command "StepColorTemperature" from "Color Control" 
 */
typedef struct __zcl_color_control_cluster_step_color_temperature_command {
  uint8_t stepMode;
  uint16_t stepSize;
  uint16_t transitionTime;
  uint16_t colorTemperatureMinimum;
  uint16_t colorTemperatureMaximum;
  uint8_t optionsMask;
  uint8_t optionsOverride;
} sl_zcl_color_control_cluster_step_color_temperature_command_t;

#define sl_zcl_color_control_cluster_step_color_temperature_command_signature  { \
  7, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_color_control_cluster_step_color_temperature_command_t, stepMode), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_color_control_cluster_step_color_temperature_command_t, stepSize), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_color_control_cluster_step_color_temperature_command_t, transitionTime), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_color_control_cluster_step_color_temperature_command_t, colorTemperatureMinimum), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_color_control_cluster_step_color_temperature_command_t, colorTemperatureMaximum), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_step_color_temperature_command_t, optionsMask), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_step_color_temperature_command_t, optionsOverride), \
}


/** @brief Structure for ZCL command "GetNodeInformationResponse" from "Chatting" 
 */
typedef struct __zcl_chatting_cluster_get_node_information_response_command {
  uint8_t status;
  uint16_t cid;
  uint16_t uid;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * addressEndpointAndNickname;
} sl_zcl_chatting_cluster_get_node_information_response_command_t;

#define sl_zcl_chatting_cluster_get_node_information_response_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_chatting_cluster_get_node_information_response_command_t, status), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_chatting_cluster_get_node_information_response_command_t, cid), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_chatting_cluster_get_node_information_response_command_t, uid), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_chatting_cluster_get_node_information_response_command_t, addressEndpointAndNickname), \
}


/** @brief Structure for ZCL command "JoinChatResponse" from "Chatting" 
 */
typedef struct __zcl_chatting_cluster_join_chat_response_command {
  uint8_t status;
  uint16_t cid;
  /* TYPE WARNING: ChatParticipant array defaults to */ uint8_t * chatParticipantList;
} sl_zcl_chatting_cluster_join_chat_response_command_t;

#define sl_zcl_chatting_cluster_join_chat_response_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_chatting_cluster_join_chat_response_command_t, status), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_chatting_cluster_join_chat_response_command_t, cid), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_chatting_cluster_join_chat_response_command_t, chatParticipantList), \
}


/** @brief Structure for ZCL command "PublishEvent" from "Events" 
 */
typedef struct __zcl_events_cluster_publish_event_command {
  uint8_t logId;
  uint16_t eventId;
  uint32_t eventTime;
  uint8_t eventControl;
  uint8_t * eventData;
} sl_zcl_events_cluster_publish_event_command_t;

#define sl_zcl_events_cluster_publish_event_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_events_cluster_publish_event_command_t, logId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_events_cluster_publish_event_command_t, eventId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_events_cluster_publish_event_command_t, eventTime), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_events_cluster_publish_event_command_t, eventControl), \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_events_cluster_publish_event_command_t, eventData), \
}


/** @brief Structure for ZCL command "ConsumerTopUpResponse" from "Prepayment" 
 */
typedef struct __zcl_prepayment_cluster_consumer_top_up_response_command {
  uint8_t resultType;
  uint32_t topUpValue;
  uint8_t sourceOfTopUp;
  uint32_t creditRemaining;
} sl_zcl_prepayment_cluster_consumer_top_up_response_command_t;

#define sl_zcl_prepayment_cluster_consumer_top_up_response_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_prepayment_cluster_consumer_top_up_response_command_t, resultType), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_prepayment_cluster_consumer_top_up_response_command_t, topUpValue), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_prepayment_cluster_consumer_top_up_response_command_t, sourceOfTopUp), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_prepayment_cluster_consumer_top_up_response_command_t, creditRemaining), \
}


/** @brief Structure for ZCL command "SetpointRaiseLower" from "Thermostat" 
 */
typedef struct __zcl_thermostat_cluster_setpoint_raise_lower_command {
  uint8_t mode;
  int8_t amount;
} sl_zcl_thermostat_cluster_setpoint_raise_lower_command_t;

#define sl_zcl_thermostat_cluster_setpoint_raise_lower_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_thermostat_cluster_setpoint_raise_lower_command_t, mode), \
  (ZAP_SIGNATURE_TYPE_INT8S), offsetof(sl_zcl_thermostat_cluster_setpoint_raise_lower_command_t, amount), \
}


/** @brief Structure for ZCL command "MoveWithOnOff" from "Level Control" 
 */
typedef struct __zcl_level_control_cluster_move_with_on_off_command {
  uint8_t moveMode;
  uint8_t rate;
} sl_zcl_level_control_cluster_move_with_on_off_command_t;

#define sl_zcl_level_control_cluster_move_with_on_off_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_level_control_cluster_move_with_on_off_command_t, moveMode), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_level_control_cluster_move_with_on_off_command_t, rate), \
}


/** @brief Structure for ZCL command "ZoneEnrollResponse" from "IAS Zone" 
 */
typedef struct __zcl_ias_zone_cluster_zone_enroll_response_command {
  uint8_t enrollResponseCode;
  uint8_t zoneId;
} sl_zcl_ias_zone_cluster_zone_enroll_response_command_t;

#define sl_zcl_ias_zone_cluster_zone_enroll_response_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_ias_zone_cluster_zone_enroll_response_command_t, enrollResponseCode), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_ias_zone_cluster_zone_enroll_response_command_t, zoneId), \
}


/** @brief Structure for ZCL command "TerminateKeyEstablishmentFromClient" from "Key Establishment" 
 */
typedef struct __zcl_key_establishment_cluster_terminate_key_establishment_from_client_command {
  uint8_t statusCode;
  uint8_t waitTime;
  uint16_t keyEstablishmentSuite;
} sl_zcl_key_establishment_cluster_terminate_key_establishment_from_client_command_t;

#define sl_zcl_key_establishment_cluster_terminate_key_establishment_from_client_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_key_establishment_cluster_terminate_key_establishment_from_client_command_t, statusCode), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_key_establishment_cluster_terminate_key_establishment_from_client_command_t, waitTime), \
  (ZAP_SIGNATURE_TYPE_BITMAP16), offsetof(sl_zcl_key_establishment_cluster_terminate_key_establishment_from_client_command_t, keyEstablishmentSuite), \
}


/** @brief Structure for ZCL command "TerminateKeyEstablishmentFromServer" from "Key Establishment" 
 */
typedef struct __zcl_key_establishment_cluster_terminate_key_establishment_from_server_command {
  uint8_t statusCode;
  uint8_t waitTime;
  uint16_t keyEstablishmentSuite;
} sl_zcl_key_establishment_cluster_terminate_key_establishment_from_server_command_t;

#define sl_zcl_key_establishment_cluster_terminate_key_establishment_from_server_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_key_establishment_cluster_terminate_key_establishment_from_server_command_t, statusCode), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_key_establishment_cluster_terminate_key_establishment_from_server_command_t, waitTime), \
  (ZAP_SIGNATURE_TYPE_BITMAP16), offsetof(sl_zcl_key_establishment_cluster_terminate_key_establishment_from_server_command_t, keyEstablishmentSuite), \
}


/** @brief Structure for ZCL command "Move" from "Level Control" 
 */
typedef struct __zcl_level_control_cluster_move_command {
  uint8_t moveMode;
  uint8_t rate;
  uint8_t optionMask;
  uint8_t optionOverride;
} sl_zcl_level_control_cluster_move_command_t;

#define sl_zcl_level_control_cluster_move_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_level_control_cluster_move_command_t, moveMode), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_level_control_cluster_move_command_t, rate), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_level_control_cluster_move_command_t, optionMask), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_level_control_cluster_move_command_t, optionOverride), \
}


/** @brief Structure for ZCL command "MoveHue" from "Color Control" 
 */
typedef struct __zcl_color_control_cluster_move_hue_command {
  uint8_t moveMode;
  uint8_t rate;
  uint8_t optionsMask;
  uint8_t optionsOverride;
} sl_zcl_color_control_cluster_move_hue_command_t;

#define sl_zcl_color_control_cluster_move_hue_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_color_control_cluster_move_hue_command_t, moveMode), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_color_control_cluster_move_hue_command_t, rate), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_move_hue_command_t, optionsMask), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_move_hue_command_t, optionsOverride), \
}


/** @brief Structure for ZCL command "MoveSaturation" from "Color Control" 
 */
typedef struct __zcl_color_control_cluster_move_saturation_command {
  uint8_t moveMode;
  uint8_t rate;
  uint8_t optionsMask;
  uint8_t optionsOverride;
} sl_zcl_color_control_cluster_move_saturation_command_t;

#define sl_zcl_color_control_cluster_move_saturation_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_color_control_cluster_move_saturation_command_t, moveMode), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_color_control_cluster_move_saturation_command_t, rate), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_move_saturation_command_t, optionsMask), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_move_saturation_command_t, optionsOverride), \
}


/** @brief Structure for ZCL command "GetPanelStatusResponse" from "IAS ACE" 
 */
typedef struct __zcl_ias_ace_cluster_get_panel_status_response_command {
  uint8_t panelStatus;
  uint8_t secondsRemaining;
  uint8_t audibleNotification;
  uint8_t alarmStatus;
} sl_zcl_ias_ace_cluster_get_panel_status_response_command_t;

#define sl_zcl_ias_ace_cluster_get_panel_status_response_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_ias_ace_cluster_get_panel_status_response_command_t, panelStatus), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_ias_ace_cluster_get_panel_status_response_command_t, secondsRemaining), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_ias_ace_cluster_get_panel_status_response_command_t, audibleNotification), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_ias_ace_cluster_get_panel_status_response_command_t, alarmStatus), \
}


/** @brief Structure for ZCL command "PanelStatusChanged" from "IAS ACE" 
 */
typedef struct __zcl_ias_ace_cluster_panel_status_changed_command {
  uint8_t panelStatus;
  uint8_t secondsRemaining;
  uint8_t audibleNotification;
  uint8_t alarmStatus;
} sl_zcl_ias_ace_cluster_panel_status_changed_command_t;

#define sl_zcl_ias_ace_cluster_panel_status_changed_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_ias_ace_cluster_panel_status_changed_command_t, panelStatus), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_ias_ace_cluster_panel_status_changed_command_t, secondsRemaining), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_ias_ace_cluster_panel_status_changed_command_t, audibleNotification), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_ias_ace_cluster_panel_status_changed_command_t, alarmStatus), \
}


/** @brief Structure for ZCL command "StepWithOnOff" from "Level Control" 
 */
typedef struct __zcl_level_control_cluster_step_with_on_off_command {
  uint8_t stepMode;
  uint8_t stepSize;
  uint16_t transitionTime;
} sl_zcl_level_control_cluster_step_with_on_off_command_t;

#define sl_zcl_level_control_cluster_step_with_on_off_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_level_control_cluster_step_with_on_off_command_t, stepMode), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_level_control_cluster_step_with_on_off_command_t, stepSize), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_level_control_cluster_step_with_on_off_command_t, transitionTime), \
}


/** @brief Structure for ZCL command "Step" from "Level Control" 
 */
typedef struct __zcl_level_control_cluster_step_command {
  uint8_t stepMode;
  uint8_t stepSize;
  uint16_t transitionTime;
  uint8_t optionMask;
  uint8_t optionOverride;
} sl_zcl_level_control_cluster_step_command_t;

#define sl_zcl_level_control_cluster_step_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_level_control_cluster_step_command_t, stepMode), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_level_control_cluster_step_command_t, stepSize), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_level_control_cluster_step_command_t, transitionTime), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_level_control_cluster_step_command_t, optionMask), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_level_control_cluster_step_command_t, optionOverride), \
}


/** @brief Structure for ZCL command "ImageNotify" from "Over the Air Bootloading" 
 */
typedef struct __zcl_over_the_air_bootloading_cluster_image_notify_command {
  uint8_t payloadType;
  uint8_t queryJitter;
  uint16_t manufacturerId;
  uint16_t imageType;
  uint32_t newFileVersion;
} sl_zcl_over_the_air_bootloading_cluster_image_notify_command_t;

#define sl_zcl_over_the_air_bootloading_cluster_image_notify_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_over_the_air_bootloading_cluster_image_notify_command_t, payloadType), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_over_the_air_bootloading_cluster_image_notify_command_t, queryJitter), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_over_the_air_bootloading_cluster_image_notify_command_t, manufacturerId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_over_the_air_bootloading_cluster_image_notify_command_t, imageType), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_over_the_air_bootloading_cluster_image_notify_command_t, newFileVersion), \
}


/** @brief Structure for ZCL command "StepHue" from "Color Control" 
 */
typedef struct __zcl_color_control_cluster_step_hue_command {
  uint8_t stepMode;
  uint8_t stepSize;
  uint8_t transitionTime;
  uint8_t optionsMask;
  uint8_t optionsOverride;
} sl_zcl_color_control_cluster_step_hue_command_t;

#define sl_zcl_color_control_cluster_step_hue_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_color_control_cluster_step_hue_command_t, stepMode), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_color_control_cluster_step_hue_command_t, stepSize), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_color_control_cluster_step_hue_command_t, transitionTime), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_step_hue_command_t, optionsMask), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_step_hue_command_t, optionsOverride), \
}


/** @brief Structure for ZCL command "StepSaturation" from "Color Control" 
 */
typedef struct __zcl_color_control_cluster_step_saturation_command {
  uint8_t stepMode;
  uint8_t stepSize;
  uint8_t transitionTime;
  uint8_t optionsMask;
  uint8_t optionsOverride;
} sl_zcl_color_control_cluster_step_saturation_command_t;

#define sl_zcl_color_control_cluster_step_saturation_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_color_control_cluster_step_saturation_command_t, stepMode), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_color_control_cluster_step_saturation_command_t, stepSize), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_color_control_cluster_step_saturation_command_t, transitionTime), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_step_saturation_command_t, optionsMask), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_step_saturation_command_t, optionsOverride), \
}


/** @brief Structure for ZCL command "GpProxyTableResponse" from "Green Power" 
 */
typedef struct __zcl_green_power_cluster_gp_proxy_table_response_command {
  uint8_t status;
  uint8_t totalNumberOfNonEmptyProxyTableEntries;
  uint8_t startIndex;
  uint8_t entriesCount;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * proxyTableEntries;
} sl_zcl_green_power_cluster_gp_proxy_table_response_command_t;

#define sl_zcl_green_power_cluster_gp_proxy_table_response_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_green_power_cluster_gp_proxy_table_response_command_t, status), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_green_power_cluster_gp_proxy_table_response_command_t, totalNumberOfNonEmptyProxyTableEntries), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_green_power_cluster_gp_proxy_table_response_command_t, startIndex), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_green_power_cluster_gp_proxy_table_response_command_t, entriesCount), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_green_power_cluster_gp_proxy_table_response_command_t, proxyTableEntries), \
}


/** @brief Structure for ZCL command "GpSinkTableResponse" from "Green Power" 
 */
typedef struct __zcl_green_power_cluster_gp_sink_table_response_command {
  uint8_t status;
  uint8_t totalNumberofNonEmptySinkTableEntries;
  uint8_t startIndex;
  uint8_t sinkTableEntriesCount;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * sinkTableEntries;
} sl_zcl_green_power_cluster_gp_sink_table_response_command_t;

#define sl_zcl_green_power_cluster_gp_sink_table_response_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_green_power_cluster_gp_sink_table_response_command_t, status), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_green_power_cluster_gp_sink_table_response_command_t, totalNumberofNonEmptySinkTableEntries), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_green_power_cluster_gp_sink_table_response_command_t, startIndex), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_green_power_cluster_gp_sink_table_response_command_t, sinkTableEntriesCount), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_green_power_cluster_gp_sink_table_response_command_t, sinkTableEntries), \
}


/** @brief Structure for ZCL command "WriteFileResponse" from "Data Sharing" 
 */
typedef struct __zcl_data_sharing_cluster_write_file_response_command {
  uint8_t status;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * fileIndex;
} sl_zcl_data_sharing_cluster_write_file_response_command_t;

#define sl_zcl_data_sharing_cluster_write_file_response_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_data_sharing_cluster_write_file_response_command_t, status), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_data_sharing_cluster_write_file_response_command_t, fileIndex), \
}


/** @brief Structure for ZCL command "ConsumerTopUp" from "Prepayment" 
 */
typedef struct __zcl_prepayment_cluster_consumer_top_up_command {
  uint8_t originatingDevice;
  uint8_t * topUpCode;
} sl_zcl_prepayment_cluster_consumer_top_up_command_t;

#define sl_zcl_prepayment_cluster_consumer_top_up_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_prepayment_cluster_consumer_top_up_command_t, originatingDevice), \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_prepayment_cluster_consumer_top_up_command_t, topUpCode), \
}


/** @brief Structure for ZCL command "GetProfile" from "Simple Metering" 
 */
typedef struct __zcl_simple_metering_cluster_get_profile_command {
  uint8_t intervalChannel;
  uint32_t endTime;
  uint8_t numberOfPeriods;
} sl_zcl_simple_metering_cluster_get_profile_command_t;

#define sl_zcl_simple_metering_cluster_get_profile_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_simple_metering_cluster_get_profile_command_t, intervalChannel), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_simple_metering_cluster_get_profile_command_t, endTime), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_simple_metering_cluster_get_profile_command_t, numberOfPeriods), \
}


/** @brief Structure for ZCL command "GpTranslationTableResponse" from "Green Power" 
 */
typedef struct __zcl_green_power_cluster_gp_translation_table_response_command {
  uint8_t status;
  uint8_t options;
  uint8_t totalNumberOfEntries;
  uint8_t startIndex;
  uint8_t entriesCount;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * translationTableList;
} sl_zcl_green_power_cluster_gp_translation_table_response_command_t;

#define sl_zcl_green_power_cluster_gp_translation_table_response_command_signature  { \
  6, \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_green_power_cluster_gp_translation_table_response_command_t, status), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_green_power_cluster_gp_translation_table_response_command_t, options), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_green_power_cluster_gp_translation_table_response_command_t, totalNumberOfEntries), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_green_power_cluster_gp_translation_table_response_command_t, startIndex), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_green_power_cluster_gp_translation_table_response_command_t, entriesCount), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_green_power_cluster_gp_translation_table_response_command_t, translationTableList), \
}


/** @brief Structure for ZCL command "GetDeviceConfiguration" from "RSSI Location" 
 */
typedef struct __zcl_rssi_location_cluster_get_device_configuration_command {
  uint8_t * targetAddress;
} sl_zcl_rssi_location_cluster_get_device_configuration_command_t;

#define sl_zcl_rssi_location_cluster_get_device_configuration_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_IEEE_ADDRESS), offsetof(sl_zcl_rssi_location_cluster_get_device_configuration_command_t, targetAddress), \
}


/** @brief Structure for ZCL command "RequestOwnLocation" from "RSSI Location" 
 */
typedef struct __zcl_rssi_location_cluster_request_own_location_command {
  uint8_t * blindNode;
} sl_zcl_rssi_location_cluster_request_own_location_command_t;

#define sl_zcl_rssi_location_cluster_request_own_location_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_IEEE_ADDRESS), offsetof(sl_zcl_rssi_location_cluster_request_own_location_command_t, blindNode), \
}


/** @brief Structure for ZCL command "DisconnectRequest" from "11073 Protocol Tunnel" 
 */
typedef struct __zcl_11073_protocol_tunnel_cluster_disconnect_request_command {
  uint8_t * managerIEEEAddress;
} sl_zcl_11073_protocol_tunnel_cluster_disconnect_request_command_t;

#define sl_zcl_11073_protocol_tunnel_cluster_disconnect_request_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_IEEE_ADDRESS), offsetof(sl_zcl_11073_protocol_tunnel_cluster_disconnect_request_command_t, managerIEEEAddress), \
}


/** @brief Structure for ZCL command "AnchorNodeAnnounce" from "RSSI Location" 
 */
typedef struct __zcl_rssi_location_cluster_anchor_node_announce_command {
  uint8_t * anchorNodeIeeeAddress;
  int16_t coordinate1;
  int16_t coordinate2;
  int16_t coordinate3;
} sl_zcl_rssi_location_cluster_anchor_node_announce_command_t;

#define sl_zcl_rssi_location_cluster_anchor_node_announce_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_IEEE_ADDRESS), offsetof(sl_zcl_rssi_location_cluster_anchor_node_announce_command_t, anchorNodeIeeeAddress), \
  (ZAP_SIGNATURE_TYPE_INT16S), offsetof(sl_zcl_rssi_location_cluster_anchor_node_announce_command_t, coordinate1), \
  (ZAP_SIGNATURE_TYPE_INT16S), offsetof(sl_zcl_rssi_location_cluster_anchor_node_announce_command_t, coordinate2), \
  (ZAP_SIGNATURE_TYPE_INT16S), offsetof(sl_zcl_rssi_location_cluster_anchor_node_announce_command_t, coordinate3), \
}


/** @brief Structure for ZCL command "RssiResponse" from "RSSI Location" 
 */
typedef struct __zcl_rssi_location_cluster_rssi_response_command {
  uint8_t * replyingDevice;
  int16_t coordinate1;
  int16_t coordinate2;
  int16_t coordinate3;
  int8_t rssi;
  uint8_t numberRssiMeasurements;
} sl_zcl_rssi_location_cluster_rssi_response_command_t;

#define sl_zcl_rssi_location_cluster_rssi_response_command_signature  { \
  6, \
  (ZAP_SIGNATURE_TYPE_IEEE_ADDRESS), offsetof(sl_zcl_rssi_location_cluster_rssi_response_command_t, replyingDevice), \
  (ZAP_SIGNATURE_TYPE_INT16S), offsetof(sl_zcl_rssi_location_cluster_rssi_response_command_t, coordinate1), \
  (ZAP_SIGNATURE_TYPE_INT16S), offsetof(sl_zcl_rssi_location_cluster_rssi_response_command_t, coordinate2), \
  (ZAP_SIGNATURE_TYPE_INT16S), offsetof(sl_zcl_rssi_location_cluster_rssi_response_command_t, coordinate3), \
  (ZAP_SIGNATURE_TYPE_INT8S), offsetof(sl_zcl_rssi_location_cluster_rssi_response_command_t, rssi), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_rssi_location_cluster_rssi_response_command_t, numberRssiMeasurements), \
}


/** @brief Structure for ZCL command "ShortAddressChange" from "SL Works With All Hubs" 
 */
typedef struct __zcl_sl_works_with_all_hubs_cluster_short_address_change_command {
  uint8_t * deviceEui64;
  uint16_t deviceShort;
} sl_zcl_sl_works_with_all_hubs_cluster_short_address_change_command_t;

#define sl_zcl_sl_works_with_all_hubs_cluster_short_address_change_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_IEEE_ADDRESS), offsetof(sl_zcl_sl_works_with_all_hubs_cluster_short_address_change_command_t, deviceEui64), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_sl_works_with_all_hubs_cluster_short_address_change_command_t, deviceShort), \
}


/** @brief Structure for ZCL command "QuerySpecificFileRequest" from "Over the Air Bootloading" 
 */
typedef struct __zcl_over_the_air_bootloading_cluster_query_specific_file_request_command {
  uint8_t * requestNodeAddress;
  uint16_t manufacturerId;
  uint16_t imageType;
  uint32_t fileVersion;
  uint16_t currentZigbeeStackVersion;
} sl_zcl_over_the_air_bootloading_cluster_query_specific_file_request_command_t;

#define sl_zcl_over_the_air_bootloading_cluster_query_specific_file_request_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_IEEE_ADDRESS), offsetof(sl_zcl_over_the_air_bootloading_cluster_query_specific_file_request_command_t, requestNodeAddress), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_over_the_air_bootloading_cluster_query_specific_file_request_command_t, manufacturerId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_over_the_air_bootloading_cluster_query_specific_file_request_command_t, imageType), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_over_the_air_bootloading_cluster_query_specific_file_request_command_t, fileVersion), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_over_the_air_bootloading_cluster_query_specific_file_request_command_t, currentZigbeeStackVersion), \
}


/** @brief Structure for ZCL command "EndpointInformation" from "ZLL Commissioning" 
 */
typedef struct __zcl_zll_commissioning_cluster_endpoint_information_command {
  uint8_t * ieeeAddress;
  uint16_t networkAddress;
  uint8_t endpointId;
  uint16_t profileId;
  uint16_t deviceId;
  uint8_t version;
} sl_zcl_zll_commissioning_cluster_endpoint_information_command_t;

#define sl_zcl_zll_commissioning_cluster_endpoint_information_command_signature  { \
  6, \
  (ZAP_SIGNATURE_TYPE_IEEE_ADDRESS), offsetof(sl_zcl_zll_commissioning_cluster_endpoint_information_command_t, ieeeAddress), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_endpoint_information_command_t, networkAddress), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_zll_commissioning_cluster_endpoint_information_command_t, endpointId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_endpoint_information_command_t, profileId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_endpoint_information_command_t, deviceId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_zll_commissioning_cluster_endpoint_information_command_t, version), \
}


/** @brief Structure for ZCL command "SendPings" from "RSSI Location" 
 */
typedef struct __zcl_rssi_location_cluster_send_pings_command {
  uint8_t * targetAddress;
  uint8_t numberRssiMeasurements;
  uint16_t calculationPeriod;
} sl_zcl_rssi_location_cluster_send_pings_command_t;

#define sl_zcl_rssi_location_cluster_send_pings_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_IEEE_ADDRESS), offsetof(sl_zcl_rssi_location_cluster_send_pings_command_t, targetAddress), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_rssi_location_cluster_send_pings_command_t, numberRssiMeasurements), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_rssi_location_cluster_send_pings_command_t, calculationPeriod), \
}


/** @brief Structure for ZCL command "ReportRssiMeasurements" from "RSSI Location" 
 */
typedef struct __zcl_rssi_location_cluster_report_rssi_measurements_command {
  uint8_t * measuringDevice;
  uint8_t neighbors;
  /* TYPE WARNING: NeighborInfo array defaults to */ uint8_t * neighborsInfo;
} sl_zcl_rssi_location_cluster_report_rssi_measurements_command_t;

#define sl_zcl_rssi_location_cluster_report_rssi_measurements_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_IEEE_ADDRESS), offsetof(sl_zcl_rssi_location_cluster_report_rssi_measurements_command_t, measuringDevice), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_rssi_location_cluster_report_rssi_measurements_command_t, neighbors), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_rssi_location_cluster_report_rssi_measurements_command_t, neighborsInfo), \
}


/** @brief Structure for ZCL command "MatchProtocolAddressResponse" from "Generic Tunnel" 
 */
typedef struct __zcl_generic_tunnel_cluster_match_protocol_address_response_command {
  uint8_t * deviceIeeeAddress;
  uint8_t * protocolAddress;
} sl_zcl_generic_tunnel_cluster_match_protocol_address_response_command_t;

#define sl_zcl_generic_tunnel_cluster_match_protocol_address_response_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_IEEE_ADDRESS), offsetof(sl_zcl_generic_tunnel_cluster_match_protocol_address_response_command_t, deviceIeeeAddress), \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_generic_tunnel_cluster_match_protocol_address_response_command_t, protocolAddress), \
}


/** @brief Structure for ZCL command "MoveColor" from "Color Control" 
 */
typedef struct __zcl_color_control_cluster_move_color_command {
  int16_t rateX;
  int16_t rateY;
  uint8_t optionsMask;
  uint8_t optionsOverride;
} sl_zcl_color_control_cluster_move_color_command_t;

#define sl_zcl_color_control_cluster_move_color_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT16S), offsetof(sl_zcl_color_control_cluster_move_color_command_t, rateX), \
  (ZAP_SIGNATURE_TYPE_INT16S), offsetof(sl_zcl_color_control_cluster_move_color_command_t, rateY), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_move_color_command_t, optionsMask), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_move_color_command_t, optionsOverride), \
}


/** @brief Structure for ZCL command "SetAbsoluteLocation" from "RSSI Location" 
 */
typedef struct __zcl_rssi_location_cluster_set_absolute_location_command {
  int16_t coordinate1;
  int16_t coordinate2;
  int16_t coordinate3;
  int16_t power;
  uint16_t pathLossExponent;
} sl_zcl_rssi_location_cluster_set_absolute_location_command_t;

#define sl_zcl_rssi_location_cluster_set_absolute_location_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_INT16S), offsetof(sl_zcl_rssi_location_cluster_set_absolute_location_command_t, coordinate1), \
  (ZAP_SIGNATURE_TYPE_INT16S), offsetof(sl_zcl_rssi_location_cluster_set_absolute_location_command_t, coordinate2), \
  (ZAP_SIGNATURE_TYPE_INT16S), offsetof(sl_zcl_rssi_location_cluster_set_absolute_location_command_t, coordinate3), \
  (ZAP_SIGNATURE_TYPE_INT16S), offsetof(sl_zcl_rssi_location_cluster_set_absolute_location_command_t, power), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_rssi_location_cluster_set_absolute_location_command_t, pathLossExponent), \
}


/** @brief Structure for ZCL command "StepColor" from "Color Control" 
 */
typedef struct __zcl_color_control_cluster_step_color_command {
  int16_t stepX;
  int16_t stepY;
  uint16_t transitionTime;
  uint8_t optionsMask;
  uint8_t optionsOverride;
} sl_zcl_color_control_cluster_step_color_command_t;

#define sl_zcl_color_control_cluster_step_color_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_INT16S), offsetof(sl_zcl_color_control_cluster_step_color_command_t, stepX), \
  (ZAP_SIGNATURE_TYPE_INT16S), offsetof(sl_zcl_color_control_cluster_step_color_command_t, stepY), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_color_control_cluster_step_color_command_t, transitionTime), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_step_color_command_t, optionsMask), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_step_color_command_t, optionsOverride), \
}


/** @brief Structure for ZCL command "SetDeviceConfiguration" from "RSSI Location" 
 */
typedef struct __zcl_rssi_location_cluster_set_device_configuration_command {
  int16_t power;
  uint16_t pathLossExponent;
  uint16_t calculationPeriod;
  uint8_t numberRssiMeasurements;
  uint16_t reportingPeriod;
} sl_zcl_rssi_location_cluster_set_device_configuration_command_t;

#define sl_zcl_rssi_location_cluster_set_device_configuration_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_INT16S), offsetof(sl_zcl_rssi_location_cluster_set_device_configuration_command_t, power), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_rssi_location_cluster_set_device_configuration_command_t, pathLossExponent), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_rssi_location_cluster_set_device_configuration_command_t, calculationPeriod), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_rssi_location_cluster_set_device_configuration_command_t, numberRssiMeasurements), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_rssi_location_cluster_set_device_configuration_command_t, reportingPeriod), \
}


/** @brief Structure for ZCL command "Identify" from "Identify" 
 */
typedef struct __zcl_identify_cluster_identify_command {
  uint16_t identifyTime;
} sl_zcl_identify_cluster_identify_command_t;

#define sl_zcl_identify_cluster_identify_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_identify_cluster_identify_command_t, identifyTime), \
}


/** @brief Structure for ZCL command "IdentifyQueryResponse" from "Identify" 
 */
typedef struct __zcl_identify_cluster_identify_query_response_command {
  uint16_t timeout;
} sl_zcl_identify_cluster_identify_query_response_command_t;

#define sl_zcl_identify_cluster_identify_query_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_identify_cluster_identify_query_response_command_t, timeout), \
}


/** @brief Structure for ZCL command "ViewGroup" from "Groups" 
 */
typedef struct __zcl_groups_cluster_view_group_command {
  uint16_t groupId;
} sl_zcl_groups_cluster_view_group_command_t;

#define sl_zcl_groups_cluster_view_group_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_groups_cluster_view_group_command_t, groupId), \
}


/** @brief Structure for ZCL command "RemoveGroup" from "Groups" 
 */
typedef struct __zcl_groups_cluster_remove_group_command {
  uint16_t groupId;
} sl_zcl_groups_cluster_remove_group_command_t;

#define sl_zcl_groups_cluster_remove_group_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_groups_cluster_remove_group_command_t, groupId), \
}


/** @brief Structure for ZCL command "RemoveAllScenes" from "Scenes" 
 */
typedef struct __zcl_scenes_cluster_remove_all_scenes_command {
  uint16_t groupId;
} sl_zcl_scenes_cluster_remove_all_scenes_command_t;

#define sl_zcl_scenes_cluster_remove_all_scenes_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_scenes_cluster_remove_all_scenes_command_t, groupId), \
}


/** @brief Structure for ZCL command "GetSceneMembership" from "Scenes" 
 */
typedef struct __zcl_scenes_cluster_get_scene_membership_command {
  uint16_t groupId;
} sl_zcl_scenes_cluster_get_scene_membership_command_t;

#define sl_zcl_scenes_cluster_get_scene_membership_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_scenes_cluster_get_scene_membership_command_t, groupId), \
}


/** @brief Structure for ZCL command "MoveToClosestFrequency" from "Level Control" 
 */
typedef struct __zcl_level_control_cluster_move_to_closest_frequency_command {
  uint16_t frequency;
} sl_zcl_level_control_cluster_move_to_closest_frequency_command_t;

#define sl_zcl_level_control_cluster_move_to_closest_frequency_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_level_control_cluster_move_to_closest_frequency_command_t, frequency), \
}


/** @brief Structure for ZCL command "SetShortPollInterval" from "Poll Control" 
 */
typedef struct __zcl_poll_control_cluster_set_short_poll_interval_command {
  uint16_t newShortPollInterval;
} sl_zcl_poll_control_cluster_set_short_poll_interval_command_t;

#define sl_zcl_poll_control_cluster_set_short_poll_interval_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_poll_control_cluster_set_short_poll_interval_command_t, newShortPollInterval), \
}


/** @brief Structure for ZCL command "GetLogRecord" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_get_log_record_command {
  uint16_t logIndex;
} sl_zcl_door_lock_cluster_get_log_record_command_t;

#define sl_zcl_door_lock_cluster_get_log_record_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_door_lock_cluster_get_log_record_command_t, logIndex), \
}


/** @brief Structure for ZCL command "GetPin" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_get_pin_command {
  uint16_t userId;
} sl_zcl_door_lock_cluster_get_pin_command_t;

#define sl_zcl_door_lock_cluster_get_pin_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_door_lock_cluster_get_pin_command_t, userId), \
}


/** @brief Structure for ZCL command "ClearPin" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_clear_pin_command {
  uint16_t userId;
} sl_zcl_door_lock_cluster_clear_pin_command_t;

#define sl_zcl_door_lock_cluster_clear_pin_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_door_lock_cluster_clear_pin_command_t, userId), \
}


/** @brief Structure for ZCL command "GetUserStatus" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_get_user_status_command {
  uint16_t userId;
} sl_zcl_door_lock_cluster_get_user_status_command_t;

#define sl_zcl_door_lock_cluster_get_user_status_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_door_lock_cluster_get_user_status_command_t, userId), \
}


/** @brief Structure for ZCL command "GetUserType" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_get_user_type_command {
  uint16_t userId;
} sl_zcl_door_lock_cluster_get_user_type_command_t;

#define sl_zcl_door_lock_cluster_get_user_type_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_door_lock_cluster_get_user_type_command_t, userId), \
}


/** @brief Structure for ZCL command "GetRfid" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_get_rfid_command {
  uint16_t userId;
} sl_zcl_door_lock_cluster_get_rfid_command_t;

#define sl_zcl_door_lock_cluster_get_rfid_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_door_lock_cluster_get_rfid_command_t, userId), \
}


/** @brief Structure for ZCL command "ClearRfid" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_clear_rfid_command {
  uint16_t userId;
} sl_zcl_door_lock_cluster_clear_rfid_command_t;

#define sl_zcl_door_lock_cluster_clear_rfid_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_door_lock_cluster_clear_rfid_command_t, userId), \
}


/** @brief Structure for ZCL command "GetDisposableSchedule" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_get_disposable_schedule_command {
  uint16_t userId;
} sl_zcl_door_lock_cluster_get_disposable_schedule_command_t;

#define sl_zcl_door_lock_cluster_get_disposable_schedule_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_door_lock_cluster_get_disposable_schedule_command_t, userId), \
}


/** @brief Structure for ZCL command "ClearDisposableSchedule" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_clear_disposable_schedule_command {
  uint16_t userId;
} sl_zcl_door_lock_cluster_clear_disposable_schedule_command_t;

#define sl_zcl_door_lock_cluster_clear_disposable_schedule_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_door_lock_cluster_clear_disposable_schedule_command_t, userId), \
}


/** @brief Structure for ZCL command "ClearBiometricCredential" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_clear_biometric_credential_command {
  uint16_t userId;
} sl_zcl_door_lock_cluster_clear_biometric_credential_command_t;

#define sl_zcl_door_lock_cluster_clear_biometric_credential_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_door_lock_cluster_clear_biometric_credential_command_t, userId), \
}


/** @brief Structure for ZCL command "WindowCoveringGoToLiftValue" from "Window Covering" 
 */
typedef struct __zcl_window_covering_cluster_window_covering_go_to_lift_value_command {
  uint16_t liftValue;
} sl_zcl_window_covering_cluster_window_covering_go_to_lift_value_command_t;

#define sl_zcl_window_covering_cluster_window_covering_go_to_lift_value_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_window_covering_cluster_window_covering_go_to_lift_value_command_t, liftValue), \
}


/** @brief Structure for ZCL command "WindowCoveringGoToTiltValue" from "Window Covering" 
 */
typedef struct __zcl_window_covering_cluster_window_covering_go_to_tilt_value_command {
  uint16_t tiltValue;
} sl_zcl_window_covering_cluster_window_covering_go_to_tilt_value_command_t;

#define sl_zcl_window_covering_cluster_window_covering_go_to_tilt_value_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_window_covering_cluster_window_covering_go_to_tilt_value_command_t, tiltValue), \
}


/** @brief Structure for ZCL command "RequestMirrorResponse" from "Simple Metering" 
 */
typedef struct __zcl_simple_metering_cluster_request_mirror_response_command {
  uint16_t endpointId;
} sl_zcl_simple_metering_cluster_request_mirror_response_command_t;

#define sl_zcl_simple_metering_cluster_request_mirror_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_simple_metering_cluster_request_mirror_response_command_t, endpointId), \
}


/** @brief Structure for ZCL command "MirrorRemoved" from "Simple Metering" 
 */
typedef struct __zcl_simple_metering_cluster_mirror_removed_command {
  uint16_t endpointId;
} sl_zcl_simple_metering_cluster_mirror_removed_command_t;

#define sl_zcl_simple_metering_cluster_mirror_removed_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_simple_metering_cluster_mirror_removed_command_t, endpointId), \
}


/** @brief Structure for ZCL command "StartSamplingResponse" from "Simple Metering" 
 */
typedef struct __zcl_simple_metering_cluster_start_sampling_response_command {
  uint16_t sampleId;
} sl_zcl_simple_metering_cluster_start_sampling_response_command_t;

#define sl_zcl_simple_metering_cluster_start_sampling_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_simple_metering_cluster_start_sampling_response_command_t, sampleId), \
}


/** @brief Structure for ZCL command "CloseTunnel" from "Tunneling" 
 */
typedef struct __zcl_tunneling_cluster_close_tunnel_command {
  uint16_t tunnelId;
} sl_zcl_tunneling_cluster_close_tunnel_command_t;

#define sl_zcl_tunneling_cluster_close_tunnel_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_tunneling_cluster_close_tunnel_command_t, tunnelId), \
}


/** @brief Structure for ZCL command "TunnelClosureNotification" from "Tunneling" 
 */
typedef struct __zcl_tunneling_cluster_tunnel_closure_notification_command {
  uint16_t tunnelId;
} sl_zcl_tunneling_cluster_tunnel_closure_notification_command_t;

#define sl_zcl_tunneling_cluster_tunnel_closure_notification_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_tunneling_cluster_tunnel_closure_notification_command_t, tunnelId), \
}


/** @brief Structure for ZCL command "GetEventConfiguration" from "Device Management" 
 */
typedef struct __zcl_device_management_cluster_get_event_configuration_command {
  uint16_t eventId;
} sl_zcl_device_management_cluster_get_event_configuration_command_t;

#define sl_zcl_device_management_cluster_get_event_configuration_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_device_management_cluster_get_event_configuration_command_t, eventId), \
}


/** @brief Structure for ZCL command "ConfigureSetRootId" from "Information" 
 */
typedef struct __zcl_information_cluster_configure_set_root_id_command {
  uint16_t rootId;
} sl_zcl_information_cluster_configure_set_root_id_command_t;

#define sl_zcl_information_cluster_configure_set_root_id_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_information_cluster_configure_set_root_id_command_t, rootId), \
}


/** @brief Structure for ZCL command "SwitchChairmanRequest" from "Chatting" 
 */
typedef struct __zcl_chatting_cluster_switch_chairman_request_command {
  uint16_t cid;
} sl_zcl_chatting_cluster_switch_chairman_request_command_t;

#define sl_zcl_chatting_cluster_switch_chairman_request_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_chatting_cluster_switch_chairman_request_command_t, cid), \
}


/** @brief Structure for ZCL command "ReadTokens" from "Configuration Cluster" 
 */
typedef struct __zcl_configuration_cluster_cluster_read_tokens_command {
  uint16_t token;
} sl_zcl_configuration_cluster_cluster_read_tokens_command_t;

#define sl_zcl_configuration_cluster_cluster_read_tokens_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_configuration_cluster_cluster_read_tokens_command_t, token), \
}


/** @brief Structure for ZCL command "EnablePeriodicRouterCheckIns" from "SL Works With All Hubs" 
 */
typedef struct __zcl_sl_works_with_all_hubs_cluster_enable_periodic_router_check_ins_command {
  uint16_t checkInInterval;
} sl_zcl_sl_works_with_all_hubs_cluster_enable_periodic_router_check_ins_command_t;

#define sl_zcl_sl_works_with_all_hubs_cluster_enable_periodic_router_check_ins_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_sl_works_with_all_hubs_cluster_enable_periodic_router_check_ins_command_t, checkInInterval), \
}


/** @brief Structure for ZCL command "RelayStatusLog" from "Thermostat" 
 */
typedef struct __zcl_thermostat_cluster_relay_status_log_command {
  uint16_t timeOfDay;
  uint16_t relayStatus;
  int16_t localTemperature;
  uint8_t humidityInPercentage;
  int16_t setpoint;
  uint16_t unreadEntries;
} sl_zcl_thermostat_cluster_relay_status_log_command_t;

#define sl_zcl_thermostat_cluster_relay_status_log_command_signature  { \
  6, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_thermostat_cluster_relay_status_log_command_t, timeOfDay), \
  (ZAP_SIGNATURE_TYPE_BITMAP16), offsetof(sl_zcl_thermostat_cluster_relay_status_log_command_t, relayStatus), \
  (ZAP_SIGNATURE_TYPE_INT16S), offsetof(sl_zcl_thermostat_cluster_relay_status_log_command_t, localTemperature), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_thermostat_cluster_relay_status_log_command_t, humidityInPercentage), \
  (ZAP_SIGNATURE_TYPE_INT16S), offsetof(sl_zcl_thermostat_cluster_relay_status_log_command_t, setpoint), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_thermostat_cluster_relay_status_log_command_t, unreadEntries), \
}


/** @brief Structure for ZCL command "JoinGame" from "Gaming" 
 */
typedef struct __zcl_gaming_cluster_join_game_command {
  uint16_t gameId;
  uint8_t joinAsMaster;
  uint8_t * nameOfGame;
} sl_zcl_gaming_cluster_join_game_command_t;

#define sl_zcl_gaming_cluster_join_game_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_gaming_cluster_join_game_command_t, gameId), \
  (ZAP_SIGNATURE_TYPE_BOOLEAN), offsetof(sl_zcl_gaming_cluster_join_game_command_t, joinAsMaster), \
  (ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_gaming_cluster_join_game_command_t, nameOfGame), \
}


/** @brief Structure for ZCL command "GameAnnouncement" from "Gaming" 
 */
typedef struct __zcl_gaming_cluster_game_announcement_command {
  uint16_t gameId;
  uint8_t gameMaster;
  uint8_t * listOfGame;
} sl_zcl_gaming_cluster_game_announcement_command_t;

#define sl_zcl_gaming_cluster_game_announcement_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_gaming_cluster_game_announcement_command_t, gameId), \
  (ZAP_SIGNATURE_TYPE_BOOLEAN), offsetof(sl_zcl_gaming_cluster_game_announcement_command_t, gameMaster), \
  (ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_gaming_cluster_game_announcement_command_t, listOfGame), \
}


/** @brief Structure for ZCL command "AddGroup" from "Groups" 
 */
typedef struct __zcl_groups_cluster_add_group_command {
  uint16_t groupId;
  uint8_t * groupName;
} sl_zcl_groups_cluster_add_group_command_t;

#define sl_zcl_groups_cluster_add_group_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_groups_cluster_add_group_command_t, groupId), \
  (ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_groups_cluster_add_group_command_t, groupName), \
}


/** @brief Structure for ZCL command "AddGroupIfIdentifying" from "Groups" 
 */
typedef struct __zcl_groups_cluster_add_group_if_identifying_command {
  uint16_t groupId;
  uint8_t * groupName;
} sl_zcl_groups_cluster_add_group_if_identifying_command_t;

#define sl_zcl_groups_cluster_add_group_if_identifying_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_groups_cluster_add_group_if_identifying_command_t, groupId), \
  (ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_groups_cluster_add_group_if_identifying_command_t, groupName), \
}


/** @brief Structure for ZCL command "JoinChatRequest" from "Chatting" 
 */
typedef struct __zcl_chatting_cluster_join_chat_request_command {
  uint16_t uid;
  uint8_t * nickname;
  uint16_t cid;
} sl_zcl_chatting_cluster_join_chat_request_command_t;

#define sl_zcl_chatting_cluster_join_chat_request_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_chatting_cluster_join_chat_request_command_t, uid), \
  (ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_chatting_cluster_join_chat_request_command_t, nickname), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_chatting_cluster_join_chat_request_command_t, cid), \
}


/** @brief Structure for ZCL command "UnlockWithTimeout" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_unlock_with_timeout_command {
  uint16_t timeoutInSeconds;
  uint8_t * pin;
} sl_zcl_door_lock_cluster_unlock_with_timeout_command_t;

#define sl_zcl_door_lock_cluster_unlock_with_timeout_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_door_lock_cluster_unlock_with_timeout_command_t, timeoutInSeconds), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_door_lock_cluster_unlock_with_timeout_command_t, pin), \
}


/** @brief Structure for ZCL command "SetUserType" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_set_user_type_command {
  uint16_t userId;
  uint8_t userType;
} sl_zcl_door_lock_cluster_set_user_type_command_t;

#define sl_zcl_door_lock_cluster_set_user_type_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_door_lock_cluster_set_user_type_command_t, userId), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_door_lock_cluster_set_user_type_command_t, userType), \
}


/** @brief Structure for ZCL command "GetUserTypeResponse" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_get_user_type_response_command {
  uint16_t userId;
  uint8_t userType;
} sl_zcl_door_lock_cluster_get_user_type_response_command_t;

#define sl_zcl_door_lock_cluster_get_user_type_response_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_door_lock_cluster_get_user_type_response_command_t, userId), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_door_lock_cluster_get_user_type_response_command_t, userType), \
}


/** @brief Structure for ZCL command "TransferDataErrorClientToServer" from "Tunneling" 
 */
typedef struct __zcl_tunneling_cluster_transfer_data_error_client_to_server_command {
  uint16_t tunnelId;
  uint8_t transferDataStatus;
} sl_zcl_tunneling_cluster_transfer_data_error_client_to_server_command_t;

#define sl_zcl_tunneling_cluster_transfer_data_error_client_to_server_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_tunneling_cluster_transfer_data_error_client_to_server_command_t, tunnelId), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_tunneling_cluster_transfer_data_error_client_to_server_command_t, transferDataStatus), \
}


/** @brief Structure for ZCL command "TransferDataErrorServerToClient" from "Tunneling" 
 */
typedef struct __zcl_tunneling_cluster_transfer_data_error_server_to_client_command {
  uint16_t tunnelId;
  uint8_t transferDataStatus;
} sl_zcl_tunneling_cluster_transfer_data_error_server_to_client_command_t;

#define sl_zcl_tunneling_cluster_transfer_data_error_server_to_client_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_tunneling_cluster_transfer_data_error_server_to_client_command_t, tunnelId), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_tunneling_cluster_transfer_data_error_server_to_client_command_t, transferDataStatus), \
}


/** @brief Structure for ZCL command "SetPin" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_set_pin_command {
  uint16_t userId;
  uint8_t userStatus;
  uint8_t userType;
  uint8_t * pin;
} sl_zcl_door_lock_cluster_set_pin_command_t;

#define sl_zcl_door_lock_cluster_set_pin_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_door_lock_cluster_set_pin_command_t, userId), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_door_lock_cluster_set_pin_command_t, userStatus), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_door_lock_cluster_set_pin_command_t, userType), \
  (ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_door_lock_cluster_set_pin_command_t, pin), \
}


/** @brief Structure for ZCL command "SetRfid" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_set_rfid_command {
  uint16_t userId;
  uint8_t userStatus;
  uint8_t userType;
  uint8_t * id;
} sl_zcl_door_lock_cluster_set_rfid_command_t;

#define sl_zcl_door_lock_cluster_set_rfid_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_door_lock_cluster_set_rfid_command_t, userId), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_door_lock_cluster_set_rfid_command_t, userStatus), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_door_lock_cluster_set_rfid_command_t, userType), \
  (ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_door_lock_cluster_set_rfid_command_t, id), \
}


/** @brief Structure for ZCL command "GetPinResponse" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_get_pin_response_command {
  uint16_t userId;
  uint8_t userStatus;
  uint8_t userType;
  uint8_t * pin;
} sl_zcl_door_lock_cluster_get_pin_response_command_t;

#define sl_zcl_door_lock_cluster_get_pin_response_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_door_lock_cluster_get_pin_response_command_t, userId), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_door_lock_cluster_get_pin_response_command_t, userStatus), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_door_lock_cluster_get_pin_response_command_t, userType), \
  (ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_door_lock_cluster_get_pin_response_command_t, pin), \
}


/** @brief Structure for ZCL command "GetRfidResponse" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_get_rfid_response_command {
  uint16_t userId;
  uint8_t userStatus;
  uint8_t userType;
  uint8_t * rfid;
} sl_zcl_door_lock_cluster_get_rfid_response_command_t;

#define sl_zcl_door_lock_cluster_get_rfid_response_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_door_lock_cluster_get_rfid_response_command_t, userId), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_door_lock_cluster_get_rfid_response_command_t, userStatus), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_door_lock_cluster_get_rfid_response_command_t, userType), \
  (ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_door_lock_cluster_get_rfid_response_command_t, rfid), \
}


/** @brief Structure for ZCL command "EnhancedMoveToHue" from "Color Control" 
 */
typedef struct __zcl_color_control_cluster_enhanced_move_to_hue_command {
  uint16_t enhancedHue;
  uint8_t direction;
  uint16_t transitionTime;
  uint8_t optionsMask;
  uint8_t optionsOverride;
} sl_zcl_color_control_cluster_enhanced_move_to_hue_command_t;

#define sl_zcl_color_control_cluster_enhanced_move_to_hue_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_color_control_cluster_enhanced_move_to_hue_command_t, enhancedHue), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_color_control_cluster_enhanced_move_to_hue_command_t, direction), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_color_control_cluster_enhanced_move_to_hue_command_t, transitionTime), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_enhanced_move_to_hue_command_t, optionsMask), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_enhanced_move_to_hue_command_t, optionsOverride), \
}


/** @brief Structure for ZCL command "RequestTunnelResponse" from "Tunneling" 
 */
typedef struct __zcl_tunneling_cluster_request_tunnel_response_command {
  uint16_t tunnelId;
  uint8_t tunnelStatus;
  uint16_t maximumIncomingTransferSize;
} sl_zcl_tunneling_cluster_request_tunnel_response_command_t;

#define sl_zcl_tunneling_cluster_request_tunnel_response_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_tunneling_cluster_request_tunnel_response_command_t, tunnelId), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_tunneling_cluster_request_tunnel_response_command_t, tunnelStatus), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_tunneling_cluster_request_tunnel_response_command_t, maximumIncomingTransferSize), \
}


/** @brief Structure for ZCL command "WriteFunctions" from "Appliance Control" 
 */
typedef struct __zcl_appliance_control_cluster_write_functions_command {
  uint16_t functionId;
  uint8_t functionDataType;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * functionData;
} sl_zcl_appliance_control_cluster_write_functions_command_t;

#define sl_zcl_appliance_control_cluster_write_functions_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_appliance_control_cluster_write_functions_command_t, functionId), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_appliance_control_cluster_write_functions_command_t, functionDataType), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_appliance_control_cluster_write_functions_command_t, functionData), \
}


/** @brief Structure for ZCL command "AckTransferDataClientToServer" from "Tunneling" 
 */
typedef struct __zcl_tunneling_cluster_ack_transfer_data_client_to_server_command {
  uint16_t tunnelId;
  uint16_t numberOfBytesLeft;
} sl_zcl_tunneling_cluster_ack_transfer_data_client_to_server_command_t;

#define sl_zcl_tunneling_cluster_ack_transfer_data_client_to_server_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_tunneling_cluster_ack_transfer_data_client_to_server_command_t, tunnelId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_tunneling_cluster_ack_transfer_data_client_to_server_command_t, numberOfBytesLeft), \
}


/** @brief Structure for ZCL command "ReadyDataClientToServer" from "Tunneling" 
 */
typedef struct __zcl_tunneling_cluster_ready_data_client_to_server_command {
  uint16_t tunnelId;
  uint16_t numberOfOctetsLeft;
} sl_zcl_tunneling_cluster_ready_data_client_to_server_command_t;

#define sl_zcl_tunneling_cluster_ready_data_client_to_server_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_tunneling_cluster_ready_data_client_to_server_command_t, tunnelId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_tunneling_cluster_ready_data_client_to_server_command_t, numberOfOctetsLeft), \
}


/** @brief Structure for ZCL command "AckTransferDataServerToClient" from "Tunneling" 
 */
typedef struct __zcl_tunneling_cluster_ack_transfer_data_server_to_client_command {
  uint16_t tunnelId;
  uint16_t numberOfBytesLeft;
} sl_zcl_tunneling_cluster_ack_transfer_data_server_to_client_command_t;

#define sl_zcl_tunneling_cluster_ack_transfer_data_server_to_client_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_tunneling_cluster_ack_transfer_data_server_to_client_command_t, tunnelId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_tunneling_cluster_ack_transfer_data_server_to_client_command_t, numberOfBytesLeft), \
}


/** @brief Structure for ZCL command "ReadyDataServerToClient" from "Tunneling" 
 */
typedef struct __zcl_tunneling_cluster_ready_data_server_to_client_command {
  uint16_t tunnelId;
  uint16_t numberOfOctetsLeft;
} sl_zcl_tunneling_cluster_ready_data_server_to_client_command_t;

#define sl_zcl_tunneling_cluster_ready_data_server_to_client_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_tunneling_cluster_ready_data_server_to_client_command_t, tunnelId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_tunneling_cluster_ready_data_server_to_client_command_t, numberOfOctetsLeft), \
}


/** @brief Structure for ZCL command "LeaveChatRequest" from "Chatting" 
 */
typedef struct __zcl_chatting_cluster_leave_chat_request_command {
  uint16_t cid;
  uint16_t uid;
} sl_zcl_chatting_cluster_leave_chat_request_command_t;

#define sl_zcl_chatting_cluster_leave_chat_request_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_chatting_cluster_leave_chat_request_command_t, cid), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_chatting_cluster_leave_chat_request_command_t, uid), \
}


/** @brief Structure for ZCL command "SwitchChairmanResponse" from "Chatting" 
 */
typedef struct __zcl_chatting_cluster_switch_chairman_response_command {
  uint16_t cid;
  uint16_t uid;
} sl_zcl_chatting_cluster_switch_chairman_response_command_t;

#define sl_zcl_chatting_cluster_switch_chairman_response_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_chatting_cluster_switch_chairman_response_command_t, cid), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_chatting_cluster_switch_chairman_response_command_t, uid), \
}


/** @brief Structure for ZCL command "GetNodeInformationRequest" from "Chatting" 
 */
typedef struct __zcl_chatting_cluster_get_node_information_request_command {
  uint16_t cid;
  uint16_t uid;
} sl_zcl_chatting_cluster_get_node_information_request_command_t;

#define sl_zcl_chatting_cluster_get_node_information_request_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_chatting_cluster_get_node_information_request_command_t, cid), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_chatting_cluster_get_node_information_request_command_t, uid), \
}


/** @brief Structure for ZCL command "MoveToColorTemperature" from "Color Control" 
 */
typedef struct __zcl_color_control_cluster_move_to_color_temperature_command {
  uint16_t colorTemperature;
  uint16_t transitionTime;
  uint8_t optionsMask;
  uint8_t optionsOverride;
} sl_zcl_color_control_cluster_move_to_color_temperature_command_t;

#define sl_zcl_color_control_cluster_move_to_color_temperature_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_color_control_cluster_move_to_color_temperature_command_t, colorTemperature), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_color_control_cluster_move_to_color_temperature_command_t, transitionTime), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_move_to_color_temperature_command_t, optionsMask), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_move_to_color_temperature_command_t, optionsOverride), \
}


/** @brief Structure for ZCL command "UserLeft" from "Chatting" 
 */
typedef struct __zcl_chatting_cluster_user_left_command {
  uint16_t cid;
  uint16_t uid;
  uint8_t * nickname;
} sl_zcl_chatting_cluster_user_left_command_t;

#define sl_zcl_chatting_cluster_user_left_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_chatting_cluster_user_left_command_t, cid), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_chatting_cluster_user_left_command_t, uid), \
  (ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_chatting_cluster_user_left_command_t, nickname), \
}


/** @brief Structure for ZCL command "UserJoined" from "Chatting" 
 */
typedef struct __zcl_chatting_cluster_user_joined_command {
  uint16_t cid;
  uint16_t uid;
  uint8_t * nickname;
} sl_zcl_chatting_cluster_user_joined_command_t;

#define sl_zcl_chatting_cluster_user_joined_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_chatting_cluster_user_joined_command_t, cid), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_chatting_cluster_user_joined_command_t, uid), \
  (ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_chatting_cluster_user_joined_command_t, nickname), \
}


/** @brief Structure for ZCL command "SwitchChairmanNotification" from "Chatting" 
 */
typedef struct __zcl_chatting_cluster_switch_chairman_notification_command {
  uint16_t cid;
  uint16_t uid;
  uint16_t address;
  uint8_t endpoint;
} sl_zcl_chatting_cluster_switch_chairman_notification_command_t;

#define sl_zcl_chatting_cluster_switch_chairman_notification_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_chatting_cluster_switch_chairman_notification_command_t, cid), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_chatting_cluster_switch_chairman_notification_command_t, uid), \
  (ZAP_SIGNATURE_TYPE_DATA16), offsetof(sl_zcl_chatting_cluster_switch_chairman_notification_command_t, address), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_chatting_cluster_switch_chairman_notification_command_t, endpoint), \
}


/** @brief Structure for ZCL command "ModifyRecordRequest" from "Data Sharing" 
 */
typedef struct __zcl_data_sharing_cluster_modify_record_request_command {
  uint16_t fileIndex;
  uint16_t fileStartRecord;
  uint16_t recordCount;
} sl_zcl_data_sharing_cluster_modify_record_request_command_t;

#define sl_zcl_data_sharing_cluster_modify_record_request_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_data_sharing_cluster_modify_record_request_command_t, fileIndex), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_data_sharing_cluster_modify_record_request_command_t, fileStartRecord), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_data_sharing_cluster_modify_record_request_command_t, recordCount), \
}


/** @brief Structure for ZCL command "MoveToColor" from "Color Control" 
 */
typedef struct __zcl_color_control_cluster_move_to_color_command {
  uint16_t colorX;
  uint16_t colorY;
  uint16_t transitionTime;
  uint8_t optionsMask;
  uint8_t optionsOverride;
} sl_zcl_color_control_cluster_move_to_color_command_t;

#define sl_zcl_color_control_cluster_move_to_color_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_color_control_cluster_move_to_color_command_t, colorX), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_color_control_cluster_move_to_color_command_t, colorY), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_color_control_cluster_move_to_color_command_t, transitionTime), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_move_to_color_command_t, optionsMask), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_move_to_color_command_t, optionsOverride), \
}


/** @brief Structure for ZCL command "ChatMessage" from "Chatting" 
 */
typedef struct __zcl_chatting_cluster_chat_message_command {
  uint16_t destinationUid;
  uint16_t sourceUid;
  uint16_t cid;
  uint8_t * nickname;
  uint8_t * message;
} sl_zcl_chatting_cluster_chat_message_command_t;

#define sl_zcl_chatting_cluster_chat_message_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_chatting_cluster_chat_message_command_t, destinationUid), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_chatting_cluster_chat_message_command_t, sourceUid), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_chatting_cluster_chat_message_command_t, cid), \
  (ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_chatting_cluster_chat_message_command_t, nickname), \
  (ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_chatting_cluster_chat_message_command_t, message), \
}


/** @brief Structure for ZCL command "EnableWwahRejoinAlgorithm" from "SL Works With All Hubs" 
 */
typedef struct __zcl_sl_works_with_all_hubs_cluster_enable_wwah_rejoin_algorithm_command {
  uint16_t fastRejoinTimeoutSeconds;
  uint16_t durationBetweenRejoinsSeconds;
  uint16_t fastRejoinFirstBackoffSeconds;
  uint16_t maxBackoffTimeSeconds;
  uint16_t maxBackoffIterations;
} sl_zcl_sl_works_with_all_hubs_cluster_enable_wwah_rejoin_algorithm_command_t;

#define sl_zcl_sl_works_with_all_hubs_cluster_enable_wwah_rejoin_algorithm_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_sl_works_with_all_hubs_cluster_enable_wwah_rejoin_algorithm_command_t, fastRejoinTimeoutSeconds), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_sl_works_with_all_hubs_cluster_enable_wwah_rejoin_algorithm_command_t, durationBetweenRejoinsSeconds), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_sl_works_with_all_hubs_cluster_enable_wwah_rejoin_algorithm_command_t, fastRejoinFirstBackoffSeconds), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_sl_works_with_all_hubs_cluster_enable_wwah_rejoin_algorithm_command_t, maxBackoffTimeSeconds), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_sl_works_with_all_hubs_cluster_enable_wwah_rejoin_algorithm_command_t, maxBackoffIterations), \
}


/** @brief Structure for ZCL command "UpgradeEndResponse" from "Over the Air Bootloading" 
 */
typedef struct __zcl_over_the_air_bootloading_cluster_upgrade_end_response_command {
  uint16_t manufacturerId;
  uint16_t imageType;
  uint32_t fileVersion;
  uint32_t currentTime;
  uint32_t upgradeTime;
} sl_zcl_over_the_air_bootloading_cluster_upgrade_end_response_command_t;

#define sl_zcl_over_the_air_bootloading_cluster_upgrade_end_response_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_over_the_air_bootloading_cluster_upgrade_end_response_command_t, manufacturerId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_over_the_air_bootloading_cluster_upgrade_end_response_command_t, imageType), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_over_the_air_bootloading_cluster_upgrade_end_response_command_t, fileVersion), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_over_the_air_bootloading_cluster_upgrade_end_response_command_t, currentTime), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_over_the_air_bootloading_cluster_upgrade_end_response_command_t, upgradeTime), \
}


/** @brief Structure for ZCL command "GetMeasurementProfileCommand" from "Electrical Measurement" 
 */
typedef struct __zcl_electrical_measurement_cluster_get_measurement_profile_command_command {
  uint16_t attributeId;
  uint32_t startTime;
  uint8_t numberOfIntervals;
} sl_zcl_electrical_measurement_cluster_get_measurement_profile_command_command_t;

#define sl_zcl_electrical_measurement_cluster_get_measurement_profile_command_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_electrical_measurement_cluster_get_measurement_profile_command_command_t, attributeId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_electrical_measurement_cluster_get_measurement_profile_command_command_t, startTime), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_electrical_measurement_cluster_get_measurement_profile_command_command_t, numberOfIntervals), \
}


/** @brief Structure for ZCL command "GetLogRecordResponse" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_get_log_record_response_command {
  uint16_t logEntryId;
  uint32_t timestamp;
  uint8_t eventType;
  uint8_t source;
  uint8_t eventIdOrAlarmCode;
  uint16_t userId;
  uint8_t * pin;
} sl_zcl_door_lock_cluster_get_log_record_response_command_t;

#define sl_zcl_door_lock_cluster_get_log_record_response_command_signature  { \
  7, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_door_lock_cluster_get_log_record_response_command_t, logEntryId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_door_lock_cluster_get_log_record_response_command_t, timestamp), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_door_lock_cluster_get_log_record_response_command_t, eventType), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_get_log_record_response_command_t, source), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_get_log_record_response_command_t, eventIdOrAlarmCode), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_door_lock_cluster_get_log_record_response_command_t, userId), \
  (ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_door_lock_cluster_get_log_record_response_command_t, pin), \
}


/** @brief Structure for ZCL command "SetDisposableSchedule" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_set_disposable_schedule_command {
  uint16_t userId;
  uint32_t localStartTime;
  uint32_t localEndTime;
} sl_zcl_door_lock_cluster_set_disposable_schedule_command_t;

#define sl_zcl_door_lock_cluster_set_disposable_schedule_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_door_lock_cluster_set_disposable_schedule_command_t, userId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_door_lock_cluster_set_disposable_schedule_command_t, localStartTime), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_door_lock_cluster_set_disposable_schedule_command_t, localEndTime), \
}


/** @brief Structure for ZCL command "ModifyFileRequest" from "Data Sharing" 
 */
typedef struct __zcl_data_sharing_cluster_modify_file_request_command {
  uint16_t fileIndex;
  uint32_t fileStartPosition;
  uint32_t octetCount;
} sl_zcl_data_sharing_cluster_modify_file_request_command_t;

#define sl_zcl_data_sharing_cluster_modify_file_request_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_data_sharing_cluster_modify_file_request_command_t, fileIndex), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_data_sharing_cluster_modify_file_request_command_t, fileStartPosition), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_data_sharing_cluster_modify_file_request_command_t, octetCount), \
}


/** @brief Structure for ZCL command "GetOverallSchedulePriceResponse" from "Power Profile" 
 */
typedef struct __zcl_power_profile_cluster_get_overall_schedule_price_response_command {
  uint16_t currency;
  uint32_t price;
  uint8_t priceTrailingDigit;
} sl_zcl_power_profile_cluster_get_overall_schedule_price_response_command_t;

#define sl_zcl_power_profile_cluster_get_overall_schedule_price_response_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_power_profile_cluster_get_overall_schedule_price_response_command_t, currency), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_power_profile_cluster_get_overall_schedule_price_response_command_t, price), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_power_profile_cluster_get_overall_schedule_price_response_command_t, priceTrailingDigit), \
}


/** @brief Structure for ZCL command "ViewScene" from "Scenes" 
 */
typedef struct __zcl_scenes_cluster_view_scene_command {
  uint16_t groupId;
  uint8_t sceneId;
} sl_zcl_scenes_cluster_view_scene_command_t;

#define sl_zcl_scenes_cluster_view_scene_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_scenes_cluster_view_scene_command_t, groupId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_scenes_cluster_view_scene_command_t, sceneId), \
}


/** @brief Structure for ZCL command "RemoveScene" from "Scenes" 
 */
typedef struct __zcl_scenes_cluster_remove_scene_command {
  uint16_t groupId;
  uint8_t sceneId;
} sl_zcl_scenes_cluster_remove_scene_command_t;

#define sl_zcl_scenes_cluster_remove_scene_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_scenes_cluster_remove_scene_command_t, groupId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_scenes_cluster_remove_scene_command_t, sceneId), \
}


/** @brief Structure for ZCL command "StoreScene" from "Scenes" 
 */
typedef struct __zcl_scenes_cluster_store_scene_command {
  uint16_t groupId;
  uint8_t sceneId;
} sl_zcl_scenes_cluster_store_scene_command_t;

#define sl_zcl_scenes_cluster_store_scene_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_scenes_cluster_store_scene_command_t, groupId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_scenes_cluster_store_scene_command_t, sceneId), \
}


/** @brief Structure for ZCL command "EnhancedViewScene" from "Scenes" 
 */
typedef struct __zcl_scenes_cluster_enhanced_view_scene_command {
  uint16_t groupId;
  uint8_t sceneId;
} sl_zcl_scenes_cluster_enhanced_view_scene_command_t;

#define sl_zcl_scenes_cluster_enhanced_view_scene_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_scenes_cluster_enhanced_view_scene_command_t, groupId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_scenes_cluster_enhanced_view_scene_command_t, sceneId), \
}


/** @brief Structure for ZCL command "SetUserStatus" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_set_user_status_command {
  uint16_t userId;
  uint8_t userStatus;
} sl_zcl_door_lock_cluster_set_user_status_command_t;

#define sl_zcl_door_lock_cluster_set_user_status_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_door_lock_cluster_set_user_status_command_t, userId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_set_user_status_command_t, userStatus), \
}


/** @brief Structure for ZCL command "GetUserStatusResponse" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_get_user_status_response_command {
  uint16_t userId;
  uint8_t status;
} sl_zcl_door_lock_cluster_get_user_status_response_command_t;

#define sl_zcl_door_lock_cluster_get_user_status_response_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_door_lock_cluster_get_user_status_response_command_t, userId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_get_user_status_response_command_t, status), \
}


/** @brief Structure for ZCL command "EnhancedMoveToHueAndSaturation" from "Color Control" 
 */
typedef struct __zcl_color_control_cluster_enhanced_move_to_hue_and_saturation_command {
  uint16_t enhancedHue;
  uint8_t saturation;
  uint16_t transitionTime;
  uint8_t optionsMask;
  uint8_t optionsOverride;
} sl_zcl_color_control_cluster_enhanced_move_to_hue_and_saturation_command_t;

#define sl_zcl_color_control_cluster_enhanced_move_to_hue_and_saturation_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_color_control_cluster_enhanced_move_to_hue_and_saturation_command_t, enhancedHue), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_color_control_cluster_enhanced_move_to_hue_and_saturation_command_t, saturation), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_color_control_cluster_enhanced_move_to_hue_and_saturation_command_t, transitionTime), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_enhanced_move_to_hue_and_saturation_command_t, optionsMask), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_enhanced_move_to_hue_and_saturation_command_t, optionsOverride), \
}


/** @brief Structure for ZCL command "AddScene" from "Scenes" 
 */
typedef struct __zcl_scenes_cluster_add_scene_command {
  uint16_t groupId;
  uint8_t sceneId;
  uint16_t transitionTime;
  uint8_t * sceneName;
  /* TYPE WARNING: SceneExtensionFieldSet array defaults to */ uint8_t * extensionFieldSets;
} sl_zcl_scenes_cluster_add_scene_command_t;

#define sl_zcl_scenes_cluster_add_scene_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_scenes_cluster_add_scene_command_t, groupId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_scenes_cluster_add_scene_command_t, sceneId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_scenes_cluster_add_scene_command_t, transitionTime), \
  (ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_scenes_cluster_add_scene_command_t, sceneName), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_scenes_cluster_add_scene_command_t, extensionFieldSets), \
}


/** @brief Structure for ZCL command "EnhancedAddScene" from "Scenes" 
 */
typedef struct __zcl_scenes_cluster_enhanced_add_scene_command {
  uint16_t groupId;
  uint8_t sceneId;
  uint16_t transitionTime;
  uint8_t * sceneName;
  /* TYPE WARNING: SceneExtensionFieldSet array defaults to */ uint8_t * extensionFieldSets;
} sl_zcl_scenes_cluster_enhanced_add_scene_command_t;

#define sl_zcl_scenes_cluster_enhanced_add_scene_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_scenes_cluster_enhanced_add_scene_command_t, groupId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_scenes_cluster_enhanced_add_scene_command_t, sceneId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_scenes_cluster_enhanced_add_scene_command_t, transitionTime), \
  (ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_scenes_cluster_enhanced_add_scene_command_t, sceneName), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_scenes_cluster_enhanced_add_scene_command_t, extensionFieldSets), \
}


/** @brief Structure for ZCL command "RecallScene" from "Scenes" 
 */
typedef struct __zcl_scenes_cluster_recall_scene_command {
  uint16_t groupId;
  uint8_t sceneId;
  uint16_t transitionTime;
} sl_zcl_scenes_cluster_recall_scene_command_t;

#define sl_zcl_scenes_cluster_recall_scene_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_scenes_cluster_recall_scene_command_t, groupId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_scenes_cluster_recall_scene_command_t, sceneId), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_scenes_cluster_recall_scene_command_t, transitionTime), \
}


/** @brief Structure for ZCL command "GetDisposableScheduleResponse" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_get_disposable_schedule_response_command {
  uint16_t userId;
  uint8_t status;
  uint32_t localStartTime;
  uint32_t localEndTime;
} sl_zcl_door_lock_cluster_get_disposable_schedule_response_command_t;

#define sl_zcl_door_lock_cluster_get_disposable_schedule_response_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_door_lock_cluster_get_disposable_schedule_response_command_t, userId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_get_disposable_schedule_response_command_t, status), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_door_lock_cluster_get_disposable_schedule_response_command_t, localStartTime), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_door_lock_cluster_get_disposable_schedule_response_command_t, localEndTime), \
}


/** @brief Structure for ZCL command "PublishEventLog" from "Events" 
 */
typedef struct __zcl_events_cluster_publish_event_log_command {
  uint16_t totalNumberOfEvents;
  uint8_t commandIndex;
  uint8_t totalCommands;
  uint8_t logPayloadControl;
  /* TYPE WARNING: EventLogPayload array defaults to */ uint8_t * logPayload;
} sl_zcl_events_cluster_publish_event_log_command_t;

#define sl_zcl_events_cluster_publish_event_log_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_events_cluster_publish_event_log_command_t, totalNumberOfEvents), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_events_cluster_publish_event_log_command_t, commandIndex), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_events_cluster_publish_event_log_command_t, totalCommands), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_events_cluster_publish_event_log_command_t, logPayloadControl), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_events_cluster_publish_event_log_command_t, logPayload), \
}


/** @brief Structure for ZCL command "TransferDataClientToServer" from "Tunneling" 
 */
typedef struct __zcl_tunneling_cluster_transfer_data_client_to_server_command {
  uint16_t tunnelId;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * data;
} sl_zcl_tunneling_cluster_transfer_data_client_to_server_command_t;

#define sl_zcl_tunneling_cluster_transfer_data_client_to_server_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_tunneling_cluster_transfer_data_client_to_server_command_t, tunnelId), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_tunneling_cluster_transfer_data_client_to_server_command_t, data), \
}


/** @brief Structure for ZCL command "TransferDataServerToClient" from "Tunneling" 
 */
typedef struct __zcl_tunneling_cluster_transfer_data_server_to_client_command {
  uint16_t tunnelId;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * data;
} sl_zcl_tunneling_cluster_transfer_data_server_to_client_command_t;

#define sl_zcl_tunneling_cluster_transfer_data_server_to_client_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_tunneling_cluster_transfer_data_server_to_client_command_t, tunnelId), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_tunneling_cluster_transfer_data_server_to_client_command_t, data), \
}


/** @brief Structure for ZCL command "SendPreference" from "Information" 
 */
typedef struct __zcl_information_cluster_send_preference_command {
  uint16_t preferenceType;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * preferencePayload;
} sl_zcl_information_cluster_send_preference_command_t;

#define sl_zcl_information_cluster_send_preference_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_information_cluster_send_preference_command_t, preferenceType), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_information_cluster_send_preference_command_t, preferencePayload), \
}


/** @brief Structure for ZCL command "ReadFileRequest" from "Data Sharing" 
 */
typedef struct __zcl_data_sharing_cluster_read_file_request_command {
  uint16_t fileIndex;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * fileStartPositionAndRequestedOctetCount;
} sl_zcl_data_sharing_cluster_read_file_request_command_t;

#define sl_zcl_data_sharing_cluster_read_file_request_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_data_sharing_cluster_read_file_request_command_t, fileIndex), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_data_sharing_cluster_read_file_request_command_t, fileStartPositionAndRequestedOctetCount), \
}


/** @brief Structure for ZCL command "ReadRecordRequest" from "Data Sharing" 
 */
typedef struct __zcl_data_sharing_cluster_read_record_request_command {
  uint16_t fileIndex;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * fileStartRecordAndRequestedRecordCount;
} sl_zcl_data_sharing_cluster_read_record_request_command_t;

#define sl_zcl_data_sharing_cluster_read_record_request_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_data_sharing_cluster_read_record_request_command_t, fileIndex), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_data_sharing_cluster_read_record_request_command_t, fileStartRecordAndRequestedRecordCount), \
}


/** @brief Structure for ZCL command "SetToken" from "Configuration Cluster" 
 */
typedef struct __zcl_configuration_cluster_cluster_set_token_command {
  uint16_t token;
  uint8_t * data;
} sl_zcl_configuration_cluster_cluster_set_token_command_t;

#define sl_zcl_configuration_cluster_cluster_set_token_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_configuration_cluster_cluster_set_token_command_t, token), \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_configuration_cluster_cluster_set_token_command_t, data), \
}


/** @brief Structure for ZCL command "ReturnToken" from "Configuration Cluster" 
 */
typedef struct __zcl_configuration_cluster_cluster_return_token_command {
  uint16_t token;
  uint8_t * data;
} sl_zcl_configuration_cluster_cluster_return_token_command_t;

#define sl_zcl_configuration_cluster_cluster_return_token_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_configuration_cluster_cluster_return_token_command_t, token), \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_configuration_cluster_cluster_return_token_command_t, data), \
}


/** @brief Structure for ZCL command "SwitchChairmanConfirm" from "Chatting" 
 */
typedef struct __zcl_chatting_cluster_switch_chairman_confirm_command {
  uint16_t cid;
  /* TYPE WARNING: NodeInformation array defaults to */ uint8_t * nodeInformationList;
} sl_zcl_chatting_cluster_switch_chairman_confirm_command_t;

#define sl_zcl_chatting_cluster_switch_chairman_confirm_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_chatting_cluster_switch_chairman_confirm_command_t, cid), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_chatting_cluster_switch_chairman_confirm_command_t, nodeInformationList), \
}


/** @brief Structure for ZCL command "GetSampledData" from "Simple Metering" 
 */
typedef struct __zcl_simple_metering_cluster_get_sampled_data_command {
  uint16_t sampleId;
  uint32_t earliestSampleTime;
  uint8_t sampleType;
  uint16_t numberOfSamples;
} sl_zcl_simple_metering_cluster_get_sampled_data_command_t;

#define sl_zcl_simple_metering_cluster_get_sampled_data_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_simple_metering_cluster_get_sampled_data_command_t, sampleId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_simple_metering_cluster_get_sampled_data_command_t, earliestSampleTime), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_simple_metering_cluster_get_sampled_data_command_t, sampleType), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_simple_metering_cluster_get_sampled_data_command_t, numberOfSamples), \
}


/** @brief Structure for ZCL command "GetSampledDataResponse" from "Simple Metering" 
 */
typedef struct __zcl_simple_metering_cluster_get_sampled_data_response_command {
  uint16_t sampleId;
  uint32_t sampleStartTime;
  uint8_t sampleType;
  uint16_t sampleRequestInterval;
  uint16_t numberOfSamples;
  /* TYPE WARNING: INT24U array defaults to */ uint8_t * samples;
} sl_zcl_simple_metering_cluster_get_sampled_data_response_command_t;

#define sl_zcl_simple_metering_cluster_get_sampled_data_response_command_signature  { \
  6, \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_simple_metering_cluster_get_sampled_data_response_command_t, sampleId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_simple_metering_cluster_get_sampled_data_response_command_t, sampleStartTime), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_simple_metering_cluster_get_sampled_data_response_command_t, sampleType), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_simple_metering_cluster_get_sampled_data_response_command_t, sampleRequestInterval), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_simple_metering_cluster_get_sampled_data_response_command_t, numberOfSamples), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_simple_metering_cluster_get_sampled_data_response_command_t, samples), \
}


/** @brief Structure for ZCL command "SetLongPollInterval" from "Poll Control" 
 */
typedef struct __zcl_poll_control_cluster_set_long_poll_interval_command {
  uint32_t newLongPollInterval;
} sl_zcl_poll_control_cluster_set_long_poll_interval_command_t;

#define sl_zcl_poll_control_cluster_set_long_poll_interval_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_poll_control_cluster_set_long_poll_interval_command_t, newLongPollInterval), \
}


/** @brief Structure for ZCL command "GetPriceMatrix" from "Price" 
 */
typedef struct __zcl_price_cluster_get_price_matrix_command {
  uint32_t issuerTariffId;
} sl_zcl_price_cluster_get_price_matrix_command_t;

#define sl_zcl_price_cluster_get_price_matrix_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_get_price_matrix_command_t, issuerTariffId), \
}


/** @brief Structure for ZCL command "GetBlockThresholds" from "Price" 
 */
typedef struct __zcl_price_cluster_get_block_thresholds_command {
  uint32_t issuerTariffId;
} sl_zcl_price_cluster_get_block_thresholds_command_t;

#define sl_zcl_price_cluster_get_block_thresholds_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_get_block_thresholds_command_t, issuerTariffId), \
}


/** @brief Structure for ZCL command "GetTierLabels" from "Price" 
 */
typedef struct __zcl_price_cluster_get_tier_labels_command {
  uint32_t issuerTariffId;
} sl_zcl_price_cluster_get_tier_labels_command_t;

#define sl_zcl_price_cluster_get_tier_labels_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_get_tier_labels_command_t, issuerTariffId), \
}


/** @brief Structure for ZCL command "SetLowCreditWarningLevel" from "Prepayment" 
 */
typedef struct __zcl_prepayment_cluster_set_low_credit_warning_level_command {
  uint32_t lowCreditWarningLevel;
} sl_zcl_prepayment_cluster_set_low_credit_warning_level_command_t;

#define sl_zcl_prepayment_cluster_set_low_credit_warning_level_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_prepayment_cluster_set_low_credit_warning_level_command_t, lowCreditWarningLevel), \
}


/** @brief Structure for ZCL command "ConfigurePushInformationTimer" from "Information" 
 */
typedef struct __zcl_information_cluster_configure_push_information_timer_command {
  uint32_t timer;
} sl_zcl_information_cluster_configure_push_information_timer_command_t;

#define sl_zcl_information_cluster_configure_push_information_timer_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_information_cluster_configure_push_information_timer_command_t, timer), \
}


/** @brief Structure for ZCL command "LogRequest" from "Appliance Statistics" 
 */
typedef struct __zcl_appliance_statistics_cluster_log_request_command {
  uint32_t logId;
} sl_zcl_appliance_statistics_cluster_log_request_command_t;

#define sl_zcl_appliance_statistics_cluster_log_request_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_appliance_statistics_cluster_log_request_command_t, logId), \
}


/** @brief Structure for ZCL command "ResetToFactoryNewRequest" from "ZLL Commissioning" 
 */
typedef struct __zcl_zll_commissioning_cluster_reset_to_factory_new_request_command {
  uint32_t transaction;
} sl_zcl_zll_commissioning_cluster_reset_to_factory_new_request_command_t;

#define sl_zcl_zll_commissioning_cluster_reset_to_factory_new_request_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_zll_commissioning_cluster_reset_to_factory_new_request_command_t, transaction), \
}


/** @brief Structure for ZCL command "CancelLoadControlEvent" from "Demand Response and Load Control" 
 */
typedef struct __zcl_demand_response_and_load_control_cluster_cancel_load_control_event_command {
  uint32_t issuerEventId;
  uint16_t deviceClass;
  uint8_t utilityEnrollmentGroup;
  uint8_t cancelControl;
  uint32_t effectiveTime;
} sl_zcl_demand_response_and_load_control_cluster_cancel_load_control_event_command_t;

#define sl_zcl_demand_response_and_load_control_cluster_cancel_load_control_event_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_demand_response_and_load_control_cluster_cancel_load_control_event_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_BITMAP16), offsetof(sl_zcl_demand_response_and_load_control_cluster_cancel_load_control_event_command_t, deviceClass), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_demand_response_and_load_control_cluster_cancel_load_control_event_command_t, utilityEnrollmentGroup), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_demand_response_and_load_control_cluster_cancel_load_control_event_command_t, cancelControl), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_demand_response_and_load_control_cluster_cancel_load_control_event_command_t, effectiveTime), \
}


/** @brief Structure for ZCL command "ManageEvent" from "Energy Management" 
 */
typedef struct __zcl_energy_management_cluster_manage_event_command {
  uint32_t issuerEventId;
  uint16_t deviceClass;
  uint8_t utilityEnrollmentGroup;
  uint8_t actionRequired;
} sl_zcl_energy_management_cluster_manage_event_command_t;

#define sl_zcl_energy_management_cluster_manage_event_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_energy_management_cluster_manage_event_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_BITMAP16), offsetof(sl_zcl_energy_management_cluster_manage_event_command_t, deviceClass), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_energy_management_cluster_manage_event_command_t, utilityEnrollmentGroup), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_energy_management_cluster_manage_event_command_t, actionRequired), \
}


/** @brief Structure for ZCL command "LoadControlEvent" from "Demand Response and Load Control" 
 */
typedef struct __zcl_demand_response_and_load_control_cluster_load_control_event_command {
  uint32_t issuerEventId;
  uint16_t deviceClass;
  uint8_t utilityEnrollmentGroup;
  uint32_t startTime;
  uint16_t durationInMinutes;
  uint8_t criticalityLevel;
  uint8_t coolingTemperatureOffset;
  uint8_t heatingTemperatureOffset;
  int16_t coolingTemperatureSetPoint;
  int16_t heatingTemperatureSetPoint;
  int8_t averageLoadAdjustmentPercentage;
  uint8_t dutyCycle;
  uint8_t eventControl;
} sl_zcl_demand_response_and_load_control_cluster_load_control_event_command_t;

#define sl_zcl_demand_response_and_load_control_cluster_load_control_event_command_signature  { \
  13, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_demand_response_and_load_control_cluster_load_control_event_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_BITMAP16), offsetof(sl_zcl_demand_response_and_load_control_cluster_load_control_event_command_t, deviceClass), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_demand_response_and_load_control_cluster_load_control_event_command_t, utilityEnrollmentGroup), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_demand_response_and_load_control_cluster_load_control_event_command_t, startTime), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_demand_response_and_load_control_cluster_load_control_event_command_t, durationInMinutes), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_demand_response_and_load_control_cluster_load_control_event_command_t, criticalityLevel), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_demand_response_and_load_control_cluster_load_control_event_command_t, coolingTemperatureOffset), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_demand_response_and_load_control_cluster_load_control_event_command_t, heatingTemperatureOffset), \
  (ZAP_SIGNATURE_TYPE_INT16S), offsetof(sl_zcl_demand_response_and_load_control_cluster_load_control_event_command_t, coolingTemperatureSetPoint), \
  (ZAP_SIGNATURE_TYPE_INT16S), offsetof(sl_zcl_demand_response_and_load_control_cluster_load_control_event_command_t, heatingTemperatureSetPoint), \
  (ZAP_SIGNATURE_TYPE_INT8S), offsetof(sl_zcl_demand_response_and_load_control_cluster_load_control_event_command_t, averageLoadAdjustmentPercentage), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_demand_response_and_load_control_cluster_load_control_event_command_t, dutyCycle), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_demand_response_and_load_control_cluster_load_control_event_command_t, eventControl), \
}


/** @brief Structure for ZCL command "CancelMessage" from "Messaging" 
 */
typedef struct __zcl_messaging_cluster_cancel_message_command {
  uint32_t messageId;
  uint8_t messageControl;
} sl_zcl_messaging_cluster_cancel_message_command_t;

#define sl_zcl_messaging_cluster_cancel_message_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_messaging_cluster_cancel_message_command_t, messageId), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_messaging_cluster_cancel_message_command_t, messageControl), \
}


/** @brief Structure for ZCL command "ScanRequest" from "ZLL Commissioning" 
 */
typedef struct __zcl_zll_commissioning_cluster_scan_request_command {
  uint32_t transaction;
  uint8_t zigbeeInformation;
  uint8_t zllInformation;
} sl_zcl_zll_commissioning_cluster_scan_request_command_t;

#define sl_zcl_zll_commissioning_cluster_scan_request_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_zll_commissioning_cluster_scan_request_command_t, transaction), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_zll_commissioning_cluster_scan_request_command_t, zigbeeInformation), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_zll_commissioning_cluster_scan_request_command_t, zllInformation), \
}


/** @brief Structure for ZCL command "DisplayProtectedMessage" from "Messaging" 
 */
typedef struct __zcl_messaging_cluster_display_protected_message_command {
  uint32_t messageId;
  uint8_t messageControl;
  uint32_t startTime;
  uint16_t durationInMinutes;
  uint8_t * message;
  uint8_t optionalExtendedMessageControl;
} sl_zcl_messaging_cluster_display_protected_message_command_t;

#define sl_zcl_messaging_cluster_display_protected_message_command_signature  { \
  6, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_messaging_cluster_display_protected_message_command_t, messageId), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_messaging_cluster_display_protected_message_command_t, messageControl), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_messaging_cluster_display_protected_message_command_t, startTime), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_messaging_cluster_display_protected_message_command_t, durationInMinutes), \
  (ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_messaging_cluster_display_protected_message_command_t, message), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_messaging_cluster_display_protected_message_command_t, optionalExtendedMessageControl), \
}


/** @brief Structure for ZCL command "DisplayMessage" from "Messaging" 
 */
typedef struct __zcl_messaging_cluster_display_message_command {
  uint32_t messageId;
  uint8_t messageControl;
  uint32_t startTime;
  uint16_t durationInMinutes;
  uint8_t * message;
  uint8_t optionalExtendedMessageControl;
} sl_zcl_messaging_cluster_display_message_command_t;

#define sl_zcl_messaging_cluster_display_message_command_signature  { \
  6, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_messaging_cluster_display_message_command_t, messageId), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_messaging_cluster_display_message_command_t, messageControl), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_messaging_cluster_display_message_command_t, startTime), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_messaging_cluster_display_message_command_t, durationInMinutes), \
  (ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_messaging_cluster_display_message_command_t, message), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_messaging_cluster_display_message_command_t, optionalExtendedMessageControl), \
}


/** @brief Structure for ZCL command "CppEventResponse" from "Price" 
 */
typedef struct __zcl_price_cluster_cpp_event_response_command {
  uint32_t issuerEventId;
  uint8_t cppAuth;
} sl_zcl_price_cluster_cpp_event_response_command_t;

#define sl_zcl_price_cluster_cpp_event_response_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_cpp_event_response_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_price_cluster_cpp_event_response_command_t, cppAuth), \
}


/** @brief Structure for ZCL command "TakeSnapshotResponse" from "Simple Metering" 
 */
typedef struct __zcl_simple_metering_cluster_take_snapshot_response_command {
  uint32_t snapshotId;
  uint8_t snapshotConfirmation;
} sl_zcl_simple_metering_cluster_take_snapshot_response_command_t;

#define sl_zcl_simple_metering_cluster_take_snapshot_response_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_simple_metering_cluster_take_snapshot_response_command_t, snapshotId), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_simple_metering_cluster_take_snapshot_response_command_t, snapshotConfirmation), \
}


/** @brief Structure for ZCL command "NetworkJoinRouterResponse" from "ZLL Commissioning" 
 */
typedef struct __zcl_zll_commissioning_cluster_network_join_router_response_command {
  uint32_t transaction;
  uint8_t status;
} sl_zcl_zll_commissioning_cluster_network_join_router_response_command_t;

#define sl_zcl_zll_commissioning_cluster_network_join_router_response_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_zll_commissioning_cluster_network_join_router_response_command_t, transaction), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_zll_commissioning_cluster_network_join_router_response_command_t, status), \
}


/** @brief Structure for ZCL command "NetworkJoinEndDeviceResponse" from "ZLL Commissioning" 
 */
typedef struct __zcl_zll_commissioning_cluster_network_join_end_device_response_command {
  uint32_t transaction;
  uint8_t status;
} sl_zcl_zll_commissioning_cluster_network_join_end_device_response_command_t;

#define sl_zcl_zll_commissioning_cluster_network_join_end_device_response_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_zll_commissioning_cluster_network_join_end_device_response_command_t, transaction), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_zll_commissioning_cluster_network_join_end_device_response_command_t, status), \
}


/** @brief Structure for ZCL command "SetSupplyStatus" from "Simple Metering" 
 */
typedef struct __zcl_simple_metering_cluster_set_supply_status_command {
  uint32_t issuerEventId;
  uint8_t supplyTamperState;
  uint8_t supplyDepletionState;
  uint8_t supplyUncontrolledFlowState;
  uint8_t loadLimitSupplyState;
} sl_zcl_simple_metering_cluster_set_supply_status_command_t;

#define sl_zcl_simple_metering_cluster_set_supply_status_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_simple_metering_cluster_set_supply_status_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_simple_metering_cluster_set_supply_status_command_t, supplyTamperState), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_simple_metering_cluster_set_supply_status_command_t, supplyDepletionState), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_simple_metering_cluster_set_supply_status_command_t, supplyUncontrolledFlowState), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_simple_metering_cluster_set_supply_status_command_t, loadLimitSupplyState), \
}


/** @brief Structure for ZCL command "GetMeasurementProfileResponseCommand" from "Electrical Measurement" 
 */
typedef struct __zcl_electrical_measurement_cluster_get_measurement_profile_response_command_command {
  uint32_t startTime;
  uint8_t status;
  uint8_t profileIntervalPeriod;
  uint8_t numberOfIntervalsDelivered;
  uint16_t attributeId;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * intervals;
} sl_zcl_electrical_measurement_cluster_get_measurement_profile_response_command_command_t;

#define sl_zcl_electrical_measurement_cluster_get_measurement_profile_response_command_command_signature  { \
  6, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_electrical_measurement_cluster_get_measurement_profile_response_command_command_t, startTime), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_electrical_measurement_cluster_get_measurement_profile_response_command_command_t, status), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_electrical_measurement_cluster_get_measurement_profile_response_command_command_t, profileIntervalPeriod), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_electrical_measurement_cluster_get_measurement_profile_response_command_command_t, numberOfIntervalsDelivered), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_electrical_measurement_cluster_get_measurement_profile_response_command_command_t, attributeId), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_electrical_measurement_cluster_get_measurement_profile_response_command_command_t, intervals), \
}


/** @brief Structure for ZCL command "NetworkStartResponse" from "ZLL Commissioning" 
 */
typedef struct __zcl_zll_commissioning_cluster_network_start_response_command {
  uint32_t transaction;
  uint8_t status;
  uint8_t * extendedPanId;
  uint8_t networkUpdateId;
  uint8_t logicalChannel;
  uint16_t panId;
} sl_zcl_zll_commissioning_cluster_network_start_response_command_t;

#define sl_zcl_zll_commissioning_cluster_network_start_response_command_signature  { \
  6, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_zll_commissioning_cluster_network_start_response_command_t, transaction), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_zll_commissioning_cluster_network_start_response_command_t, status), \
  (ZAP_SIGNATURE_TYPE_IEEE_ADDRESS), offsetof(sl_zcl_zll_commissioning_cluster_network_start_response_command_t, extendedPanId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_zll_commissioning_cluster_network_start_response_command_t, networkUpdateId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_zll_commissioning_cluster_network_start_response_command_t, logicalChannel), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_network_start_response_command_t, panId), \
}


/** @brief Structure for ZCL command "ReportEventStatus" from "Energy Management" 
 */
typedef struct __zcl_energy_management_cluster_report_event_status_command {
  uint32_t issuerEventId;
  uint8_t eventStatus;
  uint32_t eventStatusTime;
  uint8_t criticalityLevelApplied;
  uint16_t coolingTemperatureSetPointApplied;
  uint16_t heatingTemperatureSetPointApplied;
  int8_t averageLoadAdjustmentPercentageApplied;
  uint8_t dutyCycleApplied;
  uint8_t eventControl;
} sl_zcl_energy_management_cluster_report_event_status_command_t;

#define sl_zcl_energy_management_cluster_report_event_status_command_signature  { \
  9, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_energy_management_cluster_report_event_status_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_energy_management_cluster_report_event_status_command_t, eventStatus), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_energy_management_cluster_report_event_status_command_t, eventStatusTime), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_energy_management_cluster_report_event_status_command_t, criticalityLevelApplied), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_energy_management_cluster_report_event_status_command_t, coolingTemperatureSetPointApplied), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_energy_management_cluster_report_event_status_command_t, heatingTemperatureSetPointApplied), \
  (ZAP_SIGNATURE_TYPE_INT8S), offsetof(sl_zcl_energy_management_cluster_report_event_status_command_t, averageLoadAdjustmentPercentageApplied), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_energy_management_cluster_report_event_status_command_t, dutyCycleApplied), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_energy_management_cluster_report_event_status_command_t, eventControl), \
}


/** @brief Structure for ZCL command "ReportEventStatus" from "Demand Response and Load Control" 
 */
typedef struct __zcl_demand_response_and_load_control_cluster_report_event_status_command {
  uint32_t issuerEventId;
  uint8_t eventStatus;
  uint32_t eventStatusTime;
  uint8_t criticalityLevelApplied;
  uint16_t coolingTemperatureSetPointApplied;
  uint16_t heatingTemperatureSetPointApplied;
  int8_t averageLoadAdjustmentPercentageApplied;
  uint8_t dutyCycleApplied;
  uint8_t eventControl;
  uint8_t signatureType;
  /* TYPE WARNING: Signature defaults to */ uint8_t * signature;
} sl_zcl_demand_response_and_load_control_cluster_report_event_status_command_t;

#define sl_zcl_demand_response_and_load_control_cluster_report_event_status_command_signature  { \
  11, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_demand_response_and_load_control_cluster_report_event_status_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_demand_response_and_load_control_cluster_report_event_status_command_t, eventStatus), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_demand_response_and_load_control_cluster_report_event_status_command_t, eventStatusTime), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_demand_response_and_load_control_cluster_report_event_status_command_t, criticalityLevelApplied), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_demand_response_and_load_control_cluster_report_event_status_command_t, coolingTemperatureSetPointApplied), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_demand_response_and_load_control_cluster_report_event_status_command_t, heatingTemperatureSetPointApplied), \
  (ZAP_SIGNATURE_TYPE_INT8S), offsetof(sl_zcl_demand_response_and_load_control_cluster_report_event_status_command_t, averageLoadAdjustmentPercentageApplied), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_demand_response_and_load_control_cluster_report_event_status_command_t, dutyCycleApplied), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_demand_response_and_load_control_cluster_report_event_status_command_t, eventControl), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_demand_response_and_load_control_cluster_report_event_status_command_t, signatureType), \
  (ZAP_SIGNATURE_TYPE_POINTER), offsetof(sl_zcl_demand_response_and_load_control_cluster_report_event_status_command_t, signature), \
}


/** @brief Structure for ZCL command "PairingRequest" from "MDU Pairing" 
 */
typedef struct __zcl_mdu_pairing_cluster_pairing_request_command {
  uint32_t localPairingInformationVersion;
  uint8_t * eui64OfRequestingDevice;
} sl_zcl_mdu_pairing_cluster_pairing_request_command_t;

#define sl_zcl_mdu_pairing_cluster_pairing_request_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_mdu_pairing_cluster_pairing_request_command_t, localPairingInformationVersion), \
  (ZAP_SIGNATURE_TYPE_IEEE_ADDRESS), offsetof(sl_zcl_mdu_pairing_cluster_pairing_request_command_t, eui64OfRequestingDevice), \
}


/** @brief Structure for ZCL command "NetworkStartRequest" from "ZLL Commissioning" 
 */
typedef struct __zcl_zll_commissioning_cluster_network_start_request_command {
  uint32_t transaction;
  uint8_t * extendedPanId;
  uint8_t keyIndex;
  /* TYPE WARNING: security_key defaults to */ uint8_t *  encryptedNetworkKey;
  uint8_t logicalChannel;
  uint16_t panId;
  uint16_t networkAddress;
  uint16_t groupIdentifiersBegin;
  uint16_t groupIdentifiersEnd;
  uint16_t freeNetworkAddressRangeBegin;
  uint16_t freeNetworkAddressRangeEnd;
  uint16_t freeGroupIdentifierRangeBegin;
  uint16_t freeGroupIdentifierRangeEnd;
  uint8_t * initiatorIeeeAddress;
  uint16_t initiatorNetworkAddress;
} sl_zcl_zll_commissioning_cluster_network_start_request_command_t;

#define sl_zcl_zll_commissioning_cluster_network_start_request_command_signature  { \
  15, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_zll_commissioning_cluster_network_start_request_command_t, transaction), \
  (ZAP_SIGNATURE_TYPE_IEEE_ADDRESS), offsetof(sl_zcl_zll_commissioning_cluster_network_start_request_command_t, extendedPanId), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_zll_commissioning_cluster_network_start_request_command_t, keyIndex), \
  (ZAP_SIGNATURE_TYPE_SECURITY_KEY), offsetof(sl_zcl_zll_commissioning_cluster_network_start_request_command_t, encryptedNetworkKey), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_zll_commissioning_cluster_network_start_request_command_t, logicalChannel), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_network_start_request_command_t, panId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_network_start_request_command_t, networkAddress), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_network_start_request_command_t, groupIdentifiersBegin), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_network_start_request_command_t, groupIdentifiersEnd), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_network_start_request_command_t, freeNetworkAddressRangeBegin), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_network_start_request_command_t, freeNetworkAddressRangeEnd), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_network_start_request_command_t, freeGroupIdentifierRangeBegin), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_network_start_request_command_t, freeGroupIdentifierRangeEnd), \
  (ZAP_SIGNATURE_TYPE_IEEE_ADDRESS), offsetof(sl_zcl_zll_commissioning_cluster_network_start_request_command_t, initiatorIeeeAddress), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_network_start_request_command_t, initiatorNetworkAddress), \
}


/** @brief Structure for ZCL command "NetworkJoinRouterRequest" from "ZLL Commissioning" 
 */
typedef struct __zcl_zll_commissioning_cluster_network_join_router_request_command {
  uint32_t transaction;
  uint8_t * extendedPanId;
  uint8_t keyIndex;
  /* TYPE WARNING: security_key defaults to */ uint8_t *  encryptedNetworkKey;
  uint8_t networkUpdateId;
  uint8_t logicalChannel;
  uint16_t panId;
  uint16_t networkAddress;
  uint16_t groupIdentifiersBegin;
  uint16_t groupIdentifiersEnd;
  uint16_t freeNetworkAddressRangeBegin;
  uint16_t freeNetworkAddressRangeEnd;
  uint16_t freeGroupIdentifierRangeBegin;
  uint16_t freeGroupIdentifierRangeEnd;
} sl_zcl_zll_commissioning_cluster_network_join_router_request_command_t;

#define sl_zcl_zll_commissioning_cluster_network_join_router_request_command_signature  { \
  14, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_zll_commissioning_cluster_network_join_router_request_command_t, transaction), \
  (ZAP_SIGNATURE_TYPE_IEEE_ADDRESS), offsetof(sl_zcl_zll_commissioning_cluster_network_join_router_request_command_t, extendedPanId), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_zll_commissioning_cluster_network_join_router_request_command_t, keyIndex), \
  (ZAP_SIGNATURE_TYPE_SECURITY_KEY), offsetof(sl_zcl_zll_commissioning_cluster_network_join_router_request_command_t, encryptedNetworkKey), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_zll_commissioning_cluster_network_join_router_request_command_t, networkUpdateId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_zll_commissioning_cluster_network_join_router_request_command_t, logicalChannel), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_network_join_router_request_command_t, panId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_network_join_router_request_command_t, networkAddress), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_network_join_router_request_command_t, groupIdentifiersBegin), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_network_join_router_request_command_t, groupIdentifiersEnd), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_network_join_router_request_command_t, freeNetworkAddressRangeBegin), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_network_join_router_request_command_t, freeNetworkAddressRangeEnd), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_network_join_router_request_command_t, freeGroupIdentifierRangeBegin), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_network_join_router_request_command_t, freeGroupIdentifierRangeEnd), \
}


/** @brief Structure for ZCL command "NetworkJoinEndDeviceRequest" from "ZLL Commissioning" 
 */
typedef struct __zcl_zll_commissioning_cluster_network_join_end_device_request_command {
  uint32_t transaction;
  uint8_t * extendedPanId;
  uint8_t keyIndex;
  /* TYPE WARNING: security_key defaults to */ uint8_t *  encryptedNetworkKey;
  uint8_t networkUpdateId;
  uint8_t logicalChannel;
  uint16_t panId;
  uint16_t networkAddress;
  uint16_t groupIdentifiersBegin;
  uint16_t groupIdentifiersEnd;
  uint16_t freeNetworkAddressRangeBegin;
  uint16_t freeNetworkAddressRangeEnd;
  uint16_t freeGroupIdentifierRangeBegin;
  uint16_t freeGroupIdentifierRangeEnd;
} sl_zcl_zll_commissioning_cluster_network_join_end_device_request_command_t;

#define sl_zcl_zll_commissioning_cluster_network_join_end_device_request_command_signature  { \
  14, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_zll_commissioning_cluster_network_join_end_device_request_command_t, transaction), \
  (ZAP_SIGNATURE_TYPE_IEEE_ADDRESS), offsetof(sl_zcl_zll_commissioning_cluster_network_join_end_device_request_command_t, extendedPanId), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_zll_commissioning_cluster_network_join_end_device_request_command_t, keyIndex), \
  (ZAP_SIGNATURE_TYPE_SECURITY_KEY), offsetof(sl_zcl_zll_commissioning_cluster_network_join_end_device_request_command_t, encryptedNetworkKey), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_zll_commissioning_cluster_network_join_end_device_request_command_t, networkUpdateId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_zll_commissioning_cluster_network_join_end_device_request_command_t, logicalChannel), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_network_join_end_device_request_command_t, panId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_network_join_end_device_request_command_t, networkAddress), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_network_join_end_device_request_command_t, groupIdentifiersBegin), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_network_join_end_device_request_command_t, groupIdentifiersEnd), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_network_join_end_device_request_command_t, freeNetworkAddressRangeBegin), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_network_join_end_device_request_command_t, freeNetworkAddressRangeEnd), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_network_join_end_device_request_command_t, freeGroupIdentifierRangeBegin), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_network_join_end_device_request_command_t, freeGroupIdentifierRangeEnd), \
}


/** @brief Structure for ZCL command "NetworkUpdateRequest" from "ZLL Commissioning" 
 */
typedef struct __zcl_zll_commissioning_cluster_network_update_request_command {
  uint32_t transaction;
  uint8_t * extendedPanId;
  uint8_t networkUpdateId;
  uint8_t logicalChannel;
  uint16_t panId;
  uint16_t networkAddress;
} sl_zcl_zll_commissioning_cluster_network_update_request_command_t;

#define sl_zcl_zll_commissioning_cluster_network_update_request_command_signature  { \
  6, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_zll_commissioning_cluster_network_update_request_command_t, transaction), \
  (ZAP_SIGNATURE_TYPE_IEEE_ADDRESS), offsetof(sl_zcl_zll_commissioning_cluster_network_update_request_command_t, extendedPanId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_zll_commissioning_cluster_network_update_request_command_t, networkUpdateId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_zll_commissioning_cluster_network_update_request_command_t, logicalChannel), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_network_update_request_command_t, panId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_network_update_request_command_t, networkAddress), \
}


/** @brief Structure for ZCL command "IdentifyRequest" from "ZLL Commissioning" 
 */
typedef struct __zcl_zll_commissioning_cluster_identify_request_command {
  uint32_t transaction;
  uint16_t identifyDuration;
} sl_zcl_zll_commissioning_cluster_identify_request_command_t;

#define sl_zcl_zll_commissioning_cluster_identify_request_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_zll_commissioning_cluster_identify_request_command_t, transaction), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_identify_request_command_t, identifyDuration), \
}


/** @brief Structure for ZCL command "ConfigureMirror" from "Simple Metering" 
 */
typedef struct __zcl_simple_metering_cluster_configure_mirror_command {
  uint32_t issuerEventId;
  uint32_t reportingInterval;
  uint8_t mirrorNotificationReporting;
  uint8_t notificationScheme;
} sl_zcl_simple_metering_cluster_configure_mirror_command_t;

#define sl_zcl_simple_metering_cluster_configure_mirror_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_simple_metering_cluster_configure_mirror_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_INT24U), offsetof(sl_zcl_simple_metering_cluster_configure_mirror_command_t, reportingInterval), \
  (ZAP_SIGNATURE_TYPE_BOOLEAN), offsetof(sl_zcl_simple_metering_cluster_configure_mirror_command_t, mirrorNotificationReporting), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_simple_metering_cluster_configure_mirror_command_t, notificationScheme), \
}


/** @brief Structure for ZCL command "ResetLoadLimitCounter" from "Simple Metering" 
 */
typedef struct __zcl_simple_metering_cluster_reset_load_limit_counter_command {
  uint32_t providerId;
  uint32_t issuerEventId;
} sl_zcl_simple_metering_cluster_reset_load_limit_counter_command_t;

#define sl_zcl_simple_metering_cluster_reset_load_limit_counter_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_simple_metering_cluster_reset_load_limit_counter_command_t, providerId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_simple_metering_cluster_reset_load_limit_counter_command_t, issuerEventId), \
}


/** @brief Structure for ZCL command "GetSeasons" from "Calendar" 
 */
typedef struct __zcl_calendar_cluster_get_seasons_command {
  uint32_t providerId;
  uint32_t issuerCalendarId;
} sl_zcl_calendar_cluster_get_seasons_command_t;

#define sl_zcl_calendar_cluster_get_seasons_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_calendar_cluster_get_seasons_command_t, providerId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_calendar_cluster_get_seasons_command_t, issuerCalendarId), \
}


/** @brief Structure for ZCL command "CancelTariff" from "Price" 
 */
typedef struct __zcl_price_cluster_cancel_tariff_command {
  uint32_t providerId;
  uint32_t issuerTariffId;
  uint8_t tariffType;
} sl_zcl_price_cluster_cancel_tariff_command_t;

#define sl_zcl_price_cluster_cancel_tariff_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_cancel_tariff_command_t, providerId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_cancel_tariff_command_t, issuerTariffId), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_price_cluster_cancel_tariff_command_t, tariffType), \
}


/** @brief Structure for ZCL command "CancelCalendar" from "Calendar" 
 */
typedef struct __zcl_calendar_cluster_cancel_calendar_command {
  uint32_t providerId;
  uint32_t issuerCalendarId;
  uint8_t calendarType;
} sl_zcl_calendar_cluster_cancel_calendar_command_t;

#define sl_zcl_calendar_cluster_cancel_calendar_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_calendar_cluster_cancel_calendar_command_t, providerId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_calendar_cluster_cancel_calendar_command_t, issuerCalendarId), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_calendar_cluster_cancel_calendar_command_t, calendarType), \
}


/** @brief Structure for ZCL command "PublishChangeOfSupplier" from "Device Management" 
 */
typedef struct __zcl_device_management_cluster_publish_change_of_supplier_command {
  uint32_t currentProviderId;
  uint32_t issuerEventId;
  uint8_t tariffType;
  uint32_t proposedProviderId;
  uint32_t providerChangeImplementationTime;
  uint32_t providerChangeControl;
  uint8_t * proposedProviderName;
  uint8_t * proposedProviderContactDetails;
} sl_zcl_device_management_cluster_publish_change_of_supplier_command_t;

#define sl_zcl_device_management_cluster_publish_change_of_supplier_command_signature  { \
  8, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_device_management_cluster_publish_change_of_supplier_command_t, currentProviderId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_device_management_cluster_publish_change_of_supplier_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_device_management_cluster_publish_change_of_supplier_command_t, tariffType), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_device_management_cluster_publish_change_of_supplier_command_t, proposedProviderId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_device_management_cluster_publish_change_of_supplier_command_t, providerChangeImplementationTime), \
  (ZAP_SIGNATURE_TYPE_BITMAP32), offsetof(sl_zcl_device_management_cluster_publish_change_of_supplier_command_t, providerChangeControl), \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_device_management_cluster_publish_change_of_supplier_command_t, proposedProviderName), \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_device_management_cluster_publish_change_of_supplier_command_t, proposedProviderContactDetails), \
}


/** @brief Structure for ZCL command "PublishChangeOfTenancy" from "Device Management" 
 */
typedef struct __zcl_device_management_cluster_publish_change_of_tenancy_command {
  uint32_t providerId;
  uint32_t issuerEventId;
  uint8_t tariffType;
  uint32_t implementationDateTime;
  uint32_t proposedTenancyChangeControl;
} sl_zcl_device_management_cluster_publish_change_of_tenancy_command_t;

#define sl_zcl_device_management_cluster_publish_change_of_tenancy_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_device_management_cluster_publish_change_of_tenancy_command_t, providerId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_device_management_cluster_publish_change_of_tenancy_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_device_management_cluster_publish_change_of_tenancy_command_t, tariffType), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_device_management_cluster_publish_change_of_tenancy_command_t, implementationDateTime), \
  (ZAP_SIGNATURE_TYPE_BITMAP32), offsetof(sl_zcl_device_management_cluster_publish_change_of_tenancy_command_t, proposedTenancyChangeControl), \
}


/** @brief Structure for ZCL command "SetUncontrolledFlowThreshold" from "Simple Metering" 
 */
typedef struct __zcl_simple_metering_cluster_set_uncontrolled_flow_threshold_command {
  uint32_t providerId;
  uint32_t issuerEventId;
  uint16_t uncontrolledFlowThreshold;
  uint8_t unitOfMeasure;
  uint16_t multiplier;
  uint16_t divisor;
  uint8_t stabilisationPeriod;
  uint16_t measurementPeriod;
} sl_zcl_simple_metering_cluster_set_uncontrolled_flow_threshold_command_t;

#define sl_zcl_simple_metering_cluster_set_uncontrolled_flow_threshold_command_signature  { \
  8, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_simple_metering_cluster_set_uncontrolled_flow_threshold_command_t, providerId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_simple_metering_cluster_set_uncontrolled_flow_threshold_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_simple_metering_cluster_set_uncontrolled_flow_threshold_command_t, uncontrolledFlowThreshold), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_simple_metering_cluster_set_uncontrolled_flow_threshold_command_t, unitOfMeasure), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_simple_metering_cluster_set_uncontrolled_flow_threshold_command_t, multiplier), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_simple_metering_cluster_set_uncontrolled_flow_threshold_command_t, divisor), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_simple_metering_cluster_set_uncontrolled_flow_threshold_command_t, stabilisationPeriod), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_simple_metering_cluster_set_uncontrolled_flow_threshold_command_t, measurementPeriod), \
}


/** @brief Structure for ZCL command "PowerDescriptorChange" from "SL Works With All Hubs" 
 */
typedef struct __zcl_sl_works_with_all_hubs_cluster_power_descriptor_change_command {
  uint32_t currentPowerMode;
  uint32_t availablePowerSources;
  uint32_t currentPowerSource;
  uint32_t currentPowerSourceLevel;
} sl_zcl_sl_works_with_all_hubs_cluster_power_descriptor_change_command_t;

#define sl_zcl_sl_works_with_all_hubs_cluster_power_descriptor_change_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_sl_works_with_all_hubs_cluster_power_descriptor_change_command_t, currentPowerMode), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_sl_works_with_all_hubs_cluster_power_descriptor_change_command_t, availablePowerSources), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_sl_works_with_all_hubs_cluster_power_descriptor_change_command_t, currentPowerSource), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_sl_works_with_all_hubs_cluster_power_descriptor_change_command_t, currentPowerSourceLevel), \
}


/** @brief Structure for ZCL command "PublishDayProfile" from "Calendar" 
 */
typedef struct __zcl_calendar_cluster_publish_day_profile_command {
  uint32_t providerId;
  uint32_t issuerEventId;
  uint32_t issuerCalendarId;
  uint8_t dayId;
  uint8_t totalNumberOfScheduleEntries;
  uint8_t commandIndex;
  uint8_t totalNumberOfCommands;
  uint8_t calendarType;
  /* TYPE WARNING: ScheduleEntry array defaults to */ uint8_t * dayScheduleEntries;
} sl_zcl_calendar_cluster_publish_day_profile_command_t;

#define sl_zcl_calendar_cluster_publish_day_profile_command_signature  { \
  9, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_calendar_cluster_publish_day_profile_command_t, providerId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_calendar_cluster_publish_day_profile_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_calendar_cluster_publish_day_profile_command_t, issuerCalendarId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_calendar_cluster_publish_day_profile_command_t, dayId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_calendar_cluster_publish_day_profile_command_t, totalNumberOfScheduleEntries), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_calendar_cluster_publish_day_profile_command_t, commandIndex), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_calendar_cluster_publish_day_profile_command_t, totalNumberOfCommands), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_calendar_cluster_publish_day_profile_command_t, calendarType), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_calendar_cluster_publish_day_profile_command_t, dayScheduleEntries), \
}


/** @brief Structure for ZCL command "PublishWeekProfile" from "Calendar" 
 */
typedef struct __zcl_calendar_cluster_publish_week_profile_command {
  uint32_t providerId;
  uint32_t issuerEventId;
  uint32_t issuerCalendarId;
  uint8_t weekId;
  uint8_t dayIdRefMonday;
  uint8_t dayIdRefTuesday;
  uint8_t dayIdRefWednesday;
  uint8_t dayIdRefThursday;
  uint8_t dayIdRefFriday;
  uint8_t dayIdRefSaturday;
  uint8_t dayIdRefSunday;
} sl_zcl_calendar_cluster_publish_week_profile_command_t;

#define sl_zcl_calendar_cluster_publish_week_profile_command_signature  { \
  11, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_calendar_cluster_publish_week_profile_command_t, providerId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_calendar_cluster_publish_week_profile_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_calendar_cluster_publish_week_profile_command_t, issuerCalendarId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_calendar_cluster_publish_week_profile_command_t, weekId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_calendar_cluster_publish_week_profile_command_t, dayIdRefMonday), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_calendar_cluster_publish_week_profile_command_t, dayIdRefTuesday), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_calendar_cluster_publish_week_profile_command_t, dayIdRefWednesday), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_calendar_cluster_publish_week_profile_command_t, dayIdRefThursday), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_calendar_cluster_publish_week_profile_command_t, dayIdRefFriday), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_calendar_cluster_publish_week_profile_command_t, dayIdRefSaturday), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_calendar_cluster_publish_week_profile_command_t, dayIdRefSunday), \
}


/** @brief Structure for ZCL command "PublishTierLabels" from "Price" 
 */
typedef struct __zcl_price_cluster_publish_tier_labels_command {
  uint32_t providerId;
  uint32_t issuerEventId;
  uint32_t issuerTariffId;
  uint8_t commandIndex;
  uint8_t numberOfCommands;
  uint8_t numberOfLabels;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * tierLabelsPayload;
} sl_zcl_price_cluster_publish_tier_labels_command_t;

#define sl_zcl_price_cluster_publish_tier_labels_command_signature  { \
  7, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_tier_labels_command_t, providerId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_tier_labels_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_tier_labels_command_t, issuerTariffId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_price_cluster_publish_tier_labels_command_t, commandIndex), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_price_cluster_publish_tier_labels_command_t, numberOfCommands), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_price_cluster_publish_tier_labels_command_t, numberOfLabels), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_price_cluster_publish_tier_labels_command_t, tierLabelsPayload), \
}


/** @brief Structure for ZCL command "PublishSeasons" from "Calendar" 
 */
typedef struct __zcl_calendar_cluster_publish_seasons_command {
  uint32_t providerId;
  uint32_t issuerEventId;
  uint32_t issuerCalendarId;
  uint8_t commandIndex;
  uint8_t totalNumberOfCommands;
  /* TYPE WARNING: SeasonEntry array defaults to */ uint8_t * seasonEntries;
} sl_zcl_calendar_cluster_publish_seasons_command_t;

#define sl_zcl_calendar_cluster_publish_seasons_command_signature  { \
  6, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_calendar_cluster_publish_seasons_command_t, providerId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_calendar_cluster_publish_seasons_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_calendar_cluster_publish_seasons_command_t, issuerCalendarId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_calendar_cluster_publish_seasons_command_t, commandIndex), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_calendar_cluster_publish_seasons_command_t, totalNumberOfCommands), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_calendar_cluster_publish_seasons_command_t, seasonEntries), \
}


/** @brief Structure for ZCL command "PublishTariffInformation" from "Price" 
 */
typedef struct __zcl_price_cluster_publish_tariff_information_command {
  uint32_t providerId;
  uint32_t issuerEventId;
  uint32_t issuerTariffId;
  uint32_t startTime;
  uint8_t tariffTypeChargingScheme;
  uint8_t * tariffLabel;
  uint8_t numberOfPriceTiersInUse;
  uint8_t numberOfBlockThresholdsInUse;
  uint8_t unitOfMeasure;
  uint16_t currency;
  uint8_t priceTrailingDigit;
  uint32_t standingCharge;
  uint8_t tierBlockMode;
  uint32_t blockThresholdMultiplier;
  uint32_t blockThresholdDivisor;
} sl_zcl_price_cluster_publish_tariff_information_command_t;

#define sl_zcl_price_cluster_publish_tariff_information_command_signature  { \
  15, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_tariff_information_command_t, providerId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_tariff_information_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_tariff_information_command_t, issuerTariffId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_price_cluster_publish_tariff_information_command_t, startTime), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_price_cluster_publish_tariff_information_command_t, tariffTypeChargingScheme), \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_price_cluster_publish_tariff_information_command_t, tariffLabel), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_price_cluster_publish_tariff_information_command_t, numberOfPriceTiersInUse), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_price_cluster_publish_tariff_information_command_t, numberOfBlockThresholdsInUse), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_price_cluster_publish_tariff_information_command_t, unitOfMeasure), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_price_cluster_publish_tariff_information_command_t, currency), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_price_cluster_publish_tariff_information_command_t, priceTrailingDigit), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_tariff_information_command_t, standingCharge), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_price_cluster_publish_tariff_information_command_t, tierBlockMode), \
  (ZAP_SIGNATURE_TYPE_INT24U), offsetof(sl_zcl_price_cluster_publish_tariff_information_command_t, blockThresholdMultiplier), \
  (ZAP_SIGNATURE_TYPE_INT24U), offsetof(sl_zcl_price_cluster_publish_tariff_information_command_t, blockThresholdDivisor), \
}


/** @brief Structure for ZCL command "PublishCalendar" from "Calendar" 
 */
typedef struct __zcl_calendar_cluster_publish_calendar_command {
  uint32_t providerId;
  uint32_t issuerEventId;
  uint32_t issuerCalendarId;
  uint32_t startTime;
  uint8_t calendarType;
  uint8_t calendarTimeReference;
  uint8_t * calendarName;
  uint8_t numberOfSeasons;
  uint8_t numberOfWeekProfiles;
  uint8_t numberOfDayProfiles;
} sl_zcl_calendar_cluster_publish_calendar_command_t;

#define sl_zcl_calendar_cluster_publish_calendar_command_signature  { \
  10, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_calendar_cluster_publish_calendar_command_t, providerId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_calendar_cluster_publish_calendar_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_calendar_cluster_publish_calendar_command_t, issuerCalendarId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_calendar_cluster_publish_calendar_command_t, startTime), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_calendar_cluster_publish_calendar_command_t, calendarType), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_calendar_cluster_publish_calendar_command_t, calendarTimeReference), \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_calendar_cluster_publish_calendar_command_t, calendarName), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_calendar_cluster_publish_calendar_command_t, numberOfSeasons), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_calendar_cluster_publish_calendar_command_t, numberOfWeekProfiles), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_calendar_cluster_publish_calendar_command_t, numberOfDayProfiles), \
}


/** @brief Structure for ZCL command "PublishSpecialDays" from "Calendar" 
 */
typedef struct __zcl_calendar_cluster_publish_special_days_command {
  uint32_t providerId;
  uint32_t issuerEventId;
  uint32_t issuerCalendarId;
  uint32_t startTime;
  uint8_t calendarType;
  uint8_t totalNumberOfSpecialDays;
  uint8_t commandIndex;
  uint8_t totalNumberOfCommands;
  /* TYPE WARNING: SpecialDay array defaults to */ uint8_t * specialDayEntries;
} sl_zcl_calendar_cluster_publish_special_days_command_t;

#define sl_zcl_calendar_cluster_publish_special_days_command_signature  { \
  9, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_calendar_cluster_publish_special_days_command_t, providerId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_calendar_cluster_publish_special_days_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_calendar_cluster_publish_special_days_command_t, issuerCalendarId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_calendar_cluster_publish_special_days_command_t, startTime), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_calendar_cluster_publish_special_days_command_t, calendarType), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_calendar_cluster_publish_special_days_command_t, totalNumberOfSpecialDays), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_calendar_cluster_publish_special_days_command_t, commandIndex), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_calendar_cluster_publish_special_days_command_t, totalNumberOfCommands), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_calendar_cluster_publish_special_days_command_t, specialDayEntries), \
}


/** @brief Structure for ZCL command "GetDayProfiles" from "Calendar" 
 */
typedef struct __zcl_calendar_cluster_get_day_profiles_command {
  uint32_t providerId;
  uint32_t issuerCalendarId;
  uint8_t startDayId;
  uint8_t numberOfDays;
} sl_zcl_calendar_cluster_get_day_profiles_command_t;

#define sl_zcl_calendar_cluster_get_day_profiles_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_calendar_cluster_get_day_profiles_command_t, providerId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_calendar_cluster_get_day_profiles_command_t, issuerCalendarId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_calendar_cluster_get_day_profiles_command_t, startDayId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_calendar_cluster_get_day_profiles_command_t, numberOfDays), \
}


/** @brief Structure for ZCL command "GetWeekProfiles" from "Calendar" 
 */
typedef struct __zcl_calendar_cluster_get_week_profiles_command {
  uint32_t providerId;
  uint32_t issuerCalendarId;
  uint8_t startWeekId;
  uint8_t numberOfWeeks;
} sl_zcl_calendar_cluster_get_week_profiles_command_t;

#define sl_zcl_calendar_cluster_get_week_profiles_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_calendar_cluster_get_week_profiles_command_t, providerId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_calendar_cluster_get_week_profiles_command_t, issuerCalendarId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_calendar_cluster_get_week_profiles_command_t, startWeekId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_calendar_cluster_get_week_profiles_command_t, numberOfWeeks), \
}


/** @brief Structure for ZCL command "ChangePaymentMode" from "Prepayment" 
 */
typedef struct __zcl_prepayment_cluster_change_payment_mode_command {
  uint32_t providerId;
  uint32_t issuerEventId;
  uint32_t implementationDateTime;
  uint16_t proposedPaymentControlConfiguration;
  uint32_t cutOffValue;
} sl_zcl_prepayment_cluster_change_payment_mode_command_t;

#define sl_zcl_prepayment_cluster_change_payment_mode_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_prepayment_cluster_change_payment_mode_command_t, providerId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_prepayment_cluster_change_payment_mode_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_prepayment_cluster_change_payment_mode_command_t, implementationDateTime), \
  (ZAP_SIGNATURE_TYPE_BITMAP16), offsetof(sl_zcl_prepayment_cluster_change_payment_mode_command_t, proposedPaymentControlConfiguration), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_prepayment_cluster_change_payment_mode_command_t, cutOffValue), \
}


/** @brief Structure for ZCL command "PublishBillingPeriod" from "Price" 
 */
typedef struct __zcl_price_cluster_publish_billing_period_command {
  uint32_t providerId;
  uint32_t issuerEventId;
  uint32_t billingPeriodStartTime;
  uint32_t billingPeriodDuration;
  uint8_t billingPeriodDurationType;
  uint8_t tariffType;
} sl_zcl_price_cluster_publish_billing_period_command_t;

#define sl_zcl_price_cluster_publish_billing_period_command_signature  { \
  6, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_billing_period_command_t, providerId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_billing_period_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_price_cluster_publish_billing_period_command_t, billingPeriodStartTime), \
  (ZAP_SIGNATURE_TYPE_BITMAP24), offsetof(sl_zcl_price_cluster_publish_billing_period_command_t, billingPeriodDuration), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_price_cluster_publish_billing_period_command_t, billingPeriodDurationType), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_price_cluster_publish_billing_period_command_t, tariffType), \
}


/** @brief Structure for ZCL command "PublishConsolidatedBill" from "Price" 
 */
typedef struct __zcl_price_cluster_publish_consolidated_bill_command {
  uint32_t providerId;
  uint32_t issuerEventId;
  uint32_t billingPeriodStartTime;
  uint32_t billingPeriodDuration;
  uint8_t billingPeriodDurationType;
  uint8_t tariffType;
  uint32_t consolidatedBill;
  uint16_t currency;
  uint8_t billTrailingDigit;
} sl_zcl_price_cluster_publish_consolidated_bill_command_t;

#define sl_zcl_price_cluster_publish_consolidated_bill_command_signature  { \
  9, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_consolidated_bill_command_t, providerId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_consolidated_bill_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_price_cluster_publish_consolidated_bill_command_t, billingPeriodStartTime), \
  (ZAP_SIGNATURE_TYPE_BITMAP24), offsetof(sl_zcl_price_cluster_publish_consolidated_bill_command_t, billingPeriodDuration), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_price_cluster_publish_consolidated_bill_command_t, billingPeriodDurationType), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_price_cluster_publish_consolidated_bill_command_t, tariffType), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_consolidated_bill_command_t, consolidatedBill), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_price_cluster_publish_consolidated_bill_command_t, currency), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_price_cluster_publish_consolidated_bill_command_t, billTrailingDigit), \
}


/** @brief Structure for ZCL command "PriceAcknowledgement" from "Price" 
 */
typedef struct __zcl_price_cluster_price_acknowledgement_command {
  uint32_t providerId;
  uint32_t issuerEventId;
  uint32_t priceAckTime;
  uint8_t control;
} sl_zcl_price_cluster_price_acknowledgement_command_t;

#define sl_zcl_price_cluster_price_acknowledgement_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_price_acknowledgement_command_t, providerId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_price_acknowledgement_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_price_cluster_price_acknowledgement_command_t, priceAckTime), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_price_cluster_price_acknowledgement_command_t, control), \
}


/** @brief Structure for ZCL command "SupplyStatusResponse" from "Simple Metering" 
 */
typedef struct __zcl_simple_metering_cluster_supply_status_response_command {
  uint32_t providerId;
  uint32_t issuerEventId;
  uint32_t implementationDateTime;
  uint8_t supplyStatus;
} sl_zcl_simple_metering_cluster_supply_status_response_command_t;

#define sl_zcl_simple_metering_cluster_supply_status_response_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_simple_metering_cluster_supply_status_response_command_t, providerId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_simple_metering_cluster_supply_status_response_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_simple_metering_cluster_supply_status_response_command_t, implementationDateTime), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_simple_metering_cluster_supply_status_response_command_t, supplyStatus), \
}


/** @brief Structure for ZCL command "PublishCO2Value" from "Price" 
 */
typedef struct __zcl_price_cluster_publish_co2_value_command {
  uint32_t providerId;
  uint32_t issuerEventId;
  uint32_t startTime;
  uint8_t tariffType;
  uint32_t cO2Value;
  uint8_t cO2ValueUnit;
  uint8_t cO2ValueTrailingDigit;
} sl_zcl_price_cluster_publish_co2_value_command_t;

#define sl_zcl_price_cluster_publish_co2_value_command_signature  { \
  7, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_co2_value_command_t, providerId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_co2_value_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_price_cluster_publish_co2_value_command_t, startTime), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_price_cluster_publish_co2_value_command_t, tariffType), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_co2_value_command_t, cO2Value), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_price_cluster_publish_co2_value_command_t, cO2ValueUnit), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_price_cluster_publish_co2_value_command_t, cO2ValueTrailingDigit), \
}


/** @brief Structure for ZCL command "PublishCppEvent" from "Price" 
 */
typedef struct __zcl_price_cluster_publish_cpp_event_command {
  uint32_t providerId;
  uint32_t issuerEventId;
  uint32_t startTime;
  uint16_t durationInMinutes;
  uint8_t tariffType;
  uint8_t cppPriceTier;
  uint8_t cppAuth;
} sl_zcl_price_cluster_publish_cpp_event_command_t;

#define sl_zcl_price_cluster_publish_cpp_event_command_signature  { \
  7, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_cpp_event_command_t, providerId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_cpp_event_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_price_cluster_publish_cpp_event_command_t, startTime), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_price_cluster_publish_cpp_event_command_t, durationInMinutes), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_price_cluster_publish_cpp_event_command_t, tariffType), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_price_cluster_publish_cpp_event_command_t, cppPriceTier), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_price_cluster_publish_cpp_event_command_t, cppAuth), \
}


/** @brief Structure for ZCL command "PublishCurrencyConversion" from "Price" 
 */
typedef struct __zcl_price_cluster_publish_currency_conversion_command {
  uint32_t providerId;
  uint32_t issuerEventId;
  uint32_t startTime;
  uint16_t oldCurrency;
  uint16_t newCurrency;
  uint32_t conversionFactor;
  uint8_t conversionFactorTrailingDigit;
  uint32_t currencyChangeControlFlags;
} sl_zcl_price_cluster_publish_currency_conversion_command_t;

#define sl_zcl_price_cluster_publish_currency_conversion_command_signature  { \
  8, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_currency_conversion_command_t, providerId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_currency_conversion_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_price_cluster_publish_currency_conversion_command_t, startTime), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_price_cluster_publish_currency_conversion_command_t, oldCurrency), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_price_cluster_publish_currency_conversion_command_t, newCurrency), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_currency_conversion_command_t, conversionFactor), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_price_cluster_publish_currency_conversion_command_t, conversionFactorTrailingDigit), \
  (ZAP_SIGNATURE_TYPE_BITMAP32), offsetof(sl_zcl_price_cluster_publish_currency_conversion_command_t, currencyChangeControlFlags), \
}


/** @brief Structure for ZCL command "PublishBlockPeriod" from "Price" 
 */
typedef struct __zcl_price_cluster_publish_block_period_command {
  uint32_t providerId;
  uint32_t issuerEventId;
  uint32_t blockPeriodStartTime;
  uint32_t blockPeriodDuration;
  uint8_t blockPeriodControl;
  uint8_t blockPeriodDurationType;
  uint8_t tariffType;
  uint8_t tariffResolutionPeriod;
} sl_zcl_price_cluster_publish_block_period_command_t;

#define sl_zcl_price_cluster_publish_block_period_command_signature  { \
  8, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_block_period_command_t, providerId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_block_period_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_price_cluster_publish_block_period_command_t, blockPeriodStartTime), \
  (ZAP_SIGNATURE_TYPE_INT24U), offsetof(sl_zcl_price_cluster_publish_block_period_command_t, blockPeriodDuration), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_price_cluster_publish_block_period_command_t, blockPeriodControl), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_price_cluster_publish_block_period_command_t, blockPeriodDurationType), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_price_cluster_publish_block_period_command_t, tariffType), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_price_cluster_publish_block_period_command_t, tariffResolutionPeriod), \
}


/** @brief Structure for ZCL command "SetOverallDebtCap" from "Prepayment" 
 */
typedef struct __zcl_prepayment_cluster_set_overall_debt_cap_command {
  uint32_t providerId;
  uint32_t issuerEventId;
  uint32_t implementationDateTime;
  uint32_t overallDebtCap;
} sl_zcl_prepayment_cluster_set_overall_debt_cap_command_t;

#define sl_zcl_prepayment_cluster_set_overall_debt_cap_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_prepayment_cluster_set_overall_debt_cap_command_t, providerId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_prepayment_cluster_set_overall_debt_cap_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_prepayment_cluster_set_overall_debt_cap_command_t, implementationDateTime), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_prepayment_cluster_set_overall_debt_cap_command_t, overallDebtCap), \
}


/** @brief Structure for ZCL command "PublishCreditPayment" from "Price" 
 */
typedef struct __zcl_price_cluster_publish_credit_payment_command {
  uint32_t providerId;
  uint32_t issuerEventId;
  uint32_t creditPaymentDueDate;
  uint32_t creditPaymentOverDueAmount;
  uint8_t creditPaymentStatus;
  uint32_t creditPayment;
  uint32_t creditPaymentDate;
  uint8_t * creditPaymentRef;
} sl_zcl_price_cluster_publish_credit_payment_command_t;

#define sl_zcl_price_cluster_publish_credit_payment_command_signature  { \
  8, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_credit_payment_command_t, providerId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_credit_payment_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_price_cluster_publish_credit_payment_command_t, creditPaymentDueDate), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_credit_payment_command_t, creditPaymentOverDueAmount), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_price_cluster_publish_credit_payment_command_t, creditPaymentStatus), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_credit_payment_command_t, creditPayment), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_price_cluster_publish_credit_payment_command_t, creditPaymentDate), \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_price_cluster_publish_credit_payment_command_t, creditPaymentRef), \
}


/** @brief Structure for ZCL command "SetMaximumCreditLimit" from "Prepayment" 
 */
typedef struct __zcl_prepayment_cluster_set_maximum_credit_limit_command {
  uint32_t providerId;
  uint32_t issuerEventId;
  uint32_t implementationDateTime;
  uint32_t maximumCreditLevel;
  uint32_t maximumCreditPerTopUp;
} sl_zcl_prepayment_cluster_set_maximum_credit_limit_command_t;

#define sl_zcl_prepayment_cluster_set_maximum_credit_limit_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_prepayment_cluster_set_maximum_credit_limit_command_t, providerId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_prepayment_cluster_set_maximum_credit_limit_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_prepayment_cluster_set_maximum_credit_limit_command_t, implementationDateTime), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_prepayment_cluster_set_maximum_credit_limit_command_t, maximumCreditLevel), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_prepayment_cluster_set_maximum_credit_limit_command_t, maximumCreditPerTopUp), \
}


/** @brief Structure for ZCL command "PublishPriceMatrix" from "Price" 
 */
typedef struct __zcl_price_cluster_publish_price_matrix_command {
  uint32_t providerId;
  uint32_t issuerEventId;
  uint32_t startTime;
  uint32_t issuerTariffId;
  uint8_t commandIndex;
  uint8_t numberOfCommands;
  uint8_t subPayloadControl;
  /* TYPE WARNING: PriceMatrixSubPayload array defaults to */ uint8_t * payload;
} sl_zcl_price_cluster_publish_price_matrix_command_t;

#define sl_zcl_price_cluster_publish_price_matrix_command_signature  { \
  8, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_price_matrix_command_t, providerId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_price_matrix_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_price_cluster_publish_price_matrix_command_t, startTime), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_price_matrix_command_t, issuerTariffId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_price_cluster_publish_price_matrix_command_t, commandIndex), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_price_cluster_publish_price_matrix_command_t, numberOfCommands), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_price_cluster_publish_price_matrix_command_t, subPayloadControl), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_price_cluster_publish_price_matrix_command_t, payload), \
}


/** @brief Structure for ZCL command "PublishBlockThresholds" from "Price" 
 */
typedef struct __zcl_price_cluster_publish_block_thresholds_command {
  uint32_t providerId;
  uint32_t issuerEventId;
  uint32_t startTime;
  uint32_t issuerTariffId;
  uint8_t commandIndex;
  uint8_t numberOfCommands;
  uint8_t subPayloadControl;
  /* TYPE WARNING: BlockThresholdSubPayload array defaults to */ uint8_t * payload;
} sl_zcl_price_cluster_publish_block_thresholds_command_t;

#define sl_zcl_price_cluster_publish_block_thresholds_command_signature  { \
  8, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_block_thresholds_command_t, providerId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_block_thresholds_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_price_cluster_publish_block_thresholds_command_t, startTime), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_block_thresholds_command_t, issuerTariffId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_price_cluster_publish_block_thresholds_command_t, commandIndex), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_price_cluster_publish_block_thresholds_command_t, numberOfCommands), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_price_cluster_publish_block_thresholds_command_t, subPayloadControl), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_price_cluster_publish_block_thresholds_command_t, payload), \
}


/** @brief Structure for ZCL command "ChangeSupply" from "Simple Metering" 
 */
typedef struct __zcl_simple_metering_cluster_change_supply_command {
  uint32_t providerId;
  uint32_t issuerEventId;
  uint32_t requestDateTime;
  uint32_t implementationDateTime;
  uint8_t proposedSupplyStatus;
  uint8_t supplyControlBits;
} sl_zcl_simple_metering_cluster_change_supply_command_t;

#define sl_zcl_simple_metering_cluster_change_supply_command_signature  { \
  6, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_simple_metering_cluster_change_supply_command_t, providerId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_simple_metering_cluster_change_supply_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_simple_metering_cluster_change_supply_command_t, requestDateTime), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_simple_metering_cluster_change_supply_command_t, implementationDateTime), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_simple_metering_cluster_change_supply_command_t, proposedSupplyStatus), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_simple_metering_cluster_change_supply_command_t, supplyControlBits), \
}


/** @brief Structure for ZCL command "DeviceInformationRequest" from "ZLL Commissioning" 
 */
typedef struct __zcl_zll_commissioning_cluster_device_information_request_command {
  uint32_t transaction;
  uint8_t startIndex;
} sl_zcl_zll_commissioning_cluster_device_information_request_command_t;

#define sl_zcl_zll_commissioning_cluster_device_information_request_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_zll_commissioning_cluster_device_information_request_command_t, transaction), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_zll_commissioning_cluster_device_information_request_command_t, startIndex), \
}


/** @brief Structure for ZCL command "ConfigureNotificationScheme" from "Simple Metering" 
 */
typedef struct __zcl_simple_metering_cluster_configure_notification_scheme_command {
  uint32_t issuerEventId;
  uint8_t notificationScheme;
  uint32_t notificationFlagOrder;
} sl_zcl_simple_metering_cluster_configure_notification_scheme_command_t;

#define sl_zcl_simple_metering_cluster_configure_notification_scheme_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_simple_metering_cluster_configure_notification_scheme_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_simple_metering_cluster_configure_notification_scheme_command_t, notificationScheme), \
  (ZAP_SIGNATURE_TYPE_BITMAP32), offsetof(sl_zcl_simple_metering_cluster_configure_notification_scheme_command_t, notificationFlagOrder), \
}


/** @brief Structure for ZCL command "ScanResponse" from "ZLL Commissioning" 
 */
typedef struct __zcl_zll_commissioning_cluster_scan_response_command {
  uint32_t transaction;
  uint8_t rssiCorrection;
  uint8_t zigbeeInformation;
  uint8_t zllInformation;
  uint16_t keyBitmask;
  uint32_t responseId;
  uint8_t * extendedPanId;
  uint8_t networkUpdateId;
  uint8_t logicalChannel;
  uint16_t panId;
  uint16_t networkAddress;
  uint8_t numberOfSubDevices;
  uint8_t totalGroupIds;
  uint8_t endpointId;
  uint16_t profileId;
  uint16_t deviceId;
  uint8_t version;
  uint8_t groupIdCount;
} sl_zcl_zll_commissioning_cluster_scan_response_command_t;

#define sl_zcl_zll_commissioning_cluster_scan_response_command_signature  { \
  18, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_zll_commissioning_cluster_scan_response_command_t, transaction), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_zll_commissioning_cluster_scan_response_command_t, rssiCorrection), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_zll_commissioning_cluster_scan_response_command_t, zigbeeInformation), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_zll_commissioning_cluster_scan_response_command_t, zllInformation), \
  (ZAP_SIGNATURE_TYPE_BITMAP16), offsetof(sl_zcl_zll_commissioning_cluster_scan_response_command_t, keyBitmask), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_zll_commissioning_cluster_scan_response_command_t, responseId), \
  (ZAP_SIGNATURE_TYPE_IEEE_ADDRESS), offsetof(sl_zcl_zll_commissioning_cluster_scan_response_command_t, extendedPanId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_zll_commissioning_cluster_scan_response_command_t, networkUpdateId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_zll_commissioning_cluster_scan_response_command_t, logicalChannel), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_scan_response_command_t, panId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_scan_response_command_t, networkAddress), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_zll_commissioning_cluster_scan_response_command_t, numberOfSubDevices), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_zll_commissioning_cluster_scan_response_command_t, totalGroupIds), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_zll_commissioning_cluster_scan_response_command_t, endpointId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_scan_response_command_t, profileId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_zll_commissioning_cluster_scan_response_command_t, deviceId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_zll_commissioning_cluster_scan_response_command_t, version), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_zll_commissioning_cluster_scan_response_command_t, groupIdCount), \
}


/** @brief Structure for ZCL command "ConfigureNotificationFlags" from "Simple Metering" 
 */
typedef struct __zcl_simple_metering_cluster_configure_notification_flags_command {
  uint32_t issuerEventId;
  uint8_t notificationScheme;
  uint16_t notificationFlagAttributeId;
  uint16_t clusterId;
  uint16_t manufacturerCode;
  uint8_t numberOfCommands;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * commandIds;
} sl_zcl_simple_metering_cluster_configure_notification_flags_command_t;

#define sl_zcl_simple_metering_cluster_configure_notification_flags_command_signature  { \
  7, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_simple_metering_cluster_configure_notification_flags_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_simple_metering_cluster_configure_notification_flags_command_t, notificationScheme), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_simple_metering_cluster_configure_notification_flags_command_t, notificationFlagAttributeId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_simple_metering_cluster_configure_notification_flags_command_t, clusterId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_simple_metering_cluster_configure_notification_flags_command_t, manufacturerCode), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_simple_metering_cluster_configure_notification_flags_command_t, numberOfCommands), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_simple_metering_cluster_configure_notification_flags_command_t, commandIds), \
}


/** @brief Structure for ZCL command "PairingResponse" from "MDU Pairing" 
 */
typedef struct __zcl_mdu_pairing_cluster_pairing_response_command {
  uint32_t pairingInformationVersion;
  uint8_t totalNumberOfDevices;
  uint8_t commandIndex;
  uint8_t totalNumberOfCommands;
  /* TYPE WARNING: IEEE_ADDRESS array defaults to */ uint8_t * eui64s;
} sl_zcl_mdu_pairing_cluster_pairing_response_command_t;

#define sl_zcl_mdu_pairing_cluster_pairing_response_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_mdu_pairing_cluster_pairing_response_command_t, pairingInformationVersion), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_mdu_pairing_cluster_pairing_response_command_t, totalNumberOfDevices), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_mdu_pairing_cluster_pairing_response_command_t, commandIndex), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_mdu_pairing_cluster_pairing_response_command_t, totalNumberOfCommands), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_mdu_pairing_cluster_pairing_response_command_t, eui64s), \
}


/** @brief Structure for ZCL command "DeviceInformationResponse" from "ZLL Commissioning" 
 */
typedef struct __zcl_zll_commissioning_cluster_device_information_response_command {
  uint32_t transaction;
  uint8_t numberOfSubDevices;
  uint8_t startIndex;
  uint8_t deviceInformationRecordCount;
  /* TYPE WARNING: DeviceInformationRecord array defaults to */ uint8_t * deviceInformationRecordList;
} sl_zcl_zll_commissioning_cluster_device_information_response_command_t;

#define sl_zcl_zll_commissioning_cluster_device_information_response_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_zll_commissioning_cluster_device_information_response_command_t, transaction), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_zll_commissioning_cluster_device_information_response_command_t, numberOfSubDevices), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_zll_commissioning_cluster_device_information_response_command_t, startIndex), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_zll_commissioning_cluster_device_information_response_command_t, deviceInformationRecordCount), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_zll_commissioning_cluster_device_information_response_command_t, deviceInformationRecordList), \
}


/** @brief Structure for ZCL command "ScheduleSnapshot" from "Simple Metering" 
 */
typedef struct __zcl_simple_metering_cluster_schedule_snapshot_command {
  uint32_t issuerEventId;
  uint8_t commandIndex;
  uint8_t commandCount;
  /* TYPE WARNING: SnapshotSchedulePayload array defaults to */ uint8_t * snapshotSchedulePayload;
} sl_zcl_simple_metering_cluster_schedule_snapshot_command_t;

#define sl_zcl_simple_metering_cluster_schedule_snapshot_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_simple_metering_cluster_schedule_snapshot_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_simple_metering_cluster_schedule_snapshot_command_t, commandIndex), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_simple_metering_cluster_schedule_snapshot_command_t, commandCount), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_simple_metering_cluster_schedule_snapshot_command_t, snapshotSchedulePayload), \
}


/** @brief Structure for ZCL command "ChangeDebt" from "Prepayment" 
 */
typedef struct __zcl_prepayment_cluster_change_debt_command {
  uint32_t issuerEventId;
  uint8_t * debtLabel;
  uint32_t debtAmount;
  uint8_t debtRecoveryMethod;
  uint8_t debtAmountType;
  uint32_t debtRecoveryStartTime;
  uint16_t debtRecoveryCollectionTime;
  uint8_t debtRecoveryFrequency;
  uint32_t debtRecoveryAmount;
  uint16_t debtRecoveryBalancePercentage;
} sl_zcl_prepayment_cluster_change_debt_command_t;

#define sl_zcl_prepayment_cluster_change_debt_command_signature  { \
  10, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_prepayment_cluster_change_debt_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_prepayment_cluster_change_debt_command_t, debtLabel), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_prepayment_cluster_change_debt_command_t, debtAmount), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_prepayment_cluster_change_debt_command_t, debtRecoveryMethod), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_prepayment_cluster_change_debt_command_t, debtAmountType), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_prepayment_cluster_change_debt_command_t, debtRecoveryStartTime), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_prepayment_cluster_change_debt_command_t, debtRecoveryCollectionTime), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_prepayment_cluster_change_debt_command_t, debtRecoveryFrequency), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_prepayment_cluster_change_debt_command_t, debtRecoveryAmount), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_prepayment_cluster_change_debt_command_t, debtRecoveryBalancePercentage), \
}


/** @brief Structure for ZCL command "PublishPrice" from "Price" 
 */
typedef struct __zcl_price_cluster_publish_price_command {
  uint32_t providerId;
  uint8_t * rateLabel;
  uint32_t issuerEventId;
  uint32_t currentTime;
  uint8_t unitOfMeasure;
  uint16_t currency;
  uint8_t priceTrailingDigitAndPriceTier;
  uint8_t numberOfPriceTiersAndRegisterTier;
  uint32_t startTime;
  uint16_t durationInMinutes;
  uint32_t price;
  uint8_t priceRatio;
  uint32_t generationPrice;
  uint8_t generationPriceRatio;
  uint32_t alternateCostDelivered;
  uint8_t alternateCostUnit;
  uint8_t alternateCostTrailingDigit;
  uint8_t numberOfBlockThresholds;
  uint8_t priceControl;
  uint8_t numberOfGenerationTiers;
  uint8_t generationTier;
  uint8_t extendedNumberOfPriceTiers;
  uint8_t extendedPriceTier;
  uint8_t extendedRegisterTier;
} sl_zcl_price_cluster_publish_price_command_t;

#define sl_zcl_price_cluster_publish_price_command_signature  { \
  24, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_price_command_t, providerId), \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_price_cluster_publish_price_command_t, rateLabel), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_price_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_price_cluster_publish_price_command_t, currentTime), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_price_cluster_publish_price_command_t, unitOfMeasure), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_price_cluster_publish_price_command_t, currency), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_price_cluster_publish_price_command_t, priceTrailingDigitAndPriceTier), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_price_cluster_publish_price_command_t, numberOfPriceTiersAndRegisterTier), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_price_cluster_publish_price_command_t, startTime), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_price_cluster_publish_price_command_t, durationInMinutes), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_price_command_t, price), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_price_cluster_publish_price_command_t, priceRatio), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_price_command_t, generationPrice), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_price_cluster_publish_price_command_t, generationPriceRatio), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_price_command_t, alternateCostDelivered), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_price_cluster_publish_price_command_t, alternateCostUnit), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_price_cluster_publish_price_command_t, alternateCostTrailingDigit), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_price_cluster_publish_price_command_t, numberOfBlockThresholds), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_price_cluster_publish_price_command_t, priceControl), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_price_cluster_publish_price_command_t, numberOfGenerationTiers), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_price_cluster_publish_price_command_t, generationTier), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_price_cluster_publish_price_command_t, extendedNumberOfPriceTiers), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_price_cluster_publish_price_command_t, extendedPriceTier), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_price_cluster_publish_price_command_t, extendedRegisterTier), \
}


/** @brief Structure for ZCL command "ScheduleSnapshotResponse" from "Simple Metering" 
 */
typedef struct __zcl_simple_metering_cluster_schedule_snapshot_response_command {
  uint32_t issuerEventId;
  /* TYPE WARNING: SnapshotResponsePayload array defaults to */ uint8_t * snapshotResponsePayload;
} sl_zcl_simple_metering_cluster_schedule_snapshot_response_command_t;

#define sl_zcl_simple_metering_cluster_schedule_snapshot_response_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_simple_metering_cluster_schedule_snapshot_response_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_simple_metering_cluster_schedule_snapshot_response_command_t, snapshotResponsePayload), \
}


/** @brief Structure for ZCL command "SetEventConfiguration" from "Device Management" 
 */
typedef struct __zcl_device_management_cluster_set_event_configuration_command {
  uint32_t issuerEventId;
  uint32_t startDateTime;
  uint8_t eventConfiguration;
  uint8_t configurationControl;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * eventConfigurationPayload;
} sl_zcl_device_management_cluster_set_event_configuration_command_t;

#define sl_zcl_device_management_cluster_set_event_configuration_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_device_management_cluster_set_event_configuration_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_device_management_cluster_set_event_configuration_command_t, startDateTime), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_device_management_cluster_set_event_configuration_command_t, eventConfiguration), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_device_management_cluster_set_event_configuration_command_t, configurationControl), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_device_management_cluster_set_event_configuration_command_t, eventConfigurationPayload), \
}


/** @brief Structure for ZCL command "MessageConfirmation" from "Messaging" 
 */
typedef struct __zcl_messaging_cluster_message_confirmation_command {
  uint32_t messageId;
  uint32_t confirmationTime;
  uint8_t messageConfirmationControl;
  uint8_t * messageResponse;
} sl_zcl_messaging_cluster_message_confirmation_command_t;

#define sl_zcl_messaging_cluster_message_confirmation_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_messaging_cluster_message_confirmation_command_t, messageId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_messaging_cluster_message_confirmation_command_t, confirmationTime), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_messaging_cluster_message_confirmation_command_t, messageConfirmationControl), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_messaging_cluster_message_confirmation_command_t, messageResponse), \
}


/** @brief Structure for ZCL command "StartSampling" from "Simple Metering" 
 */
typedef struct __zcl_simple_metering_cluster_start_sampling_command {
  uint32_t issuerEventId;
  uint32_t startSamplingTime;
  uint8_t sampleType;
  uint16_t sampleRequestInterval;
  uint16_t maxNumberOfSamples;
} sl_zcl_simple_metering_cluster_start_sampling_command_t;

#define sl_zcl_simple_metering_cluster_start_sampling_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_simple_metering_cluster_start_sampling_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_simple_metering_cluster_start_sampling_command_t, startSamplingTime), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_simple_metering_cluster_start_sampling_command_t, sampleType), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_simple_metering_cluster_start_sampling_command_t, sampleRequestInterval), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_simple_metering_cluster_start_sampling_command_t, maxNumberOfSamples), \
}


/** @brief Structure for ZCL command "CreditAdjustment" from "Prepayment" 
 */
typedef struct __zcl_prepayment_cluster_credit_adjustment_command {
  uint32_t issuerEventId;
  uint32_t startTime;
  uint8_t creditAdjustmentType;
  uint32_t creditAdjustmentValue;
} sl_zcl_prepayment_cluster_credit_adjustment_command_t;

#define sl_zcl_prepayment_cluster_credit_adjustment_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_prepayment_cluster_credit_adjustment_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_prepayment_cluster_credit_adjustment_command_t, startTime), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_prepayment_cluster_credit_adjustment_command_t, creditAdjustmentType), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_prepayment_cluster_credit_adjustment_command_t, creditAdjustmentValue), \
}


/** @brief Structure for ZCL command "RequestNewPasswordResponse" from "Device Management" 
 */
typedef struct __zcl_device_management_cluster_request_new_password_response_command {
  uint32_t issuerEventId;
  uint32_t implementationDateTime;
  uint16_t durationInMinutes;
  uint8_t passwordType;
  uint8_t * password;
} sl_zcl_device_management_cluster_request_new_password_response_command_t;

#define sl_zcl_device_management_cluster_request_new_password_response_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_device_management_cluster_request_new_password_response_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_device_management_cluster_request_new_password_response_command_t, implementationDateTime), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_device_management_cluster_request_new_password_response_command_t, durationInMinutes), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_device_management_cluster_request_new_password_response_command_t, passwordType), \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_device_management_cluster_request_new_password_response_command_t, password), \
}


/** @brief Structure for ZCL command "PublishConversionFactor" from "Price" 
 */
typedef struct __zcl_price_cluster_publish_conversion_factor_command {
  uint32_t issuerEventId;
  uint32_t startTime;
  uint32_t conversionFactor;
  uint8_t conversionFactorTrailingDigit;
} sl_zcl_price_cluster_publish_conversion_factor_command_t;

#define sl_zcl_price_cluster_publish_conversion_factor_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_conversion_factor_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_price_cluster_publish_conversion_factor_command_t, startTime), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_conversion_factor_command_t, conversionFactor), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_price_cluster_publish_conversion_factor_command_t, conversionFactorTrailingDigit), \
}


/** @brief Structure for ZCL command "PublishCalorificValue" from "Price" 
 */
typedef struct __zcl_price_cluster_publish_calorific_value_command {
  uint32_t issuerEventId;
  uint32_t startTime;
  uint32_t calorificValue;
  uint8_t calorificValueUnit;
  uint8_t calorificValueTrailingDigit;
} sl_zcl_price_cluster_publish_calorific_value_command_t;

#define sl_zcl_price_cluster_publish_calorific_value_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_calorific_value_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_price_cluster_publish_calorific_value_command_t, startTime), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_publish_calorific_value_command_t, calorificValue), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_price_cluster_publish_calorific_value_command_t, calorificValueUnit), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_price_cluster_publish_calorific_value_command_t, calorificValueTrailingDigit), \
}


/** @brief Structure for ZCL command "EmergencyCreditSetup" from "Prepayment" 
 */
typedef struct __zcl_prepayment_cluster_emergency_credit_setup_command {
  uint32_t issuerEventId;
  uint32_t startTime;
  uint32_t emergencyCreditLimit;
  uint32_t emergencyCreditThreshold;
} sl_zcl_prepayment_cluster_emergency_credit_setup_command_t;

#define sl_zcl_prepayment_cluster_emergency_credit_setup_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_prepayment_cluster_emergency_credit_setup_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_prepayment_cluster_emergency_credit_setup_command_t, startTime), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_prepayment_cluster_emergency_credit_setup_command_t, emergencyCreditLimit), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_prepayment_cluster_emergency_credit_setup_command_t, emergencyCreditThreshold), \
}


/** @brief Structure for ZCL command "UpdateSiteId" from "Device Management" 
 */
typedef struct __zcl_device_management_cluster_update_site_id_command {
  uint32_t issuerEventId;
  uint32_t siteIdTime;
  uint32_t providerId;
  uint8_t * siteId;
} sl_zcl_device_management_cluster_update_site_id_command_t;

#define sl_zcl_device_management_cluster_update_site_id_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_device_management_cluster_update_site_id_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_device_management_cluster_update_site_id_command_t, siteIdTime), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_device_management_cluster_update_site_id_command_t, providerId), \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_device_management_cluster_update_site_id_command_t, siteId), \
}


/** @brief Structure for ZCL command "UpdateCIN" from "Device Management" 
 */
typedef struct __zcl_device_management_cluster_update_cin_command {
  uint32_t issuerEventId;
  uint32_t implementationTime;
  uint32_t providerId;
  uint8_t * customerIdNumber;
} sl_zcl_device_management_cluster_update_cin_command_t;

#define sl_zcl_device_management_cluster_update_cin_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_device_management_cluster_update_cin_command_t, issuerEventId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_device_management_cluster_update_cin_command_t, implementationTime), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_device_management_cluster_update_cin_command_t, providerId), \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_device_management_cluster_update_cin_command_t, customerIdNumber), \
}


/** @brief Structure for ZCL command "PublishSnapshot" from "Simple Metering" 
 */
typedef struct __zcl_simple_metering_cluster_publish_snapshot_command {
  uint32_t snapshotId;
  uint32_t snapshotTime;
  uint8_t totalSnapshotsFound;
  uint8_t commandIndex;
  uint8_t totalCommands;
  uint32_t snapshotCause;
  uint8_t snapshotPayloadType;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * snapshotPayload;
} sl_zcl_simple_metering_cluster_publish_snapshot_command_t;

#define sl_zcl_simple_metering_cluster_publish_snapshot_command_signature  { \
  8, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_simple_metering_cluster_publish_snapshot_command_t, snapshotId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_simple_metering_cluster_publish_snapshot_command_t, snapshotTime), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_simple_metering_cluster_publish_snapshot_command_t, totalSnapshotsFound), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_simple_metering_cluster_publish_snapshot_command_t, commandIndex), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_simple_metering_cluster_publish_snapshot_command_t, totalCommands), \
  (ZAP_SIGNATURE_TYPE_BITMAP32), offsetof(sl_zcl_simple_metering_cluster_publish_snapshot_command_t, snapshotCause), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_simple_metering_cluster_publish_snapshot_command_t, snapshotPayloadType), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_simple_metering_cluster_publish_snapshot_command_t, snapshotPayload), \
}


/** @brief Structure for ZCL command "PublishPrepaySnapshot" from "Prepayment" 
 */
typedef struct __zcl_prepayment_cluster_publish_prepay_snapshot_command {
  uint32_t snapshotId;
  uint32_t snapshotTime;
  uint8_t totalSnapshotsFound;
  uint8_t commandIndex;
  uint8_t totalNumberOfCommands;
  uint32_t snapshotCause;
  uint8_t snapshotPayloadType;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * snapshotPayload;
} sl_zcl_prepayment_cluster_publish_prepay_snapshot_command_t;

#define sl_zcl_prepayment_cluster_publish_prepay_snapshot_command_signature  { \
  8, \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_prepayment_cluster_publish_prepay_snapshot_command_t, snapshotId), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_prepayment_cluster_publish_prepay_snapshot_command_t, snapshotTime), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_prepayment_cluster_publish_prepay_snapshot_command_t, totalSnapshotsFound), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_prepayment_cluster_publish_prepay_snapshot_command_t, commandIndex), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_prepayment_cluster_publish_prepay_snapshot_command_t, totalNumberOfCommands), \
  (ZAP_SIGNATURE_TYPE_BITMAP32), offsetof(sl_zcl_prepayment_cluster_publish_prepay_snapshot_command_t, snapshotCause), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_prepayment_cluster_publish_prepay_snapshot_command_t, snapshotPayloadType), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_prepayment_cluster_publish_prepay_snapshot_command_t, snapshotPayload), \
}


/** @brief Structure for ZCL command "PowerProfileRequest" from "Power Profile" 
 */
typedef struct __zcl_power_profile_cluster_power_profile_request_command {
  uint8_t powerProfileId;
} sl_zcl_power_profile_cluster_power_profile_request_command_t;

#define sl_zcl_power_profile_cluster_power_profile_request_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_power_profile_cluster_power_profile_request_command_t, powerProfileId), \
}


/** @brief Structure for ZCL command "PowerProfileScheduleConstraintsRequest" from "Power Profile" 
 */
typedef struct __zcl_power_profile_cluster_power_profile_schedule_constraints_request_command {
  uint8_t powerProfileId;
} sl_zcl_power_profile_cluster_power_profile_schedule_constraints_request_command_t;

#define sl_zcl_power_profile_cluster_power_profile_schedule_constraints_request_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_power_profile_cluster_power_profile_schedule_constraints_request_command_t, powerProfileId), \
}


/** @brief Structure for ZCL command "EnergyPhasesScheduleStateRequest" from "Power Profile" 
 */
typedef struct __zcl_power_profile_cluster_energy_phases_schedule_state_request_command {
  uint8_t powerProfileId;
} sl_zcl_power_profile_cluster_energy_phases_schedule_state_request_command_t;

#define sl_zcl_power_profile_cluster_energy_phases_schedule_state_request_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_power_profile_cluster_energy_phases_schedule_state_request_command_t, powerProfileId), \
}


/** @brief Structure for ZCL command "GetPowerProfilePrice" from "Power Profile" 
 */
typedef struct __zcl_power_profile_cluster_get_power_profile_price_command {
  uint8_t powerProfileId;
} sl_zcl_power_profile_cluster_get_power_profile_price_command_t;

#define sl_zcl_power_profile_cluster_get_power_profile_price_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_power_profile_cluster_get_power_profile_price_command_t, powerProfileId), \
}


/** @brief Structure for ZCL command "EnergyPhasesScheduleRequest" from "Power Profile" 
 */
typedef struct __zcl_power_profile_cluster_energy_phases_schedule_request_command {
  uint8_t powerProfileId;
} sl_zcl_power_profile_cluster_energy_phases_schedule_request_command_t;

#define sl_zcl_power_profile_cluster_energy_phases_schedule_request_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_power_profile_cluster_energy_phases_schedule_request_command_t, powerProfileId), \
}


/** @brief Structure for ZCL command "GetHolidaySchedule" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_get_holiday_schedule_command {
  uint8_t scheduleId;
} sl_zcl_door_lock_cluster_get_holiday_schedule_command_t;

#define sl_zcl_door_lock_cluster_get_holiday_schedule_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_get_holiday_schedule_command_t, scheduleId), \
}


/** @brief Structure for ZCL command "ClearHolidaySchedule" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_clear_holiday_schedule_command {
  uint8_t scheduleId;
} sl_zcl_door_lock_cluster_clear_holiday_schedule_command_t;

#define sl_zcl_door_lock_cluster_clear_holiday_schedule_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_clear_holiday_schedule_command_t, scheduleId), \
}


/** @brief Structure for ZCL command "LockDoorResponse" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_lock_door_response_command {
  uint8_t status;
} sl_zcl_door_lock_cluster_lock_door_response_command_t;

#define sl_zcl_door_lock_cluster_lock_door_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_lock_door_response_command_t, status), \
}


/** @brief Structure for ZCL command "UnlockDoorResponse" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_unlock_door_response_command {
  uint8_t status;
} sl_zcl_door_lock_cluster_unlock_door_response_command_t;

#define sl_zcl_door_lock_cluster_unlock_door_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_unlock_door_response_command_t, status), \
}


/** @brief Structure for ZCL command "ToggleResponse" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_toggle_response_command {
  uint8_t status;
} sl_zcl_door_lock_cluster_toggle_response_command_t;

#define sl_zcl_door_lock_cluster_toggle_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_toggle_response_command_t, status), \
}


/** @brief Structure for ZCL command "UnlockWithTimeoutResponse" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_unlock_with_timeout_response_command {
  uint8_t status;
} sl_zcl_door_lock_cluster_unlock_with_timeout_response_command_t;

#define sl_zcl_door_lock_cluster_unlock_with_timeout_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_unlock_with_timeout_response_command_t, status), \
}


/** @brief Structure for ZCL command "ClearPinResponse" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_clear_pin_response_command {
  uint8_t status;
} sl_zcl_door_lock_cluster_clear_pin_response_command_t;

#define sl_zcl_door_lock_cluster_clear_pin_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_clear_pin_response_command_t, status), \
}


/** @brief Structure for ZCL command "ClearAllPinsResponse" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_clear_all_pins_response_command {
  uint8_t status;
} sl_zcl_door_lock_cluster_clear_all_pins_response_command_t;

#define sl_zcl_door_lock_cluster_clear_all_pins_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_clear_all_pins_response_command_t, status), \
}


/** @brief Structure for ZCL command "SetUserStatusResponse" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_set_user_status_response_command {
  uint8_t status;
} sl_zcl_door_lock_cluster_set_user_status_response_command_t;

#define sl_zcl_door_lock_cluster_set_user_status_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_set_user_status_response_command_t, status), \
}


/** @brief Structure for ZCL command "SetWeekdayScheduleResponse" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_set_weekday_schedule_response_command {
  uint8_t status;
} sl_zcl_door_lock_cluster_set_weekday_schedule_response_command_t;

#define sl_zcl_door_lock_cluster_set_weekday_schedule_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_set_weekday_schedule_response_command_t, status), \
}


/** @brief Structure for ZCL command "ClearWeekdayScheduleResponse" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_clear_weekday_schedule_response_command {
  uint8_t status;
} sl_zcl_door_lock_cluster_clear_weekday_schedule_response_command_t;

#define sl_zcl_door_lock_cluster_clear_weekday_schedule_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_clear_weekday_schedule_response_command_t, status), \
}


/** @brief Structure for ZCL command "SetYeardayScheduleResponse" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_set_yearday_schedule_response_command {
  uint8_t status;
} sl_zcl_door_lock_cluster_set_yearday_schedule_response_command_t;

#define sl_zcl_door_lock_cluster_set_yearday_schedule_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_set_yearday_schedule_response_command_t, status), \
}


/** @brief Structure for ZCL command "ClearYeardayScheduleResponse" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_clear_yearday_schedule_response_command {
  uint8_t status;
} sl_zcl_door_lock_cluster_clear_yearday_schedule_response_command_t;

#define sl_zcl_door_lock_cluster_clear_yearday_schedule_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_clear_yearday_schedule_response_command_t, status), \
}


/** @brief Structure for ZCL command "SetHolidayScheduleResponse" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_set_holiday_schedule_response_command {
  uint8_t status;
} sl_zcl_door_lock_cluster_set_holiday_schedule_response_command_t;

#define sl_zcl_door_lock_cluster_set_holiday_schedule_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_set_holiday_schedule_response_command_t, status), \
}


/** @brief Structure for ZCL command "ClearHolidayScheduleResponse" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_clear_holiday_schedule_response_command {
  uint8_t status;
} sl_zcl_door_lock_cluster_clear_holiday_schedule_response_command_t;

#define sl_zcl_door_lock_cluster_clear_holiday_schedule_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_clear_holiday_schedule_response_command_t, status), \
}


/** @brief Structure for ZCL command "SetUserTypeResponse" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_set_user_type_response_command {
  uint8_t status;
} sl_zcl_door_lock_cluster_set_user_type_response_command_t;

#define sl_zcl_door_lock_cluster_set_user_type_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_set_user_type_response_command_t, status), \
}


/** @brief Structure for ZCL command "ClearRfidResponse" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_clear_rfid_response_command {
  uint8_t status;
} sl_zcl_door_lock_cluster_clear_rfid_response_command_t;

#define sl_zcl_door_lock_cluster_clear_rfid_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_clear_rfid_response_command_t, status), \
}


/** @brief Structure for ZCL command "ClearAllRfidsResponse" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_clear_all_rfids_response_command {
  uint8_t status;
} sl_zcl_door_lock_cluster_clear_all_rfids_response_command_t;

#define sl_zcl_door_lock_cluster_clear_all_rfids_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_clear_all_rfids_response_command_t, status), \
}


/** @brief Structure for ZCL command "SetDisposableScheduleResponse" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_set_disposable_schedule_response_command {
  uint8_t status;
} sl_zcl_door_lock_cluster_set_disposable_schedule_response_command_t;

#define sl_zcl_door_lock_cluster_set_disposable_schedule_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_set_disposable_schedule_response_command_t, status), \
}


/** @brief Structure for ZCL command "ClearDisposableScheduleResponse" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_clear_disposable_schedule_response_command {
  uint8_t status;
} sl_zcl_door_lock_cluster_clear_disposable_schedule_response_command_t;

#define sl_zcl_door_lock_cluster_clear_disposable_schedule_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_clear_disposable_schedule_response_command_t, status), \
}


/** @brief Structure for ZCL command "ClearBiometricCredentialResponse" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_clear_biometric_credential_response_command {
  uint8_t status;
} sl_zcl_door_lock_cluster_clear_biometric_credential_response_command_t;

#define sl_zcl_door_lock_cluster_clear_biometric_credential_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_clear_biometric_credential_response_command_t, status), \
}


/** @brief Structure for ZCL command "ClearAllBiometricCredentialsResponse" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_clear_all_biometric_credentials_response_command {
  uint8_t status;
} sl_zcl_door_lock_cluster_clear_all_biometric_credentials_response_command_t;

#define sl_zcl_door_lock_cluster_clear_all_biometric_credentials_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_clear_all_biometric_credentials_response_command_t, status), \
}


/** @brief Structure for ZCL command "WindowCoveringGoToLiftPercentage" from "Window Covering" 
 */
typedef struct __zcl_window_covering_cluster_window_covering_go_to_lift_percentage_command {
  uint8_t percentageLiftValue;
} sl_zcl_window_covering_cluster_window_covering_go_to_lift_percentage_command_t;

#define sl_zcl_window_covering_cluster_window_covering_go_to_lift_percentage_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_window_covering_cluster_window_covering_go_to_lift_percentage_command_t, percentageLiftValue), \
}


/** @brief Structure for ZCL command "WindowCoveringGoToTiltPercentage" from "Window Covering" 
 */
typedef struct __zcl_window_covering_cluster_window_covering_go_to_tilt_percentage_command {
  uint8_t percentageTiltValue;
} sl_zcl_window_covering_cluster_window_covering_go_to_tilt_percentage_command_t;

#define sl_zcl_window_covering_cluster_window_covering_go_to_tilt_percentage_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_window_covering_cluster_window_covering_go_to_tilt_percentage_command_t, percentageTiltValue), \
}


/** @brief Structure for ZCL command "BarrierControlGoToPercent" from "Barrier Control" 
 */
typedef struct __zcl_barrier_control_cluster_barrier_control_go_to_percent_command {
  uint8_t percentOpen;
} sl_zcl_barrier_control_cluster_barrier_control_go_to_percent_command_t;

#define sl_zcl_barrier_control_cluster_barrier_control_go_to_percent_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_barrier_control_cluster_barrier_control_go_to_percent_command_t, percentOpen), \
}


/** @brief Structure for ZCL command "GetZoneInformation" from "IAS ACE" 
 */
typedef struct __zcl_ias_ace_cluster_get_zone_information_command {
  uint8_t zoneId;
} sl_zcl_ias_ace_cluster_get_zone_information_command_t;

#define sl_zcl_ias_ace_cluster_get_zone_information_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_ias_ace_cluster_get_zone_information_command_t, zoneId), \
}


/** @brief Structure for ZCL command "GetSupportedTunnelProtocols" from "Tunneling" 
 */
typedef struct __zcl_tunneling_cluster_get_supported_tunnel_protocols_command {
  uint8_t protocolOffset;
} sl_zcl_tunneling_cluster_get_supported_tunnel_protocols_command_t;

#define sl_zcl_tunneling_cluster_get_supported_tunnel_protocols_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_tunneling_cluster_get_supported_tunnel_protocols_command_t, protocolOffset), \
}


/** @brief Structure for ZCL command "SuspendZclMessages" from "Sub-GHz" 
 */
typedef struct __zcl_sub_g_hz_cluster_suspend_zcl_messages_command {
  uint8_t period;
} sl_zcl_sub_g_hz_cluster_suspend_zcl_messages_command_t;

#define sl_zcl_sub_g_hz_cluster_suspend_zcl_messages_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_sub_g_hz_cluster_suspend_zcl_messages_command_t, period), \
}


/** @brief Structure for ZCL command "GetGroupIdentifiersRequest" from "ZLL Commissioning" 
 */
typedef struct __zcl_zll_commissioning_cluster_get_group_identifiers_request_command {
  uint8_t startIndex;
} sl_zcl_zll_commissioning_cluster_get_group_identifiers_request_command_t;

#define sl_zcl_zll_commissioning_cluster_get_group_identifiers_request_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_zll_commissioning_cluster_get_group_identifiers_request_command_t, startIndex), \
}


/** @brief Structure for ZCL command "GetEndpointListRequest" from "ZLL Commissioning" 
 */
typedef struct __zcl_zll_commissioning_cluster_get_endpoint_list_request_command {
  uint8_t startIndex;
} sl_zcl_zll_commissioning_cluster_get_endpoint_list_request_command_t;

#define sl_zcl_zll_commissioning_cluster_get_endpoint_list_request_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_zll_commissioning_cluster_get_endpoint_list_request_command_t, startIndex), \
}


/** @brief Structure for ZCL command "CommandOne" from "Sample Mfg Specific Cluster" 
 */
typedef struct __zcl_sample_mfg_specific_cluster_cluster_command_one_command {
  uint8_t argOne;
} sl_zcl_sample_mfg_specific_cluster_cluster_command_one_command_t;

#define sl_zcl_sample_mfg_specific_cluster_cluster_command_one_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_sample_mfg_specific_cluster_cluster_command_one_command_t, argOne), \
}


/** @brief Structure for ZCL command "CommandTwo" from "Sample Mfg Specific Cluster 2" 
 */
typedef struct __zcl_sample_mfg_specific_cluster_2_cluster_command_two_command {
  uint8_t argOne;
} sl_zcl_sample_mfg_specific_cluster_2_cluster_command_two_command_t;

#define sl_zcl_sample_mfg_specific_cluster_2_cluster_command_two_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_sample_mfg_specific_cluster_2_cluster_command_two_command_t, argOne), \
}


/** @brief Structure for ZCL command "SetMacPollFailureWaitTime" from "SL Works With All Hubs" 
 */
typedef struct __zcl_sl_works_with_all_hubs_cluster_set_mac_poll_failure_wait_time_command {
  uint8_t waitTime;
} sl_zcl_sl_works_with_all_hubs_cluster_set_mac_poll_failure_wait_time_command_t;

#define sl_zcl_sl_works_with_all_hubs_cluster_set_mac_poll_failure_wait_time_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_sl_works_with_all_hubs_cluster_set_mac_poll_failure_wait_time_command_t, waitTime), \
}


/** @brief Structure for ZCL command "DebugReportQuery" from "SL Works With All Hubs" 
 */
typedef struct __zcl_sl_works_with_all_hubs_cluster_debug_report_query_command {
  uint8_t debugReportId;
} sl_zcl_sl_works_with_all_hubs_cluster_debug_report_query_command_t;

#define sl_zcl_sl_works_with_all_hubs_cluster_debug_report_query_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_sl_works_with_all_hubs_cluster_debug_report_query_command_t, debugReportId), \
}


/** @brief Structure for ZCL command "MirrorReportAttributeResponse" from "Simple Metering" 
 */
typedef struct __zcl_simple_metering_cluster_mirror_report_attribute_response_command {
  uint8_t notificationScheme;
  /* TYPE WARNING: BITMAP32 array defaults to */ uint8_t * notificationFlags;
} sl_zcl_simple_metering_cluster_mirror_report_attribute_response_command_t;

#define sl_zcl_simple_metering_cluster_mirror_report_attribute_response_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_simple_metering_cluster_mirror_report_attribute_response_command_t, notificationScheme), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_simple_metering_cluster_mirror_report_attribute_response_command_t, notificationFlags), \
}


/** @brief Structure for ZCL command "GeneralResponse" from "Gaming" 
 */
typedef struct __zcl_gaming_cluster_general_response_command {
  uint8_t commandId;
  uint8_t status;
  uint8_t * message;
} sl_zcl_gaming_cluster_general_response_command_t;

#define sl_zcl_gaming_cluster_general_response_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_gaming_cluster_general_response_command_t, commandId), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_gaming_cluster_general_response_command_t, status), \
  (ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_gaming_cluster_general_response_command_t, message), \
}


/** @brief Structure for ZCL command "LocationDataResponse" from "RSSI Location" 
 */
typedef struct __zcl_rssi_location_cluster_location_data_response_command {
  uint8_t status;
  uint8_t locationType;
  int16_t coordinate1;
  int16_t coordinate2;
  int16_t coordinate3;
  int16_t power;
  uint16_t pathLossExponent;
  uint8_t locationMethod;
  uint8_t qualityMeasure;
  uint16_t locationAge;
} sl_zcl_rssi_location_cluster_location_data_response_command_t;

#define sl_zcl_rssi_location_cluster_location_data_response_command_signature  { \
  10, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_rssi_location_cluster_location_data_response_command_t, status), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_rssi_location_cluster_location_data_response_command_t, locationType), \
  (ZAP_SIGNATURE_TYPE_INT16S), offsetof(sl_zcl_rssi_location_cluster_location_data_response_command_t, coordinate1), \
  (ZAP_SIGNATURE_TYPE_INT16S), offsetof(sl_zcl_rssi_location_cluster_location_data_response_command_t, coordinate2), \
  (ZAP_SIGNATURE_TYPE_INT16S), offsetof(sl_zcl_rssi_location_cluster_location_data_response_command_t, coordinate3), \
  (ZAP_SIGNATURE_TYPE_INT16S), offsetof(sl_zcl_rssi_location_cluster_location_data_response_command_t, power), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_rssi_location_cluster_location_data_response_command_t, pathLossExponent), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_rssi_location_cluster_location_data_response_command_t, locationMethod), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_rssi_location_cluster_location_data_response_command_t, qualityMeasure), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_rssi_location_cluster_location_data_response_command_t, locationAge), \
}


/** @brief Structure for ZCL command "GetLocalesSupportedResponse" from "Basic" 
 */
typedef struct __zcl_basic_cluster_get_locales_supported_response_command {
  uint8_t discoveryComplete;
  /* TYPE WARNING: CHAR_STRING array defaults to */ uint8_t * localeSupported;
} sl_zcl_basic_cluster_get_locales_supported_response_command_t;

#define sl_zcl_basic_cluster_get_locales_supported_response_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_basic_cluster_get_locales_supported_response_command_t, discoveryComplete), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_basic_cluster_get_locales_supported_response_command_t, localeSupported), \
}


/** @brief Structure for ZCL command "ZoneStatusChanged" from "IAS ACE" 
 */
typedef struct __zcl_ias_ace_cluster_zone_status_changed_command {
  uint8_t zoneId;
  uint16_t zoneStatus;
  uint8_t audibleNotification;
  uint8_t * zoneLabel;
} sl_zcl_ias_ace_cluster_zone_status_changed_command_t;

#define sl_zcl_ias_ace_cluster_zone_status_changed_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_ias_ace_cluster_zone_status_changed_command_t, zoneId), \
  (ZAP_SIGNATURE_TYPE_ENUM16), offsetof(sl_zcl_ias_ace_cluster_zone_status_changed_command_t, zoneStatus), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_ias_ace_cluster_zone_status_changed_command_t, audibleNotification), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_ias_ace_cluster_zone_status_changed_command_t, zoneLabel), \
}


/** @brief Structure for ZCL command "GetZoneInformationResponse" from "IAS ACE" 
 */
typedef struct __zcl_ias_ace_cluster_get_zone_information_response_command {
  uint8_t zoneId;
  uint16_t zoneType;
  uint8_t * ieeeAddress;
  uint8_t * zoneLabel;
} sl_zcl_ias_ace_cluster_get_zone_information_response_command_t;

#define sl_zcl_ias_ace_cluster_get_zone_information_response_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_ias_ace_cluster_get_zone_information_response_command_t, zoneId), \
  (ZAP_SIGNATURE_TYPE_ENUM16), offsetof(sl_zcl_ias_ace_cluster_get_zone_information_response_command_t, zoneType), \
  (ZAP_SIGNATURE_TYPE_IEEE_ADDRESS), offsetof(sl_zcl_ias_ace_cluster_get_zone_information_response_command_t, ieeeAddress), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_ias_ace_cluster_get_zone_information_response_command_t, zoneLabel), \
}


/** @brief Structure for ZCL command "VoiceTransmissionResponse" from "Voice over ZigBee" 
 */
typedef struct __zcl_voice_over_zig_bee_cluster_voice_transmission_response_command {
  uint8_t sequenceNumber;
  uint8_t errorFlag;
} sl_zcl_voice_over_zig_bee_cluster_voice_transmission_response_command_t;

#define sl_zcl_voice_over_zig_bee_cluster_voice_transmission_response_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_voice_over_zig_bee_cluster_voice_transmission_response_command_t, sequenceNumber), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_voice_over_zig_bee_cluster_voice_transmission_response_command_t, errorFlag), \
}


/** @brief Structure for ZCL command "EventsNotification" from "Appliance Events and Alert" 
 */
typedef struct __zcl_appliance_events_and_alert_cluster_events_notification_command {
  uint8_t eventHeader;
  uint8_t eventId;
} sl_zcl_appliance_events_and_alert_cluster_events_notification_command_t;

#define sl_zcl_appliance_events_and_alert_cluster_events_notification_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_appliance_events_and_alert_cluster_events_notification_command_t, eventHeader), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_appliance_events_and_alert_cluster_events_notification_command_t, eventId), \
}


/** @brief Structure for ZCL command "GetAlarmResponse" from "Alarms" 
 */
typedef struct __zcl_alarms_cluster_get_alarm_response_command {
  uint8_t status;
  uint8_t alarmCode;
  uint16_t clusterId;
  uint32_t timeStamp;
} sl_zcl_alarms_cluster_get_alarm_response_command_t;

#define sl_zcl_alarms_cluster_get_alarm_response_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_alarms_cluster_get_alarm_response_command_t, status), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_alarms_cluster_get_alarm_response_command_t, alarmCode), \
  (ZAP_SIGNATURE_TYPE_CLUSTER_ID), offsetof(sl_zcl_alarms_cluster_get_alarm_response_command_t, clusterId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_alarms_cluster_get_alarm_response_command_t, timeStamp), \
}


/** @brief Structure for ZCL command "MoveToHue" from "Color Control" 
 */
typedef struct __zcl_color_control_cluster_move_to_hue_command {
  uint8_t hue;
  uint8_t direction;
  uint16_t transitionTime;
  uint8_t optionsMask;
  uint8_t optionsOverride;
} sl_zcl_color_control_cluster_move_to_hue_command_t;

#define sl_zcl_color_control_cluster_move_to_hue_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_color_control_cluster_move_to_hue_command_t, hue), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_color_control_cluster_move_to_hue_command_t, direction), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_color_control_cluster_move_to_hue_command_t, transitionTime), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_move_to_hue_command_t, optionsMask), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_move_to_hue_command_t, optionsOverride), \
}


/** @brief Structure for ZCL command "ProgrammingEventNotification" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_programming_event_notification_command {
  uint8_t source;
  uint8_t eventCode;
  uint16_t userId;
  uint8_t * pin;
  uint8_t userType;
  uint8_t userStatus;
  uint32_t timeStamp;
  uint8_t * data;
} sl_zcl_door_lock_cluster_programming_event_notification_command_t;

#define sl_zcl_door_lock_cluster_programming_event_notification_command_signature  { \
  8, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_programming_event_notification_command_t, source), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_door_lock_cluster_programming_event_notification_command_t, eventCode), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_door_lock_cluster_programming_event_notification_command_t, userId), \
  (ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_door_lock_cluster_programming_event_notification_command_t, pin), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_door_lock_cluster_programming_event_notification_command_t, userType), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_door_lock_cluster_programming_event_notification_command_t, userStatus), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_door_lock_cluster_programming_event_notification_command_t, timeStamp), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_door_lock_cluster_programming_event_notification_command_t, data), \
}


/** @brief Structure for ZCL command "OperationEventNotification" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_operation_event_notification_command {
  uint8_t source;
  uint8_t eventCode;
  uint16_t userId;
  uint8_t * pin;
  uint32_t timeStamp;
  uint8_t * data;
} sl_zcl_door_lock_cluster_operation_event_notification_command_t;

#define sl_zcl_door_lock_cluster_operation_event_notification_command_signature  { \
  6, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_operation_event_notification_command_t, source), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_door_lock_cluster_operation_event_notification_command_t, eventCode), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_door_lock_cluster_operation_event_notification_command_t, userId), \
  (ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_door_lock_cluster_operation_event_notification_command_t, pin), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_door_lock_cluster_operation_event_notification_command_t, timeStamp), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_door_lock_cluster_operation_event_notification_command_t, data), \
}


/** @brief Structure for ZCL command "GetProfileInfoResponseCommand" from "Electrical Measurement" 
 */
typedef struct __zcl_electrical_measurement_cluster_get_profile_info_response_command_command {
  uint8_t profileCount;
  uint8_t profileIntervalPeriod;
  uint8_t maxNumberOfIntervals;
  /* TYPE WARNING: INT16U array defaults to */ uint8_t * listOfAttributes;
} sl_zcl_electrical_measurement_cluster_get_profile_info_response_command_command_t;

#define sl_zcl_electrical_measurement_cluster_get_profile_info_response_command_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_electrical_measurement_cluster_get_profile_info_response_command_command_t, profileCount), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_electrical_measurement_cluster_get_profile_info_response_command_command_t, profileIntervalPeriod), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_electrical_measurement_cluster_get_profile_info_response_command_command_t, maxNumberOfIntervals), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_electrical_measurement_cluster_get_profile_info_response_command_command_t, listOfAttributes), \
}


/** @brief Structure for ZCL command "BypassResponse" from "IAS ACE" 
 */
typedef struct __zcl_ias_ace_cluster_bypass_response_command {
  uint8_t numberOfZones;
  /* TYPE WARNING: IasAceBypassResult array defaults to */ uint8_t * bypassResult;
} sl_zcl_ias_ace_cluster_bypass_response_command_t;

#define sl_zcl_ias_ace_cluster_bypass_response_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_ias_ace_cluster_bypass_response_command_t, numberOfZones), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_ias_ace_cluster_bypass_response_command_t, bypassResult), \
}


/** @brief Structure for ZCL command "DeviceConfigurationResponse" from "RSSI Location" 
 */
typedef struct __zcl_rssi_location_cluster_device_configuration_response_command {
  uint8_t status;
  int16_t power;
  uint16_t pathLossExponent;
  uint16_t calculationPeriod;
  uint8_t numberRssiMeasurements;
  uint16_t reportingPeriod;
} sl_zcl_rssi_location_cluster_device_configuration_response_command_t;

#define sl_zcl_rssi_location_cluster_device_configuration_response_command_signature  { \
  6, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_rssi_location_cluster_device_configuration_response_command_t, status), \
  (ZAP_SIGNATURE_TYPE_INT16S), offsetof(sl_zcl_rssi_location_cluster_device_configuration_response_command_t, power), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_rssi_location_cluster_device_configuration_response_command_t, pathLossExponent), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_rssi_location_cluster_device_configuration_response_command_t, calculationPeriod), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_rssi_location_cluster_device_configuration_response_command_t, numberRssiMeasurements), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_rssi_location_cluster_device_configuration_response_command_t, reportingPeriod), \
}


/** @brief Structure for ZCL command "AddGroupResponse" from "Groups" 
 */
typedef struct __zcl_groups_cluster_add_group_response_command {
  uint8_t status;
  uint16_t groupId;
} sl_zcl_groups_cluster_add_group_response_command_t;

#define sl_zcl_groups_cluster_add_group_response_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_groups_cluster_add_group_response_command_t, status), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_groups_cluster_add_group_response_command_t, groupId), \
}


/** @brief Structure for ZCL command "RemoveGroupResponse" from "Groups" 
 */
typedef struct __zcl_groups_cluster_remove_group_response_command {
  uint8_t status;
  uint16_t groupId;
} sl_zcl_groups_cluster_remove_group_response_command_t;

#define sl_zcl_groups_cluster_remove_group_response_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_groups_cluster_remove_group_response_command_t, status), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_groups_cluster_remove_group_response_command_t, groupId), \
}


/** @brief Structure for ZCL command "RemoveAllScenesResponse" from "Scenes" 
 */
typedef struct __zcl_scenes_cluster_remove_all_scenes_response_command {
  uint8_t status;
  uint16_t groupId;
} sl_zcl_scenes_cluster_remove_all_scenes_response_command_t;

#define sl_zcl_scenes_cluster_remove_all_scenes_response_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_scenes_cluster_remove_all_scenes_response_command_t, status), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_scenes_cluster_remove_all_scenes_response_command_t, groupId), \
}


/** @brief Structure for ZCL command "MoveToLevelWithOnOff" from "Level Control" 
 */
typedef struct __zcl_level_control_cluster_move_to_level_with_on_off_command {
  uint8_t level;
  uint16_t transitionTime;
} sl_zcl_level_control_cluster_move_to_level_with_on_off_command_t;

#define sl_zcl_level_control_cluster_move_to_level_with_on_off_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_level_control_cluster_move_to_level_with_on_off_command_t, level), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_level_control_cluster_move_to_level_with_on_off_command_t, transitionTime), \
}


/** @brief Structure for ZCL command "GetWeekdaySchedule" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_get_weekday_schedule_command {
  uint8_t scheduleId;
  uint16_t userId;
} sl_zcl_door_lock_cluster_get_weekday_schedule_command_t;

#define sl_zcl_door_lock_cluster_get_weekday_schedule_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_get_weekday_schedule_command_t, scheduleId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_door_lock_cluster_get_weekday_schedule_command_t, userId), \
}


/** @brief Structure for ZCL command "ClearWeekdaySchedule" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_clear_weekday_schedule_command {
  uint8_t scheduleId;
  uint16_t userId;
} sl_zcl_door_lock_cluster_clear_weekday_schedule_command_t;

#define sl_zcl_door_lock_cluster_clear_weekday_schedule_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_clear_weekday_schedule_command_t, scheduleId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_door_lock_cluster_clear_weekday_schedule_command_t, userId), \
}


/** @brief Structure for ZCL command "GetYeardaySchedule" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_get_yearday_schedule_command {
  uint8_t scheduleId;
  uint16_t userId;
} sl_zcl_door_lock_cluster_get_yearday_schedule_command_t;

#define sl_zcl_door_lock_cluster_get_yearday_schedule_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_get_yearday_schedule_command_t, scheduleId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_door_lock_cluster_get_yearday_schedule_command_t, userId), \
}


/** @brief Structure for ZCL command "ClearYeardaySchedule" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_clear_yearday_schedule_command {
  uint8_t scheduleId;
  uint16_t userId;
} sl_zcl_door_lock_cluster_clear_yearday_schedule_command_t;

#define sl_zcl_door_lock_cluster_clear_yearday_schedule_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_clear_yearday_schedule_command_t, scheduleId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_door_lock_cluster_clear_yearday_schedule_command_t, userId), \
}


/** @brief Structure for ZCL command "SetPendingNetworkUpdate" from "SL Works With All Hubs" 
 */
typedef struct __zcl_sl_works_with_all_hubs_cluster_set_pending_network_update_command {
  uint8_t channel;
  uint16_t panId;
} sl_zcl_sl_works_with_all_hubs_cluster_set_pending_network_update_command_t;

#define sl_zcl_sl_works_with_all_hubs_cluster_set_pending_network_update_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_sl_works_with_all_hubs_cluster_set_pending_network_update_command_t, channel), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_sl_works_with_all_hubs_cluster_set_pending_network_update_command_t, panId), \
}


/** @brief Structure for ZCL command "GetNotifiedMessage" from "Simple Metering" 
 */
typedef struct __zcl_simple_metering_cluster_get_notified_message_command {
  uint8_t notificationScheme;
  uint16_t notificationFlagAttributeId;
  uint32_t notificationFlagsN;
} sl_zcl_simple_metering_cluster_get_notified_message_command_t;

#define sl_zcl_simple_metering_cluster_get_notified_message_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_simple_metering_cluster_get_notified_message_command_t, notificationScheme), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_simple_metering_cluster_get_notified_message_command_t, notificationFlagAttributeId), \
  (ZAP_SIGNATURE_TYPE_BITMAP32), offsetof(sl_zcl_simple_metering_cluster_get_notified_message_command_t, notificationFlagsN), \
}


/** @brief Structure for ZCL command "SetWeekdaySchedule" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_set_weekday_schedule_command {
  uint8_t scheduleId;
  uint16_t userId;
  uint8_t daysMask;
  uint8_t startHour;
  uint8_t startMinute;
  uint8_t endHour;
  uint8_t endMinute;
} sl_zcl_door_lock_cluster_set_weekday_schedule_command_t;

#define sl_zcl_door_lock_cluster_set_weekday_schedule_command_signature  { \
  7, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_set_weekday_schedule_command_t, scheduleId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_door_lock_cluster_set_weekday_schedule_command_t, userId), \
  (ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_door_lock_cluster_set_weekday_schedule_command_t, daysMask), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_set_weekday_schedule_command_t, startHour), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_set_weekday_schedule_command_t, startMinute), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_set_weekday_schedule_command_t, endHour), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_set_weekday_schedule_command_t, endMinute), \
}


/** @brief Structure for ZCL command "MoveToLevel" from "Level Control" 
 */
typedef struct __zcl_level_control_cluster_move_to_level_command {
  uint8_t level;
  uint16_t transitionTime;
  uint8_t optionMask;
  uint8_t optionOverride;
} sl_zcl_level_control_cluster_move_to_level_command_t;

#define sl_zcl_level_control_cluster_move_to_level_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_level_control_cluster_move_to_level_command_t, level), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_level_control_cluster_move_to_level_command_t, transitionTime), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_level_control_cluster_move_to_level_command_t, optionMask), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_level_control_cluster_move_to_level_command_t, optionOverride), \
}


/** @brief Structure for ZCL command "MoveToSaturation" from "Color Control" 
 */
typedef struct __zcl_color_control_cluster_move_to_saturation_command {
  uint8_t saturation;
  uint16_t transitionTime;
  uint8_t optionsMask;
  uint8_t optionsOverride;
} sl_zcl_color_control_cluster_move_to_saturation_command_t;

#define sl_zcl_color_control_cluster_move_to_saturation_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_color_control_cluster_move_to_saturation_command_t, saturation), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_color_control_cluster_move_to_saturation_command_t, transitionTime), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_move_to_saturation_command_t, optionsMask), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_move_to_saturation_command_t, optionsOverride), \
}


/** @brief Structure for ZCL command "RequestTunnel" from "Tunneling" 
 */
typedef struct __zcl_tunneling_cluster_request_tunnel_command {
  uint8_t protocolId;
  uint16_t manufacturerCode;
  uint8_t flowControlSupport;
  uint16_t maximumIncomingTransferSize;
} sl_zcl_tunneling_cluster_request_tunnel_command_t;

#define sl_zcl_tunneling_cluster_request_tunnel_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_tunneling_cluster_request_tunnel_command_t, protocolId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_tunneling_cluster_request_tunnel_command_t, manufacturerCode), \
  (ZAP_SIGNATURE_TYPE_BOOLEAN), offsetof(sl_zcl_tunneling_cluster_request_tunnel_command_t, flowControlSupport), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_tunneling_cluster_request_tunnel_command_t, maximumIncomingTransferSize), \
}


/** @brief Structure for ZCL command "ViewGroupResponse" from "Groups" 
 */
typedef struct __zcl_groups_cluster_view_group_response_command {
  uint8_t status;
  uint16_t groupId;
  uint8_t * groupName;
} sl_zcl_groups_cluster_view_group_response_command_t;

#define sl_zcl_groups_cluster_view_group_response_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_groups_cluster_view_group_response_command_t, status), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_groups_cluster_view_group_response_command_t, groupId), \
  (ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_groups_cluster_view_group_response_command_t, groupName), \
}


/** @brief Structure for ZCL command "PowerProfileScheduleConstraintsNotification" from "Power Profile" 
 */
typedef struct __zcl_power_profile_cluster_power_profile_schedule_constraints_notification_command {
  uint8_t powerProfileId;
  uint16_t startAfter;
  uint16_t stopBefore;
} sl_zcl_power_profile_cluster_power_profile_schedule_constraints_notification_command_t;

#define sl_zcl_power_profile_cluster_power_profile_schedule_constraints_notification_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_power_profile_cluster_power_profile_schedule_constraints_notification_command_t, powerProfileId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_power_profile_cluster_power_profile_schedule_constraints_notification_command_t, startAfter), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_power_profile_cluster_power_profile_schedule_constraints_notification_command_t, stopBefore), \
}


/** @brief Structure for ZCL command "PowerProfileScheduleConstraintsResponse" from "Power Profile" 
 */
typedef struct __zcl_power_profile_cluster_power_profile_schedule_constraints_response_command {
  uint8_t powerProfileId;
  uint16_t startAfter;
  uint16_t stopBefore;
} sl_zcl_power_profile_cluster_power_profile_schedule_constraints_response_command_t;

#define sl_zcl_power_profile_cluster_power_profile_schedule_constraints_response_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_power_profile_cluster_power_profile_schedule_constraints_response_command_t, powerProfileId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_power_profile_cluster_power_profile_schedule_constraints_response_command_t, startAfter), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_power_profile_cluster_power_profile_schedule_constraints_response_command_t, stopBefore), \
}


/** @brief Structure for ZCL command "UpgradeEndRequest" from "Over the Air Bootloading" 
 */
typedef struct __zcl_over_the_air_bootloading_cluster_upgrade_end_request_command {
  uint8_t status;
  uint16_t manufacturerId;
  uint16_t imageType;
  uint32_t fileVersion;
} sl_zcl_over_the_air_bootloading_cluster_upgrade_end_request_command_t;

#define sl_zcl_over_the_air_bootloading_cluster_upgrade_end_request_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_over_the_air_bootloading_cluster_upgrade_end_request_command_t, status), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_over_the_air_bootloading_cluster_upgrade_end_request_command_t, manufacturerId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_over_the_air_bootloading_cluster_upgrade_end_request_command_t, imageType), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_over_the_air_bootloading_cluster_upgrade_end_request_command_t, fileVersion), \
}


/** @brief Structure for ZCL command "QueryNextImageRequest" from "Over the Air Bootloading" 
 */
typedef struct __zcl_over_the_air_bootloading_cluster_query_next_image_request_command {
  uint8_t fieldControl;
  uint16_t manufacturerId;
  uint16_t imageType;
  uint32_t currentFileVersion;
  uint16_t hardwareVersion;
} sl_zcl_over_the_air_bootloading_cluster_query_next_image_request_command_t;

#define sl_zcl_over_the_air_bootloading_cluster_query_next_image_request_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_over_the_air_bootloading_cluster_query_next_image_request_command_t, fieldControl), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_over_the_air_bootloading_cluster_query_next_image_request_command_t, manufacturerId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_over_the_air_bootloading_cluster_query_next_image_request_command_t, imageType), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_over_the_air_bootloading_cluster_query_next_image_request_command_t, currentFileVersion), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_over_the_air_bootloading_cluster_query_next_image_request_command_t, hardwareVersion), \
}


/** @brief Structure for ZCL command "QueryNextImageResponse" from "Over the Air Bootloading" 
 */
typedef struct __zcl_over_the_air_bootloading_cluster_query_next_image_response_command {
  uint8_t status;
  uint16_t manufacturerId;
  uint16_t imageType;
  uint32_t fileVersion;
  uint32_t imageSize;
} sl_zcl_over_the_air_bootloading_cluster_query_next_image_response_command_t;

#define sl_zcl_over_the_air_bootloading_cluster_query_next_image_response_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_over_the_air_bootloading_cluster_query_next_image_response_command_t, status), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_over_the_air_bootloading_cluster_query_next_image_response_command_t, manufacturerId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_over_the_air_bootloading_cluster_query_next_image_response_command_t, imageType), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_over_the_air_bootloading_cluster_query_next_image_response_command_t, fileVersion), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_over_the_air_bootloading_cluster_query_next_image_response_command_t, imageSize), \
}


/** @brief Structure for ZCL command "QuerySpecificFileResponse" from "Over the Air Bootloading" 
 */
typedef struct __zcl_over_the_air_bootloading_cluster_query_specific_file_response_command {
  uint8_t status;
  uint16_t manufacturerId;
  uint16_t imageType;
  uint32_t fileVersion;
  uint32_t imageSize;
} sl_zcl_over_the_air_bootloading_cluster_query_specific_file_response_command_t;

#define sl_zcl_over_the_air_bootloading_cluster_query_specific_file_response_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_over_the_air_bootloading_cluster_query_specific_file_response_command_t, status), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_over_the_air_bootloading_cluster_query_specific_file_response_command_t, manufacturerId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_over_the_air_bootloading_cluster_query_specific_file_response_command_t, imageType), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_over_the_air_bootloading_cluster_query_specific_file_response_command_t, fileVersion), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_over_the_air_bootloading_cluster_query_specific_file_response_command_t, imageSize), \
}


/** @brief Structure for ZCL command "ImageBlockRequest" from "Over the Air Bootloading" 
 */
typedef struct __zcl_over_the_air_bootloading_cluster_image_block_request_command {
  uint8_t fieldControl;
  uint16_t manufacturerId;
  uint16_t imageType;
  uint32_t fileVersion;
  uint32_t fileOffset;
  uint8_t maxDataSize;
  uint8_t * requestNodeAddress;
} sl_zcl_over_the_air_bootloading_cluster_image_block_request_command_t;

#define sl_zcl_over_the_air_bootloading_cluster_image_block_request_command_signature  { \
  7, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_over_the_air_bootloading_cluster_image_block_request_command_t, fieldControl), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_over_the_air_bootloading_cluster_image_block_request_command_t, manufacturerId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_over_the_air_bootloading_cluster_image_block_request_command_t, imageType), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_over_the_air_bootloading_cluster_image_block_request_command_t, fileVersion), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_over_the_air_bootloading_cluster_image_block_request_command_t, fileOffset), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_over_the_air_bootloading_cluster_image_block_request_command_t, maxDataSize), \
  (ZAP_SIGNATURE_TYPE_IEEE_ADDRESS), offsetof(sl_zcl_over_the_air_bootloading_cluster_image_block_request_command_t, requestNodeAddress), \
}


/** @brief Structure for ZCL command "ImagePageRequest" from "Over the Air Bootloading" 
 */
typedef struct __zcl_over_the_air_bootloading_cluster_image_page_request_command {
  uint8_t fieldControl;
  uint16_t manufacturerId;
  uint16_t imageType;
  uint32_t fileVersion;
  uint32_t fileOffset;
  uint8_t maxDataSize;
  uint16_t pageSize;
  uint16_t responseSpacing;
  uint8_t * requestNodeAddress;
} sl_zcl_over_the_air_bootloading_cluster_image_page_request_command_t;

#define sl_zcl_over_the_air_bootloading_cluster_image_page_request_command_signature  { \
  9, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_over_the_air_bootloading_cluster_image_page_request_command_t, fieldControl), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_over_the_air_bootloading_cluster_image_page_request_command_t, manufacturerId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_over_the_air_bootloading_cluster_image_page_request_command_t, imageType), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_over_the_air_bootloading_cluster_image_page_request_command_t, fileVersion), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_over_the_air_bootloading_cluster_image_page_request_command_t, fileOffset), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_over_the_air_bootloading_cluster_image_page_request_command_t, maxDataSize), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_over_the_air_bootloading_cluster_image_page_request_command_t, pageSize), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_over_the_air_bootloading_cluster_image_page_request_command_t, responseSpacing), \
  (ZAP_SIGNATURE_TYPE_IEEE_ADDRESS), offsetof(sl_zcl_over_the_air_bootloading_cluster_image_page_request_command_t, requestNodeAddress), \
}


/** @brief Structure for ZCL command "ImageBlockResponse" from "Over the Air Bootloading" 
 */
typedef struct __zcl_over_the_air_bootloading_cluster_image_block_response_command {
  uint8_t status;
  uint16_t manufacturerId;
  uint16_t imageType;
  uint32_t fileVersion;
  uint32_t fileOffset;
  uint8_t dataSize;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * imageData;
} sl_zcl_over_the_air_bootloading_cluster_image_block_response_command_t;

#define sl_zcl_over_the_air_bootloading_cluster_image_block_response_command_signature  { \
  7, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_over_the_air_bootloading_cluster_image_block_response_command_t, status), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_over_the_air_bootloading_cluster_image_block_response_command_t, manufacturerId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_over_the_air_bootloading_cluster_image_block_response_command_t, imageType), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_over_the_air_bootloading_cluster_image_block_response_command_t, fileVersion), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_over_the_air_bootloading_cluster_image_block_response_command_t, fileOffset), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_over_the_air_bootloading_cluster_image_block_response_command_t, dataSize), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_over_the_air_bootloading_cluster_image_block_response_command_t, imageData), \
}


/** @brief Structure for ZCL command "SetYeardaySchedule" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_set_yearday_schedule_command {
  uint8_t scheduleId;
  uint16_t userId;
  uint32_t localStartTime;
  uint32_t localEndTime;
} sl_zcl_door_lock_cluster_set_yearday_schedule_command_t;

#define sl_zcl_door_lock_cluster_set_yearday_schedule_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_set_yearday_schedule_command_t, scheduleId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_door_lock_cluster_set_yearday_schedule_command_t, userId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_door_lock_cluster_set_yearday_schedule_command_t, localStartTime), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_door_lock_cluster_set_yearday_schedule_command_t, localEndTime), \
}


/** @brief Structure for ZCL command "GetPowerProfilePriceResponse" from "Power Profile" 
 */
typedef struct __zcl_power_profile_cluster_get_power_profile_price_response_command {
  uint8_t powerProfileId;
  uint16_t currency;
  uint32_t price;
  uint8_t priceTrailingDigit;
} sl_zcl_power_profile_cluster_get_power_profile_price_response_command_t;

#define sl_zcl_power_profile_cluster_get_power_profile_price_response_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_power_profile_cluster_get_power_profile_price_response_command_t, powerProfileId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_power_profile_cluster_get_power_profile_price_response_command_t, currency), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_power_profile_cluster_get_power_profile_price_response_command_t, price), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_power_profile_cluster_get_power_profile_price_response_command_t, priceTrailingDigit), \
}


/** @brief Structure for ZCL command "GetPowerProfilePriceExtendedResponse" from "Power Profile" 
 */
typedef struct __zcl_power_profile_cluster_get_power_profile_price_extended_response_command {
  uint8_t powerProfileId;
  uint16_t currency;
  uint32_t price;
  uint8_t priceTrailingDigit;
} sl_zcl_power_profile_cluster_get_power_profile_price_extended_response_command_t;

#define sl_zcl_power_profile_cluster_get_power_profile_price_extended_response_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_power_profile_cluster_get_power_profile_price_extended_response_command_t, powerProfileId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_power_profile_cluster_get_power_profile_price_extended_response_command_t, currency), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_power_profile_cluster_get_power_profile_price_extended_response_command_t, price), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_power_profile_cluster_get_power_profile_price_extended_response_command_t, priceTrailingDigit), \
}


/** @brief Structure for ZCL command "AddSceneResponse" from "Scenes" 
 */
typedef struct __zcl_scenes_cluster_add_scene_response_command {
  uint8_t status;
  uint16_t groupId;
  uint8_t sceneId;
} sl_zcl_scenes_cluster_add_scene_response_command_t;

#define sl_zcl_scenes_cluster_add_scene_response_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_scenes_cluster_add_scene_response_command_t, status), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_scenes_cluster_add_scene_response_command_t, groupId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_scenes_cluster_add_scene_response_command_t, sceneId), \
}


/** @brief Structure for ZCL command "RemoveSceneResponse" from "Scenes" 
 */
typedef struct __zcl_scenes_cluster_remove_scene_response_command {
  uint8_t status;
  uint16_t groupId;
  uint8_t sceneId;
} sl_zcl_scenes_cluster_remove_scene_response_command_t;

#define sl_zcl_scenes_cluster_remove_scene_response_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_scenes_cluster_remove_scene_response_command_t, status), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_scenes_cluster_remove_scene_response_command_t, groupId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_scenes_cluster_remove_scene_response_command_t, sceneId), \
}


/** @brief Structure for ZCL command "StoreSceneResponse" from "Scenes" 
 */
typedef struct __zcl_scenes_cluster_store_scene_response_command {
  uint8_t status;
  uint16_t groupId;
  uint8_t sceneId;
} sl_zcl_scenes_cluster_store_scene_response_command_t;

#define sl_zcl_scenes_cluster_store_scene_response_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_scenes_cluster_store_scene_response_command_t, status), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_scenes_cluster_store_scene_response_command_t, groupId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_scenes_cluster_store_scene_response_command_t, sceneId), \
}


/** @brief Structure for ZCL command "EnhancedAddSceneResponse" from "Scenes" 
 */
typedef struct __zcl_scenes_cluster_enhanced_add_scene_response_command {
  uint8_t status;
  uint16_t groupId;
  uint8_t sceneId;
} sl_zcl_scenes_cluster_enhanced_add_scene_response_command_t;

#define sl_zcl_scenes_cluster_enhanced_add_scene_response_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_scenes_cluster_enhanced_add_scene_response_command_t, status), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_scenes_cluster_enhanced_add_scene_response_command_t, groupId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_scenes_cluster_enhanced_add_scene_response_command_t, sceneId), \
}


/** @brief Structure for ZCL command "CopySceneResponse" from "Scenes" 
 */
typedef struct __zcl_scenes_cluster_copy_scene_response_command {
  uint8_t status;
  uint16_t groupIdFrom;
  uint8_t sceneIdFrom;
} sl_zcl_scenes_cluster_copy_scene_response_command_t;

#define sl_zcl_scenes_cluster_copy_scene_response_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_scenes_cluster_copy_scene_response_command_t, status), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_scenes_cluster_copy_scene_response_command_t, groupIdFrom), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_scenes_cluster_copy_scene_response_command_t, sceneIdFrom), \
}


/** @brief Structure for ZCL command "EnhancedViewSceneResponse" from "Scenes" 
 */
typedef struct __zcl_scenes_cluster_enhanced_view_scene_response_command {
  uint8_t status;
  uint16_t groupId;
  uint8_t sceneId;
  uint16_t transitionTime;
  uint8_t * sceneName;
  /* TYPE WARNING: SceneExtensionFieldSet array defaults to */ uint8_t * extensionFieldSets;
} sl_zcl_scenes_cluster_enhanced_view_scene_response_command_t;

#define sl_zcl_scenes_cluster_enhanced_view_scene_response_command_signature  { \
  6, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_scenes_cluster_enhanced_view_scene_response_command_t, status), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_scenes_cluster_enhanced_view_scene_response_command_t, groupId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_scenes_cluster_enhanced_view_scene_response_command_t, sceneId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_scenes_cluster_enhanced_view_scene_response_command_t, transitionTime), \
  (ZAP_SIGNATURE_TYPE_CHAR_STRING), offsetof(sl_zcl_scenes_cluster_enhanced_view_scene_response_command_t, sceneName), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_scenes_cluster_enhanced_view_scene_response_command_t, extensionFieldSets), \
}


/** @brief Structure for ZCL command "GetYeardayScheduleResponse" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_get_yearday_schedule_response_command {
  uint8_t scheduleId;
  uint16_t userId;
  uint8_t status;
  uint32_t localStartTime;
  uint32_t localEndTime;
} sl_zcl_door_lock_cluster_get_yearday_schedule_response_command_t;

#define sl_zcl_door_lock_cluster_get_yearday_schedule_response_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_get_yearday_schedule_response_command_t, scheduleId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_door_lock_cluster_get_yearday_schedule_response_command_t, userId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_get_yearday_schedule_response_command_t, status), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_door_lock_cluster_get_yearday_schedule_response_command_t, localStartTime), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_door_lock_cluster_get_yearday_schedule_response_command_t, localEndTime), \
}


/** @brief Structure for ZCL command "GetWeekdayScheduleResponse" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_get_weekday_schedule_response_command {
  uint8_t scheduleId;
  uint16_t userId;
  uint8_t status;
  uint8_t daysMask;
  uint8_t startHour;
  uint8_t startMinute;
  uint8_t endHour;
  uint8_t endMinute;
} sl_zcl_door_lock_cluster_get_weekday_schedule_response_command_t;

#define sl_zcl_door_lock_cluster_get_weekday_schedule_response_command_signature  { \
  8, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_get_weekday_schedule_response_command_t, scheduleId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_door_lock_cluster_get_weekday_schedule_response_command_t, userId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_get_weekday_schedule_response_command_t, status), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_get_weekday_schedule_response_command_t, daysMask), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_get_weekday_schedule_response_command_t, startHour), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_get_weekday_schedule_response_command_t, startMinute), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_get_weekday_schedule_response_command_t, endHour), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_get_weekday_schedule_response_command_t, endMinute), \
}


/** @brief Structure for ZCL command "RequestPreferenceResponse" from "Information" 
 */
typedef struct __zcl_information_cluster_request_preference_response_command {
  uint8_t statusFeedback;
  uint16_t preferenceType;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * preferencePayload;
} sl_zcl_information_cluster_request_preference_response_command_t;

#define sl_zcl_information_cluster_request_preference_response_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_information_cluster_request_preference_response_command_t, statusFeedback), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_information_cluster_request_preference_response_command_t, preferenceType), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_information_cluster_request_preference_response_command_t, preferencePayload), \
}


/** @brief Structure for ZCL command "GetGroupMembership" from "Groups" 
 */
typedef struct __zcl_groups_cluster_get_group_membership_command {
  uint8_t groupCount;
  /* TYPE WARNING: INT16U array defaults to */ uint8_t * groupList;
} sl_zcl_groups_cluster_get_group_membership_command_t;

#define sl_zcl_groups_cluster_get_group_membership_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_groups_cluster_get_group_membership_command_t, groupCount), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_groups_cluster_get_group_membership_command_t, groupList), \
}


/** @brief Structure for ZCL command "NewDebugReportNotification" from "SL Works With All Hubs" 
 */
typedef struct __zcl_sl_works_with_all_hubs_cluster_new_debug_report_notification_command {
  uint8_t debugReportId;
  uint32_t debugReportSize;
} sl_zcl_sl_works_with_all_hubs_cluster_new_debug_report_notification_command_t;

#define sl_zcl_sl_works_with_all_hubs_cluster_new_debug_report_notification_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_sl_works_with_all_hubs_cluster_new_debug_report_notification_command_t, debugReportId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_sl_works_with_all_hubs_cluster_new_debug_report_notification_command_t, debugReportSize), \
}


/** @brief Structure for ZCL command "SetHolidaySchedule" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_set_holiday_schedule_command {
  uint8_t scheduleId;
  uint32_t localStartTime;
  uint32_t localEndTime;
  uint8_t operatingModeDuringHoliday;
} sl_zcl_door_lock_cluster_set_holiday_schedule_command_t;

#define sl_zcl_door_lock_cluster_set_holiday_schedule_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_set_holiday_schedule_command_t, scheduleId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_door_lock_cluster_set_holiday_schedule_command_t, localStartTime), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_door_lock_cluster_set_holiday_schedule_command_t, localEndTime), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_door_lock_cluster_set_holiday_schedule_command_t, operatingModeDuringHoliday), \
}


/** @brief Structure for ZCL command "LogQueueResponse" from "Appliance Statistics" 
 */
typedef struct __zcl_appliance_statistics_cluster_log_queue_response_command {
  uint8_t logQueueSize;
  /* TYPE WARNING: INT32U array defaults to */ uint8_t * logIds;
} sl_zcl_appliance_statistics_cluster_log_queue_response_command_t;

#define sl_zcl_appliance_statistics_cluster_log_queue_response_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_appliance_statistics_cluster_log_queue_response_command_t, logQueueSize), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_appliance_statistics_cluster_log_queue_response_command_t, logIds), \
}


/** @brief Structure for ZCL command "StatisticsAvailable" from "Appliance Statistics" 
 */
typedef struct __zcl_appliance_statistics_cluster_statistics_available_command {
  uint8_t logQueueSize;
  /* TYPE WARNING: INT32U array defaults to */ uint8_t * logIds;
} sl_zcl_appliance_statistics_cluster_statistics_available_command_t;

#define sl_zcl_appliance_statistics_cluster_statistics_available_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_appliance_statistics_cluster_statistics_available_command_t, logQueueSize), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_appliance_statistics_cluster_statistics_available_command_t, logIds), \
}


/** @brief Structure for ZCL command "stream" from "MFGLIB Cluster" 
 */
typedef struct __zcl_mfglib_cluster_cluster_stream_command {
  uint8_t channel;
  int8_t power;
  uint16_t time;
} sl_zcl_mfglib_cluster_cluster_stream_command_t;

#define sl_zcl_mfglib_cluster_cluster_stream_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_mfglib_cluster_cluster_stream_command_t, channel), \
  (ZAP_SIGNATURE_TYPE_INT8S), offsetof(sl_zcl_mfglib_cluster_cluster_stream_command_t, power), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_mfglib_cluster_cluster_stream_command_t, time), \
}


/** @brief Structure for ZCL command "tone" from "MFGLIB Cluster" 
 */
typedef struct __zcl_mfglib_cluster_cluster_tone_command {
  uint8_t channel;
  int8_t power;
  uint16_t time;
} sl_zcl_mfglib_cluster_cluster_tone_command_t;

#define sl_zcl_mfglib_cluster_cluster_tone_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_mfglib_cluster_cluster_tone_command_t, channel), \
  (ZAP_SIGNATURE_TYPE_INT8S), offsetof(sl_zcl_mfglib_cluster_cluster_tone_command_t, power), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_mfglib_cluster_cluster_tone_command_t, time), \
}


/** @brief Structure for ZCL command "rxMode" from "MFGLIB Cluster" 
 */
typedef struct __zcl_mfglib_cluster_cluster_rx_mode_command {
  uint8_t channel;
  int8_t power;
  uint16_t time;
} sl_zcl_mfglib_cluster_cluster_rx_mode_command_t;

#define sl_zcl_mfglib_cluster_cluster_rx_mode_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_mfglib_cluster_cluster_rx_mode_command_t, channel), \
  (ZAP_SIGNATURE_TYPE_INT8S), offsetof(sl_zcl_mfglib_cluster_cluster_rx_mode_command_t, power), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_mfglib_cluster_cluster_rx_mode_command_t, time), \
}


/** @brief Structure for ZCL command "InitiateTestMode" from "IAS Zone" 
 */
typedef struct __zcl_ias_zone_cluster_initiate_test_mode_command {
  uint8_t testModeDuration;
  uint8_t currentZoneSensitivityLevel;
} sl_zcl_ias_zone_cluster_initiate_test_mode_command_t;

#define sl_zcl_ias_zone_cluster_initiate_test_mode_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_ias_zone_cluster_initiate_test_mode_command_t, testModeDuration), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_ias_zone_cluster_initiate_test_mode_command_t, currentZoneSensitivityLevel), \
}


/** @brief Structure for ZCL command "RequestFastPollMode" from "Simple Metering" 
 */
typedef struct __zcl_simple_metering_cluster_request_fast_poll_mode_command {
  uint8_t fastPollUpdatePeriod;
  uint8_t duration;
} sl_zcl_simple_metering_cluster_request_fast_poll_mode_command_t;

#define sl_zcl_simple_metering_cluster_request_fast_poll_mode_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_simple_metering_cluster_request_fast_poll_mode_command_t, fastPollUpdatePeriod), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_simple_metering_cluster_request_fast_poll_mode_command_t, duration), \
}


/** @brief Structure for ZCL command "GetZoneStatus" from "IAS ACE" 
 */
typedef struct __zcl_ias_ace_cluster_get_zone_status_command {
  uint8_t startingZoneId;
  uint8_t maxNumberOfZoneIds;
  uint8_t zoneStatusMaskFlag;
  uint16_t zoneStatusMask;
} sl_zcl_ias_ace_cluster_get_zone_status_command_t;

#define sl_zcl_ias_ace_cluster_get_zone_status_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_ias_ace_cluster_get_zone_status_command_t, startingZoneId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_ias_ace_cluster_get_zone_status_command_t, maxNumberOfZoneIds), \
  (ZAP_SIGNATURE_TYPE_BOOLEAN), offsetof(sl_zcl_ias_ace_cluster_get_zone_status_command_t, zoneStatusMaskFlag), \
  (ZAP_SIGNATURE_TYPE_BITMAP16), offsetof(sl_zcl_ias_ace_cluster_get_zone_status_command_t, zoneStatusMask), \
}


/** @brief Structure for ZCL command "MoveToHueAndSaturation" from "Color Control" 
 */
typedef struct __zcl_color_control_cluster_move_to_hue_and_saturation_command {
  uint8_t hue;
  uint8_t saturation;
  uint16_t transitionTime;
  uint8_t optionsMask;
  uint8_t optionsOverride;
} sl_zcl_color_control_cluster_move_to_hue_and_saturation_command_t;

#define sl_zcl_color_control_cluster_move_to_hue_and_saturation_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_color_control_cluster_move_to_hue_and_saturation_command_t, hue), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_color_control_cluster_move_to_hue_and_saturation_command_t, saturation), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_color_control_cluster_move_to_hue_and_saturation_command_t, transitionTime), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_move_to_hue_and_saturation_command_t, optionsMask), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_BITMAP8), offsetof(sl_zcl_color_control_cluster_move_to_hue_and_saturation_command_t, optionsOverride), \
}


/** @brief Structure for ZCL command "GetGroupMembershipResponse" from "Groups" 
 */
typedef struct __zcl_groups_cluster_get_group_membership_response_command {
  uint8_t capacity;
  uint8_t groupCount;
  /* TYPE WARNING: INT16U array defaults to */ uint8_t * groupList;
} sl_zcl_groups_cluster_get_group_membership_response_command_t;

#define sl_zcl_groups_cluster_get_group_membership_response_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_groups_cluster_get_group_membership_response_command_t, capacity), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_groups_cluster_get_group_membership_response_command_t, groupCount), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_groups_cluster_get_group_membership_response_command_t, groupList), \
}


/** @brief Structure for ZCL command "EnableWwahAppEventRetryAlgorithm" from "SL Works With All Hubs" 
 */
typedef struct __zcl_sl_works_with_all_hubs_cluster_enable_wwah_app_event_retry_algorithm_command {
  uint8_t firstBackoffTimeSeconds;
  uint8_t backoffSeqCommonRatio;
  uint32_t maxBackoffTimeSeconds;
  uint8_t maxRedeliveryAttempts;
} sl_zcl_sl_works_with_all_hubs_cluster_enable_wwah_app_event_retry_algorithm_command_t;

#define sl_zcl_sl_works_with_all_hubs_cluster_enable_wwah_app_event_retry_algorithm_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_sl_works_with_all_hubs_cluster_enable_wwah_app_event_retry_algorithm_command_t, firstBackoffTimeSeconds), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_sl_works_with_all_hubs_cluster_enable_wwah_app_event_retry_algorithm_command_t, backoffSeqCommonRatio), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_sl_works_with_all_hubs_cluster_enable_wwah_app_event_retry_algorithm_command_t, maxBackoffTimeSeconds), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_sl_works_with_all_hubs_cluster_enable_wwah_app_event_retry_algorithm_command_t, maxRedeliveryAttempts), \
}


/** @brief Structure for ZCL command "GetHolidayScheduleResponse" from "Door Lock" 
 */
typedef struct __zcl_door_lock_cluster_get_holiday_schedule_response_command {
  uint8_t scheduleId;
  uint8_t status;
  uint32_t localStartTime;
  uint32_t localEndTime;
  uint8_t operatingModeDuringHoliday;
} sl_zcl_door_lock_cluster_get_holiday_schedule_response_command_t;

#define sl_zcl_door_lock_cluster_get_holiday_schedule_response_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_get_holiday_schedule_response_command_t, scheduleId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_door_lock_cluster_get_holiday_schedule_response_command_t, status), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_door_lock_cluster_get_holiday_schedule_response_command_t, localStartTime), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_door_lock_cluster_get_holiday_schedule_response_command_t, localEndTime), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_door_lock_cluster_get_holiday_schedule_response_command_t, operatingModeDuringHoliday), \
}


/** @brief Structure for ZCL command "PowerProfileNotification" from "Power Profile" 
 */
typedef struct __zcl_power_profile_cluster_power_profile_notification_command {
  uint8_t totalProfileNum;
  uint8_t powerProfileId;
  uint8_t numOfTransferredPhases;
  /* TYPE WARNING: TransferredPhase array defaults to */ uint8_t * transferredPhases;
} sl_zcl_power_profile_cluster_power_profile_notification_command_t;

#define sl_zcl_power_profile_cluster_power_profile_notification_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_power_profile_cluster_power_profile_notification_command_t, totalProfileNum), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_power_profile_cluster_power_profile_notification_command_t, powerProfileId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_power_profile_cluster_power_profile_notification_command_t, numOfTransferredPhases), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_power_profile_cluster_power_profile_notification_command_t, transferredPhases), \
}


/** @brief Structure for ZCL command "PowerProfileResponse" from "Power Profile" 
 */
typedef struct __zcl_power_profile_cluster_power_profile_response_command {
  uint8_t totalProfileNum;
  uint8_t powerProfileId;
  uint8_t numOfTransferredPhases;
  /* TYPE WARNING: TransferredPhase array defaults to */ uint8_t * transferredPhases;
} sl_zcl_power_profile_cluster_power_profile_response_command_t;

#define sl_zcl_power_profile_cluster_power_profile_response_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_power_profile_cluster_power_profile_response_command_t, totalProfileNum), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_power_profile_cluster_power_profile_response_command_t, powerProfileId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_power_profile_cluster_power_profile_response_command_t, numOfTransferredPhases), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_power_profile_cluster_power_profile_response_command_t, transferredPhases), \
}


/** @brief Structure for ZCL command "GetGroupIdentifiersResponse" from "ZLL Commissioning" 
 */
typedef struct __zcl_zll_commissioning_cluster_get_group_identifiers_response_command {
  uint8_t total;
  uint8_t startIndex;
  uint8_t count;
  /* TYPE WARNING: GroupInformationRecord array defaults to */ uint8_t * groupInformationRecordList;
} sl_zcl_zll_commissioning_cluster_get_group_identifiers_response_command_t;

#define sl_zcl_zll_commissioning_cluster_get_group_identifiers_response_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_zll_commissioning_cluster_get_group_identifiers_response_command_t, total), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_zll_commissioning_cluster_get_group_identifiers_response_command_t, startIndex), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_zll_commissioning_cluster_get_group_identifiers_response_command_t, count), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_zll_commissioning_cluster_get_group_identifiers_response_command_t, groupInformationRecordList), \
}


/** @brief Structure for ZCL command "GetEndpointListResponse" from "ZLL Commissioning" 
 */
typedef struct __zcl_zll_commissioning_cluster_get_endpoint_list_response_command {
  uint8_t total;
  uint8_t startIndex;
  uint8_t count;
  /* TYPE WARNING: EndpointInformationRecord array defaults to */ uint8_t * endpointInformationRecordList;
} sl_zcl_zll_commissioning_cluster_get_endpoint_list_response_command_t;

#define sl_zcl_zll_commissioning_cluster_get_endpoint_list_response_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_zll_commissioning_cluster_get_endpoint_list_response_command_t, total), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_zll_commissioning_cluster_get_endpoint_list_response_command_t, startIndex), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_zll_commissioning_cluster_get_endpoint_list_response_command_t, count), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_zll_commissioning_cluster_get_endpoint_list_response_command_t, endpointInformationRecordList), \
}


/** @brief Structure for ZCL command "EnergyPhasesScheduleNotification" from "Power Profile" 
 */
typedef struct __zcl_power_profile_cluster_energy_phases_schedule_notification_command {
  uint8_t powerProfileId;
  uint8_t numOfScheduledPhases;
  /* TYPE WARNING: ScheduledPhase array defaults to */ uint8_t * scheduledPhases;
} sl_zcl_power_profile_cluster_energy_phases_schedule_notification_command_t;

#define sl_zcl_power_profile_cluster_energy_phases_schedule_notification_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_power_profile_cluster_energy_phases_schedule_notification_command_t, powerProfileId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_power_profile_cluster_energy_phases_schedule_notification_command_t, numOfScheduledPhases), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_power_profile_cluster_energy_phases_schedule_notification_command_t, scheduledPhases), \
}


/** @brief Structure for ZCL command "EnergyPhasesScheduleResponse" from "Power Profile" 
 */
typedef struct __zcl_power_profile_cluster_energy_phases_schedule_response_command {
  uint8_t powerProfileId;
  uint8_t numOfScheduledPhases;
  /* TYPE WARNING: ScheduledPhase array defaults to */ uint8_t * scheduledPhases;
} sl_zcl_power_profile_cluster_energy_phases_schedule_response_command_t;

#define sl_zcl_power_profile_cluster_energy_phases_schedule_response_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_power_profile_cluster_energy_phases_schedule_response_command_t, powerProfileId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_power_profile_cluster_energy_phases_schedule_response_command_t, numOfScheduledPhases), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_power_profile_cluster_energy_phases_schedule_response_command_t, scheduledPhases), \
}


/** @brief Structure for ZCL command "EnergyPhasesScheduleStateResponse" from "Power Profile" 
 */
typedef struct __zcl_power_profile_cluster_energy_phases_schedule_state_response_command {
  uint8_t powerProfileId;
  uint8_t numOfScheduledPhases;
  /* TYPE WARNING: ScheduledPhase array defaults to */ uint8_t * scheduledPhases;
} sl_zcl_power_profile_cluster_energy_phases_schedule_state_response_command_t;

#define sl_zcl_power_profile_cluster_energy_phases_schedule_state_response_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_power_profile_cluster_energy_phases_schedule_state_response_command_t, powerProfileId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_power_profile_cluster_energy_phases_schedule_state_response_command_t, numOfScheduledPhases), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_power_profile_cluster_energy_phases_schedule_state_response_command_t, scheduledPhases), \
}


/** @brief Structure for ZCL command "EnergyPhasesScheduleStateNotification" from "Power Profile" 
 */
typedef struct __zcl_power_profile_cluster_energy_phases_schedule_state_notification_command {
  uint8_t powerProfileId;
  uint8_t numOfScheduledPhases;
  /* TYPE WARNING: ScheduledPhase array defaults to */ uint8_t * scheduledPhases;
} sl_zcl_power_profile_cluster_energy_phases_schedule_state_notification_command_t;

#define sl_zcl_power_profile_cluster_energy_phases_schedule_state_notification_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_power_profile_cluster_energy_phases_schedule_state_notification_command_t, powerProfileId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_power_profile_cluster_energy_phases_schedule_state_notification_command_t, numOfScheduledPhases), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_power_profile_cluster_energy_phases_schedule_state_notification_command_t, scheduledPhases), \
}


/** @brief Structure for ZCL command "PublishTopUpLog" from "Prepayment" 
 */
typedef struct __zcl_prepayment_cluster_publish_top_up_log_command {
  uint8_t commandIndex;
  uint8_t totalNumberOfCommands;
  /* TYPE WARNING: TopUpPayload array defaults to */ uint8_t * topUpPayload;
} sl_zcl_prepayment_cluster_publish_top_up_log_command_t;

#define sl_zcl_prepayment_cluster_publish_top_up_log_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_prepayment_cluster_publish_top_up_log_command_t, commandIndex), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_prepayment_cluster_publish_top_up_log_command_t, totalNumberOfCommands), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_prepayment_cluster_publish_top_up_log_command_t, topUpPayload), \
}


/** @brief Structure for ZCL command "PublishDebtLog" from "Prepayment" 
 */
typedef struct __zcl_prepayment_cluster_publish_debt_log_command {
  uint8_t commandIndex;
  uint8_t totalNumberOfCommands;
  /* TYPE WARNING: DebtPayload array defaults to */ uint8_t * debtPayload;
} sl_zcl_prepayment_cluster_publish_debt_log_command_t;

#define sl_zcl_prepayment_cluster_publish_debt_log_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_prepayment_cluster_publish_debt_log_command_t, commandIndex), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_prepayment_cluster_publish_debt_log_command_t, totalNumberOfCommands), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_prepayment_cluster_publish_debt_log_command_t, debtPayload), \
}


/** @brief Structure for ZCL command "ReportEventConfiguration" from "Device Management" 
 */
typedef struct __zcl_device_management_cluster_report_event_configuration_command {
  uint8_t commandIndex;
  uint8_t totalCommands;
  /* TYPE WARNING: EventConfigurationPayload array defaults to */ uint8_t * eventConfigurationPayload;
} sl_zcl_device_management_cluster_report_event_configuration_command_t;

#define sl_zcl_device_management_cluster_report_event_configuration_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_device_management_cluster_report_event_configuration_command_t, commandIndex), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_device_management_cluster_report_event_configuration_command_t, totalCommands), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_device_management_cluster_report_event_configuration_command_t, eventConfigurationPayload), \
}


/** @brief Structure for ZCL command "SetBypassedZoneList" from "IAS ACE" 
 */
typedef struct __zcl_ias_ace_cluster_set_bypassed_zone_list_command {
  uint8_t numberOfZones;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * zoneIds;
} sl_zcl_ias_ace_cluster_set_bypassed_zone_list_command_t;

#define sl_zcl_ias_ace_cluster_set_bypassed_zone_list_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_ias_ace_cluster_set_bypassed_zone_list_command_t, numberOfZones), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_ias_ace_cluster_set_bypassed_zone_list_command_t, zoneIds), \
}


/** @brief Structure for ZCL command "RequestInformationResponse" from "Information" 
 */
typedef struct __zcl_information_cluster_request_information_response_command {
  uint8_t number;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * buffer;
} sl_zcl_information_cluster_request_information_response_command_t;

#define sl_zcl_information_cluster_request_information_response_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_information_cluster_request_information_response_command_t, number), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_information_cluster_request_information_response_command_t, buffer), \
}


/** @brief Structure for ZCL command "DebugReportQueryResponse" from "SL Works With All Hubs" 
 */
typedef struct __zcl_sl_works_with_all_hubs_cluster_debug_report_query_response_command {
  uint8_t debugReportId;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * debugReportData;
} sl_zcl_sl_works_with_all_hubs_cluster_debug_report_query_response_command_t;

#define sl_zcl_sl_works_with_all_hubs_cluster_debug_report_query_response_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_sl_works_with_all_hubs_cluster_debug_report_query_response_command_t, debugReportId), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_sl_works_with_all_hubs_cluster_debug_report_query_response_command_t, debugReportData), \
}


/** @brief Structure for ZCL command "PowerProfileStateResponse" from "Power Profile" 
 */
typedef struct __zcl_power_profile_cluster_power_profile_state_response_command {
  uint8_t powerProfileCount;
  /* TYPE WARNING: PowerProfileRecord array defaults to */ uint8_t * powerProfileRecords;
} sl_zcl_power_profile_cluster_power_profile_state_response_command_t;

#define sl_zcl_power_profile_cluster_power_profile_state_response_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_power_profile_cluster_power_profile_state_response_command_t, powerProfileCount), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_power_profile_cluster_power_profile_state_response_command_t, powerProfileRecords), \
}


/** @brief Structure for ZCL command "PowerProfilesStateNotification" from "Power Profile" 
 */
typedef struct __zcl_power_profile_cluster_power_profiles_state_notification_command {
  uint8_t powerProfileCount;
  /* TYPE WARNING: PowerProfileRecord array defaults to */ uint8_t * powerProfileRecords;
} sl_zcl_power_profile_cluster_power_profiles_state_notification_command_t;

#define sl_zcl_power_profile_cluster_power_profiles_state_notification_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_power_profile_cluster_power_profiles_state_notification_command_t, powerProfileCount), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_power_profile_cluster_power_profiles_state_notification_command_t, powerProfileRecords), \
}


/** @brief Structure for ZCL command "RequestFastPollModeResponse" from "Simple Metering" 
 */
typedef struct __zcl_simple_metering_cluster_request_fast_poll_mode_response_command {
  uint8_t appliedUpdatePeriod;
  uint32_t fastPollModeEndtime;
} sl_zcl_simple_metering_cluster_request_fast_poll_mode_response_command_t;

#define sl_zcl_simple_metering_cluster_request_fast_poll_mode_response_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_simple_metering_cluster_request_fast_poll_mode_response_command_t, appliedUpdatePeriod), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_simple_metering_cluster_request_fast_poll_mode_response_command_t, fastPollModeEndtime), \
}


/** @brief Structure for ZCL command "GpTranslationTableRequest" from "Green Power" 
 */
typedef struct __zcl_green_power_cluster_gp_translation_table_request_command {
  uint8_t startIndex;
} sl_zcl_green_power_cluster_gp_translation_table_request_command_t;

#define sl_zcl_green_power_cluster_gp_translation_table_request_command_signature  { \
  1, \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_green_power_cluster_gp_translation_table_request_command_t, startIndex), \
}


/** @brief Structure for ZCL command "PushInformation" from "Information" 
 */
typedef struct __zcl_information_cluster_push_information_command {
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * contents;
} sl_zcl_information_cluster_push_information_command_t;

#define sl_zcl_information_cluster_push_information_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_information_cluster_push_information_command_t, contents), \
}


/** @brief Structure for ZCL command "SendPreferenceResponse" from "Information" 
 */
typedef struct __zcl_information_cluster_send_preference_response_command {
  /* TYPE WARNING: Status array defaults to */ uint8_t * statusFeedbackList;
} sl_zcl_information_cluster_send_preference_response_command_t;

#define sl_zcl_information_cluster_send_preference_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_information_cluster_send_preference_response_command_t, statusFeedbackList), \
}


/** @brief Structure for ZCL command "RequestPreferenceConfirmation" from "Information" 
 */
typedef struct __zcl_information_cluster_request_preference_confirmation_command {
  /* TYPE WARNING: Status array defaults to */ uint8_t * statusFeedbackList;
} sl_zcl_information_cluster_request_preference_confirmation_command_t;

#define sl_zcl_information_cluster_request_preference_confirmation_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_information_cluster_request_preference_confirmation_command_t, statusFeedbackList), \
}


/** @brief Structure for ZCL command "VoiceTransmission" from "Voice over ZigBee" 
 */
typedef struct __zcl_voice_over_zig_bee_cluster_voice_transmission_command {
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * voiceData;
} sl_zcl_voice_over_zig_bee_cluster_voice_transmission_command_t;

#define sl_zcl_voice_over_zig_bee_cluster_voice_transmission_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_voice_over_zig_bee_cluster_voice_transmission_command_t, voiceData), \
}


/** @brief Structure for ZCL command "MatchProtocolAddress" from "Generic Tunnel" 
 */
typedef struct __zcl_generic_tunnel_cluster_match_protocol_address_command {
  uint8_t * protocolAddress;
} sl_zcl_generic_tunnel_cluster_match_protocol_address_command_t;

#define sl_zcl_generic_tunnel_cluster_match_protocol_address_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_generic_tunnel_cluster_match_protocol_address_command_t, protocolAddress), \
}


/** @brief Structure for ZCL command "AdvertiseProtocolAddress" from "Generic Tunnel" 
 */
typedef struct __zcl_generic_tunnel_cluster_advertise_protocol_address_command {
  uint8_t * protocolAddress;
} sl_zcl_generic_tunnel_cluster_advertise_protocol_address_command_t;

#define sl_zcl_generic_tunnel_cluster_advertise_protocol_address_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_generic_tunnel_cluster_advertise_protocol_address_command_t, protocolAddress), \
}


/** @brief Structure for ZCL command "TransferAPDU" from "11073 Protocol Tunnel" 
 */
typedef struct __zcl_11073_protocol_tunnel_cluster_transfer_apdu_command {
  uint8_t * apdu;
} sl_zcl_11073_protocol_tunnel_cluster_transfer_apdu_command_t;

#define sl_zcl_11073_protocol_tunnel_cluster_transfer_apdu_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_11073_protocol_tunnel_cluster_transfer_apdu_command_t, apdu), \
}


/** @brief Structure for ZCL command "TransferApduFromClient" from "ISO 7816 Protocol Tunnel" 
 */
typedef struct __zcl_iso_7816_protocol_tunnel_cluster_transfer_apdu_from_client_command {
  uint8_t * apdu;
} sl_zcl_iso_7816_protocol_tunnel_cluster_transfer_apdu_from_client_command_t;

#define sl_zcl_iso_7816_protocol_tunnel_cluster_transfer_apdu_from_client_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_iso_7816_protocol_tunnel_cluster_transfer_apdu_from_client_command_t, apdu), \
}


/** @brief Structure for ZCL command "TransferApduFromServer" from "ISO 7816 Protocol Tunnel" 
 */
typedef struct __zcl_iso_7816_protocol_tunnel_cluster_transfer_apdu_from_server_command {
  uint8_t * apdu;
} sl_zcl_iso_7816_protocol_tunnel_cluster_transfer_apdu_from_server_command_t;

#define sl_zcl_iso_7816_protocol_tunnel_cluster_transfer_apdu_from_server_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_iso_7816_protocol_tunnel_cluster_transfer_apdu_from_server_command_t, apdu), \
}


/** @brief Structure for ZCL command "UnlockTokens" from "Configuration Cluster" 
 */
typedef struct __zcl_configuration_cluster_cluster_unlock_tokens_command {
  uint8_t * data;
} sl_zcl_configuration_cluster_cluster_unlock_tokens_command_t;

#define sl_zcl_configuration_cluster_cluster_unlock_tokens_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_configuration_cluster_cluster_unlock_tokens_command_t, data), \
}


/** @brief Structure for ZCL command "TransactionEnd" from "Payment" 
 */
typedef struct __zcl_payment_cluster_transaction_end_command {
  uint8_t * serialNumber;
  uint8_t status;
} sl_zcl_payment_cluster_transaction_end_command_t;

#define sl_zcl_payment_cluster_transaction_end_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_payment_cluster_transaction_end_command_t, serialNumber), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_payment_cluster_transaction_end_command_t, status), \
}


/** @brief Structure for ZCL command "BillStatusNotification" from "Billing" 
 */
typedef struct __zcl_billing_cluster_bill_status_notification_command {
  uint8_t * userId;
  uint8_t status;
} sl_zcl_billing_cluster_bill_status_notification_command_t;

#define sl_zcl_billing_cluster_bill_status_notification_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_billing_cluster_bill_status_notification_command_t, userId), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_billing_cluster_bill_status_notification_command_t, status), \
}


/** @brief Structure for ZCL command "PaymentConfirm" from "Payment" 
 */
typedef struct __zcl_payment_cluster_payment_confirm_command {
  uint8_t * serialNumber;
  uint16_t transId;
  uint8_t transStatus;
} sl_zcl_payment_cluster_payment_confirm_command_t;

#define sl_zcl_payment_cluster_payment_confirm_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_payment_cluster_payment_confirm_command_t, serialNumber), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_payment_cluster_payment_confirm_command_t, transId), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_payment_cluster_payment_confirm_command_t, transStatus), \
}


/** @brief Structure for ZCL command "Subscribe" from "Billing" 
 */
typedef struct __zcl_billing_cluster_subscribe_command {
  uint8_t * userId;
  uint16_t serviceId;
  uint16_t serviceProviderId;
} sl_zcl_billing_cluster_subscribe_command_t;

#define sl_zcl_billing_cluster_subscribe_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_billing_cluster_subscribe_command_t, userId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_billing_cluster_subscribe_command_t, serviceId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_billing_cluster_subscribe_command_t, serviceProviderId), \
}


/** @brief Structure for ZCL command "Unsubscribe" from "Billing" 
 */
typedef struct __zcl_billing_cluster_unsubscribe_command {
  uint8_t * userId;
  uint16_t serviceId;
  uint16_t serviceProviderId;
} sl_zcl_billing_cluster_unsubscribe_command_t;

#define sl_zcl_billing_cluster_unsubscribe_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_billing_cluster_unsubscribe_command_t, userId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_billing_cluster_unsubscribe_command_t, serviceId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_billing_cluster_unsubscribe_command_t, serviceProviderId), \
}


/** @brief Structure for ZCL command "StartBillingSession" from "Billing" 
 */
typedef struct __zcl_billing_cluster_start_billing_session_command {
  uint8_t * userId;
  uint16_t serviceId;
  uint16_t serviceProviderId;
} sl_zcl_billing_cluster_start_billing_session_command_t;

#define sl_zcl_billing_cluster_start_billing_session_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_billing_cluster_start_billing_session_command_t, userId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_billing_cluster_start_billing_session_command_t, serviceId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_billing_cluster_start_billing_session_command_t, serviceProviderId), \
}


/** @brief Structure for ZCL command "StopBillingSession" from "Billing" 
 */
typedef struct __zcl_billing_cluster_stop_billing_session_command {
  uint8_t * userId;
  uint16_t serviceId;
  uint16_t serviceProviderId;
} sl_zcl_billing_cluster_stop_billing_session_command_t;

#define sl_zcl_billing_cluster_stop_billing_session_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_billing_cluster_stop_billing_session_command_t, userId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_billing_cluster_stop_billing_session_command_t, serviceId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_billing_cluster_stop_billing_session_command_t, serviceProviderId), \
}


/** @brief Structure for ZCL command "SessionKeepAlive" from "Billing" 
 */
typedef struct __zcl_billing_cluster_session_keep_alive_command {
  uint8_t * userId;
  uint16_t serviceId;
  uint16_t serviceProviderId;
} sl_zcl_billing_cluster_session_keep_alive_command_t;

#define sl_zcl_billing_cluster_session_keep_alive_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_billing_cluster_session_keep_alive_command_t, userId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_billing_cluster_session_keep_alive_command_t, serviceId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_billing_cluster_session_keep_alive_command_t, serviceProviderId), \
}


/** @brief Structure for ZCL command "CheckBillStatus" from "Billing" 
 */
typedef struct __zcl_billing_cluster_check_bill_status_command {
  uint8_t * userId;
  uint16_t serviceId;
  uint16_t serviceProviderId;
} sl_zcl_billing_cluster_check_bill_status_command_t;

#define sl_zcl_billing_cluster_check_bill_status_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_billing_cluster_check_bill_status_command_t, userId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_billing_cluster_check_bill_status_command_t, serviceId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_billing_cluster_check_bill_status_command_t, serviceProviderId), \
}


/** @brief Structure for ZCL command "BuyRequest" from "Payment" 
 */
typedef struct __zcl_payment_cluster_buy_request_command {
  uint8_t * userId;
  uint16_t userType;
  uint16_t serviceId;
  uint8_t * goodId;
} sl_zcl_payment_cluster_buy_request_command_t;

#define sl_zcl_payment_cluster_buy_request_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_payment_cluster_buy_request_command_t, userId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_payment_cluster_buy_request_command_t, userType), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_payment_cluster_buy_request_command_t, serviceId), \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_payment_cluster_buy_request_command_t, goodId), \
}


/** @brief Structure for ZCL command "AcceptPayment" from "Payment" 
 */
typedef struct __zcl_payment_cluster_accept_payment_command {
  uint8_t * userId;
  uint16_t userType;
  uint16_t serviceId;
  uint8_t * goodId;
} sl_zcl_payment_cluster_accept_payment_command_t;

#define sl_zcl_payment_cluster_accept_payment_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_payment_cluster_accept_payment_command_t, userId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_payment_cluster_accept_payment_command_t, userType), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_payment_cluster_accept_payment_command_t, serviceId), \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_payment_cluster_accept_payment_command_t, goodId), \
}


/** @brief Structure for ZCL command "SendBillRecord" from "Billing" 
 */
typedef struct __zcl_billing_cluster_send_bill_record_command {
  uint8_t * userId;
  uint16_t serviceId;
  uint16_t serviceProviderId;
  uint8_t * timestamp;
  uint16_t duration;
} sl_zcl_billing_cluster_send_bill_record_command_t;

#define sl_zcl_billing_cluster_send_bill_record_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_billing_cluster_send_bill_record_command_t, userId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_billing_cluster_send_bill_record_command_t, serviceId), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_billing_cluster_send_bill_record_command_t, serviceProviderId), \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_billing_cluster_send_bill_record_command_t, timestamp), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_billing_cluster_send_bill_record_command_t, duration), \
}


/** @brief Structure for ZCL command "ReceiptDelivery" from "Payment" 
 */
typedef struct __zcl_payment_cluster_receipt_delivery_command {
  uint8_t * serialNumber;
  uint32_t currency;
  uint8_t priceTrailingDigit;
  uint32_t price;
  uint8_t * timestamp;
} sl_zcl_payment_cluster_receipt_delivery_command_t;

#define sl_zcl_payment_cluster_receipt_delivery_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_payment_cluster_receipt_delivery_command_t, serialNumber), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_payment_cluster_receipt_delivery_command_t, currency), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_payment_cluster_receipt_delivery_command_t, priceTrailingDigit), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_payment_cluster_receipt_delivery_command_t, price), \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_payment_cluster_receipt_delivery_command_t, timestamp), \
}


/** @brief Structure for ZCL command "BuyConfirm" from "Payment" 
 */
typedef struct __zcl_payment_cluster_buy_confirm_command {
  uint8_t * serialNumber;
  uint32_t currency;
  uint8_t priceTrailingDigit;
  uint32_t price;
  uint8_t * timestamp;
  uint16_t transId;
  uint8_t transStatus;
} sl_zcl_payment_cluster_buy_confirm_command_t;

#define sl_zcl_payment_cluster_buy_confirm_command_signature  { \
  7, \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_payment_cluster_buy_confirm_command_t, serialNumber), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_payment_cluster_buy_confirm_command_t, currency), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_payment_cluster_buy_confirm_command_t, priceTrailingDigit), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_payment_cluster_buy_confirm_command_t, price), \
  (ZAP_SIGNATURE_TYPE_OCTET_STRING), offsetof(sl_zcl_payment_cluster_buy_confirm_command_t, timestamp), \
  (ZAP_SIGNATURE_TYPE_INT16U), offsetof(sl_zcl_payment_cluster_buy_confirm_command_t, transId), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_payment_cluster_buy_confirm_command_t, transStatus), \
}


/** @brief Structure for ZCL command "EphemeralDataRequest" from "Key Establishment" 
 */
typedef struct __zcl_key_establishment_cluster_ephemeral_data_request_command {
  /* TYPE WARNING: EphemeralData defaults to */ uint8_t * ephemeralData;
} sl_zcl_key_establishment_cluster_ephemeral_data_request_command_t;

#define sl_zcl_key_establishment_cluster_ephemeral_data_request_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_POINTER), offsetof(sl_zcl_key_establishment_cluster_ephemeral_data_request_command_t, ephemeralData), \
}


/** @brief Structure for ZCL command "ConfirmKeyDataRequest" from "Key Establishment" 
 */
typedef struct __zcl_key_establishment_cluster_confirm_key_data_request_command {
  /* TYPE WARNING: Smac defaults to */ uint8_t * secureMessageAuthenticationCode;
} sl_zcl_key_establishment_cluster_confirm_key_data_request_command_t;

#define sl_zcl_key_establishment_cluster_confirm_key_data_request_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_POINTER), offsetof(sl_zcl_key_establishment_cluster_confirm_key_data_request_command_t, secureMessageAuthenticationCode), \
}


/** @brief Structure for ZCL command "EphemeralDataResponse" from "Key Establishment" 
 */
typedef struct __zcl_key_establishment_cluster_ephemeral_data_response_command {
  /* TYPE WARNING: EphemeralData defaults to */ uint8_t * ephemeralData;
} sl_zcl_key_establishment_cluster_ephemeral_data_response_command_t;

#define sl_zcl_key_establishment_cluster_ephemeral_data_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_POINTER), offsetof(sl_zcl_key_establishment_cluster_ephemeral_data_response_command_t, ephemeralData), \
}


/** @brief Structure for ZCL command "ConfirmKeyDataResponse" from "Key Establishment" 
 */
typedef struct __zcl_key_establishment_cluster_confirm_key_data_response_command {
  /* TYPE WARNING: Smac defaults to */ uint8_t * secureMessageAuthenticationCode;
} sl_zcl_key_establishment_cluster_confirm_key_data_response_command_t;

#define sl_zcl_key_establishment_cluster_confirm_key_data_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_POINTER), offsetof(sl_zcl_key_establishment_cluster_confirm_key_data_response_command_t, secureMessageAuthenticationCode), \
}


/** @brief Structure for ZCL command "PushInformationResponse" from "Information" 
 */
typedef struct __zcl_information_cluster_push_information_response_command {
  /* TYPE WARNING: Notification array defaults to */ uint8_t * notificationList;
} sl_zcl_information_cluster_push_information_response_command_t;

#define sl_zcl_information_cluster_push_information_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_information_cluster_push_information_response_command_t, notificationList), \
}


/** @brief Structure for ZCL command "UpdateResponse" from "Information" 
 */
typedef struct __zcl_information_cluster_update_response_command {
  /* TYPE WARNING: Notification array defaults to */ uint8_t * notificationList;
} sl_zcl_information_cluster_update_response_command_t;

#define sl_zcl_information_cluster_update_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_information_cluster_update_response_command_t, notificationList), \
}


/** @brief Structure for ZCL command "DeleteResponse" from "Information" 
 */
typedef struct __zcl_information_cluster_delete_response_command {
  /* TYPE WARNING: Notification array defaults to */ uint8_t * notificationList;
} sl_zcl_information_cluster_delete_response_command_t;

#define sl_zcl_information_cluster_delete_response_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_information_cluster_delete_response_command_t, notificationList), \
}


/** @brief Structure for ZCL command "LogNotification" from "Appliance Statistics" 
 */
typedef struct __zcl_appliance_statistics_cluster_log_notification_command {
  uint32_t timeStamp;
  uint32_t logId;
  uint32_t logLength;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * logPayload;
} sl_zcl_appliance_statistics_cluster_log_notification_command_t;

#define sl_zcl_appliance_statistics_cluster_log_notification_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_TIME_OF_DAY), offsetof(sl_zcl_appliance_statistics_cluster_log_notification_command_t, timeStamp), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_appliance_statistics_cluster_log_notification_command_t, logId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_appliance_statistics_cluster_log_notification_command_t, logLength), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_appliance_statistics_cluster_log_notification_command_t, logPayload), \
}


/** @brief Structure for ZCL command "LogResponse" from "Appliance Statistics" 
 */
typedef struct __zcl_appliance_statistics_cluster_log_response_command {
  uint32_t timeStamp;
  uint32_t logId;
  uint32_t logLength;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * logPayload;
} sl_zcl_appliance_statistics_cluster_log_response_command_t;

#define sl_zcl_appliance_statistics_cluster_log_response_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_TIME_OF_DAY), offsetof(sl_zcl_appliance_statistics_cluster_log_response_command_t, timeStamp), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_appliance_statistics_cluster_log_response_command_t, logId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_appliance_statistics_cluster_log_response_command_t, logLength), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_appliance_statistics_cluster_log_response_command_t, logPayload), \
}


/** @brief Structure for ZCL command "GetMessageCancellation" from "Messaging" 
 */
typedef struct __zcl_messaging_cluster_get_message_cancellation_command {
  uint32_t earliestImplementationTime;
} sl_zcl_messaging_cluster_get_message_cancellation_command_t;

#define sl_zcl_messaging_cluster_get_message_cancellation_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_messaging_cluster_get_message_cancellation_command_t, earliestImplementationTime), \
}


/** @brief Structure for ZCL command "CancelAllMessages" from "Messaging" 
 */
typedef struct __zcl_messaging_cluster_cancel_all_messages_command {
  uint32_t implementationDateTime;
} sl_zcl_messaging_cluster_cancel_all_messages_command_t;

#define sl_zcl_messaging_cluster_cancel_all_messages_command_signature  { \
  1, \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_messaging_cluster_cancel_all_messages_command_t, implementationDateTime), \
}


/** @brief Structure for ZCL command "SelectAvailableEmergencyCredit" from "Prepayment" 
 */
typedef struct __zcl_prepayment_cluster_select_available_emergency_credit_command {
  uint32_t commandIssueDateTime;
  uint8_t originatingDevice;
} sl_zcl_prepayment_cluster_select_available_emergency_credit_command_t;

#define sl_zcl_prepayment_cluster_select_available_emergency_credit_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_prepayment_cluster_select_available_emergency_credit_command_t, commandIssueDateTime), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_prepayment_cluster_select_available_emergency_credit_command_t, originatingDevice), \
}


/** @brief Structure for ZCL command "GetProfileResponse" from "Simple Metering" 
 */
typedef struct __zcl_simple_metering_cluster_get_profile_response_command {
  uint32_t endTime;
  uint8_t status;
  uint8_t profileIntervalPeriod;
  uint8_t numberOfPeriodsDelivered;
  /* TYPE WARNING: INT24U array defaults to */ uint8_t * intervals;
} sl_zcl_simple_metering_cluster_get_profile_response_command_t;

#define sl_zcl_simple_metering_cluster_get_profile_response_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_simple_metering_cluster_get_profile_response_command_t, endTime), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_simple_metering_cluster_get_profile_response_command_t, status), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_simple_metering_cluster_get_profile_response_command_t, profileIntervalPeriod), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_simple_metering_cluster_get_profile_response_command_t, numberOfPeriodsDelivered), \
  (ZAP_SIGNATURE_TYPE_ARRAY), offsetof(sl_zcl_simple_metering_cluster_get_profile_response_command_t, intervals), \
}


/** @brief Structure for ZCL command "GetConversionFactor" from "Price" 
 */
typedef struct __zcl_price_cluster_get_conversion_factor_command {
  uint32_t earliestStartTime;
  uint32_t minIssuerEventId;
  uint8_t numberOfCommands;
} sl_zcl_price_cluster_get_conversion_factor_command_t;

#define sl_zcl_price_cluster_get_conversion_factor_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_price_cluster_get_conversion_factor_command_t, earliestStartTime), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_get_conversion_factor_command_t, minIssuerEventId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_price_cluster_get_conversion_factor_command_t, numberOfCommands), \
}


/** @brief Structure for ZCL command "GetCalorificValue" from "Price" 
 */
typedef struct __zcl_price_cluster_get_calorific_value_command {
  uint32_t earliestStartTime;
  uint32_t minIssuerEventId;
  uint8_t numberOfCommands;
} sl_zcl_price_cluster_get_calorific_value_command_t;

#define sl_zcl_price_cluster_get_calorific_value_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_price_cluster_get_calorific_value_command_t, earliestStartTime), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_get_calorific_value_command_t, minIssuerEventId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_price_cluster_get_calorific_value_command_t, numberOfCommands), \
}


/** @brief Structure for ZCL command "GetTariffInformation" from "Price" 
 */
typedef struct __zcl_price_cluster_get_tariff_information_command {
  uint32_t earliestStartTime;
  uint32_t minIssuerEventId;
  uint8_t numberOfCommands;
  uint8_t tariffType;
} sl_zcl_price_cluster_get_tariff_information_command_t;

#define sl_zcl_price_cluster_get_tariff_information_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_price_cluster_get_tariff_information_command_t, earliestStartTime), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_get_tariff_information_command_t, minIssuerEventId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_price_cluster_get_tariff_information_command_t, numberOfCommands), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_price_cluster_get_tariff_information_command_t, tariffType), \
}


/** @brief Structure for ZCL command "GetCO2Value" from "Price" 
 */
typedef struct __zcl_price_cluster_get_co2_value_command {
  uint32_t earliestStartTime;
  uint32_t minIssuerEventId;
  uint8_t numberOfCommands;
  uint8_t tariffType;
} sl_zcl_price_cluster_get_co2_value_command_t;

#define sl_zcl_price_cluster_get_co2_value_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_price_cluster_get_co2_value_command_t, earliestStartTime), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_get_co2_value_command_t, minIssuerEventId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_price_cluster_get_co2_value_command_t, numberOfCommands), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_price_cluster_get_co2_value_command_t, tariffType), \
}


/** @brief Structure for ZCL command "GetBillingPeriod" from "Price" 
 */
typedef struct __zcl_price_cluster_get_billing_period_command {
  uint32_t earliestStartTime;
  uint32_t minIssuerEventId;
  uint8_t numberOfCommands;
  uint8_t tariffType;
} sl_zcl_price_cluster_get_billing_period_command_t;

#define sl_zcl_price_cluster_get_billing_period_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_price_cluster_get_billing_period_command_t, earliestStartTime), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_get_billing_period_command_t, minIssuerEventId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_price_cluster_get_billing_period_command_t, numberOfCommands), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_price_cluster_get_billing_period_command_t, tariffType), \
}


/** @brief Structure for ZCL command "GetConsolidatedBill" from "Price" 
 */
typedef struct __zcl_price_cluster_get_consolidated_bill_command {
  uint32_t earliestStartTime;
  uint32_t minIssuerEventId;
  uint8_t numberOfCommands;
  uint8_t tariffType;
} sl_zcl_price_cluster_get_consolidated_bill_command_t;

#define sl_zcl_price_cluster_get_consolidated_bill_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_price_cluster_get_consolidated_bill_command_t, earliestStartTime), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_price_cluster_get_consolidated_bill_command_t, minIssuerEventId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_price_cluster_get_consolidated_bill_command_t, numberOfCommands), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_price_cluster_get_consolidated_bill_command_t, tariffType), \
}


/** @brief Structure for ZCL command "GetCalendar" from "Calendar" 
 */
typedef struct __zcl_calendar_cluster_get_calendar_command {
  uint32_t earliestStartTime;
  uint32_t minIssuerEventId;
  uint8_t numberOfCalendars;
  uint8_t calendarType;
  uint32_t providerId;
} sl_zcl_calendar_cluster_get_calendar_command_t;

#define sl_zcl_calendar_cluster_get_calendar_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_calendar_cluster_get_calendar_command_t, earliestStartTime), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_calendar_cluster_get_calendar_command_t, minIssuerEventId), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_calendar_cluster_get_calendar_command_t, numberOfCalendars), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_calendar_cluster_get_calendar_command_t, calendarType), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_calendar_cluster_get_calendar_command_t, providerId), \
}


/** @brief Structure for ZCL command "GetScheduledPrices" from "Price" 
 */
typedef struct __zcl_price_cluster_get_scheduled_prices_command {
  uint32_t startTime;
  uint8_t numberOfEvents;
} sl_zcl_price_cluster_get_scheduled_prices_command_t;

#define sl_zcl_price_cluster_get_scheduled_prices_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_price_cluster_get_scheduled_prices_command_t, startTime), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_price_cluster_get_scheduled_prices_command_t, numberOfEvents), \
}


/** @brief Structure for ZCL command "GetCreditPayment" from "Price" 
 */
typedef struct __zcl_price_cluster_get_credit_payment_command {
  uint32_t latestEndTime;
  uint8_t numberOfRecords;
} sl_zcl_price_cluster_get_credit_payment_command_t;

#define sl_zcl_price_cluster_get_credit_payment_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_price_cluster_get_credit_payment_command_t, latestEndTime), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_price_cluster_get_credit_payment_command_t, numberOfRecords), \
}


/** @brief Structure for ZCL command "GetTopUpLog" from "Prepayment" 
 */
typedef struct __zcl_prepayment_cluster_get_top_up_log_command {
  uint32_t latestEndTime;
  uint8_t numberOfRecords;
} sl_zcl_prepayment_cluster_get_top_up_log_command_t;

#define sl_zcl_prepayment_cluster_get_top_up_log_command_signature  { \
  2, \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_prepayment_cluster_get_top_up_log_command_t, latestEndTime), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_prepayment_cluster_get_top_up_log_command_t, numberOfRecords), \
}


/** @brief Structure for ZCL command "GetBlockPeriods" from "Price" 
 */
typedef struct __zcl_price_cluster_get_block_periods_command {
  uint32_t startTime;
  uint8_t numberOfEvents;
  uint8_t tariffType;
} sl_zcl_price_cluster_get_block_periods_command_t;

#define sl_zcl_price_cluster_get_block_periods_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_price_cluster_get_block_periods_command_t, startTime), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_price_cluster_get_block_periods_command_t, numberOfEvents), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_price_cluster_get_block_periods_command_t, tariffType), \
}


/** @brief Structure for ZCL command "GetDebtRepaymentLog" from "Prepayment" 
 */
typedef struct __zcl_prepayment_cluster_get_debt_repayment_log_command {
  uint32_t latestEndTime;
  uint8_t numberOfDebts;
  uint8_t debtType;
} sl_zcl_prepayment_cluster_get_debt_repayment_log_command_t;

#define sl_zcl_prepayment_cluster_get_debt_repayment_log_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_prepayment_cluster_get_debt_repayment_log_command_t, latestEndTime), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_prepayment_cluster_get_debt_repayment_log_command_t, numberOfDebts), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_prepayment_cluster_get_debt_repayment_log_command_t, debtType), \
}


/** @brief Structure for ZCL command "GetSpecialDays" from "Calendar" 
 */
typedef struct __zcl_calendar_cluster_get_special_days_command {
  uint32_t startTime;
  uint8_t numberOfEvents;
  uint8_t calendarType;
  uint32_t providerId;
  uint32_t issuerCalendarId;
} sl_zcl_calendar_cluster_get_special_days_command_t;

#define sl_zcl_calendar_cluster_get_special_days_command_signature  { \
  5, \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_calendar_cluster_get_special_days_command_t, startTime), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_calendar_cluster_get_special_days_command_t, numberOfEvents), \
  (ZAP_SIGNATURE_TYPE_ENUM8), offsetof(sl_zcl_calendar_cluster_get_special_days_command_t, calendarType), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_calendar_cluster_get_special_days_command_t, providerId), \
  (ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_calendar_cluster_get_special_days_command_t, issuerCalendarId), \
}


/** @brief Structure for ZCL command "GetScheduledEvents" from "Demand Response and Load Control" 
 */
typedef struct __zcl_demand_response_and_load_control_cluster_get_scheduled_events_command {
  uint32_t startTime;
  uint8_t numberOfEvents;
  uint32_t issuerEventId;
} sl_zcl_demand_response_and_load_control_cluster_get_scheduled_events_command_t;

#define sl_zcl_demand_response_and_load_control_cluster_get_scheduled_events_command_signature  { \
  3, \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_demand_response_and_load_control_cluster_get_scheduled_events_command_t, startTime), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_demand_response_and_load_control_cluster_get_scheduled_events_command_t, numberOfEvents), \
  (SL_SIGNATURE_FIELD_MASK_OPTIONAL_FIELD|ZAP_SIGNATURE_TYPE_INT32U), offsetof(sl_zcl_demand_response_and_load_control_cluster_get_scheduled_events_command_t, issuerEventId), \
}


/** @brief Structure for ZCL command "GetSnapshot" from "Simple Metering" 
 */
typedef struct __zcl_simple_metering_cluster_get_snapshot_command {
  uint32_t earliestStartTime;
  uint32_t latestEndTime;
  uint8_t snapshotOffset;
  uint32_t snapshotCause;
} sl_zcl_simple_metering_cluster_get_snapshot_command_t;

#define sl_zcl_simple_metering_cluster_get_snapshot_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_simple_metering_cluster_get_snapshot_command_t, earliestStartTime), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_simple_metering_cluster_get_snapshot_command_t, latestEndTime), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_simple_metering_cluster_get_snapshot_command_t, snapshotOffset), \
  (ZAP_SIGNATURE_TYPE_BITMAP32), offsetof(sl_zcl_simple_metering_cluster_get_snapshot_command_t, snapshotCause), \
}


/** @brief Structure for ZCL command "GetPrepaySnapshot" from "Prepayment" 
 */
typedef struct __zcl_prepayment_cluster_get_prepay_snapshot_command {
  uint32_t earliestStartTime;
  uint32_t latestEndTime;
  uint8_t snapshotOffset;
  uint32_t snapshotCause;
} sl_zcl_prepayment_cluster_get_prepay_snapshot_command_t;

#define sl_zcl_prepayment_cluster_get_prepay_snapshot_command_signature  { \
  4, \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_prepayment_cluster_get_prepay_snapshot_command_t, earliestStartTime), \
  (ZAP_SIGNATURE_TYPE_UTC_TIME), offsetof(sl_zcl_prepayment_cluster_get_prepay_snapshot_command_t, latestEndTime), \
  (ZAP_SIGNATURE_TYPE_INT8U), offsetof(sl_zcl_prepayment_cluster_get_prepay_snapshot_command_t, snapshotOffset), \
  (ZAP_SIGNATURE_TYPE_BITMAP32), offsetof(sl_zcl_prepayment_cluster_get_prepay_snapshot_command_t, snapshotCause), \
}


/** @brief Structure for ZCL command "GpPairingSearch" from "Green Power" 
 */
typedef struct __zcl_green_power_cluster_gp_pairing_search_command {
  uint16_t options;
  uint32_t gpdSrcId;
  uint8_t * gpdIeee;
  uint8_t endpoint;
} sl_zcl_green_power_cluster_gp_pairing_search_command_t;


/** @brief Structure for ZCL command "GpNotification" from "Green Power" 
 */
typedef struct __zcl_green_power_cluster_gp_notification_command {
  uint16_t options;
  uint32_t gpdSrcId;
  uint8_t * gpdIeee;
  uint8_t gpdEndpoint;
  uint32_t gpdSecurityFrameCounter;
  uint8_t gpdCommandId;
  uint8_t * gpdCommandPayload;
  uint16_t gppShortAddress;
  uint8_t gppDistance;
} sl_zcl_green_power_cluster_gp_notification_command_t;


/** @brief Structure for ZCL command "GpCommissioningNotification" from "Green Power" 
 */
typedef struct __zcl_green_power_cluster_gp_commissioning_notification_command {
  uint16_t options;
  uint32_t gpdSrcId;
  uint8_t * gpdIeee;
  uint8_t endpoint;
  uint32_t gpdSecurityFrameCounter;
  uint8_t gpdCommandId;
  uint8_t * gpdCommandPayload;
  uint16_t gppShortAddress;
  uint8_t gppLink;
  uint32_t mic;
} sl_zcl_green_power_cluster_gp_commissioning_notification_command_t;


/** @brief Structure for ZCL command "GpTranslationTableUpdate" from "Green Power" 
 */
typedef struct __zcl_green_power_cluster_gp_translation_table_update_command {
  uint16_t options;
  uint32_t gpdSrcId;
  uint8_t * gpdIeee;
  uint8_t endpoint;
  /* TYPE WARNING: GpTranslationTableUpdateTranslation array defaults to */ uint8_t * translations;
} sl_zcl_green_power_cluster_gp_translation_table_update_command_t;


/** @brief Structure for ZCL command "GpPairing" from "Green Power" 
 */
typedef struct __zcl_green_power_cluster_gp_pairing_command {
  uint32_t options;
  uint32_t gpdSrcId;
  uint8_t * gpdIeee;
  uint8_t endpoint;
  uint8_t * sinkIeeeAddress;
  uint16_t sinkNwkAddress;
  uint16_t sinkGroupId;
  uint8_t deviceId;
  uint32_t gpdSecurityFrameCounter;
  /* TYPE WARNING: security_key defaults to */ uint8_t *  gpdKey;
  uint16_t assignedAlias;
  uint8_t groupcastRadius;
} sl_zcl_green_power_cluster_gp_pairing_command_t;


/** @brief Structure for ZCL command "GpProxyTableRequest" from "Green Power" 
 */
typedef struct __zcl_green_power_cluster_gp_proxy_table_request_command {
  uint8_t options;
  uint32_t gpdSrcId;
  uint8_t * gpdIeee;
  uint8_t endpoint;
  uint8_t index;
} sl_zcl_green_power_cluster_gp_proxy_table_request_command_t;


/** @brief Structure for ZCL command "GpPairingConfiguration" from "Green Power" 
 */
typedef struct __zcl_green_power_cluster_gp_pairing_configuration_command {
  uint8_t actions;
  uint16_t options;
  uint32_t gpdSrcId;
  uint8_t * gpdIeee;
  uint8_t endpoint;
  uint8_t deviceId;
  uint8_t groupListCount;
  /* TYPE WARNING: GpPairingConfigurationGroupList array defaults to */ uint8_t * groupList;
  uint16_t gpdAssignedAlias;
  uint8_t groupcastRadius;
  uint8_t securityOptions;
  uint32_t gpdSecurityFrameCounter;
  /* TYPE WARNING: security_key defaults to */ uint8_t *  gpdSecurityKey;
  uint8_t numberOfPairedEndpoints;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * pairedEndpoints;
  uint8_t applicationInformation;
  uint16_t manufacturerId;
  uint16_t modeId;
  uint8_t numberOfGpdCommands;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * gpdCommandIdList;
  uint8_t clusterIdListCount;
  /* TYPE WARNING: INT16U array defaults to */ uint8_t * clusterListServer;
  /* TYPE WARNING: INT16U array defaults to */ uint8_t * clusterListClient;
  uint8_t switchInformationLength;
  uint8_t switchConfiguration;
  uint8_t currentContactStatus;
  uint8_t totalNumberOfReports;
  uint8_t numberOfReports;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * reportDescriptor;
} sl_zcl_green_power_cluster_gp_pairing_configuration_command_t;


/** @brief Structure for ZCL command "GpResponse" from "Green Power" 
 */
typedef struct __zcl_green_power_cluster_gp_response_command {
  uint8_t options;
  uint16_t tempMasterShortAddress;
  uint8_t tempMasterTxChannel;
  uint32_t gpdSrcId;
  uint8_t * gpdIeee;
  uint8_t endpoint;
  uint8_t gpdCommandId;
  uint8_t * gpdCommandPayload;
} sl_zcl_green_power_cluster_gp_response_command_t;


/** @brief Structure for ZCL command "GpProxyCommissioningMode" from "Green Power" 
 */
typedef struct __zcl_green_power_cluster_gp_proxy_commissioning_mode_command {
  uint8_t options;
  uint16_t commissioningWindow;
  uint8_t channel;
} sl_zcl_green_power_cluster_gp_proxy_commissioning_mode_command_t;


/** @brief Structure for ZCL command "GpNotificationResponse" from "Green Power" 
 */
typedef struct __zcl_green_power_cluster_gp_notification_response_command {
  uint8_t options;
  uint32_t gpdSrcId;
  uint8_t * gpdIeee;
  uint8_t endpoint;
  uint32_t gpdSecurityFrameCounter;
} sl_zcl_green_power_cluster_gp_notification_response_command_t;


/** @brief Structure for ZCL command "GpTunnelingStop" from "Green Power" 
 */
typedef struct __zcl_green_power_cluster_gp_tunneling_stop_command {
  uint8_t options;
  uint32_t gpdSrcId;
  uint8_t * gpdIeee;
  uint8_t endpoint;
  uint32_t gpdSecurityFrameCounter;
  uint16_t gppShortAddress;
  int8_t gppDistance;
} sl_zcl_green_power_cluster_gp_tunneling_stop_command_t;


/** @brief Structure for ZCL command "PoweringOffNotification" from "SL Works With All Hubs" 
 */
typedef struct __zcl_sl_works_with_all_hubs_cluster_powering_off_notification_command {
  uint8_t powerNotificationReason;
  uint16_t manufacturerId;
  uint8_t manufacturerReasonLength;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * manufacturerReason;
} sl_zcl_sl_works_with_all_hubs_cluster_powering_off_notification_command_t;


/** @brief Structure for ZCL command "PoweringOnNotification" from "SL Works With All Hubs" 
 */
typedef struct __zcl_sl_works_with_all_hubs_cluster_powering_on_notification_command {
  uint8_t powerNotificationReason;
  uint16_t manufacturerId;
  uint8_t manufacturerReasonLength;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * manufacturerReason;
} sl_zcl_sl_works_with_all_hubs_cluster_powering_on_notification_command_t;


/** @brief Structure for ZCL command "GpSinkTableRequest" from "Green Power" 
 */
typedef struct __zcl_green_power_cluster_gp_sink_table_request_command {
  uint8_t options;
  uint32_t gpdSrcId;
  uint8_t * gpdIeee;
  uint8_t endpoint;
  uint8_t index;
} sl_zcl_green_power_cluster_gp_sink_table_request_command_t;


/** @brief Structure for ZCL command "EnableApsLinkKeyAuthorization" from "SL Works With All Hubs" 
 */
typedef struct __zcl_sl_works_with_all_hubs_cluster_enable_aps_link_key_authorization_command {
  uint8_t numberExemptClusters;
  /* TYPE WARNING: CLUSTER_ID array defaults to */ uint8_t * clusterId;
} sl_zcl_sl_works_with_all_hubs_cluster_enable_aps_link_key_authorization_command_t;


/** @brief Structure for ZCL command "DisableApsLinkKeyAuthorization" from "SL Works With All Hubs" 
 */
typedef struct __zcl_sl_works_with_all_hubs_cluster_disable_aps_link_key_authorization_command {
  uint8_t numberExemptClusters;
  /* TYPE WARNING: CLUSTER_ID array defaults to */ uint8_t * clusterId;
} sl_zcl_sl_works_with_all_hubs_cluster_disable_aps_link_key_authorization_command_t;


/** @brief Structure for ZCL command "RequireApsAcksOnUnicasts" from "SL Works With All Hubs" 
 */
typedef struct __zcl_sl_works_with_all_hubs_cluster_require_aps_acks_on_unicasts_command {
  uint8_t numberExemptClusters;
  /* TYPE WARNING: CLUSTER_ID array defaults to */ uint8_t * clusterId;
} sl_zcl_sl_works_with_all_hubs_cluster_require_aps_acks_on_unicasts_command_t;


/** @brief Structure for ZCL command "UseTrustCenterForClusterServer" from "SL Works With All Hubs" 
 */
typedef struct __zcl_sl_works_with_all_hubs_cluster_use_trust_center_for_cluster_server_command {
  uint8_t numberOfClusters;
  /* TYPE WARNING: CLUSTER_ID array defaults to */ uint8_t * clusterId;
} sl_zcl_sl_works_with_all_hubs_cluster_use_trust_center_for_cluster_server_command_t;


/** @brief Structure for ZCL command "ApsAckEnablementQueryResponse" from "SL Works With All Hubs" 
 */
typedef struct __zcl_sl_works_with_all_hubs_cluster_aps_ack_enablement_query_response_command {
  uint8_t numberExemptClusters;
  /* TYPE WARNING: CLUSTER_ID array defaults to */ uint8_t * clusterId;
} sl_zcl_sl_works_with_all_hubs_cluster_aps_ack_enablement_query_response_command_t;


/** @brief Structure for ZCL command "TrustCenterForClusterServerQueryResponse" from "SL Works With All Hubs" 
 */
typedef struct __zcl_sl_works_with_all_hubs_cluster_trust_center_for_cluster_server_query_response_command {
  uint8_t numberOfClusters;
  /* TYPE WARNING: CLUSTER_ID array defaults to */ uint8_t * clusterId;
} sl_zcl_sl_works_with_all_hubs_cluster_trust_center_for_cluster_server_query_response_command_t;


/** @brief Structure for ZCL command "ViewSceneResponse" from "Scenes" 
 */
typedef struct __zcl_scenes_cluster_view_scene_response_command {
  uint8_t status;
  uint16_t groupId;
  uint8_t sceneId;
  uint16_t transitionTime;
  uint8_t * sceneName;
  /* TYPE WARNING: SceneExtensionFieldSet array defaults to */ uint8_t * extensionFieldSets;
} sl_zcl_scenes_cluster_view_scene_response_command_t;


/** @brief Structure for ZCL command "GetSceneMembershipResponse" from "Scenes" 
 */
typedef struct __zcl_scenes_cluster_get_scene_membership_response_command {
  uint8_t status;
  uint8_t capacity;
  uint16_t groupId;
  uint8_t sceneCount;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * sceneList;
} sl_zcl_scenes_cluster_get_scene_membership_response_command_t;


/** @brief Structure for ZCL command "UseTrustCenterForClusterServerResponse" from "SL Works With All Hubs" 
 */
typedef struct __zcl_sl_works_with_all_hubs_cluster_use_trust_center_for_cluster_server_response_command {
  uint8_t status;
  uint8_t clusterStatusLength;
  /* TYPE WARNING: WwahClusterStatusToUseTC array defaults to */ uint8_t * clusterStatus;
} sl_zcl_sl_works_with_all_hubs_cluster_use_trust_center_for_cluster_server_response_command_t;


/** @brief Structure for ZCL command "Bypass" from "IAS ACE" 
 */
typedef struct __zcl_ias_ace_cluster_bypass_command {
  uint8_t numberOfZones;
  /* TYPE WARNING: INT8U array defaults to */ uint8_t * zoneIds;
  uint8_t * armDisarmCode;
} sl_zcl_ias_ace_cluster_bypass_command_t;


/** @brief Structure for ZCL command "SurveyBeaconsResponse" from "SL Works With All Hubs" 
 */
typedef struct __zcl_sl_works_with_all_hubs_cluster_survey_beacons_response_command {
  uint8_t numberOfBeacons;
  /* TYPE WARNING: WwahBeaconSurvey array defaults to */ uint8_t * beacon;
} sl_zcl_sl_works_with_all_hubs_cluster_survey_beacons_response_command_t;



#endif //End of SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT

/** @} END addtogroup */
#endif // __ZAP_COMMAND_STRUCTS_H__

