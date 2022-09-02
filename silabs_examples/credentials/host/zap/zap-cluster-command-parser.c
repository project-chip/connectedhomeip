/***************************************************************************//**
 * @file  zap-cluster-command-parser.c
 * @brief This file contains a set of generated functions that parse the
 * incomming zcl command messages.
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

#include "sl_component_catalog.h"
#ifdef SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT

#include <stdint.h>

#include "af.h"
#include "zap-type.h"
#include "zap-cluster-command-parser.h"
#include "sl_signature_decode.h"

// Generated interface function
static EmberAfStatus sli_do_decode(EmberAfClusterCommand *cmd,
                                   const sl_signature_t sig,
                                   uint8_t *outputBuffer) {
  sl_signature_decode_status_t ret;
  ret = sl_signature_decode(cmd->buffer, 
                            cmd->bufLen, 
                            cmd->payloadStartIndex, 
                            sig, 
                            outputBuffer);
  if ( ret == SL_SIGNATURE_DECODE_OK )
    return EMBER_ZCL_STATUS_SUCCESS;
  else
    return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
}


// Signature for GetZoneIdMapResponse command
static const sl_signature_t sig_ias_ace_cluster_get_zone_id_map_response_command = sl_zcl_ias_ace_cluster_get_zone_id_map_response_command_signature;
// Command parser for GetZoneIdMapResponse command
EmberAfStatus zcl_decode_ias_ace_cluster_get_zone_id_map_response_command (EmberAfClusterCommand * cmd, sl_zcl_ias_ace_cluster_get_zone_id_map_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_ias_ace_cluster_get_zone_id_map_response_command, (uint8_t*)cmd_struct);
} 


// Signature for ZoneStatusChangeNotification command
static const sl_signature_t sig_ias_zone_cluster_zone_status_change_notification_command = sl_zcl_ias_zone_cluster_zone_status_change_notification_command_signature;
// Command parser for ZoneStatusChangeNotification command
EmberAfStatus zcl_decode_ias_zone_cluster_zone_status_change_notification_command (EmberAfClusterCommand * cmd, sl_zcl_ias_zone_cluster_zone_status_change_notification_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_ias_zone_cluster_zone_status_change_notification_command, (uint8_t*)cmd_struct);
} 


// Signature for InitiateKeyEstablishmentRequest command
static const sl_signature_t sig_key_establishment_cluster_initiate_key_establishment_request_command = sl_zcl_key_establishment_cluster_initiate_key_establishment_request_command_signature;
// Command parser for InitiateKeyEstablishmentRequest command
EmberAfStatus zcl_decode_key_establishment_cluster_initiate_key_establishment_request_command (EmberAfClusterCommand * cmd, sl_zcl_key_establishment_cluster_initiate_key_establishment_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_key_establishment_cluster_initiate_key_establishment_request_command, (uint8_t*)cmd_struct);
} 


// Signature for InitiateKeyEstablishmentResponse command
static const sl_signature_t sig_key_establishment_cluster_initiate_key_establishment_response_command = sl_zcl_key_establishment_cluster_initiate_key_establishment_response_command_signature;
// Command parser for InitiateKeyEstablishmentResponse command
EmberAfStatus zcl_decode_key_establishment_cluster_initiate_key_establishment_response_command (EmberAfClusterCommand * cmd, sl_zcl_key_establishment_cluster_initiate_key_establishment_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_key_establishment_cluster_initiate_key_establishment_response_command, (uint8_t*)cmd_struct);
} 


// Signature for TakeSnapshot command
static const sl_signature_t sig_simple_metering_cluster_take_snapshot_command = sl_zcl_simple_metering_cluster_take_snapshot_command_signature;
// Command parser for TakeSnapshot command
EmberAfStatus zcl_decode_simple_metering_cluster_take_snapshot_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_take_snapshot_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_simple_metering_cluster_take_snapshot_command, (uint8_t*)cmd_struct);
} 


// Signature for ActionControl command
static const sl_signature_t sig_gaming_cluster_action_control_command = sl_zcl_gaming_cluster_action_control_command_signature;
// Command parser for ActionControl command
EmberAfStatus zcl_decode_gaming_cluster_action_control_command (EmberAfClusterCommand * cmd, sl_zcl_gaming_cluster_action_control_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_gaming_cluster_action_control_command, (uint8_t*)cmd_struct);
} 


// Signature for EZModeInvoke command
static const sl_signature_t sig_identify_cluster_ez_mode_invoke_command = sl_zcl_identify_cluster_ez_mode_invoke_command_signature;
// Command parser for EZModeInvoke command
EmberAfStatus zcl_decode_identify_cluster_ez_mode_invoke_command (EmberAfClusterCommand * cmd, sl_zcl_identify_cluster_ez_mode_invoke_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_identify_cluster_ez_mode_invoke_command, (uint8_t*)cmd_struct);
} 


// Signature for RssiPing command
static const sl_signature_t sig_rssi_location_cluster_rssi_ping_command = sl_zcl_rssi_location_cluster_rssi_ping_command_signature;
// Command parser for RssiPing command
EmberAfStatus zcl_decode_rssi_location_cluster_rssi_ping_command (EmberAfClusterCommand * cmd, sl_zcl_rssi_location_cluster_rssi_ping_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_rssi_location_cluster_rssi_ping_command, (uint8_t*)cmd_struct);
} 


// Signature for Squawk command
static const sl_signature_t sig_ias_wd_cluster_squawk_command = sl_zcl_ias_wd_cluster_squawk_command_signature;
// Command parser for Squawk command
EmberAfStatus zcl_decode_ias_wd_cluster_squawk_command (EmberAfClusterCommand * cmd, sl_zcl_ias_wd_cluster_squawk_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_ias_wd_cluster_squawk_command, (uint8_t*)cmd_struct);
} 


// Signature for GetCurrentPrice command
static const sl_signature_t sig_price_cluster_get_current_price_command = sl_zcl_price_cluster_get_current_price_command_signature;
// Command parser for GetCurrentPrice command
EmberAfStatus zcl_decode_price_cluster_get_current_price_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_get_current_price_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_price_cluster_get_current_price_command, (uint8_t*)cmd_struct);
} 


// Signature for CancelAllLoadControlEvents command
static const sl_signature_t sig_demand_response_and_load_control_cluster_cancel_all_load_control_events_command = sl_zcl_demand_response_and_load_control_cluster_cancel_all_load_control_events_command_signature;
// Command parser for CancelAllLoadControlEvents command
EmberAfStatus zcl_decode_demand_response_and_load_control_cluster_cancel_all_load_control_events_command (EmberAfClusterCommand * cmd, sl_zcl_demand_response_and_load_control_cluster_cancel_all_load_control_events_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_demand_response_and_load_control_cluster_cancel_all_load_control_events_command, (uint8_t*)cmd_struct);
} 


// Signature for ClearEventLogResponse command
static const sl_signature_t sig_events_cluster_clear_event_log_response_command = sl_zcl_events_cluster_clear_event_log_response_command_signature;
// Command parser for ClearEventLogResponse command
EmberAfStatus zcl_decode_events_cluster_clear_event_log_response_command (EmberAfClusterCommand * cmd, sl_zcl_events_cluster_clear_event_log_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_events_cluster_clear_event_log_response_command, (uint8_t*)cmd_struct);
} 


// Signature for GetAlertsResponse command
static const sl_signature_t sig_appliance_events_and_alert_cluster_get_alerts_response_command = sl_zcl_appliance_events_and_alert_cluster_get_alerts_response_command_signature;
// Command parser for GetAlertsResponse command
EmberAfStatus zcl_decode_appliance_events_and_alert_cluster_get_alerts_response_command (EmberAfClusterCommand * cmd, sl_zcl_appliance_events_and_alert_cluster_get_alerts_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_appliance_events_and_alert_cluster_get_alerts_response_command, (uint8_t*)cmd_struct);
} 


// Signature for AlertsNotification command
static const sl_signature_t sig_appliance_events_and_alert_cluster_alerts_notification_command = sl_zcl_appliance_events_and_alert_cluster_alerts_notification_command_signature;
// Command parser for AlertsNotification command
EmberAfStatus zcl_decode_appliance_events_and_alert_cluster_alerts_notification_command (EmberAfClusterCommand * cmd, sl_zcl_appliance_events_and_alert_cluster_alerts_notification_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_appliance_events_and_alert_cluster_alerts_notification_command, (uint8_t*)cmd_struct);
} 


// Signature for GetWeeklySchedule command
static const sl_signature_t sig_thermostat_cluster_get_weekly_schedule_command = sl_zcl_thermostat_cluster_get_weekly_schedule_command_signature;
// Command parser for GetWeeklySchedule command
EmberAfStatus zcl_decode_thermostat_cluster_get_weekly_schedule_command (EmberAfClusterCommand * cmd, sl_zcl_thermostat_cluster_get_weekly_schedule_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_thermostat_cluster_get_weekly_schedule_command, (uint8_t*)cmd_struct);
} 


// Signature for EstablishmentRequest command
static const sl_signature_t sig_voice_over_zig_bee_cluster_establishment_request_command = sl_zcl_voice_over_zig_bee_cluster_establishment_request_command_signature;
// Command parser for EstablishmentRequest command
EmberAfStatus zcl_decode_voice_over_zig_bee_cluster_establishment_request_command (EmberAfClusterCommand * cmd, sl_zcl_voice_over_zig_bee_cluster_establishment_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_voice_over_zig_bee_cluster_establishment_request_command, (uint8_t*)cmd_struct);
} 


// Signature for ColorLoopSet command
static const sl_signature_t sig_color_control_cluster_color_loop_set_command = sl_zcl_color_control_cluster_color_loop_set_command_signature;
// Command parser for ColorLoopSet command
EmberAfStatus zcl_decode_color_control_cluster_color_loop_set_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_color_loop_set_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_color_control_cluster_color_loop_set_command, (uint8_t*)cmd_struct);
} 


// Signature for LocationDataNotification command
static const sl_signature_t sig_rssi_location_cluster_location_data_notification_command = sl_zcl_rssi_location_cluster_location_data_notification_command_signature;
// Command parser for LocationDataNotification command
EmberAfStatus zcl_decode_rssi_location_cluster_location_data_notification_command (EmberAfClusterCommand * cmd, sl_zcl_rssi_location_cluster_location_data_notification_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_rssi_location_cluster_location_data_notification_command, (uint8_t*)cmd_struct);
} 


// Signature for CompactLocationDataNotification command
static const sl_signature_t sig_rssi_location_cluster_compact_location_data_notification_command = sl_zcl_rssi_location_cluster_compact_location_data_notification_command_signature;
// Command parser for CompactLocationDataNotification command
EmberAfStatus zcl_decode_rssi_location_cluster_compact_location_data_notification_command (EmberAfClusterCommand * cmd, sl_zcl_rssi_location_cluster_compact_location_data_notification_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_rssi_location_cluster_compact_location_data_notification_command, (uint8_t*)cmd_struct);
} 


// Signature for OnWithTimedOff command
static const sl_signature_t sig_on_off_cluster_on_with_timed_off_command = sl_zcl_on_off_cluster_on_with_timed_off_command_signature;
// Command parser for OnWithTimedOff command
EmberAfStatus zcl_decode_on_off_cluster_on_with_timed_off_command (EmberAfClusterCommand * cmd, sl_zcl_on_off_cluster_on_with_timed_off_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_on_off_cluster_on_with_timed_off_command, (uint8_t*)cmd_struct);
} 


// Signature for GpSinkCommissioningMode command
static const sl_signature_t sig_green_power_cluster_gp_sink_commissioning_mode_command = sl_zcl_green_power_cluster_gp_sink_commissioning_mode_command_signature;
// Command parser for GpSinkCommissioningMode command
EmberAfStatus zcl_decode_green_power_cluster_gp_sink_commissioning_mode_command (EmberAfClusterCommand * cmd, sl_zcl_green_power_cluster_gp_sink_commissioning_mode_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_green_power_cluster_gp_sink_commissioning_mode_command, (uint8_t*)cmd_struct);
} 


// Signature for CopyScene command
static const sl_signature_t sig_scenes_cluster_copy_scene_command = sl_zcl_scenes_cluster_copy_scene_command_signature;
// Command parser for CopyScene command
EmberAfStatus zcl_decode_scenes_cluster_copy_scene_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_copy_scene_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_scenes_cluster_copy_scene_command, (uint8_t*)cmd_struct);
} 


// Signature for StartWarning command
static const sl_signature_t sig_ias_wd_cluster_start_warning_command = sl_zcl_ias_wd_cluster_start_warning_command_signature;
// Command parser for StartWarning command
EmberAfStatus zcl_decode_ias_wd_cluster_start_warning_command (EmberAfClusterCommand * cmd, sl_zcl_ias_wd_cluster_start_warning_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_ias_wd_cluster_start_warning_command, (uint8_t*)cmd_struct);
} 


// Signature for GetEventLog command
static const sl_signature_t sig_events_cluster_get_event_log_command = sl_zcl_events_cluster_get_event_log_command_signature;
// Command parser for GetEventLog command
EmberAfStatus zcl_decode_events_cluster_get_event_log_command (EmberAfClusterCommand * cmd, sl_zcl_events_cluster_get_event_log_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_events_cluster_get_event_log_command, (uint8_t*)cmd_struct);
} 


// Signature for Delete command
static const sl_signature_t sig_information_cluster_delete_command = sl_zcl_information_cluster_delete_command_signature;
// Command parser for Delete command
EmberAfStatus zcl_decode_information_cluster_delete_command (EmberAfClusterCommand * cmd, sl_zcl_information_cluster_delete_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_information_cluster_delete_command, (uint8_t*)cmd_struct);
} 


// Signature for ChangePaymentModeResponse command
static const sl_signature_t sig_prepayment_cluster_change_payment_mode_response_command = sl_zcl_prepayment_cluster_change_payment_mode_response_command_signature;
// Command parser for ChangePaymentModeResponse command
EmberAfStatus zcl_decode_prepayment_cluster_change_payment_mode_response_command (EmberAfClusterCommand * cmd, sl_zcl_prepayment_cluster_change_payment_mode_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_prepayment_cluster_change_payment_mode_response_command, (uint8_t*)cmd_struct);
} 


// Signature for SaveStartupParameters command
static const sl_signature_t sig_commissioning_cluster_save_startup_parameters_command = sl_zcl_commissioning_cluster_save_startup_parameters_command_signature;
// Command parser for SaveStartupParameters command
EmberAfStatus zcl_decode_commissioning_cluster_save_startup_parameters_command (EmberAfClusterCommand * cmd, sl_zcl_commissioning_cluster_save_startup_parameters_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_commissioning_cluster_save_startup_parameters_command, (uint8_t*)cmd_struct);
} 


// Signature for RestoreStartupParameters command
static const sl_signature_t sig_commissioning_cluster_restore_startup_parameters_command = sl_zcl_commissioning_cluster_restore_startup_parameters_command_signature;
// Command parser for RestoreStartupParameters command
EmberAfStatus zcl_decode_commissioning_cluster_restore_startup_parameters_command (EmberAfClusterCommand * cmd, sl_zcl_commissioning_cluster_restore_startup_parameters_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_commissioning_cluster_restore_startup_parameters_command, (uint8_t*)cmd_struct);
} 


// Signature for ResetStartupParameters command
static const sl_signature_t sig_commissioning_cluster_reset_startup_parameters_command = sl_zcl_commissioning_cluster_reset_startup_parameters_command_signature;
// Command parser for ResetStartupParameters command
EmberAfStatus zcl_decode_commissioning_cluster_reset_startup_parameters_command (EmberAfClusterCommand * cmd, sl_zcl_commissioning_cluster_reset_startup_parameters_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_commissioning_cluster_reset_startup_parameters_command, (uint8_t*)cmd_struct);
} 


// Signature for GetLocationData command
static const sl_signature_t sig_rssi_location_cluster_get_location_data_command = sl_zcl_rssi_location_cluster_get_location_data_command_signature;
// Command parser for GetLocationData command
EmberAfStatus zcl_decode_rssi_location_cluster_get_location_data_command (EmberAfClusterCommand * cmd, sl_zcl_rssi_location_cluster_get_location_data_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_rssi_location_cluster_get_location_data_command, (uint8_t*)cmd_struct);
} 


// Signature for GetPowerProfilePriceExtended command
static const sl_signature_t sig_power_profile_cluster_get_power_profile_price_extended_command = sl_zcl_power_profile_cluster_get_power_profile_price_extended_command_signature;
// Command parser for GetPowerProfilePriceExtended command
EmberAfStatus zcl_decode_power_profile_cluster_get_power_profile_price_extended_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_get_power_profile_price_extended_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_power_profile_cluster_get_power_profile_price_extended_command, (uint8_t*)cmd_struct);
} 


// Signature for RestartDevice command
static const sl_signature_t sig_commissioning_cluster_restart_device_command = sl_zcl_commissioning_cluster_restart_device_command_signature;
// Command parser for RestartDevice command
EmberAfStatus zcl_decode_commissioning_cluster_restart_device_command (EmberAfClusterCommand * cmd, sl_zcl_commissioning_cluster_restart_device_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_commissioning_cluster_restart_device_command, (uint8_t*)cmd_struct);
} 


// Signature for TransferPartitionedFrame command
static const sl_signature_t sig_partition_cluster_transfer_partitioned_frame_command = sl_zcl_partition_cluster_transfer_partitioned_frame_command_signature;
// Command parser for TransferPartitionedFrame command
EmberAfStatus zcl_decode_partition_cluster_transfer_partitioned_frame_command (EmberAfClusterCommand * cmd, sl_zcl_partition_cluster_transfer_partitioned_frame_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_partition_cluster_transfer_partitioned_frame_command, (uint8_t*)cmd_struct);
} 


// Signature for MultipleAck command
static const sl_signature_t sig_partition_cluster_multiple_ack_command = sl_zcl_partition_cluster_multiple_ack_command_signature;
// Command parser for MultipleAck command
EmberAfStatus zcl_decode_partition_cluster_multiple_ack_command (EmberAfClusterCommand * cmd, sl_zcl_partition_cluster_multiple_ack_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_partition_cluster_multiple_ack_command, (uint8_t*)cmd_struct);
} 


// Signature for WriteFileRequest command
static const sl_signature_t sig_data_sharing_cluster_write_file_request_command = sl_zcl_data_sharing_cluster_write_file_request_command_signature;
// Command parser for WriteFileRequest command
EmberAfStatus zcl_decode_data_sharing_cluster_write_file_request_command (EmberAfClusterCommand * cmd, sl_zcl_data_sharing_cluster_write_file_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_data_sharing_cluster_write_file_request_command, (uint8_t*)cmd_struct);
} 


// Signature for FileTransmission command
static const sl_signature_t sig_data_sharing_cluster_file_transmission_command = sl_zcl_data_sharing_cluster_file_transmission_command_signature;
// Command parser for FileTransmission command
EmberAfStatus zcl_decode_data_sharing_cluster_file_transmission_command (EmberAfClusterCommand * cmd, sl_zcl_data_sharing_cluster_file_transmission_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_data_sharing_cluster_file_transmission_command, (uint8_t*)cmd_struct);
} 


// Signature for RecordTransmission command
static const sl_signature_t sig_data_sharing_cluster_record_transmission_command = sl_zcl_data_sharing_cluster_record_transmission_command_signature;
// Command parser for RecordTransmission command
EmberAfStatus zcl_decode_data_sharing_cluster_record_transmission_command (EmberAfClusterCommand * cmd, sl_zcl_data_sharing_cluster_record_transmission_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_data_sharing_cluster_record_transmission_command, (uint8_t*)cmd_struct);
} 


// Signature for SearchChatResponse command
static const sl_signature_t sig_chatting_cluster_search_chat_response_command = sl_zcl_chatting_cluster_search_chat_response_command_signature;
// Command parser for SearchChatResponse command
EmberAfStatus zcl_decode_chatting_cluster_search_chat_response_command (EmberAfClusterCommand * cmd, sl_zcl_chatting_cluster_search_chat_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_chatting_cluster_search_chat_response_command, (uint8_t*)cmd_struct);
} 


// Signature for Stop command
static const sl_signature_t sig_level_control_cluster_stop_command = sl_zcl_level_control_cluster_stop_command_signature;
// Command parser for Stop command
EmberAfStatus zcl_decode_level_control_cluster_stop_command (EmberAfClusterCommand * cmd, sl_zcl_level_control_cluster_stop_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_level_control_cluster_stop_command, (uint8_t*)cmd_struct);
} 


// Signature for StopMoveStep command
static const sl_signature_t sig_color_control_cluster_stop_move_step_command = sl_zcl_color_control_cluster_stop_move_step_command_signature;
// Command parser for StopMoveStep command
EmberAfStatus zcl_decode_color_control_cluster_stop_move_step_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_stop_move_step_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_color_control_cluster_stop_move_step_command, (uint8_t*)cmd_struct);
} 


// Signature for ConfigureDeliveryEnable command
static const sl_signature_t sig_information_cluster_configure_delivery_enable_command = sl_zcl_information_cluster_configure_delivery_enable_command_signature;
// Command parser for ConfigureDeliveryEnable command
EmberAfStatus zcl_decode_information_cluster_configure_delivery_enable_command (EmberAfClusterCommand * cmd, sl_zcl_information_cluster_configure_delivery_enable_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_information_cluster_configure_delivery_enable_command, (uint8_t*)cmd_struct);
} 


// Signature for SurveyBeacons command
static const sl_signature_t sig_sl_works_with_all_hubs_cluster_survey_beacons_command = sl_zcl_sl_works_with_all_hubs_cluster_survey_beacons_command_signature;
// Command parser for SurveyBeacons command
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_survey_beacons_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_survey_beacons_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_sl_works_with_all_hubs_cluster_survey_beacons_command, (uint8_t*)cmd_struct);
} 


// Signature for CheckInResponse command
static const sl_signature_t sig_poll_control_cluster_check_in_response_command = sl_zcl_poll_control_cluster_check_in_response_command_signature;
// Command parser for CheckInResponse command
EmberAfStatus zcl_decode_poll_control_cluster_check_in_response_command (EmberAfClusterCommand * cmd, sl_zcl_poll_control_cluster_check_in_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_poll_control_cluster_check_in_response_command, (uint8_t*)cmd_struct);
} 


// Signature for GetZoneStatusResponse command
static const sl_signature_t sig_ias_ace_cluster_get_zone_status_response_command = sl_zcl_ias_ace_cluster_get_zone_status_response_command_signature;
// Command parser for GetZoneStatusResponse command
EmberAfStatus zcl_decode_ias_ace_cluster_get_zone_status_response_command (EmberAfClusterCommand * cmd, sl_zcl_ias_ace_cluster_get_zone_status_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_ias_ace_cluster_get_zone_status_response_command, (uint8_t*)cmd_struct);
} 


// Signature for SupportedTunnelProtocolsResponse command
static const sl_signature_t sig_tunneling_cluster_supported_tunnel_protocols_response_command = sl_zcl_tunneling_cluster_supported_tunnel_protocols_response_command_signature;
// Command parser for SupportedTunnelProtocolsResponse command
EmberAfStatus zcl_decode_tunneling_cluster_supported_tunnel_protocols_response_command (EmberAfClusterCommand * cmd, sl_zcl_tunneling_cluster_supported_tunnel_protocols_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_tunneling_cluster_supported_tunnel_protocols_response_command, (uint8_t*)cmd_struct);
} 


// Signature for ConfigureNodeDescription command
static const sl_signature_t sig_information_cluster_configure_node_description_command = sl_zcl_information_cluster_configure_node_description_command_signature;
// Command parser for ConfigureNodeDescription command
EmberAfStatus zcl_decode_information_cluster_configure_node_description_command (EmberAfClusterCommand * cmd, sl_zcl_information_cluster_configure_node_description_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_information_cluster_configure_node_description_command, (uint8_t*)cmd_struct);
} 


// Signature for StartChatRequest command
static const sl_signature_t sig_chatting_cluster_start_chat_request_command = sl_zcl_chatting_cluster_start_chat_request_command_signature;
// Command parser for StartChatRequest command
EmberAfStatus zcl_decode_chatting_cluster_start_chat_request_command (EmberAfClusterCommand * cmd, sl_zcl_chatting_cluster_start_chat_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_chatting_cluster_start_chat_request_command, (uint8_t*)cmd_struct);
} 


// Signature for GetLocalesSupported command
static const sl_signature_t sig_basic_cluster_get_locales_supported_command = sl_zcl_basic_cluster_get_locales_supported_command_signature;
// Command parser for GetLocalesSupported command
EmberAfStatus zcl_decode_basic_cluster_get_locales_supported_command (EmberAfClusterCommand * cmd, sl_zcl_basic_cluster_get_locales_supported_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_basic_cluster_get_locales_supported_command, (uint8_t*)cmd_struct);
} 


// Signature for LockDoor command
static const sl_signature_t sig_door_lock_cluster_lock_door_command = sl_zcl_door_lock_cluster_lock_door_command_signature;
// Command parser for LockDoor command
EmberAfStatus zcl_decode_door_lock_cluster_lock_door_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_lock_door_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_lock_door_command, (uint8_t*)cmd_struct);
} 


// Signature for UnlockDoor command
static const sl_signature_t sig_door_lock_cluster_unlock_door_command = sl_zcl_door_lock_cluster_unlock_door_command_signature;
// Command parser for UnlockDoor command
EmberAfStatus zcl_decode_door_lock_cluster_unlock_door_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_unlock_door_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_unlock_door_command, (uint8_t*)cmd_struct);
} 


// Signature for Toggle command
static const sl_signature_t sig_door_lock_cluster_toggle_command = sl_zcl_door_lock_cluster_toggle_command_signature;
// Command parser for Toggle command
EmberAfStatus zcl_decode_door_lock_cluster_toggle_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_toggle_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_toggle_command, (uint8_t*)cmd_struct);
} 


// Signature for ApsLinkKeyAuthorizationQuery command
static const sl_signature_t sig_sl_works_with_all_hubs_cluster_aps_link_key_authorization_query_command = sl_zcl_sl_works_with_all_hubs_cluster_aps_link_key_authorization_query_command_signature;
// Command parser for ApsLinkKeyAuthorizationQuery command
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_aps_link_key_authorization_query_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_aps_link_key_authorization_query_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_sl_works_with_all_hubs_cluster_aps_link_key_authorization_query_command, (uint8_t*)cmd_struct);
} 


// Signature for ReadHandshakeParam command
static const sl_signature_t sig_partition_cluster_read_handshake_param_command = sl_zcl_partition_cluster_read_handshake_param_command_signature;
// Command parser for ReadHandshakeParam command
EmberAfStatus zcl_decode_partition_cluster_read_handshake_param_command (EmberAfClusterCommand * cmd, sl_zcl_partition_cluster_read_handshake_param_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_partition_cluster_read_handshake_param_command, (uint8_t*)cmd_struct);
} 


// Signature for ApsLinkKeyAuthorizationQueryResponse command
static const sl_signature_t sig_sl_works_with_all_hubs_cluster_aps_link_key_authorization_query_response_command = sl_zcl_sl_works_with_all_hubs_cluster_aps_link_key_authorization_query_response_command_signature;
// Command parser for ApsLinkKeyAuthorizationQueryResponse command
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_aps_link_key_authorization_query_response_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_aps_link_key_authorization_query_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_sl_works_with_all_hubs_cluster_aps_link_key_authorization_query_response_command, (uint8_t*)cmd_struct);
} 


// Signature for WriteHandshakeParam command
static const sl_signature_t sig_partition_cluster_write_handshake_param_command = sl_zcl_partition_cluster_write_handshake_param_command_signature;
// Command parser for WriteHandshakeParam command
EmberAfStatus zcl_decode_partition_cluster_write_handshake_param_command (EmberAfClusterCommand * cmd, sl_zcl_partition_cluster_write_handshake_param_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_partition_cluster_write_handshake_param_command, (uint8_t*)cmd_struct);
} 


// Signature for ReadHandshakeParamResponse command
static const sl_signature_t sig_partition_cluster_read_handshake_param_response_command = sl_zcl_partition_cluster_read_handshake_param_response_command_signature;
// Command parser for ReadHandshakeParamResponse command
EmberAfStatus zcl_decode_partition_cluster_read_handshake_param_response_command (EmberAfClusterCommand * cmd, sl_zcl_partition_cluster_read_handshake_param_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_partition_cluster_read_handshake_param_response_command, (uint8_t*)cmd_struct);
} 


// Signature for PathDeletion command
static const sl_signature_t sig_data_rate_control_cluster_path_deletion_command = sl_zcl_data_rate_control_cluster_path_deletion_command_signature;
// Command parser for PathDeletion command
EmberAfStatus zcl_decode_data_rate_control_cluster_path_deletion_command (EmberAfClusterCommand * cmd, sl_zcl_data_rate_control_cluster_path_deletion_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_data_rate_control_cluster_path_deletion_command, (uint8_t*)cmd_struct);
} 


// Signature for PathCreation command
static const sl_signature_t sig_data_rate_control_cluster_path_creation_command = sl_zcl_data_rate_control_cluster_path_creation_command_signature;
// Command parser for PathCreation command
EmberAfStatus zcl_decode_data_rate_control_cluster_path_creation_command (EmberAfClusterCommand * cmd, sl_zcl_data_rate_control_cluster_path_creation_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_data_rate_control_cluster_path_creation_command, (uint8_t*)cmd_struct);
} 


// Signature for DataRateNotification command
static const sl_signature_t sig_data_rate_control_cluster_data_rate_notification_command = sl_zcl_data_rate_control_cluster_data_rate_notification_command_signature;
// Command parser for DataRateNotification command
EmberAfStatus zcl_decode_data_rate_control_cluster_data_rate_notification_command (EmberAfClusterCommand * cmd, sl_zcl_data_rate_control_cluster_data_rate_notification_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_data_rate_control_cluster_data_rate_notification_command, (uint8_t*)cmd_struct);
} 


// Signature for DataRateControl command
static const sl_signature_t sig_data_rate_control_cluster_data_rate_control_command = sl_zcl_data_rate_control_cluster_data_rate_control_command_signature;
// Command parser for DataRateControl command
EmberAfStatus zcl_decode_data_rate_control_cluster_data_rate_control_command (EmberAfClusterCommand * cmd, sl_zcl_data_rate_control_cluster_data_rate_control_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_data_rate_control_cluster_data_rate_control_command, (uint8_t*)cmd_struct);
} 


// Signature for TransferNpdu command
static const sl_signature_t sig_ba_cnet_protocol_tunnel_cluster_transfer_npdu_command = sl_zcl_ba_cnet_protocol_tunnel_cluster_transfer_npdu_command_signature;
// Command parser for TransferNpdu command
EmberAfStatus zcl_decode_ba_cnet_protocol_tunnel_cluster_transfer_npdu_command (EmberAfClusterCommand * cmd, sl_zcl_ba_cnet_protocol_tunnel_cluster_transfer_npdu_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_ba_cnet_protocol_tunnel_cluster_transfer_npdu_command, (uint8_t*)cmd_struct);
} 


// Signature for ZoneEnrollRequest command
static const sl_signature_t sig_ias_zone_cluster_zone_enroll_request_command = sl_zcl_ias_zone_cluster_zone_enroll_request_command_signature;
// Command parser for ZoneEnrollRequest command
EmberAfStatus zcl_decode_ias_zone_cluster_zone_enroll_request_command (EmberAfClusterCommand * cmd, sl_zcl_ias_zone_cluster_zone_enroll_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_ias_zone_cluster_zone_enroll_request_command, (uint8_t*)cmd_struct);
} 


// Signature for RestartDeviceResponse command
static const sl_signature_t sig_commissioning_cluster_restart_device_response_command = sl_zcl_commissioning_cluster_restart_device_response_command_signature;
// Command parser for RestartDeviceResponse command
EmberAfStatus zcl_decode_commissioning_cluster_restart_device_response_command (EmberAfClusterCommand * cmd, sl_zcl_commissioning_cluster_restart_device_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_commissioning_cluster_restart_device_response_command, (uint8_t*)cmd_struct);
} 


// Signature for SaveStartupParametersResponse command
static const sl_signature_t sig_commissioning_cluster_save_startup_parameters_response_command = sl_zcl_commissioning_cluster_save_startup_parameters_response_command_signature;
// Command parser for SaveStartupParametersResponse command
EmberAfStatus zcl_decode_commissioning_cluster_save_startup_parameters_response_command (EmberAfClusterCommand * cmd, sl_zcl_commissioning_cluster_save_startup_parameters_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_commissioning_cluster_save_startup_parameters_response_command, (uint8_t*)cmd_struct);
} 


// Signature for RestoreStartupParametersResponse command
static const sl_signature_t sig_commissioning_cluster_restore_startup_parameters_response_command = sl_zcl_commissioning_cluster_restore_startup_parameters_response_command_signature;
// Command parser for RestoreStartupParametersResponse command
EmberAfStatus zcl_decode_commissioning_cluster_restore_startup_parameters_response_command (EmberAfClusterCommand * cmd, sl_zcl_commissioning_cluster_restore_startup_parameters_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_commissioning_cluster_restore_startup_parameters_response_command, (uint8_t*)cmd_struct);
} 


// Signature for ResetStartupParametersResponse command
static const sl_signature_t sig_commissioning_cluster_reset_startup_parameters_response_command = sl_zcl_commissioning_cluster_reset_startup_parameters_response_command_signature;
// Command parser for ResetStartupParametersResponse command
EmberAfStatus zcl_decode_commissioning_cluster_reset_startup_parameters_response_command (EmberAfClusterCommand * cmd, sl_zcl_commissioning_cluster_reset_startup_parameters_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_commissioning_cluster_reset_startup_parameters_response_command, (uint8_t*)cmd_struct);
} 


// Signature for ExecutionOfACommand command
static const sl_signature_t sig_appliance_control_cluster_execution_of_a_command_command = sl_zcl_appliance_control_cluster_execution_of_a_command_command_signature;
// Command parser for ExecutionOfACommand command
EmberAfStatus zcl_decode_appliance_control_cluster_execution_of_a_command_command (EmberAfClusterCommand * cmd, sl_zcl_appliance_control_cluster_execution_of_a_command_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_appliance_control_cluster_execution_of_a_command_command, (uint8_t*)cmd_struct);
} 


// Signature for OverloadWarning command
static const sl_signature_t sig_appliance_control_cluster_overload_warning_command = sl_zcl_appliance_control_cluster_overload_warning_command_signature;
// Command parser for OverloadWarning command
EmberAfStatus zcl_decode_appliance_control_cluster_overload_warning_command (EmberAfClusterCommand * cmd, sl_zcl_appliance_control_cluster_overload_warning_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_appliance_control_cluster_overload_warning_command, (uint8_t*)cmd_struct);
} 


// Signature for SetPinResponse command
static const sl_signature_t sig_door_lock_cluster_set_pin_response_command = sl_zcl_door_lock_cluster_set_pin_response_command_signature;
// Command parser for SetPinResponse command
EmberAfStatus zcl_decode_door_lock_cluster_set_pin_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_set_pin_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_set_pin_response_command, (uint8_t*)cmd_struct);
} 


// Signature for SetRfidResponse command
static const sl_signature_t sig_door_lock_cluster_set_rfid_response_command = sl_zcl_door_lock_cluster_set_rfid_response_command_signature;
// Command parser for SetRfidResponse command
EmberAfStatus zcl_decode_door_lock_cluster_set_rfid_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_set_rfid_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_set_rfid_response_command, (uint8_t*)cmd_struct);
} 


// Signature for ArmResponse command
static const sl_signature_t sig_ias_ace_cluster_arm_response_command = sl_zcl_ias_ace_cluster_arm_response_command_signature;
// Command parser for ArmResponse command
EmberAfStatus zcl_decode_ias_ace_cluster_arm_response_command (EmberAfClusterCommand * cmd, sl_zcl_ias_ace_cluster_arm_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_ias_ace_cluster_arm_response_command, (uint8_t*)cmd_struct);
} 


// Signature for ConnectStatusNotification command
static const sl_signature_t sig_11073_protocol_tunnel_cluster_connect_status_notification_command = sl_zcl_11073_protocol_tunnel_cluster_connect_status_notification_command_signature;
// Command parser for ConnectStatusNotification command
EmberAfStatus zcl_decode_11073_protocol_tunnel_cluster_connect_status_notification_command (EmberAfClusterCommand * cmd, sl_zcl_11073_protocol_tunnel_cluster_connect_status_notification_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_11073_protocol_tunnel_cluster_connect_status_notification_command, (uint8_t*)cmd_struct);
} 


// Signature for LocalChangeSupply command
static const sl_signature_t sig_simple_metering_cluster_local_change_supply_command = sl_zcl_simple_metering_cluster_local_change_supply_command_signature;
// Command parser for LocalChangeSupply command
EmberAfStatus zcl_decode_simple_metering_cluster_local_change_supply_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_local_change_supply_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_simple_metering_cluster_local_change_supply_command, (uint8_t*)cmd_struct);
} 


// Signature for RequestNewPassword command
static const sl_signature_t sig_device_management_cluster_request_new_password_command = sl_zcl_device_management_cluster_request_new_password_command_signature;
// Command parser for RequestNewPassword command
EmberAfStatus zcl_decode_device_management_cluster_request_new_password_command (EmberAfClusterCommand * cmd, sl_zcl_device_management_cluster_request_new_password_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_device_management_cluster_request_new_password_command, (uint8_t*)cmd_struct);
} 


// Signature for ClearEventLogRequest command
static const sl_signature_t sig_events_cluster_clear_event_log_request_command = sl_zcl_events_cluster_clear_event_log_request_command_signature;
// Command parser for ClearEventLogRequest command
EmberAfStatus zcl_decode_events_cluster_clear_event_log_request_command (EmberAfClusterCommand * cmd, sl_zcl_events_cluster_clear_event_log_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_events_cluster_clear_event_log_request_command, (uint8_t*)cmd_struct);
} 


// Signature for ControlResponse command
static const sl_signature_t sig_voice_over_zig_bee_cluster_control_response_command = sl_zcl_voice_over_zig_bee_cluster_control_response_command_signature;
// Command parser for ControlResponse command
EmberAfStatus zcl_decode_voice_over_zig_bee_cluster_control_response_command (EmberAfClusterCommand * cmd, sl_zcl_voice_over_zig_bee_cluster_control_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_voice_over_zig_bee_cluster_control_response_command, (uint8_t*)cmd_struct);
} 


// Signature for Control command
static const sl_signature_t sig_voice_over_zig_bee_cluster_control_command = sl_zcl_voice_over_zig_bee_cluster_control_command_signature;
// Command parser for Control command
EmberAfStatus zcl_decode_voice_over_zig_bee_cluster_control_command (EmberAfClusterCommand * cmd, sl_zcl_voice_over_zig_bee_cluster_control_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_voice_over_zig_bee_cluster_control_command, (uint8_t*)cmd_struct);
} 


// Signature for SetIasZoneEnrollmentMethod command
static const sl_signature_t sig_sl_works_with_all_hubs_cluster_set_ias_zone_enrollment_method_command = sl_zcl_sl_works_with_all_hubs_cluster_set_ias_zone_enrollment_method_command_signature;
// Command parser for SetIasZoneEnrollmentMethod command
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_set_ias_zone_enrollment_method_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_set_ias_zone_enrollment_method_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_sl_works_with_all_hubs_cluster_set_ias_zone_enrollment_method_command, (uint8_t*)cmd_struct);
} 


// Signature for UpdateCommissionState command
static const sl_signature_t sig_identify_cluster_update_commission_state_command = sl_zcl_identify_cluster_update_commission_state_command_signature;
// Command parser for UpdateCommissionState command
EmberAfStatus zcl_decode_identify_cluster_update_commission_state_command (EmberAfClusterCommand * cmd, sl_zcl_identify_cluster_update_commission_state_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_identify_cluster_update_commission_state_command, (uint8_t*)cmd_struct);
} 


// Signature for SetWeeklySchedule command
static const sl_signature_t sig_thermostat_cluster_set_weekly_schedule_command = sl_zcl_thermostat_cluster_set_weekly_schedule_command_signature;
// Command parser for SetWeeklySchedule command
EmberAfStatus zcl_decode_thermostat_cluster_set_weekly_schedule_command (EmberAfClusterCommand * cmd, sl_zcl_thermostat_cluster_set_weekly_schedule_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_thermostat_cluster_set_weekly_schedule_command, (uint8_t*)cmd_struct);
} 


// Signature for CurrentWeeklySchedule command
static const sl_signature_t sig_thermostat_cluster_current_weekly_schedule_command = sl_zcl_thermostat_cluster_current_weekly_schedule_command_signature;
// Command parser for CurrentWeeklySchedule command
EmberAfStatus zcl_decode_thermostat_cluster_current_weekly_schedule_command (EmberAfClusterCommand * cmd, sl_zcl_thermostat_cluster_current_weekly_schedule_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_thermostat_cluster_current_weekly_schedule_command, (uint8_t*)cmd_struct);
} 


// Signature for SignalStateResponse command
static const sl_signature_t sig_appliance_control_cluster_signal_state_response_command = sl_zcl_appliance_control_cluster_signal_state_response_command_signature;
// Command parser for SignalStateResponse command
EmberAfStatus zcl_decode_appliance_control_cluster_signal_state_response_command (EmberAfClusterCommand * cmd, sl_zcl_appliance_control_cluster_signal_state_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_appliance_control_cluster_signal_state_response_command, (uint8_t*)cmd_struct);
} 


// Signature for SignalStateNotification command
static const sl_signature_t sig_appliance_control_cluster_signal_state_notification_command = sl_zcl_appliance_control_cluster_signal_state_notification_command_signature;
// Command parser for SignalStateNotification command
EmberAfStatus zcl_decode_appliance_control_cluster_signal_state_notification_command (EmberAfClusterCommand * cmd, sl_zcl_appliance_control_cluster_signal_state_notification_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_appliance_control_cluster_signal_state_notification_command, (uint8_t*)cmd_struct);
} 


// Signature for RequestInformation command
static const sl_signature_t sig_information_cluster_request_information_command = sl_zcl_information_cluster_request_information_command_signature;
// Command parser for RequestInformation command
EmberAfStatus zcl_decode_information_cluster_request_information_command (EmberAfClusterCommand * cmd, sl_zcl_information_cluster_request_information_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_information_cluster_request_information_command, (uint8_t*)cmd_struct);
} 


// Signature for Update command
static const sl_signature_t sig_information_cluster_update_command = sl_zcl_information_cluster_update_command_signature;
// Command parser for Update command
EmberAfStatus zcl_decode_information_cluster_update_command (EmberAfClusterCommand * cmd, sl_zcl_information_cluster_update_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_information_cluster_update_command, (uint8_t*)cmd_struct);
} 


// Signature for Arm command
static const sl_signature_t sig_ias_ace_cluster_arm_command = sl_zcl_ias_ace_cluster_arm_command_signature;
// Command parser for Arm command
EmberAfStatus zcl_decode_ias_ace_cluster_arm_command (EmberAfClusterCommand * cmd, sl_zcl_ias_ace_cluster_arm_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_ias_ace_cluster_arm_command, (uint8_t*)cmd_struct);
} 


// Signature for ResetAlarm command
static const sl_signature_t sig_alarms_cluster_reset_alarm_command = sl_zcl_alarms_cluster_reset_alarm_command_signature;
// Command parser for ResetAlarm command
EmberAfStatus zcl_decode_alarms_cluster_reset_alarm_command (EmberAfClusterCommand * cmd, sl_zcl_alarms_cluster_reset_alarm_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_alarms_cluster_reset_alarm_command, (uint8_t*)cmd_struct);
} 


// Signature for Alarm command
static const sl_signature_t sig_alarms_cluster_alarm_command = sl_zcl_alarms_cluster_alarm_command_signature;
// Command parser for Alarm command
EmberAfStatus zcl_decode_alarms_cluster_alarm_command (EmberAfClusterCommand * cmd, sl_zcl_alarms_cluster_alarm_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_alarms_cluster_alarm_command, (uint8_t*)cmd_struct);
} 


// Signature for TriggerEffect command
static const sl_signature_t sig_identify_cluster_trigger_effect_command = sl_zcl_identify_cluster_trigger_effect_command_signature;
// Command parser for TriggerEffect command
EmberAfStatus zcl_decode_identify_cluster_trigger_effect_command (EmberAfClusterCommand * cmd, sl_zcl_identify_cluster_trigger_effect_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_identify_cluster_trigger_effect_command, (uint8_t*)cmd_struct);
} 


// Signature for OffWithEffect command
static const sl_signature_t sig_on_off_cluster_off_with_effect_command = sl_zcl_on_off_cluster_off_with_effect_command_signature;
// Command parser for OffWithEffect command
EmberAfStatus zcl_decode_on_off_cluster_off_with_effect_command (EmberAfClusterCommand * cmd, sl_zcl_on_off_cluster_off_with_effect_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_on_off_cluster_off_with_effect_command, (uint8_t*)cmd_struct);
} 


// Signature for EstablishmentResponse command
static const sl_signature_t sig_voice_over_zig_bee_cluster_establishment_response_command = sl_zcl_voice_over_zig_bee_cluster_establishment_response_command_signature;
// Command parser for EstablishmentResponse command
EmberAfStatus zcl_decode_voice_over_zig_bee_cluster_establishment_response_command (EmberAfClusterCommand * cmd, sl_zcl_voice_over_zig_bee_cluster_establishment_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_voice_over_zig_bee_cluster_establishment_response_command, (uint8_t*)cmd_struct);
} 


// Signature for SearchGame command
static const sl_signature_t sig_gaming_cluster_search_game_command = sl_zcl_gaming_cluster_search_game_command_signature;
// Command parser for SearchGame command
EmberAfStatus zcl_decode_gaming_cluster_search_game_command (EmberAfClusterCommand * cmd, sl_zcl_gaming_cluster_search_game_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_gaming_cluster_search_game_command, (uint8_t*)cmd_struct);
} 


// Signature for StartChatResponse command
static const sl_signature_t sig_chatting_cluster_start_chat_response_command = sl_zcl_chatting_cluster_start_chat_response_command_signature;
// Command parser for StartChatResponse command
EmberAfStatus zcl_decode_chatting_cluster_start_chat_response_command (EmberAfClusterCommand * cmd, sl_zcl_chatting_cluster_start_chat_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_chatting_cluster_start_chat_response_command, (uint8_t*)cmd_struct);
} 


// Signature for EnhancedMoveHue command
static const sl_signature_t sig_color_control_cluster_enhanced_move_hue_command = sl_zcl_color_control_cluster_enhanced_move_hue_command_signature;
// Command parser for EnhancedMoveHue command
EmberAfStatus zcl_decode_color_control_cluster_enhanced_move_hue_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_enhanced_move_hue_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_color_control_cluster_enhanced_move_hue_command, (uint8_t*)cmd_struct);
} 


// Signature for ConnectRequest command
static const sl_signature_t sig_11073_protocol_tunnel_cluster_connect_request_command = sl_zcl_11073_protocol_tunnel_cluster_connect_request_command_signature;
// Command parser for ConnectRequest command
EmberAfStatus zcl_decode_11073_protocol_tunnel_cluster_connect_request_command (EmberAfClusterCommand * cmd, sl_zcl_11073_protocol_tunnel_cluster_connect_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_11073_protocol_tunnel_cluster_connect_request_command, (uint8_t*)cmd_struct);
} 


// Signature for EnhancedStepHue command
static const sl_signature_t sig_color_control_cluster_enhanced_step_hue_command = sl_zcl_color_control_cluster_enhanced_step_hue_command_signature;
// Command parser for EnhancedStepHue command
EmberAfStatus zcl_decode_color_control_cluster_enhanced_step_hue_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_enhanced_step_hue_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_color_control_cluster_enhanced_step_hue_command, (uint8_t*)cmd_struct);
} 


// Signature for MoveColorTemperature command
static const sl_signature_t sig_color_control_cluster_move_color_temperature_command = sl_zcl_color_control_cluster_move_color_temperature_command_signature;
// Command parser for MoveColorTemperature command
EmberAfStatus zcl_decode_color_control_cluster_move_color_temperature_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_move_color_temperature_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_color_control_cluster_move_color_temperature_command, (uint8_t*)cmd_struct);
} 


// Signature for StepColorTemperature command
static const sl_signature_t sig_color_control_cluster_step_color_temperature_command = sl_zcl_color_control_cluster_step_color_temperature_command_signature;
// Command parser for StepColorTemperature command
EmberAfStatus zcl_decode_color_control_cluster_step_color_temperature_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_step_color_temperature_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_color_control_cluster_step_color_temperature_command, (uint8_t*)cmd_struct);
} 


// Signature for GetNodeInformationResponse command
static const sl_signature_t sig_chatting_cluster_get_node_information_response_command = sl_zcl_chatting_cluster_get_node_information_response_command_signature;
// Command parser for GetNodeInformationResponse command
EmberAfStatus zcl_decode_chatting_cluster_get_node_information_response_command (EmberAfClusterCommand * cmd, sl_zcl_chatting_cluster_get_node_information_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_chatting_cluster_get_node_information_response_command, (uint8_t*)cmd_struct);
} 


// Signature for JoinChatResponse command
static const sl_signature_t sig_chatting_cluster_join_chat_response_command = sl_zcl_chatting_cluster_join_chat_response_command_signature;
// Command parser for JoinChatResponse command
EmberAfStatus zcl_decode_chatting_cluster_join_chat_response_command (EmberAfClusterCommand * cmd, sl_zcl_chatting_cluster_join_chat_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_chatting_cluster_join_chat_response_command, (uint8_t*)cmd_struct);
} 


// Signature for PublishEvent command
static const sl_signature_t sig_events_cluster_publish_event_command = sl_zcl_events_cluster_publish_event_command_signature;
// Command parser for PublishEvent command
EmberAfStatus zcl_decode_events_cluster_publish_event_command (EmberAfClusterCommand * cmd, sl_zcl_events_cluster_publish_event_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_events_cluster_publish_event_command, (uint8_t*)cmd_struct);
} 


// Signature for ConsumerTopUpResponse command
static const sl_signature_t sig_prepayment_cluster_consumer_top_up_response_command = sl_zcl_prepayment_cluster_consumer_top_up_response_command_signature;
// Command parser for ConsumerTopUpResponse command
EmberAfStatus zcl_decode_prepayment_cluster_consumer_top_up_response_command (EmberAfClusterCommand * cmd, sl_zcl_prepayment_cluster_consumer_top_up_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_prepayment_cluster_consumer_top_up_response_command, (uint8_t*)cmd_struct);
} 


// Signature for SetpointRaiseLower command
static const sl_signature_t sig_thermostat_cluster_setpoint_raise_lower_command = sl_zcl_thermostat_cluster_setpoint_raise_lower_command_signature;
// Command parser for SetpointRaiseLower command
EmberAfStatus zcl_decode_thermostat_cluster_setpoint_raise_lower_command (EmberAfClusterCommand * cmd, sl_zcl_thermostat_cluster_setpoint_raise_lower_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_thermostat_cluster_setpoint_raise_lower_command, (uint8_t*)cmd_struct);
} 


// Signature for MoveWithOnOff command
static const sl_signature_t sig_level_control_cluster_move_with_on_off_command = sl_zcl_level_control_cluster_move_with_on_off_command_signature;
// Command parser for MoveWithOnOff command
EmberAfStatus zcl_decode_level_control_cluster_move_with_on_off_command (EmberAfClusterCommand * cmd, sl_zcl_level_control_cluster_move_with_on_off_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_level_control_cluster_move_with_on_off_command, (uint8_t*)cmd_struct);
} 


// Signature for ZoneEnrollResponse command
static const sl_signature_t sig_ias_zone_cluster_zone_enroll_response_command = sl_zcl_ias_zone_cluster_zone_enroll_response_command_signature;
// Command parser for ZoneEnrollResponse command
EmberAfStatus zcl_decode_ias_zone_cluster_zone_enroll_response_command (EmberAfClusterCommand * cmd, sl_zcl_ias_zone_cluster_zone_enroll_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_ias_zone_cluster_zone_enroll_response_command, (uint8_t*)cmd_struct);
} 


// Signature for TerminateKeyEstablishmentFromClient command
static const sl_signature_t sig_key_establishment_cluster_terminate_key_establishment_from_client_command = sl_zcl_key_establishment_cluster_terminate_key_establishment_from_client_command_signature;
// Command parser for TerminateKeyEstablishmentFromClient command
EmberAfStatus zcl_decode_key_establishment_cluster_terminate_key_establishment_from_client_command (EmberAfClusterCommand * cmd, sl_zcl_key_establishment_cluster_terminate_key_establishment_from_client_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_key_establishment_cluster_terminate_key_establishment_from_client_command, (uint8_t*)cmd_struct);
} 


// Signature for TerminateKeyEstablishmentFromServer command
static const sl_signature_t sig_key_establishment_cluster_terminate_key_establishment_from_server_command = sl_zcl_key_establishment_cluster_terminate_key_establishment_from_server_command_signature;
// Command parser for TerminateKeyEstablishmentFromServer command
EmberAfStatus zcl_decode_key_establishment_cluster_terminate_key_establishment_from_server_command (EmberAfClusterCommand * cmd, sl_zcl_key_establishment_cluster_terminate_key_establishment_from_server_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_key_establishment_cluster_terminate_key_establishment_from_server_command, (uint8_t*)cmd_struct);
} 


// Signature for Move command
static const sl_signature_t sig_level_control_cluster_move_command = sl_zcl_level_control_cluster_move_command_signature;
// Command parser for Move command
EmberAfStatus zcl_decode_level_control_cluster_move_command (EmberAfClusterCommand * cmd, sl_zcl_level_control_cluster_move_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_level_control_cluster_move_command, (uint8_t*)cmd_struct);
} 


// Signature for MoveHue command
static const sl_signature_t sig_color_control_cluster_move_hue_command = sl_zcl_color_control_cluster_move_hue_command_signature;
// Command parser for MoveHue command
EmberAfStatus zcl_decode_color_control_cluster_move_hue_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_move_hue_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_color_control_cluster_move_hue_command, (uint8_t*)cmd_struct);
} 


// Signature for MoveSaturation command
static const sl_signature_t sig_color_control_cluster_move_saturation_command = sl_zcl_color_control_cluster_move_saturation_command_signature;
// Command parser for MoveSaturation command
EmberAfStatus zcl_decode_color_control_cluster_move_saturation_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_move_saturation_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_color_control_cluster_move_saturation_command, (uint8_t*)cmd_struct);
} 


// Signature for GetPanelStatusResponse command
static const sl_signature_t sig_ias_ace_cluster_get_panel_status_response_command = sl_zcl_ias_ace_cluster_get_panel_status_response_command_signature;
// Command parser for GetPanelStatusResponse command
EmberAfStatus zcl_decode_ias_ace_cluster_get_panel_status_response_command (EmberAfClusterCommand * cmd, sl_zcl_ias_ace_cluster_get_panel_status_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_ias_ace_cluster_get_panel_status_response_command, (uint8_t*)cmd_struct);
} 


// Signature for PanelStatusChanged command
static const sl_signature_t sig_ias_ace_cluster_panel_status_changed_command = sl_zcl_ias_ace_cluster_panel_status_changed_command_signature;
// Command parser for PanelStatusChanged command
EmberAfStatus zcl_decode_ias_ace_cluster_panel_status_changed_command (EmberAfClusterCommand * cmd, sl_zcl_ias_ace_cluster_panel_status_changed_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_ias_ace_cluster_panel_status_changed_command, (uint8_t*)cmd_struct);
} 


// Signature for StepWithOnOff command
static const sl_signature_t sig_level_control_cluster_step_with_on_off_command = sl_zcl_level_control_cluster_step_with_on_off_command_signature;
// Command parser for StepWithOnOff command
EmberAfStatus zcl_decode_level_control_cluster_step_with_on_off_command (EmberAfClusterCommand * cmd, sl_zcl_level_control_cluster_step_with_on_off_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_level_control_cluster_step_with_on_off_command, (uint8_t*)cmd_struct);
} 


// Signature for Step command
static const sl_signature_t sig_level_control_cluster_step_command = sl_zcl_level_control_cluster_step_command_signature;
// Command parser for Step command
EmberAfStatus zcl_decode_level_control_cluster_step_command (EmberAfClusterCommand * cmd, sl_zcl_level_control_cluster_step_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_level_control_cluster_step_command, (uint8_t*)cmd_struct);
} 


// Signature for ImageNotify command
static const sl_signature_t sig_over_the_air_bootloading_cluster_image_notify_command = sl_zcl_over_the_air_bootloading_cluster_image_notify_command_signature;
// Command parser for ImageNotify command
EmberAfStatus zcl_decode_over_the_air_bootloading_cluster_image_notify_command (EmberAfClusterCommand * cmd, sl_zcl_over_the_air_bootloading_cluster_image_notify_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_over_the_air_bootloading_cluster_image_notify_command, (uint8_t*)cmd_struct);
} 


// Signature for StepHue command
static const sl_signature_t sig_color_control_cluster_step_hue_command = sl_zcl_color_control_cluster_step_hue_command_signature;
// Command parser for StepHue command
EmberAfStatus zcl_decode_color_control_cluster_step_hue_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_step_hue_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_color_control_cluster_step_hue_command, (uint8_t*)cmd_struct);
} 


// Signature for StepSaturation command
static const sl_signature_t sig_color_control_cluster_step_saturation_command = sl_zcl_color_control_cluster_step_saturation_command_signature;
// Command parser for StepSaturation command
EmberAfStatus zcl_decode_color_control_cluster_step_saturation_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_step_saturation_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_color_control_cluster_step_saturation_command, (uint8_t*)cmd_struct);
} 


// Signature for GpProxyTableResponse command
static const sl_signature_t sig_green_power_cluster_gp_proxy_table_response_command = sl_zcl_green_power_cluster_gp_proxy_table_response_command_signature;
// Command parser for GpProxyTableResponse command
EmberAfStatus zcl_decode_green_power_cluster_gp_proxy_table_response_command (EmberAfClusterCommand * cmd, sl_zcl_green_power_cluster_gp_proxy_table_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_green_power_cluster_gp_proxy_table_response_command, (uint8_t*)cmd_struct);
} 


// Signature for GpSinkTableResponse command
static const sl_signature_t sig_green_power_cluster_gp_sink_table_response_command = sl_zcl_green_power_cluster_gp_sink_table_response_command_signature;
// Command parser for GpSinkTableResponse command
EmberAfStatus zcl_decode_green_power_cluster_gp_sink_table_response_command (EmberAfClusterCommand * cmd, sl_zcl_green_power_cluster_gp_sink_table_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_green_power_cluster_gp_sink_table_response_command, (uint8_t*)cmd_struct);
} 


// Signature for WriteFileResponse command
static const sl_signature_t sig_data_sharing_cluster_write_file_response_command = sl_zcl_data_sharing_cluster_write_file_response_command_signature;
// Command parser for WriteFileResponse command
EmberAfStatus zcl_decode_data_sharing_cluster_write_file_response_command (EmberAfClusterCommand * cmd, sl_zcl_data_sharing_cluster_write_file_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_data_sharing_cluster_write_file_response_command, (uint8_t*)cmd_struct);
} 


// Signature for ConsumerTopUp command
static const sl_signature_t sig_prepayment_cluster_consumer_top_up_command = sl_zcl_prepayment_cluster_consumer_top_up_command_signature;
// Command parser for ConsumerTopUp command
EmberAfStatus zcl_decode_prepayment_cluster_consumer_top_up_command (EmberAfClusterCommand * cmd, sl_zcl_prepayment_cluster_consumer_top_up_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_prepayment_cluster_consumer_top_up_command, (uint8_t*)cmd_struct);
} 


// Signature for GetProfile command
static const sl_signature_t sig_simple_metering_cluster_get_profile_command = sl_zcl_simple_metering_cluster_get_profile_command_signature;
// Command parser for GetProfile command
EmberAfStatus zcl_decode_simple_metering_cluster_get_profile_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_get_profile_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_simple_metering_cluster_get_profile_command, (uint8_t*)cmd_struct);
} 


// Signature for GpTranslationTableResponse command
static const sl_signature_t sig_green_power_cluster_gp_translation_table_response_command = sl_zcl_green_power_cluster_gp_translation_table_response_command_signature;
// Command parser for GpTranslationTableResponse command
EmberAfStatus zcl_decode_green_power_cluster_gp_translation_table_response_command (EmberAfClusterCommand * cmd, sl_zcl_green_power_cluster_gp_translation_table_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_green_power_cluster_gp_translation_table_response_command, (uint8_t*)cmd_struct);
} 


// Signature for GetDeviceConfiguration command
static const sl_signature_t sig_rssi_location_cluster_get_device_configuration_command = sl_zcl_rssi_location_cluster_get_device_configuration_command_signature;
// Command parser for GetDeviceConfiguration command
EmberAfStatus zcl_decode_rssi_location_cluster_get_device_configuration_command (EmberAfClusterCommand * cmd, sl_zcl_rssi_location_cluster_get_device_configuration_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_rssi_location_cluster_get_device_configuration_command, (uint8_t*)cmd_struct);
} 


// Signature for RequestOwnLocation command
static const sl_signature_t sig_rssi_location_cluster_request_own_location_command = sl_zcl_rssi_location_cluster_request_own_location_command_signature;
// Command parser for RequestOwnLocation command
EmberAfStatus zcl_decode_rssi_location_cluster_request_own_location_command (EmberAfClusterCommand * cmd, sl_zcl_rssi_location_cluster_request_own_location_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_rssi_location_cluster_request_own_location_command, (uint8_t*)cmd_struct);
} 


// Signature for DisconnectRequest command
static const sl_signature_t sig_11073_protocol_tunnel_cluster_disconnect_request_command = sl_zcl_11073_protocol_tunnel_cluster_disconnect_request_command_signature;
// Command parser for DisconnectRequest command
EmberAfStatus zcl_decode_11073_protocol_tunnel_cluster_disconnect_request_command (EmberAfClusterCommand * cmd, sl_zcl_11073_protocol_tunnel_cluster_disconnect_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_11073_protocol_tunnel_cluster_disconnect_request_command, (uint8_t*)cmd_struct);
} 


// Signature for AnchorNodeAnnounce command
static const sl_signature_t sig_rssi_location_cluster_anchor_node_announce_command = sl_zcl_rssi_location_cluster_anchor_node_announce_command_signature;
// Command parser for AnchorNodeAnnounce command
EmberAfStatus zcl_decode_rssi_location_cluster_anchor_node_announce_command (EmberAfClusterCommand * cmd, sl_zcl_rssi_location_cluster_anchor_node_announce_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_rssi_location_cluster_anchor_node_announce_command, (uint8_t*)cmd_struct);
} 


// Signature for RssiResponse command
static const sl_signature_t sig_rssi_location_cluster_rssi_response_command = sl_zcl_rssi_location_cluster_rssi_response_command_signature;
// Command parser for RssiResponse command
EmberAfStatus zcl_decode_rssi_location_cluster_rssi_response_command (EmberAfClusterCommand * cmd, sl_zcl_rssi_location_cluster_rssi_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_rssi_location_cluster_rssi_response_command, (uint8_t*)cmd_struct);
} 


// Signature for ShortAddressChange command
static const sl_signature_t sig_sl_works_with_all_hubs_cluster_short_address_change_command = sl_zcl_sl_works_with_all_hubs_cluster_short_address_change_command_signature;
// Command parser for ShortAddressChange command
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_short_address_change_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_short_address_change_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_sl_works_with_all_hubs_cluster_short_address_change_command, (uint8_t*)cmd_struct);
} 


// Signature for QuerySpecificFileRequest command
static const sl_signature_t sig_over_the_air_bootloading_cluster_query_specific_file_request_command = sl_zcl_over_the_air_bootloading_cluster_query_specific_file_request_command_signature;
// Command parser for QuerySpecificFileRequest command
EmberAfStatus zcl_decode_over_the_air_bootloading_cluster_query_specific_file_request_command (EmberAfClusterCommand * cmd, sl_zcl_over_the_air_bootloading_cluster_query_specific_file_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_over_the_air_bootloading_cluster_query_specific_file_request_command, (uint8_t*)cmd_struct);
} 


// Signature for EndpointInformation command
static const sl_signature_t sig_zll_commissioning_cluster_endpoint_information_command = sl_zcl_zll_commissioning_cluster_endpoint_information_command_signature;
// Command parser for EndpointInformation command
EmberAfStatus zcl_decode_zll_commissioning_cluster_endpoint_information_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_endpoint_information_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_zll_commissioning_cluster_endpoint_information_command, (uint8_t*)cmd_struct);
} 


// Signature for SendPings command
static const sl_signature_t sig_rssi_location_cluster_send_pings_command = sl_zcl_rssi_location_cluster_send_pings_command_signature;
// Command parser for SendPings command
EmberAfStatus zcl_decode_rssi_location_cluster_send_pings_command (EmberAfClusterCommand * cmd, sl_zcl_rssi_location_cluster_send_pings_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_rssi_location_cluster_send_pings_command, (uint8_t*)cmd_struct);
} 


// Signature for ReportRssiMeasurements command
static const sl_signature_t sig_rssi_location_cluster_report_rssi_measurements_command = sl_zcl_rssi_location_cluster_report_rssi_measurements_command_signature;
// Command parser for ReportRssiMeasurements command
EmberAfStatus zcl_decode_rssi_location_cluster_report_rssi_measurements_command (EmberAfClusterCommand * cmd, sl_zcl_rssi_location_cluster_report_rssi_measurements_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_rssi_location_cluster_report_rssi_measurements_command, (uint8_t*)cmd_struct);
} 


// Signature for MatchProtocolAddressResponse command
static const sl_signature_t sig_generic_tunnel_cluster_match_protocol_address_response_command = sl_zcl_generic_tunnel_cluster_match_protocol_address_response_command_signature;
// Command parser for MatchProtocolAddressResponse command
EmberAfStatus zcl_decode_generic_tunnel_cluster_match_protocol_address_response_command (EmberAfClusterCommand * cmd, sl_zcl_generic_tunnel_cluster_match_protocol_address_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_generic_tunnel_cluster_match_protocol_address_response_command, (uint8_t*)cmd_struct);
} 


// Signature for MoveColor command
static const sl_signature_t sig_color_control_cluster_move_color_command = sl_zcl_color_control_cluster_move_color_command_signature;
// Command parser for MoveColor command
EmberAfStatus zcl_decode_color_control_cluster_move_color_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_move_color_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_color_control_cluster_move_color_command, (uint8_t*)cmd_struct);
} 


// Signature for SetAbsoluteLocation command
static const sl_signature_t sig_rssi_location_cluster_set_absolute_location_command = sl_zcl_rssi_location_cluster_set_absolute_location_command_signature;
// Command parser for SetAbsoluteLocation command
EmberAfStatus zcl_decode_rssi_location_cluster_set_absolute_location_command (EmberAfClusterCommand * cmd, sl_zcl_rssi_location_cluster_set_absolute_location_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_rssi_location_cluster_set_absolute_location_command, (uint8_t*)cmd_struct);
} 


// Signature for StepColor command
static const sl_signature_t sig_color_control_cluster_step_color_command = sl_zcl_color_control_cluster_step_color_command_signature;
// Command parser for StepColor command
EmberAfStatus zcl_decode_color_control_cluster_step_color_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_step_color_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_color_control_cluster_step_color_command, (uint8_t*)cmd_struct);
} 


// Signature for SetDeviceConfiguration command
static const sl_signature_t sig_rssi_location_cluster_set_device_configuration_command = sl_zcl_rssi_location_cluster_set_device_configuration_command_signature;
// Command parser for SetDeviceConfiguration command
EmberAfStatus zcl_decode_rssi_location_cluster_set_device_configuration_command (EmberAfClusterCommand * cmd, sl_zcl_rssi_location_cluster_set_device_configuration_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_rssi_location_cluster_set_device_configuration_command, (uint8_t*)cmd_struct);
} 


// Signature for Identify command
static const sl_signature_t sig_identify_cluster_identify_command = sl_zcl_identify_cluster_identify_command_signature;
// Command parser for Identify command
EmberAfStatus zcl_decode_identify_cluster_identify_command (EmberAfClusterCommand * cmd, sl_zcl_identify_cluster_identify_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_identify_cluster_identify_command, (uint8_t*)cmd_struct);
} 


// Signature for IdentifyQueryResponse command
static const sl_signature_t sig_identify_cluster_identify_query_response_command = sl_zcl_identify_cluster_identify_query_response_command_signature;
// Command parser for IdentifyQueryResponse command
EmberAfStatus zcl_decode_identify_cluster_identify_query_response_command (EmberAfClusterCommand * cmd, sl_zcl_identify_cluster_identify_query_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_identify_cluster_identify_query_response_command, (uint8_t*)cmd_struct);
} 


// Signature for ViewGroup command
static const sl_signature_t sig_groups_cluster_view_group_command = sl_zcl_groups_cluster_view_group_command_signature;
// Command parser for ViewGroup command
EmberAfStatus zcl_decode_groups_cluster_view_group_command (EmberAfClusterCommand * cmd, sl_zcl_groups_cluster_view_group_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_groups_cluster_view_group_command, (uint8_t*)cmd_struct);
} 


// Signature for RemoveGroup command
static const sl_signature_t sig_groups_cluster_remove_group_command = sl_zcl_groups_cluster_remove_group_command_signature;
// Command parser for RemoveGroup command
EmberAfStatus zcl_decode_groups_cluster_remove_group_command (EmberAfClusterCommand * cmd, sl_zcl_groups_cluster_remove_group_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_groups_cluster_remove_group_command, (uint8_t*)cmd_struct);
} 


// Signature for RemoveAllScenes command
static const sl_signature_t sig_scenes_cluster_remove_all_scenes_command = sl_zcl_scenes_cluster_remove_all_scenes_command_signature;
// Command parser for RemoveAllScenes command
EmberAfStatus zcl_decode_scenes_cluster_remove_all_scenes_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_remove_all_scenes_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_scenes_cluster_remove_all_scenes_command, (uint8_t*)cmd_struct);
} 


// Signature for GetSceneMembership command
static const sl_signature_t sig_scenes_cluster_get_scene_membership_command = sl_zcl_scenes_cluster_get_scene_membership_command_signature;
// Command parser for GetSceneMembership command
EmberAfStatus zcl_decode_scenes_cluster_get_scene_membership_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_get_scene_membership_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_scenes_cluster_get_scene_membership_command, (uint8_t*)cmd_struct);
} 


// Signature for MoveToClosestFrequency command
static const sl_signature_t sig_level_control_cluster_move_to_closest_frequency_command = sl_zcl_level_control_cluster_move_to_closest_frequency_command_signature;
// Command parser for MoveToClosestFrequency command
EmberAfStatus zcl_decode_level_control_cluster_move_to_closest_frequency_command (EmberAfClusterCommand * cmd, sl_zcl_level_control_cluster_move_to_closest_frequency_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_level_control_cluster_move_to_closest_frequency_command, (uint8_t*)cmd_struct);
} 


// Signature for SetShortPollInterval command
static const sl_signature_t sig_poll_control_cluster_set_short_poll_interval_command = sl_zcl_poll_control_cluster_set_short_poll_interval_command_signature;
// Command parser for SetShortPollInterval command
EmberAfStatus zcl_decode_poll_control_cluster_set_short_poll_interval_command (EmberAfClusterCommand * cmd, sl_zcl_poll_control_cluster_set_short_poll_interval_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_poll_control_cluster_set_short_poll_interval_command, (uint8_t*)cmd_struct);
} 


// Signature for GetLogRecord command
static const sl_signature_t sig_door_lock_cluster_get_log_record_command = sl_zcl_door_lock_cluster_get_log_record_command_signature;
// Command parser for GetLogRecord command
EmberAfStatus zcl_decode_door_lock_cluster_get_log_record_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_log_record_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_get_log_record_command, (uint8_t*)cmd_struct);
} 


// Signature for GetPin command
static const sl_signature_t sig_door_lock_cluster_get_pin_command = sl_zcl_door_lock_cluster_get_pin_command_signature;
// Command parser for GetPin command
EmberAfStatus zcl_decode_door_lock_cluster_get_pin_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_pin_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_get_pin_command, (uint8_t*)cmd_struct);
} 


// Signature for ClearPin command
static const sl_signature_t sig_door_lock_cluster_clear_pin_command = sl_zcl_door_lock_cluster_clear_pin_command_signature;
// Command parser for ClearPin command
EmberAfStatus zcl_decode_door_lock_cluster_clear_pin_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_clear_pin_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_clear_pin_command, (uint8_t*)cmd_struct);
} 


// Signature for GetUserStatus command
static const sl_signature_t sig_door_lock_cluster_get_user_status_command = sl_zcl_door_lock_cluster_get_user_status_command_signature;
// Command parser for GetUserStatus command
EmberAfStatus zcl_decode_door_lock_cluster_get_user_status_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_user_status_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_get_user_status_command, (uint8_t*)cmd_struct);
} 


// Signature for GetUserType command
static const sl_signature_t sig_door_lock_cluster_get_user_type_command = sl_zcl_door_lock_cluster_get_user_type_command_signature;
// Command parser for GetUserType command
EmberAfStatus zcl_decode_door_lock_cluster_get_user_type_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_user_type_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_get_user_type_command, (uint8_t*)cmd_struct);
} 


// Signature for GetRfid command
static const sl_signature_t sig_door_lock_cluster_get_rfid_command = sl_zcl_door_lock_cluster_get_rfid_command_signature;
// Command parser for GetRfid command
EmberAfStatus zcl_decode_door_lock_cluster_get_rfid_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_rfid_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_get_rfid_command, (uint8_t*)cmd_struct);
} 


// Signature for ClearRfid command
static const sl_signature_t sig_door_lock_cluster_clear_rfid_command = sl_zcl_door_lock_cluster_clear_rfid_command_signature;
// Command parser for ClearRfid command
EmberAfStatus zcl_decode_door_lock_cluster_clear_rfid_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_clear_rfid_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_clear_rfid_command, (uint8_t*)cmd_struct);
} 


// Signature for GetDisposableSchedule command
static const sl_signature_t sig_door_lock_cluster_get_disposable_schedule_command = sl_zcl_door_lock_cluster_get_disposable_schedule_command_signature;
// Command parser for GetDisposableSchedule command
EmberAfStatus zcl_decode_door_lock_cluster_get_disposable_schedule_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_disposable_schedule_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_get_disposable_schedule_command, (uint8_t*)cmd_struct);
} 


// Signature for ClearDisposableSchedule command
static const sl_signature_t sig_door_lock_cluster_clear_disposable_schedule_command = sl_zcl_door_lock_cluster_clear_disposable_schedule_command_signature;
// Command parser for ClearDisposableSchedule command
EmberAfStatus zcl_decode_door_lock_cluster_clear_disposable_schedule_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_clear_disposable_schedule_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_clear_disposable_schedule_command, (uint8_t*)cmd_struct);
} 


// Signature for ClearBiometricCredential command
static const sl_signature_t sig_door_lock_cluster_clear_biometric_credential_command = sl_zcl_door_lock_cluster_clear_biometric_credential_command_signature;
// Command parser for ClearBiometricCredential command
EmberAfStatus zcl_decode_door_lock_cluster_clear_biometric_credential_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_clear_biometric_credential_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_clear_biometric_credential_command, (uint8_t*)cmd_struct);
} 


// Signature for WindowCoveringGoToLiftValue command
static const sl_signature_t sig_window_covering_cluster_window_covering_go_to_lift_value_command = sl_zcl_window_covering_cluster_window_covering_go_to_lift_value_command_signature;
// Command parser for WindowCoveringGoToLiftValue command
EmberAfStatus zcl_decode_window_covering_cluster_window_covering_go_to_lift_value_command (EmberAfClusterCommand * cmd, sl_zcl_window_covering_cluster_window_covering_go_to_lift_value_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_window_covering_cluster_window_covering_go_to_lift_value_command, (uint8_t*)cmd_struct);
} 


// Signature for WindowCoveringGoToTiltValue command
static const sl_signature_t sig_window_covering_cluster_window_covering_go_to_tilt_value_command = sl_zcl_window_covering_cluster_window_covering_go_to_tilt_value_command_signature;
// Command parser for WindowCoveringGoToTiltValue command
EmberAfStatus zcl_decode_window_covering_cluster_window_covering_go_to_tilt_value_command (EmberAfClusterCommand * cmd, sl_zcl_window_covering_cluster_window_covering_go_to_tilt_value_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_window_covering_cluster_window_covering_go_to_tilt_value_command, (uint8_t*)cmd_struct);
} 


// Signature for RequestMirrorResponse command
static const sl_signature_t sig_simple_metering_cluster_request_mirror_response_command = sl_zcl_simple_metering_cluster_request_mirror_response_command_signature;
// Command parser for RequestMirrorResponse command
EmberAfStatus zcl_decode_simple_metering_cluster_request_mirror_response_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_request_mirror_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_simple_metering_cluster_request_mirror_response_command, (uint8_t*)cmd_struct);
} 


// Signature for MirrorRemoved command
static const sl_signature_t sig_simple_metering_cluster_mirror_removed_command = sl_zcl_simple_metering_cluster_mirror_removed_command_signature;
// Command parser for MirrorRemoved command
EmberAfStatus zcl_decode_simple_metering_cluster_mirror_removed_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_mirror_removed_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_simple_metering_cluster_mirror_removed_command, (uint8_t*)cmd_struct);
} 


// Signature for StartSamplingResponse command
static const sl_signature_t sig_simple_metering_cluster_start_sampling_response_command = sl_zcl_simple_metering_cluster_start_sampling_response_command_signature;
// Command parser for StartSamplingResponse command
EmberAfStatus zcl_decode_simple_metering_cluster_start_sampling_response_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_start_sampling_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_simple_metering_cluster_start_sampling_response_command, (uint8_t*)cmd_struct);
} 


// Signature for CloseTunnel command
static const sl_signature_t sig_tunneling_cluster_close_tunnel_command = sl_zcl_tunneling_cluster_close_tunnel_command_signature;
// Command parser for CloseTunnel command
EmberAfStatus zcl_decode_tunneling_cluster_close_tunnel_command (EmberAfClusterCommand * cmd, sl_zcl_tunneling_cluster_close_tunnel_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_tunneling_cluster_close_tunnel_command, (uint8_t*)cmd_struct);
} 


// Signature for TunnelClosureNotification command
static const sl_signature_t sig_tunneling_cluster_tunnel_closure_notification_command = sl_zcl_tunneling_cluster_tunnel_closure_notification_command_signature;
// Command parser for TunnelClosureNotification command
EmberAfStatus zcl_decode_tunneling_cluster_tunnel_closure_notification_command (EmberAfClusterCommand * cmd, sl_zcl_tunneling_cluster_tunnel_closure_notification_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_tunneling_cluster_tunnel_closure_notification_command, (uint8_t*)cmd_struct);
} 


// Signature for GetEventConfiguration command
static const sl_signature_t sig_device_management_cluster_get_event_configuration_command = sl_zcl_device_management_cluster_get_event_configuration_command_signature;
// Command parser for GetEventConfiguration command
EmberAfStatus zcl_decode_device_management_cluster_get_event_configuration_command (EmberAfClusterCommand * cmd, sl_zcl_device_management_cluster_get_event_configuration_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_device_management_cluster_get_event_configuration_command, (uint8_t*)cmd_struct);
} 


// Signature for ConfigureSetRootId command
static const sl_signature_t sig_information_cluster_configure_set_root_id_command = sl_zcl_information_cluster_configure_set_root_id_command_signature;
// Command parser for ConfigureSetRootId command
EmberAfStatus zcl_decode_information_cluster_configure_set_root_id_command (EmberAfClusterCommand * cmd, sl_zcl_information_cluster_configure_set_root_id_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_information_cluster_configure_set_root_id_command, (uint8_t*)cmd_struct);
} 


// Signature for SwitchChairmanRequest command
static const sl_signature_t sig_chatting_cluster_switch_chairman_request_command = sl_zcl_chatting_cluster_switch_chairman_request_command_signature;
// Command parser for SwitchChairmanRequest command
EmberAfStatus zcl_decode_chatting_cluster_switch_chairman_request_command (EmberAfClusterCommand * cmd, sl_zcl_chatting_cluster_switch_chairman_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_chatting_cluster_switch_chairman_request_command, (uint8_t*)cmd_struct);
} 


// Signature for ReadTokens command
static const sl_signature_t sig_configuration_cluster_cluster_read_tokens_command = sl_zcl_configuration_cluster_cluster_read_tokens_command_signature;
// Command parser for ReadTokens command
EmberAfStatus zcl_decode_configuration_cluster_cluster_read_tokens_command (EmberAfClusterCommand * cmd, sl_zcl_configuration_cluster_cluster_read_tokens_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_configuration_cluster_cluster_read_tokens_command, (uint8_t*)cmd_struct);
} 


// Signature for EnablePeriodicRouterCheckIns command
static const sl_signature_t sig_sl_works_with_all_hubs_cluster_enable_periodic_router_check_ins_command = sl_zcl_sl_works_with_all_hubs_cluster_enable_periodic_router_check_ins_command_signature;
// Command parser for EnablePeriodicRouterCheckIns command
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_enable_periodic_router_check_ins_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_enable_periodic_router_check_ins_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_sl_works_with_all_hubs_cluster_enable_periodic_router_check_ins_command, (uint8_t*)cmd_struct);
} 


// Signature for RelayStatusLog command
static const sl_signature_t sig_thermostat_cluster_relay_status_log_command = sl_zcl_thermostat_cluster_relay_status_log_command_signature;
// Command parser for RelayStatusLog command
EmberAfStatus zcl_decode_thermostat_cluster_relay_status_log_command (EmberAfClusterCommand * cmd, sl_zcl_thermostat_cluster_relay_status_log_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_thermostat_cluster_relay_status_log_command, (uint8_t*)cmd_struct);
} 


// Signature for JoinGame command
static const sl_signature_t sig_gaming_cluster_join_game_command = sl_zcl_gaming_cluster_join_game_command_signature;
// Command parser for JoinGame command
EmberAfStatus zcl_decode_gaming_cluster_join_game_command (EmberAfClusterCommand * cmd, sl_zcl_gaming_cluster_join_game_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_gaming_cluster_join_game_command, (uint8_t*)cmd_struct);
} 


// Signature for GameAnnouncement command
static const sl_signature_t sig_gaming_cluster_game_announcement_command = sl_zcl_gaming_cluster_game_announcement_command_signature;
// Command parser for GameAnnouncement command
EmberAfStatus zcl_decode_gaming_cluster_game_announcement_command (EmberAfClusterCommand * cmd, sl_zcl_gaming_cluster_game_announcement_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_gaming_cluster_game_announcement_command, (uint8_t*)cmd_struct);
} 


// Signature for AddGroup command
static const sl_signature_t sig_groups_cluster_add_group_command = sl_zcl_groups_cluster_add_group_command_signature;
// Command parser for AddGroup command
EmberAfStatus zcl_decode_groups_cluster_add_group_command (EmberAfClusterCommand * cmd, sl_zcl_groups_cluster_add_group_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_groups_cluster_add_group_command, (uint8_t*)cmd_struct);
} 


// Signature for AddGroupIfIdentifying command
static const sl_signature_t sig_groups_cluster_add_group_if_identifying_command = sl_zcl_groups_cluster_add_group_if_identifying_command_signature;
// Command parser for AddGroupIfIdentifying command
EmberAfStatus zcl_decode_groups_cluster_add_group_if_identifying_command (EmberAfClusterCommand * cmd, sl_zcl_groups_cluster_add_group_if_identifying_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_groups_cluster_add_group_if_identifying_command, (uint8_t*)cmd_struct);
} 


// Signature for JoinChatRequest command
static const sl_signature_t sig_chatting_cluster_join_chat_request_command = sl_zcl_chatting_cluster_join_chat_request_command_signature;
// Command parser for JoinChatRequest command
EmberAfStatus zcl_decode_chatting_cluster_join_chat_request_command (EmberAfClusterCommand * cmd, sl_zcl_chatting_cluster_join_chat_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_chatting_cluster_join_chat_request_command, (uint8_t*)cmd_struct);
} 


// Signature for UnlockWithTimeout command
static const sl_signature_t sig_door_lock_cluster_unlock_with_timeout_command = sl_zcl_door_lock_cluster_unlock_with_timeout_command_signature;
// Command parser for UnlockWithTimeout command
EmberAfStatus zcl_decode_door_lock_cluster_unlock_with_timeout_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_unlock_with_timeout_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_unlock_with_timeout_command, (uint8_t*)cmd_struct);
} 


// Signature for SetUserType command
static const sl_signature_t sig_door_lock_cluster_set_user_type_command = sl_zcl_door_lock_cluster_set_user_type_command_signature;
// Command parser for SetUserType command
EmberAfStatus zcl_decode_door_lock_cluster_set_user_type_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_set_user_type_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_set_user_type_command, (uint8_t*)cmd_struct);
} 


// Signature for GetUserTypeResponse command
static const sl_signature_t sig_door_lock_cluster_get_user_type_response_command = sl_zcl_door_lock_cluster_get_user_type_response_command_signature;
// Command parser for GetUserTypeResponse command
EmberAfStatus zcl_decode_door_lock_cluster_get_user_type_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_user_type_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_get_user_type_response_command, (uint8_t*)cmd_struct);
} 


// Signature for TransferDataErrorClientToServer command
static const sl_signature_t sig_tunneling_cluster_transfer_data_error_client_to_server_command = sl_zcl_tunneling_cluster_transfer_data_error_client_to_server_command_signature;
// Command parser for TransferDataErrorClientToServer command
EmberAfStatus zcl_decode_tunneling_cluster_transfer_data_error_client_to_server_command (EmberAfClusterCommand * cmd, sl_zcl_tunneling_cluster_transfer_data_error_client_to_server_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_tunneling_cluster_transfer_data_error_client_to_server_command, (uint8_t*)cmd_struct);
} 


// Signature for TransferDataErrorServerToClient command
static const sl_signature_t sig_tunneling_cluster_transfer_data_error_server_to_client_command = sl_zcl_tunneling_cluster_transfer_data_error_server_to_client_command_signature;
// Command parser for TransferDataErrorServerToClient command
EmberAfStatus zcl_decode_tunneling_cluster_transfer_data_error_server_to_client_command (EmberAfClusterCommand * cmd, sl_zcl_tunneling_cluster_transfer_data_error_server_to_client_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_tunneling_cluster_transfer_data_error_server_to_client_command, (uint8_t*)cmd_struct);
} 


// Signature for SetPin command
static const sl_signature_t sig_door_lock_cluster_set_pin_command = sl_zcl_door_lock_cluster_set_pin_command_signature;
// Command parser for SetPin command
EmberAfStatus zcl_decode_door_lock_cluster_set_pin_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_set_pin_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_set_pin_command, (uint8_t*)cmd_struct);
} 


// Signature for SetRfid command
static const sl_signature_t sig_door_lock_cluster_set_rfid_command = sl_zcl_door_lock_cluster_set_rfid_command_signature;
// Command parser for SetRfid command
EmberAfStatus zcl_decode_door_lock_cluster_set_rfid_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_set_rfid_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_set_rfid_command, (uint8_t*)cmd_struct);
} 


// Signature for GetPinResponse command
static const sl_signature_t sig_door_lock_cluster_get_pin_response_command = sl_zcl_door_lock_cluster_get_pin_response_command_signature;
// Command parser for GetPinResponse command
EmberAfStatus zcl_decode_door_lock_cluster_get_pin_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_pin_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_get_pin_response_command, (uint8_t*)cmd_struct);
} 


// Signature for GetRfidResponse command
static const sl_signature_t sig_door_lock_cluster_get_rfid_response_command = sl_zcl_door_lock_cluster_get_rfid_response_command_signature;
// Command parser for GetRfidResponse command
EmberAfStatus zcl_decode_door_lock_cluster_get_rfid_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_rfid_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_get_rfid_response_command, (uint8_t*)cmd_struct);
} 


// Signature for EnhancedMoveToHue command
static const sl_signature_t sig_color_control_cluster_enhanced_move_to_hue_command = sl_zcl_color_control_cluster_enhanced_move_to_hue_command_signature;
// Command parser for EnhancedMoveToHue command
EmberAfStatus zcl_decode_color_control_cluster_enhanced_move_to_hue_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_enhanced_move_to_hue_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_color_control_cluster_enhanced_move_to_hue_command, (uint8_t*)cmd_struct);
} 


// Signature for RequestTunnelResponse command
static const sl_signature_t sig_tunneling_cluster_request_tunnel_response_command = sl_zcl_tunneling_cluster_request_tunnel_response_command_signature;
// Command parser for RequestTunnelResponse command
EmberAfStatus zcl_decode_tunneling_cluster_request_tunnel_response_command (EmberAfClusterCommand * cmd, sl_zcl_tunneling_cluster_request_tunnel_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_tunneling_cluster_request_tunnel_response_command, (uint8_t*)cmd_struct);
} 


// Signature for WriteFunctions command
static const sl_signature_t sig_appliance_control_cluster_write_functions_command = sl_zcl_appliance_control_cluster_write_functions_command_signature;
// Command parser for WriteFunctions command
EmberAfStatus zcl_decode_appliance_control_cluster_write_functions_command (EmberAfClusterCommand * cmd, sl_zcl_appliance_control_cluster_write_functions_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_appliance_control_cluster_write_functions_command, (uint8_t*)cmd_struct);
} 


// Signature for AckTransferDataClientToServer command
static const sl_signature_t sig_tunneling_cluster_ack_transfer_data_client_to_server_command = sl_zcl_tunneling_cluster_ack_transfer_data_client_to_server_command_signature;
// Command parser for AckTransferDataClientToServer command
EmberAfStatus zcl_decode_tunneling_cluster_ack_transfer_data_client_to_server_command (EmberAfClusterCommand * cmd, sl_zcl_tunneling_cluster_ack_transfer_data_client_to_server_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_tunneling_cluster_ack_transfer_data_client_to_server_command, (uint8_t*)cmd_struct);
} 


// Signature for ReadyDataClientToServer command
static const sl_signature_t sig_tunneling_cluster_ready_data_client_to_server_command = sl_zcl_tunneling_cluster_ready_data_client_to_server_command_signature;
// Command parser for ReadyDataClientToServer command
EmberAfStatus zcl_decode_tunneling_cluster_ready_data_client_to_server_command (EmberAfClusterCommand * cmd, sl_zcl_tunneling_cluster_ready_data_client_to_server_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_tunneling_cluster_ready_data_client_to_server_command, (uint8_t*)cmd_struct);
} 


// Signature for AckTransferDataServerToClient command
static const sl_signature_t sig_tunneling_cluster_ack_transfer_data_server_to_client_command = sl_zcl_tunneling_cluster_ack_transfer_data_server_to_client_command_signature;
// Command parser for AckTransferDataServerToClient command
EmberAfStatus zcl_decode_tunneling_cluster_ack_transfer_data_server_to_client_command (EmberAfClusterCommand * cmd, sl_zcl_tunneling_cluster_ack_transfer_data_server_to_client_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_tunneling_cluster_ack_transfer_data_server_to_client_command, (uint8_t*)cmd_struct);
} 


// Signature for ReadyDataServerToClient command
static const sl_signature_t sig_tunneling_cluster_ready_data_server_to_client_command = sl_zcl_tunneling_cluster_ready_data_server_to_client_command_signature;
// Command parser for ReadyDataServerToClient command
EmberAfStatus zcl_decode_tunneling_cluster_ready_data_server_to_client_command (EmberAfClusterCommand * cmd, sl_zcl_tunneling_cluster_ready_data_server_to_client_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_tunneling_cluster_ready_data_server_to_client_command, (uint8_t*)cmd_struct);
} 


// Signature for LeaveChatRequest command
static const sl_signature_t sig_chatting_cluster_leave_chat_request_command = sl_zcl_chatting_cluster_leave_chat_request_command_signature;
// Command parser for LeaveChatRequest command
EmberAfStatus zcl_decode_chatting_cluster_leave_chat_request_command (EmberAfClusterCommand * cmd, sl_zcl_chatting_cluster_leave_chat_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_chatting_cluster_leave_chat_request_command, (uint8_t*)cmd_struct);
} 


// Signature for SwitchChairmanResponse command
static const sl_signature_t sig_chatting_cluster_switch_chairman_response_command = sl_zcl_chatting_cluster_switch_chairman_response_command_signature;
// Command parser for SwitchChairmanResponse command
EmberAfStatus zcl_decode_chatting_cluster_switch_chairman_response_command (EmberAfClusterCommand * cmd, sl_zcl_chatting_cluster_switch_chairman_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_chatting_cluster_switch_chairman_response_command, (uint8_t*)cmd_struct);
} 


// Signature for GetNodeInformationRequest command
static const sl_signature_t sig_chatting_cluster_get_node_information_request_command = sl_zcl_chatting_cluster_get_node_information_request_command_signature;
// Command parser for GetNodeInformationRequest command
EmberAfStatus zcl_decode_chatting_cluster_get_node_information_request_command (EmberAfClusterCommand * cmd, sl_zcl_chatting_cluster_get_node_information_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_chatting_cluster_get_node_information_request_command, (uint8_t*)cmd_struct);
} 


// Signature for MoveToColorTemperature command
static const sl_signature_t sig_color_control_cluster_move_to_color_temperature_command = sl_zcl_color_control_cluster_move_to_color_temperature_command_signature;
// Command parser for MoveToColorTemperature command
EmberAfStatus zcl_decode_color_control_cluster_move_to_color_temperature_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_move_to_color_temperature_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_color_control_cluster_move_to_color_temperature_command, (uint8_t*)cmd_struct);
} 


// Signature for UserLeft command
static const sl_signature_t sig_chatting_cluster_user_left_command = sl_zcl_chatting_cluster_user_left_command_signature;
// Command parser for UserLeft command
EmberAfStatus zcl_decode_chatting_cluster_user_left_command (EmberAfClusterCommand * cmd, sl_zcl_chatting_cluster_user_left_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_chatting_cluster_user_left_command, (uint8_t*)cmd_struct);
} 


// Signature for UserJoined command
static const sl_signature_t sig_chatting_cluster_user_joined_command = sl_zcl_chatting_cluster_user_joined_command_signature;
// Command parser for UserJoined command
EmberAfStatus zcl_decode_chatting_cluster_user_joined_command (EmberAfClusterCommand * cmd, sl_zcl_chatting_cluster_user_joined_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_chatting_cluster_user_joined_command, (uint8_t*)cmd_struct);
} 


// Signature for SwitchChairmanNotification command
static const sl_signature_t sig_chatting_cluster_switch_chairman_notification_command = sl_zcl_chatting_cluster_switch_chairman_notification_command_signature;
// Command parser for SwitchChairmanNotification command
EmberAfStatus zcl_decode_chatting_cluster_switch_chairman_notification_command (EmberAfClusterCommand * cmd, sl_zcl_chatting_cluster_switch_chairman_notification_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_chatting_cluster_switch_chairman_notification_command, (uint8_t*)cmd_struct);
} 


// Signature for ModifyRecordRequest command
static const sl_signature_t sig_data_sharing_cluster_modify_record_request_command = sl_zcl_data_sharing_cluster_modify_record_request_command_signature;
// Command parser for ModifyRecordRequest command
EmberAfStatus zcl_decode_data_sharing_cluster_modify_record_request_command (EmberAfClusterCommand * cmd, sl_zcl_data_sharing_cluster_modify_record_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_data_sharing_cluster_modify_record_request_command, (uint8_t*)cmd_struct);
} 


// Signature for MoveToColor command
static const sl_signature_t sig_color_control_cluster_move_to_color_command = sl_zcl_color_control_cluster_move_to_color_command_signature;
// Command parser for MoveToColor command
EmberAfStatus zcl_decode_color_control_cluster_move_to_color_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_move_to_color_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_color_control_cluster_move_to_color_command, (uint8_t*)cmd_struct);
} 


// Signature for ChatMessage command
static const sl_signature_t sig_chatting_cluster_chat_message_command = sl_zcl_chatting_cluster_chat_message_command_signature;
// Command parser for ChatMessage command
EmberAfStatus zcl_decode_chatting_cluster_chat_message_command (EmberAfClusterCommand * cmd, sl_zcl_chatting_cluster_chat_message_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_chatting_cluster_chat_message_command, (uint8_t*)cmd_struct);
} 


// Signature for EnableWwahRejoinAlgorithm command
static const sl_signature_t sig_sl_works_with_all_hubs_cluster_enable_wwah_rejoin_algorithm_command = sl_zcl_sl_works_with_all_hubs_cluster_enable_wwah_rejoin_algorithm_command_signature;
// Command parser for EnableWwahRejoinAlgorithm command
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_enable_wwah_rejoin_algorithm_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_enable_wwah_rejoin_algorithm_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_sl_works_with_all_hubs_cluster_enable_wwah_rejoin_algorithm_command, (uint8_t*)cmd_struct);
} 


// Signature for UpgradeEndResponse command
static const sl_signature_t sig_over_the_air_bootloading_cluster_upgrade_end_response_command = sl_zcl_over_the_air_bootloading_cluster_upgrade_end_response_command_signature;
// Command parser for UpgradeEndResponse command
EmberAfStatus zcl_decode_over_the_air_bootloading_cluster_upgrade_end_response_command (EmberAfClusterCommand * cmd, sl_zcl_over_the_air_bootloading_cluster_upgrade_end_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_over_the_air_bootloading_cluster_upgrade_end_response_command, (uint8_t*)cmd_struct);
} 


// Signature for GetMeasurementProfileCommand command
static const sl_signature_t sig_electrical_measurement_cluster_get_measurement_profile_command_command = sl_zcl_electrical_measurement_cluster_get_measurement_profile_command_command_signature;
// Command parser for GetMeasurementProfileCommand command
EmberAfStatus zcl_decode_electrical_measurement_cluster_get_measurement_profile_command_command (EmberAfClusterCommand * cmd, sl_zcl_electrical_measurement_cluster_get_measurement_profile_command_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_electrical_measurement_cluster_get_measurement_profile_command_command, (uint8_t*)cmd_struct);
} 


// Signature for GetLogRecordResponse command
static const sl_signature_t sig_door_lock_cluster_get_log_record_response_command = sl_zcl_door_lock_cluster_get_log_record_response_command_signature;
// Command parser for GetLogRecordResponse command
EmberAfStatus zcl_decode_door_lock_cluster_get_log_record_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_log_record_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_get_log_record_response_command, (uint8_t*)cmd_struct);
} 


// Signature for SetDisposableSchedule command
static const sl_signature_t sig_door_lock_cluster_set_disposable_schedule_command = sl_zcl_door_lock_cluster_set_disposable_schedule_command_signature;
// Command parser for SetDisposableSchedule command
EmberAfStatus zcl_decode_door_lock_cluster_set_disposable_schedule_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_set_disposable_schedule_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_set_disposable_schedule_command, (uint8_t*)cmd_struct);
} 


// Signature for ModifyFileRequest command
static const sl_signature_t sig_data_sharing_cluster_modify_file_request_command = sl_zcl_data_sharing_cluster_modify_file_request_command_signature;
// Command parser for ModifyFileRequest command
EmberAfStatus zcl_decode_data_sharing_cluster_modify_file_request_command (EmberAfClusterCommand * cmd, sl_zcl_data_sharing_cluster_modify_file_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_data_sharing_cluster_modify_file_request_command, (uint8_t*)cmd_struct);
} 


// Signature for GetOverallSchedulePriceResponse command
static const sl_signature_t sig_power_profile_cluster_get_overall_schedule_price_response_command = sl_zcl_power_profile_cluster_get_overall_schedule_price_response_command_signature;
// Command parser for GetOverallSchedulePriceResponse command
EmberAfStatus zcl_decode_power_profile_cluster_get_overall_schedule_price_response_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_get_overall_schedule_price_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_power_profile_cluster_get_overall_schedule_price_response_command, (uint8_t*)cmd_struct);
} 


// Signature for ViewScene command
static const sl_signature_t sig_scenes_cluster_view_scene_command = sl_zcl_scenes_cluster_view_scene_command_signature;
// Command parser for ViewScene command
EmberAfStatus zcl_decode_scenes_cluster_view_scene_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_view_scene_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_scenes_cluster_view_scene_command, (uint8_t*)cmd_struct);
} 


// Signature for RemoveScene command
static const sl_signature_t sig_scenes_cluster_remove_scene_command = sl_zcl_scenes_cluster_remove_scene_command_signature;
// Command parser for RemoveScene command
EmberAfStatus zcl_decode_scenes_cluster_remove_scene_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_remove_scene_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_scenes_cluster_remove_scene_command, (uint8_t*)cmd_struct);
} 


// Signature for StoreScene command
static const sl_signature_t sig_scenes_cluster_store_scene_command = sl_zcl_scenes_cluster_store_scene_command_signature;
// Command parser for StoreScene command
EmberAfStatus zcl_decode_scenes_cluster_store_scene_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_store_scene_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_scenes_cluster_store_scene_command, (uint8_t*)cmd_struct);
} 


// Signature for EnhancedViewScene command
static const sl_signature_t sig_scenes_cluster_enhanced_view_scene_command = sl_zcl_scenes_cluster_enhanced_view_scene_command_signature;
// Command parser for EnhancedViewScene command
EmberAfStatus zcl_decode_scenes_cluster_enhanced_view_scene_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_enhanced_view_scene_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_scenes_cluster_enhanced_view_scene_command, (uint8_t*)cmd_struct);
} 


// Signature for SetUserStatus command
static const sl_signature_t sig_door_lock_cluster_set_user_status_command = sl_zcl_door_lock_cluster_set_user_status_command_signature;
// Command parser for SetUserStatus command
EmberAfStatus zcl_decode_door_lock_cluster_set_user_status_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_set_user_status_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_set_user_status_command, (uint8_t*)cmd_struct);
} 


// Signature for GetUserStatusResponse command
static const sl_signature_t sig_door_lock_cluster_get_user_status_response_command = sl_zcl_door_lock_cluster_get_user_status_response_command_signature;
// Command parser for GetUserStatusResponse command
EmberAfStatus zcl_decode_door_lock_cluster_get_user_status_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_user_status_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_get_user_status_response_command, (uint8_t*)cmd_struct);
} 


// Signature for EnhancedMoveToHueAndSaturation command
static const sl_signature_t sig_color_control_cluster_enhanced_move_to_hue_and_saturation_command = sl_zcl_color_control_cluster_enhanced_move_to_hue_and_saturation_command_signature;
// Command parser for EnhancedMoveToHueAndSaturation command
EmberAfStatus zcl_decode_color_control_cluster_enhanced_move_to_hue_and_saturation_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_enhanced_move_to_hue_and_saturation_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_color_control_cluster_enhanced_move_to_hue_and_saturation_command, (uint8_t*)cmd_struct);
} 


// Signature for AddScene command
static const sl_signature_t sig_scenes_cluster_add_scene_command = sl_zcl_scenes_cluster_add_scene_command_signature;
// Command parser for AddScene command
EmberAfStatus zcl_decode_scenes_cluster_add_scene_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_add_scene_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_scenes_cluster_add_scene_command, (uint8_t*)cmd_struct);
} 


// Signature for EnhancedAddScene command
static const sl_signature_t sig_scenes_cluster_enhanced_add_scene_command = sl_zcl_scenes_cluster_enhanced_add_scene_command_signature;
// Command parser for EnhancedAddScene command
EmberAfStatus zcl_decode_scenes_cluster_enhanced_add_scene_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_enhanced_add_scene_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_scenes_cluster_enhanced_add_scene_command, (uint8_t*)cmd_struct);
} 


// Signature for RecallScene command
static const sl_signature_t sig_scenes_cluster_recall_scene_command = sl_zcl_scenes_cluster_recall_scene_command_signature;
// Command parser for RecallScene command
EmberAfStatus zcl_decode_scenes_cluster_recall_scene_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_recall_scene_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_scenes_cluster_recall_scene_command, (uint8_t*)cmd_struct);
} 


// Signature for GetDisposableScheduleResponse command
static const sl_signature_t sig_door_lock_cluster_get_disposable_schedule_response_command = sl_zcl_door_lock_cluster_get_disposable_schedule_response_command_signature;
// Command parser for GetDisposableScheduleResponse command
EmberAfStatus zcl_decode_door_lock_cluster_get_disposable_schedule_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_disposable_schedule_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_get_disposable_schedule_response_command, (uint8_t*)cmd_struct);
} 


// Signature for PublishEventLog command
static const sl_signature_t sig_events_cluster_publish_event_log_command = sl_zcl_events_cluster_publish_event_log_command_signature;
// Command parser for PublishEventLog command
EmberAfStatus zcl_decode_events_cluster_publish_event_log_command (EmberAfClusterCommand * cmd, sl_zcl_events_cluster_publish_event_log_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_events_cluster_publish_event_log_command, (uint8_t*)cmd_struct);
} 


// Signature for TransferDataClientToServer command
static const sl_signature_t sig_tunneling_cluster_transfer_data_client_to_server_command = sl_zcl_tunneling_cluster_transfer_data_client_to_server_command_signature;
// Command parser for TransferDataClientToServer command
EmberAfStatus zcl_decode_tunneling_cluster_transfer_data_client_to_server_command (EmberAfClusterCommand * cmd, sl_zcl_tunneling_cluster_transfer_data_client_to_server_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_tunneling_cluster_transfer_data_client_to_server_command, (uint8_t*)cmd_struct);
} 


// Signature for TransferDataServerToClient command
static const sl_signature_t sig_tunneling_cluster_transfer_data_server_to_client_command = sl_zcl_tunneling_cluster_transfer_data_server_to_client_command_signature;
// Command parser for TransferDataServerToClient command
EmberAfStatus zcl_decode_tunneling_cluster_transfer_data_server_to_client_command (EmberAfClusterCommand * cmd, sl_zcl_tunneling_cluster_transfer_data_server_to_client_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_tunneling_cluster_transfer_data_server_to_client_command, (uint8_t*)cmd_struct);
} 


// Signature for SendPreference command
static const sl_signature_t sig_information_cluster_send_preference_command = sl_zcl_information_cluster_send_preference_command_signature;
// Command parser for SendPreference command
EmberAfStatus zcl_decode_information_cluster_send_preference_command (EmberAfClusterCommand * cmd, sl_zcl_information_cluster_send_preference_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_information_cluster_send_preference_command, (uint8_t*)cmd_struct);
} 


// Signature for ReadFileRequest command
static const sl_signature_t sig_data_sharing_cluster_read_file_request_command = sl_zcl_data_sharing_cluster_read_file_request_command_signature;
// Command parser for ReadFileRequest command
EmberAfStatus zcl_decode_data_sharing_cluster_read_file_request_command (EmberAfClusterCommand * cmd, sl_zcl_data_sharing_cluster_read_file_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_data_sharing_cluster_read_file_request_command, (uint8_t*)cmd_struct);
} 


// Signature for ReadRecordRequest command
static const sl_signature_t sig_data_sharing_cluster_read_record_request_command = sl_zcl_data_sharing_cluster_read_record_request_command_signature;
// Command parser for ReadRecordRequest command
EmberAfStatus zcl_decode_data_sharing_cluster_read_record_request_command (EmberAfClusterCommand * cmd, sl_zcl_data_sharing_cluster_read_record_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_data_sharing_cluster_read_record_request_command, (uint8_t*)cmd_struct);
} 


// Signature for SetToken command
static const sl_signature_t sig_configuration_cluster_cluster_set_token_command = sl_zcl_configuration_cluster_cluster_set_token_command_signature;
// Command parser for SetToken command
EmberAfStatus zcl_decode_configuration_cluster_cluster_set_token_command (EmberAfClusterCommand * cmd, sl_zcl_configuration_cluster_cluster_set_token_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_configuration_cluster_cluster_set_token_command, (uint8_t*)cmd_struct);
} 


// Signature for ReturnToken command
static const sl_signature_t sig_configuration_cluster_cluster_return_token_command = sl_zcl_configuration_cluster_cluster_return_token_command_signature;
// Command parser for ReturnToken command
EmberAfStatus zcl_decode_configuration_cluster_cluster_return_token_command (EmberAfClusterCommand * cmd, sl_zcl_configuration_cluster_cluster_return_token_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_configuration_cluster_cluster_return_token_command, (uint8_t*)cmd_struct);
} 


// Signature for SwitchChairmanConfirm command
static const sl_signature_t sig_chatting_cluster_switch_chairman_confirm_command = sl_zcl_chatting_cluster_switch_chairman_confirm_command_signature;
// Command parser for SwitchChairmanConfirm command
EmberAfStatus zcl_decode_chatting_cluster_switch_chairman_confirm_command (EmberAfClusterCommand * cmd, sl_zcl_chatting_cluster_switch_chairman_confirm_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_chatting_cluster_switch_chairman_confirm_command, (uint8_t*)cmd_struct);
} 


// Signature for GetSampledData command
static const sl_signature_t sig_simple_metering_cluster_get_sampled_data_command = sl_zcl_simple_metering_cluster_get_sampled_data_command_signature;
// Command parser for GetSampledData command
EmberAfStatus zcl_decode_simple_metering_cluster_get_sampled_data_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_get_sampled_data_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_simple_metering_cluster_get_sampled_data_command, (uint8_t*)cmd_struct);
} 


// Signature for GetSampledDataResponse command
static const sl_signature_t sig_simple_metering_cluster_get_sampled_data_response_command = sl_zcl_simple_metering_cluster_get_sampled_data_response_command_signature;
// Command parser for GetSampledDataResponse command
EmberAfStatus zcl_decode_simple_metering_cluster_get_sampled_data_response_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_get_sampled_data_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_simple_metering_cluster_get_sampled_data_response_command, (uint8_t*)cmd_struct);
} 


// Signature for SetLongPollInterval command
static const sl_signature_t sig_poll_control_cluster_set_long_poll_interval_command = sl_zcl_poll_control_cluster_set_long_poll_interval_command_signature;
// Command parser for SetLongPollInterval command
EmberAfStatus zcl_decode_poll_control_cluster_set_long_poll_interval_command (EmberAfClusterCommand * cmd, sl_zcl_poll_control_cluster_set_long_poll_interval_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_poll_control_cluster_set_long_poll_interval_command, (uint8_t*)cmd_struct);
} 


// Signature for GetPriceMatrix command
static const sl_signature_t sig_price_cluster_get_price_matrix_command = sl_zcl_price_cluster_get_price_matrix_command_signature;
// Command parser for GetPriceMatrix command
EmberAfStatus zcl_decode_price_cluster_get_price_matrix_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_get_price_matrix_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_price_cluster_get_price_matrix_command, (uint8_t*)cmd_struct);
} 


// Signature for GetBlockThresholds command
static const sl_signature_t sig_price_cluster_get_block_thresholds_command = sl_zcl_price_cluster_get_block_thresholds_command_signature;
// Command parser for GetBlockThresholds command
EmberAfStatus zcl_decode_price_cluster_get_block_thresholds_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_get_block_thresholds_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_price_cluster_get_block_thresholds_command, (uint8_t*)cmd_struct);
} 


// Signature for GetTierLabels command
static const sl_signature_t sig_price_cluster_get_tier_labels_command = sl_zcl_price_cluster_get_tier_labels_command_signature;
// Command parser for GetTierLabels command
EmberAfStatus zcl_decode_price_cluster_get_tier_labels_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_get_tier_labels_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_price_cluster_get_tier_labels_command, (uint8_t*)cmd_struct);
} 


// Signature for SetLowCreditWarningLevel command
static const sl_signature_t sig_prepayment_cluster_set_low_credit_warning_level_command = sl_zcl_prepayment_cluster_set_low_credit_warning_level_command_signature;
// Command parser for SetLowCreditWarningLevel command
EmberAfStatus zcl_decode_prepayment_cluster_set_low_credit_warning_level_command (EmberAfClusterCommand * cmd, sl_zcl_prepayment_cluster_set_low_credit_warning_level_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_prepayment_cluster_set_low_credit_warning_level_command, (uint8_t*)cmd_struct);
} 


// Signature for ConfigurePushInformationTimer command
static const sl_signature_t sig_information_cluster_configure_push_information_timer_command = sl_zcl_information_cluster_configure_push_information_timer_command_signature;
// Command parser for ConfigurePushInformationTimer command
EmberAfStatus zcl_decode_information_cluster_configure_push_information_timer_command (EmberAfClusterCommand * cmd, sl_zcl_information_cluster_configure_push_information_timer_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_information_cluster_configure_push_information_timer_command, (uint8_t*)cmd_struct);
} 


// Signature for LogRequest command
static const sl_signature_t sig_appliance_statistics_cluster_log_request_command = sl_zcl_appliance_statistics_cluster_log_request_command_signature;
// Command parser for LogRequest command
EmberAfStatus zcl_decode_appliance_statistics_cluster_log_request_command (EmberAfClusterCommand * cmd, sl_zcl_appliance_statistics_cluster_log_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_appliance_statistics_cluster_log_request_command, (uint8_t*)cmd_struct);
} 


// Signature for ResetToFactoryNewRequest command
static const sl_signature_t sig_zll_commissioning_cluster_reset_to_factory_new_request_command = sl_zcl_zll_commissioning_cluster_reset_to_factory_new_request_command_signature;
// Command parser for ResetToFactoryNewRequest command
EmberAfStatus zcl_decode_zll_commissioning_cluster_reset_to_factory_new_request_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_reset_to_factory_new_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_zll_commissioning_cluster_reset_to_factory_new_request_command, (uint8_t*)cmd_struct);
} 


// Signature for CancelLoadControlEvent command
static const sl_signature_t sig_demand_response_and_load_control_cluster_cancel_load_control_event_command = sl_zcl_demand_response_and_load_control_cluster_cancel_load_control_event_command_signature;
// Command parser for CancelLoadControlEvent command
EmberAfStatus zcl_decode_demand_response_and_load_control_cluster_cancel_load_control_event_command (EmberAfClusterCommand * cmd, sl_zcl_demand_response_and_load_control_cluster_cancel_load_control_event_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_demand_response_and_load_control_cluster_cancel_load_control_event_command, (uint8_t*)cmd_struct);
} 


// Signature for ManageEvent command
static const sl_signature_t sig_energy_management_cluster_manage_event_command = sl_zcl_energy_management_cluster_manage_event_command_signature;
// Command parser for ManageEvent command
EmberAfStatus zcl_decode_energy_management_cluster_manage_event_command (EmberAfClusterCommand * cmd, sl_zcl_energy_management_cluster_manage_event_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_energy_management_cluster_manage_event_command, (uint8_t*)cmd_struct);
} 


// Signature for LoadControlEvent command
static const sl_signature_t sig_demand_response_and_load_control_cluster_load_control_event_command = sl_zcl_demand_response_and_load_control_cluster_load_control_event_command_signature;
// Command parser for LoadControlEvent command
EmberAfStatus zcl_decode_demand_response_and_load_control_cluster_load_control_event_command (EmberAfClusterCommand * cmd, sl_zcl_demand_response_and_load_control_cluster_load_control_event_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_demand_response_and_load_control_cluster_load_control_event_command, (uint8_t*)cmd_struct);
} 


// Signature for CancelMessage command
static const sl_signature_t sig_messaging_cluster_cancel_message_command = sl_zcl_messaging_cluster_cancel_message_command_signature;
// Command parser for CancelMessage command
EmberAfStatus zcl_decode_messaging_cluster_cancel_message_command (EmberAfClusterCommand * cmd, sl_zcl_messaging_cluster_cancel_message_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_messaging_cluster_cancel_message_command, (uint8_t*)cmd_struct);
} 


// Signature for ScanRequest command
static const sl_signature_t sig_zll_commissioning_cluster_scan_request_command = sl_zcl_zll_commissioning_cluster_scan_request_command_signature;
// Command parser for ScanRequest command
EmberAfStatus zcl_decode_zll_commissioning_cluster_scan_request_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_scan_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_zll_commissioning_cluster_scan_request_command, (uint8_t*)cmd_struct);
} 


// Signature for DisplayProtectedMessage command
static const sl_signature_t sig_messaging_cluster_display_protected_message_command = sl_zcl_messaging_cluster_display_protected_message_command_signature;
// Command parser for DisplayProtectedMessage command
EmberAfStatus zcl_decode_messaging_cluster_display_protected_message_command (EmberAfClusterCommand * cmd, sl_zcl_messaging_cluster_display_protected_message_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_messaging_cluster_display_protected_message_command, (uint8_t*)cmd_struct);
} 


// Signature for DisplayMessage command
static const sl_signature_t sig_messaging_cluster_display_message_command = sl_zcl_messaging_cluster_display_message_command_signature;
// Command parser for DisplayMessage command
EmberAfStatus zcl_decode_messaging_cluster_display_message_command (EmberAfClusterCommand * cmd, sl_zcl_messaging_cluster_display_message_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_messaging_cluster_display_message_command, (uint8_t*)cmd_struct);
} 


// Signature for CppEventResponse command
static const sl_signature_t sig_price_cluster_cpp_event_response_command = sl_zcl_price_cluster_cpp_event_response_command_signature;
// Command parser for CppEventResponse command
EmberAfStatus zcl_decode_price_cluster_cpp_event_response_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_cpp_event_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_price_cluster_cpp_event_response_command, (uint8_t*)cmd_struct);
} 


// Signature for TakeSnapshotResponse command
static const sl_signature_t sig_simple_metering_cluster_take_snapshot_response_command = sl_zcl_simple_metering_cluster_take_snapshot_response_command_signature;
// Command parser for TakeSnapshotResponse command
EmberAfStatus zcl_decode_simple_metering_cluster_take_snapshot_response_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_take_snapshot_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_simple_metering_cluster_take_snapshot_response_command, (uint8_t*)cmd_struct);
} 


// Signature for NetworkJoinRouterResponse command
static const sl_signature_t sig_zll_commissioning_cluster_network_join_router_response_command = sl_zcl_zll_commissioning_cluster_network_join_router_response_command_signature;
// Command parser for NetworkJoinRouterResponse command
EmberAfStatus zcl_decode_zll_commissioning_cluster_network_join_router_response_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_network_join_router_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_zll_commissioning_cluster_network_join_router_response_command, (uint8_t*)cmd_struct);
} 


// Signature for NetworkJoinEndDeviceResponse command
static const sl_signature_t sig_zll_commissioning_cluster_network_join_end_device_response_command = sl_zcl_zll_commissioning_cluster_network_join_end_device_response_command_signature;
// Command parser for NetworkJoinEndDeviceResponse command
EmberAfStatus zcl_decode_zll_commissioning_cluster_network_join_end_device_response_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_network_join_end_device_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_zll_commissioning_cluster_network_join_end_device_response_command, (uint8_t*)cmd_struct);
} 


// Signature for SetSupplyStatus command
static const sl_signature_t sig_simple_metering_cluster_set_supply_status_command = sl_zcl_simple_metering_cluster_set_supply_status_command_signature;
// Command parser for SetSupplyStatus command
EmberAfStatus zcl_decode_simple_metering_cluster_set_supply_status_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_set_supply_status_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_simple_metering_cluster_set_supply_status_command, (uint8_t*)cmd_struct);
} 


// Signature for GetMeasurementProfileResponseCommand command
static const sl_signature_t sig_electrical_measurement_cluster_get_measurement_profile_response_command_command = sl_zcl_electrical_measurement_cluster_get_measurement_profile_response_command_command_signature;
// Command parser for GetMeasurementProfileResponseCommand command
EmberAfStatus zcl_decode_electrical_measurement_cluster_get_measurement_profile_response_command_command (EmberAfClusterCommand * cmd, sl_zcl_electrical_measurement_cluster_get_measurement_profile_response_command_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_electrical_measurement_cluster_get_measurement_profile_response_command_command, (uint8_t*)cmd_struct);
} 


// Signature for NetworkStartResponse command
static const sl_signature_t sig_zll_commissioning_cluster_network_start_response_command = sl_zcl_zll_commissioning_cluster_network_start_response_command_signature;
// Command parser for NetworkStartResponse command
EmberAfStatus zcl_decode_zll_commissioning_cluster_network_start_response_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_network_start_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_zll_commissioning_cluster_network_start_response_command, (uint8_t*)cmd_struct);
} 


// Signature for ReportEventStatus command
static const sl_signature_t sig_energy_management_cluster_report_event_status_command = sl_zcl_energy_management_cluster_report_event_status_command_signature;
// Command parser for ReportEventStatus command
EmberAfStatus zcl_decode_energy_management_cluster_report_event_status_command (EmberAfClusterCommand * cmd, sl_zcl_energy_management_cluster_report_event_status_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_energy_management_cluster_report_event_status_command, (uint8_t*)cmd_struct);
} 


// Signature for ReportEventStatus command
static const sl_signature_t sig_demand_response_and_load_control_cluster_report_event_status_command = sl_zcl_demand_response_and_load_control_cluster_report_event_status_command_signature;
// Command parser for ReportEventStatus command
EmberAfStatus zcl_decode_demand_response_and_load_control_cluster_report_event_status_command (EmberAfClusterCommand * cmd, sl_zcl_demand_response_and_load_control_cluster_report_event_status_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_demand_response_and_load_control_cluster_report_event_status_command, (uint8_t*)cmd_struct);
} 


// Signature for PairingRequest command
static const sl_signature_t sig_mdu_pairing_cluster_pairing_request_command = sl_zcl_mdu_pairing_cluster_pairing_request_command_signature;
// Command parser for PairingRequest command
EmberAfStatus zcl_decode_mdu_pairing_cluster_pairing_request_command (EmberAfClusterCommand * cmd, sl_zcl_mdu_pairing_cluster_pairing_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_mdu_pairing_cluster_pairing_request_command, (uint8_t*)cmd_struct);
} 


// Signature for NetworkStartRequest command
static const sl_signature_t sig_zll_commissioning_cluster_network_start_request_command = sl_zcl_zll_commissioning_cluster_network_start_request_command_signature;
// Command parser for NetworkStartRequest command
EmberAfStatus zcl_decode_zll_commissioning_cluster_network_start_request_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_network_start_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_zll_commissioning_cluster_network_start_request_command, (uint8_t*)cmd_struct);
} 


// Signature for NetworkJoinRouterRequest command
static const sl_signature_t sig_zll_commissioning_cluster_network_join_router_request_command = sl_zcl_zll_commissioning_cluster_network_join_router_request_command_signature;
// Command parser for NetworkJoinRouterRequest command
EmberAfStatus zcl_decode_zll_commissioning_cluster_network_join_router_request_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_network_join_router_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_zll_commissioning_cluster_network_join_router_request_command, (uint8_t*)cmd_struct);
} 


// Signature for NetworkJoinEndDeviceRequest command
static const sl_signature_t sig_zll_commissioning_cluster_network_join_end_device_request_command = sl_zcl_zll_commissioning_cluster_network_join_end_device_request_command_signature;
// Command parser for NetworkJoinEndDeviceRequest command
EmberAfStatus zcl_decode_zll_commissioning_cluster_network_join_end_device_request_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_network_join_end_device_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_zll_commissioning_cluster_network_join_end_device_request_command, (uint8_t*)cmd_struct);
} 


// Signature for NetworkUpdateRequest command
static const sl_signature_t sig_zll_commissioning_cluster_network_update_request_command = sl_zcl_zll_commissioning_cluster_network_update_request_command_signature;
// Command parser for NetworkUpdateRequest command
EmberAfStatus zcl_decode_zll_commissioning_cluster_network_update_request_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_network_update_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_zll_commissioning_cluster_network_update_request_command, (uint8_t*)cmd_struct);
} 


// Signature for IdentifyRequest command
static const sl_signature_t sig_zll_commissioning_cluster_identify_request_command = sl_zcl_zll_commissioning_cluster_identify_request_command_signature;
// Command parser for IdentifyRequest command
EmberAfStatus zcl_decode_zll_commissioning_cluster_identify_request_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_identify_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_zll_commissioning_cluster_identify_request_command, (uint8_t*)cmd_struct);
} 


// Signature for ConfigureMirror command
static const sl_signature_t sig_simple_metering_cluster_configure_mirror_command = sl_zcl_simple_metering_cluster_configure_mirror_command_signature;
// Command parser for ConfigureMirror command
EmberAfStatus zcl_decode_simple_metering_cluster_configure_mirror_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_configure_mirror_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_simple_metering_cluster_configure_mirror_command, (uint8_t*)cmd_struct);
} 


// Signature for ResetLoadLimitCounter command
static const sl_signature_t sig_simple_metering_cluster_reset_load_limit_counter_command = sl_zcl_simple_metering_cluster_reset_load_limit_counter_command_signature;
// Command parser for ResetLoadLimitCounter command
EmberAfStatus zcl_decode_simple_metering_cluster_reset_load_limit_counter_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_reset_load_limit_counter_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_simple_metering_cluster_reset_load_limit_counter_command, (uint8_t*)cmd_struct);
} 


// Signature for GetSeasons command
static const sl_signature_t sig_calendar_cluster_get_seasons_command = sl_zcl_calendar_cluster_get_seasons_command_signature;
// Command parser for GetSeasons command
EmberAfStatus zcl_decode_calendar_cluster_get_seasons_command (EmberAfClusterCommand * cmd, sl_zcl_calendar_cluster_get_seasons_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_calendar_cluster_get_seasons_command, (uint8_t*)cmd_struct);
} 


// Signature for CancelTariff command
static const sl_signature_t sig_price_cluster_cancel_tariff_command = sl_zcl_price_cluster_cancel_tariff_command_signature;
// Command parser for CancelTariff command
EmberAfStatus zcl_decode_price_cluster_cancel_tariff_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_cancel_tariff_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_price_cluster_cancel_tariff_command, (uint8_t*)cmd_struct);
} 


// Signature for CancelCalendar command
static const sl_signature_t sig_calendar_cluster_cancel_calendar_command = sl_zcl_calendar_cluster_cancel_calendar_command_signature;
// Command parser for CancelCalendar command
EmberAfStatus zcl_decode_calendar_cluster_cancel_calendar_command (EmberAfClusterCommand * cmd, sl_zcl_calendar_cluster_cancel_calendar_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_calendar_cluster_cancel_calendar_command, (uint8_t*)cmd_struct);
} 


// Signature for PublishChangeOfSupplier command
static const sl_signature_t sig_device_management_cluster_publish_change_of_supplier_command = sl_zcl_device_management_cluster_publish_change_of_supplier_command_signature;
// Command parser for PublishChangeOfSupplier command
EmberAfStatus zcl_decode_device_management_cluster_publish_change_of_supplier_command (EmberAfClusterCommand * cmd, sl_zcl_device_management_cluster_publish_change_of_supplier_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_device_management_cluster_publish_change_of_supplier_command, (uint8_t*)cmd_struct);
} 


// Signature for PublishChangeOfTenancy command
static const sl_signature_t sig_device_management_cluster_publish_change_of_tenancy_command = sl_zcl_device_management_cluster_publish_change_of_tenancy_command_signature;
// Command parser for PublishChangeOfTenancy command
EmberAfStatus zcl_decode_device_management_cluster_publish_change_of_tenancy_command (EmberAfClusterCommand * cmd, sl_zcl_device_management_cluster_publish_change_of_tenancy_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_device_management_cluster_publish_change_of_tenancy_command, (uint8_t*)cmd_struct);
} 


// Signature for SetUncontrolledFlowThreshold command
static const sl_signature_t sig_simple_metering_cluster_set_uncontrolled_flow_threshold_command = sl_zcl_simple_metering_cluster_set_uncontrolled_flow_threshold_command_signature;
// Command parser for SetUncontrolledFlowThreshold command
EmberAfStatus zcl_decode_simple_metering_cluster_set_uncontrolled_flow_threshold_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_set_uncontrolled_flow_threshold_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_simple_metering_cluster_set_uncontrolled_flow_threshold_command, (uint8_t*)cmd_struct);
} 


// Signature for PowerDescriptorChange command
static const sl_signature_t sig_sl_works_with_all_hubs_cluster_power_descriptor_change_command = sl_zcl_sl_works_with_all_hubs_cluster_power_descriptor_change_command_signature;
// Command parser for PowerDescriptorChange command
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_power_descriptor_change_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_power_descriptor_change_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_sl_works_with_all_hubs_cluster_power_descriptor_change_command, (uint8_t*)cmd_struct);
} 


// Signature for PublishDayProfile command
static const sl_signature_t sig_calendar_cluster_publish_day_profile_command = sl_zcl_calendar_cluster_publish_day_profile_command_signature;
// Command parser for PublishDayProfile command
EmberAfStatus zcl_decode_calendar_cluster_publish_day_profile_command (EmberAfClusterCommand * cmd, sl_zcl_calendar_cluster_publish_day_profile_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_calendar_cluster_publish_day_profile_command, (uint8_t*)cmd_struct);
} 


// Signature for PublishWeekProfile command
static const sl_signature_t sig_calendar_cluster_publish_week_profile_command = sl_zcl_calendar_cluster_publish_week_profile_command_signature;
// Command parser for PublishWeekProfile command
EmberAfStatus zcl_decode_calendar_cluster_publish_week_profile_command (EmberAfClusterCommand * cmd, sl_zcl_calendar_cluster_publish_week_profile_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_calendar_cluster_publish_week_profile_command, (uint8_t*)cmd_struct);
} 


// Signature for PublishTierLabels command
static const sl_signature_t sig_price_cluster_publish_tier_labels_command = sl_zcl_price_cluster_publish_tier_labels_command_signature;
// Command parser for PublishTierLabels command
EmberAfStatus zcl_decode_price_cluster_publish_tier_labels_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_publish_tier_labels_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_price_cluster_publish_tier_labels_command, (uint8_t*)cmd_struct);
} 


// Signature for PublishSeasons command
static const sl_signature_t sig_calendar_cluster_publish_seasons_command = sl_zcl_calendar_cluster_publish_seasons_command_signature;
// Command parser for PublishSeasons command
EmberAfStatus zcl_decode_calendar_cluster_publish_seasons_command (EmberAfClusterCommand * cmd, sl_zcl_calendar_cluster_publish_seasons_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_calendar_cluster_publish_seasons_command, (uint8_t*)cmd_struct);
} 


// Signature for PublishTariffInformation command
static const sl_signature_t sig_price_cluster_publish_tariff_information_command = sl_zcl_price_cluster_publish_tariff_information_command_signature;
// Command parser for PublishTariffInformation command
EmberAfStatus zcl_decode_price_cluster_publish_tariff_information_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_publish_tariff_information_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_price_cluster_publish_tariff_information_command, (uint8_t*)cmd_struct);
} 


// Signature for PublishCalendar command
static const sl_signature_t sig_calendar_cluster_publish_calendar_command = sl_zcl_calendar_cluster_publish_calendar_command_signature;
// Command parser for PublishCalendar command
EmberAfStatus zcl_decode_calendar_cluster_publish_calendar_command (EmberAfClusterCommand * cmd, sl_zcl_calendar_cluster_publish_calendar_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_calendar_cluster_publish_calendar_command, (uint8_t*)cmd_struct);
} 


// Signature for PublishSpecialDays command
static const sl_signature_t sig_calendar_cluster_publish_special_days_command = sl_zcl_calendar_cluster_publish_special_days_command_signature;
// Command parser for PublishSpecialDays command
EmberAfStatus zcl_decode_calendar_cluster_publish_special_days_command (EmberAfClusterCommand * cmd, sl_zcl_calendar_cluster_publish_special_days_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_calendar_cluster_publish_special_days_command, (uint8_t*)cmd_struct);
} 


// Signature for GetDayProfiles command
static const sl_signature_t sig_calendar_cluster_get_day_profiles_command = sl_zcl_calendar_cluster_get_day_profiles_command_signature;
// Command parser for GetDayProfiles command
EmberAfStatus zcl_decode_calendar_cluster_get_day_profiles_command (EmberAfClusterCommand * cmd, sl_zcl_calendar_cluster_get_day_profiles_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_calendar_cluster_get_day_profiles_command, (uint8_t*)cmd_struct);
} 


// Signature for GetWeekProfiles command
static const sl_signature_t sig_calendar_cluster_get_week_profiles_command = sl_zcl_calendar_cluster_get_week_profiles_command_signature;
// Command parser for GetWeekProfiles command
EmberAfStatus zcl_decode_calendar_cluster_get_week_profiles_command (EmberAfClusterCommand * cmd, sl_zcl_calendar_cluster_get_week_profiles_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_calendar_cluster_get_week_profiles_command, (uint8_t*)cmd_struct);
} 


// Signature for ChangePaymentMode command
static const sl_signature_t sig_prepayment_cluster_change_payment_mode_command = sl_zcl_prepayment_cluster_change_payment_mode_command_signature;
// Command parser for ChangePaymentMode command
EmberAfStatus zcl_decode_prepayment_cluster_change_payment_mode_command (EmberAfClusterCommand * cmd, sl_zcl_prepayment_cluster_change_payment_mode_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_prepayment_cluster_change_payment_mode_command, (uint8_t*)cmd_struct);
} 


// Signature for PublishBillingPeriod command
static const sl_signature_t sig_price_cluster_publish_billing_period_command = sl_zcl_price_cluster_publish_billing_period_command_signature;
// Command parser for PublishBillingPeriod command
EmberAfStatus zcl_decode_price_cluster_publish_billing_period_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_publish_billing_period_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_price_cluster_publish_billing_period_command, (uint8_t*)cmd_struct);
} 


// Signature for PublishConsolidatedBill command
static const sl_signature_t sig_price_cluster_publish_consolidated_bill_command = sl_zcl_price_cluster_publish_consolidated_bill_command_signature;
// Command parser for PublishConsolidatedBill command
EmberAfStatus zcl_decode_price_cluster_publish_consolidated_bill_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_publish_consolidated_bill_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_price_cluster_publish_consolidated_bill_command, (uint8_t*)cmd_struct);
} 


// Signature for PriceAcknowledgement command
static const sl_signature_t sig_price_cluster_price_acknowledgement_command = sl_zcl_price_cluster_price_acknowledgement_command_signature;
// Command parser for PriceAcknowledgement command
EmberAfStatus zcl_decode_price_cluster_price_acknowledgement_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_price_acknowledgement_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_price_cluster_price_acknowledgement_command, (uint8_t*)cmd_struct);
} 


// Signature for SupplyStatusResponse command
static const sl_signature_t sig_simple_metering_cluster_supply_status_response_command = sl_zcl_simple_metering_cluster_supply_status_response_command_signature;
// Command parser for SupplyStatusResponse command
EmberAfStatus zcl_decode_simple_metering_cluster_supply_status_response_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_supply_status_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_simple_metering_cluster_supply_status_response_command, (uint8_t*)cmd_struct);
} 


// Signature for PublishCO2Value command
static const sl_signature_t sig_price_cluster_publish_co2_value_command = sl_zcl_price_cluster_publish_co2_value_command_signature;
// Command parser for PublishCO2Value command
EmberAfStatus zcl_decode_price_cluster_publish_co2_value_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_publish_co2_value_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_price_cluster_publish_co2_value_command, (uint8_t*)cmd_struct);
} 


// Signature for PublishCppEvent command
static const sl_signature_t sig_price_cluster_publish_cpp_event_command = sl_zcl_price_cluster_publish_cpp_event_command_signature;
// Command parser for PublishCppEvent command
EmberAfStatus zcl_decode_price_cluster_publish_cpp_event_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_publish_cpp_event_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_price_cluster_publish_cpp_event_command, (uint8_t*)cmd_struct);
} 


// Signature for PublishCurrencyConversion command
static const sl_signature_t sig_price_cluster_publish_currency_conversion_command = sl_zcl_price_cluster_publish_currency_conversion_command_signature;
// Command parser for PublishCurrencyConversion command
EmberAfStatus zcl_decode_price_cluster_publish_currency_conversion_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_publish_currency_conversion_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_price_cluster_publish_currency_conversion_command, (uint8_t*)cmd_struct);
} 


// Signature for PublishBlockPeriod command
static const sl_signature_t sig_price_cluster_publish_block_period_command = sl_zcl_price_cluster_publish_block_period_command_signature;
// Command parser for PublishBlockPeriod command
EmberAfStatus zcl_decode_price_cluster_publish_block_period_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_publish_block_period_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_price_cluster_publish_block_period_command, (uint8_t*)cmd_struct);
} 


// Signature for SetOverallDebtCap command
static const sl_signature_t sig_prepayment_cluster_set_overall_debt_cap_command = sl_zcl_prepayment_cluster_set_overall_debt_cap_command_signature;
// Command parser for SetOverallDebtCap command
EmberAfStatus zcl_decode_prepayment_cluster_set_overall_debt_cap_command (EmberAfClusterCommand * cmd, sl_zcl_prepayment_cluster_set_overall_debt_cap_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_prepayment_cluster_set_overall_debt_cap_command, (uint8_t*)cmd_struct);
} 


// Signature for PublishCreditPayment command
static const sl_signature_t sig_price_cluster_publish_credit_payment_command = sl_zcl_price_cluster_publish_credit_payment_command_signature;
// Command parser for PublishCreditPayment command
EmberAfStatus zcl_decode_price_cluster_publish_credit_payment_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_publish_credit_payment_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_price_cluster_publish_credit_payment_command, (uint8_t*)cmd_struct);
} 


// Signature for SetMaximumCreditLimit command
static const sl_signature_t sig_prepayment_cluster_set_maximum_credit_limit_command = sl_zcl_prepayment_cluster_set_maximum_credit_limit_command_signature;
// Command parser for SetMaximumCreditLimit command
EmberAfStatus zcl_decode_prepayment_cluster_set_maximum_credit_limit_command (EmberAfClusterCommand * cmd, sl_zcl_prepayment_cluster_set_maximum_credit_limit_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_prepayment_cluster_set_maximum_credit_limit_command, (uint8_t*)cmd_struct);
} 


// Signature for PublishPriceMatrix command
static const sl_signature_t sig_price_cluster_publish_price_matrix_command = sl_zcl_price_cluster_publish_price_matrix_command_signature;
// Command parser for PublishPriceMatrix command
EmberAfStatus zcl_decode_price_cluster_publish_price_matrix_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_publish_price_matrix_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_price_cluster_publish_price_matrix_command, (uint8_t*)cmd_struct);
} 


// Signature for PublishBlockThresholds command
static const sl_signature_t sig_price_cluster_publish_block_thresholds_command = sl_zcl_price_cluster_publish_block_thresholds_command_signature;
// Command parser for PublishBlockThresholds command
EmberAfStatus zcl_decode_price_cluster_publish_block_thresholds_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_publish_block_thresholds_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_price_cluster_publish_block_thresholds_command, (uint8_t*)cmd_struct);
} 


// Signature for ChangeSupply command
static const sl_signature_t sig_simple_metering_cluster_change_supply_command = sl_zcl_simple_metering_cluster_change_supply_command_signature;
// Command parser for ChangeSupply command
EmberAfStatus zcl_decode_simple_metering_cluster_change_supply_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_change_supply_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_simple_metering_cluster_change_supply_command, (uint8_t*)cmd_struct);
} 


// Signature for DeviceInformationRequest command
static const sl_signature_t sig_zll_commissioning_cluster_device_information_request_command = sl_zcl_zll_commissioning_cluster_device_information_request_command_signature;
// Command parser for DeviceInformationRequest command
EmberAfStatus zcl_decode_zll_commissioning_cluster_device_information_request_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_device_information_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_zll_commissioning_cluster_device_information_request_command, (uint8_t*)cmd_struct);
} 


// Signature for ConfigureNotificationScheme command
static const sl_signature_t sig_simple_metering_cluster_configure_notification_scheme_command = sl_zcl_simple_metering_cluster_configure_notification_scheme_command_signature;
// Command parser for ConfigureNotificationScheme command
EmberAfStatus zcl_decode_simple_metering_cluster_configure_notification_scheme_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_configure_notification_scheme_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_simple_metering_cluster_configure_notification_scheme_command, (uint8_t*)cmd_struct);
} 


// Signature for ScanResponse command
static const sl_signature_t sig_zll_commissioning_cluster_scan_response_command = sl_zcl_zll_commissioning_cluster_scan_response_command_signature;
// Command parser for ScanResponse command
EmberAfStatus zcl_decode_zll_commissioning_cluster_scan_response_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_scan_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_zll_commissioning_cluster_scan_response_command, (uint8_t*)cmd_struct);
} 


// Signature for ConfigureNotificationFlags command
static const sl_signature_t sig_simple_metering_cluster_configure_notification_flags_command = sl_zcl_simple_metering_cluster_configure_notification_flags_command_signature;
// Command parser for ConfigureNotificationFlags command
EmberAfStatus zcl_decode_simple_metering_cluster_configure_notification_flags_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_configure_notification_flags_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_simple_metering_cluster_configure_notification_flags_command, (uint8_t*)cmd_struct);
} 


// Signature for PairingResponse command
static const sl_signature_t sig_mdu_pairing_cluster_pairing_response_command = sl_zcl_mdu_pairing_cluster_pairing_response_command_signature;
// Command parser for PairingResponse command
EmberAfStatus zcl_decode_mdu_pairing_cluster_pairing_response_command (EmberAfClusterCommand * cmd, sl_zcl_mdu_pairing_cluster_pairing_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_mdu_pairing_cluster_pairing_response_command, (uint8_t*)cmd_struct);
} 


// Signature for DeviceInformationResponse command
static const sl_signature_t sig_zll_commissioning_cluster_device_information_response_command = sl_zcl_zll_commissioning_cluster_device_information_response_command_signature;
// Command parser for DeviceInformationResponse command
EmberAfStatus zcl_decode_zll_commissioning_cluster_device_information_response_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_device_information_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_zll_commissioning_cluster_device_information_response_command, (uint8_t*)cmd_struct);
} 


// Signature for ScheduleSnapshot command
static const sl_signature_t sig_simple_metering_cluster_schedule_snapshot_command = sl_zcl_simple_metering_cluster_schedule_snapshot_command_signature;
// Command parser for ScheduleSnapshot command
EmberAfStatus zcl_decode_simple_metering_cluster_schedule_snapshot_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_schedule_snapshot_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_simple_metering_cluster_schedule_snapshot_command, (uint8_t*)cmd_struct);
} 


// Signature for ChangeDebt command
static const sl_signature_t sig_prepayment_cluster_change_debt_command = sl_zcl_prepayment_cluster_change_debt_command_signature;
// Command parser for ChangeDebt command
EmberAfStatus zcl_decode_prepayment_cluster_change_debt_command (EmberAfClusterCommand * cmd, sl_zcl_prepayment_cluster_change_debt_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_prepayment_cluster_change_debt_command, (uint8_t*)cmd_struct);
} 


// Signature for PublishPrice command
static const sl_signature_t sig_price_cluster_publish_price_command = sl_zcl_price_cluster_publish_price_command_signature;
// Command parser for PublishPrice command
EmberAfStatus zcl_decode_price_cluster_publish_price_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_publish_price_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_price_cluster_publish_price_command, (uint8_t*)cmd_struct);
} 


// Signature for ScheduleSnapshotResponse command
static const sl_signature_t sig_simple_metering_cluster_schedule_snapshot_response_command = sl_zcl_simple_metering_cluster_schedule_snapshot_response_command_signature;
// Command parser for ScheduleSnapshotResponse command
EmberAfStatus zcl_decode_simple_metering_cluster_schedule_snapshot_response_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_schedule_snapshot_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_simple_metering_cluster_schedule_snapshot_response_command, (uint8_t*)cmd_struct);
} 


// Signature for SetEventConfiguration command
static const sl_signature_t sig_device_management_cluster_set_event_configuration_command = sl_zcl_device_management_cluster_set_event_configuration_command_signature;
// Command parser for SetEventConfiguration command
EmberAfStatus zcl_decode_device_management_cluster_set_event_configuration_command (EmberAfClusterCommand * cmd, sl_zcl_device_management_cluster_set_event_configuration_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_device_management_cluster_set_event_configuration_command, (uint8_t*)cmd_struct);
} 


// Signature for MessageConfirmation command
static const sl_signature_t sig_messaging_cluster_message_confirmation_command = sl_zcl_messaging_cluster_message_confirmation_command_signature;
// Command parser for MessageConfirmation command
EmberAfStatus zcl_decode_messaging_cluster_message_confirmation_command (EmberAfClusterCommand * cmd, sl_zcl_messaging_cluster_message_confirmation_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_messaging_cluster_message_confirmation_command, (uint8_t*)cmd_struct);
} 


// Signature for StartSampling command
static const sl_signature_t sig_simple_metering_cluster_start_sampling_command = sl_zcl_simple_metering_cluster_start_sampling_command_signature;
// Command parser for StartSampling command
EmberAfStatus zcl_decode_simple_metering_cluster_start_sampling_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_start_sampling_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_simple_metering_cluster_start_sampling_command, (uint8_t*)cmd_struct);
} 


// Signature for CreditAdjustment command
static const sl_signature_t sig_prepayment_cluster_credit_adjustment_command = sl_zcl_prepayment_cluster_credit_adjustment_command_signature;
// Command parser for CreditAdjustment command
EmberAfStatus zcl_decode_prepayment_cluster_credit_adjustment_command (EmberAfClusterCommand * cmd, sl_zcl_prepayment_cluster_credit_adjustment_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_prepayment_cluster_credit_adjustment_command, (uint8_t*)cmd_struct);
} 


// Signature for RequestNewPasswordResponse command
static const sl_signature_t sig_device_management_cluster_request_new_password_response_command = sl_zcl_device_management_cluster_request_new_password_response_command_signature;
// Command parser for RequestNewPasswordResponse command
EmberAfStatus zcl_decode_device_management_cluster_request_new_password_response_command (EmberAfClusterCommand * cmd, sl_zcl_device_management_cluster_request_new_password_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_device_management_cluster_request_new_password_response_command, (uint8_t*)cmd_struct);
} 


// Signature for PublishConversionFactor command
static const sl_signature_t sig_price_cluster_publish_conversion_factor_command = sl_zcl_price_cluster_publish_conversion_factor_command_signature;
// Command parser for PublishConversionFactor command
EmberAfStatus zcl_decode_price_cluster_publish_conversion_factor_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_publish_conversion_factor_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_price_cluster_publish_conversion_factor_command, (uint8_t*)cmd_struct);
} 


// Signature for PublishCalorificValue command
static const sl_signature_t sig_price_cluster_publish_calorific_value_command = sl_zcl_price_cluster_publish_calorific_value_command_signature;
// Command parser for PublishCalorificValue command
EmberAfStatus zcl_decode_price_cluster_publish_calorific_value_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_publish_calorific_value_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_price_cluster_publish_calorific_value_command, (uint8_t*)cmd_struct);
} 


// Signature for EmergencyCreditSetup command
static const sl_signature_t sig_prepayment_cluster_emergency_credit_setup_command = sl_zcl_prepayment_cluster_emergency_credit_setup_command_signature;
// Command parser for EmergencyCreditSetup command
EmberAfStatus zcl_decode_prepayment_cluster_emergency_credit_setup_command (EmberAfClusterCommand * cmd, sl_zcl_prepayment_cluster_emergency_credit_setup_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_prepayment_cluster_emergency_credit_setup_command, (uint8_t*)cmd_struct);
} 


// Signature for UpdateSiteId command
static const sl_signature_t sig_device_management_cluster_update_site_id_command = sl_zcl_device_management_cluster_update_site_id_command_signature;
// Command parser for UpdateSiteId command
EmberAfStatus zcl_decode_device_management_cluster_update_site_id_command (EmberAfClusterCommand * cmd, sl_zcl_device_management_cluster_update_site_id_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_device_management_cluster_update_site_id_command, (uint8_t*)cmd_struct);
} 


// Signature for UpdateCIN command
static const sl_signature_t sig_device_management_cluster_update_cin_command = sl_zcl_device_management_cluster_update_cin_command_signature;
// Command parser for UpdateCIN command
EmberAfStatus zcl_decode_device_management_cluster_update_cin_command (EmberAfClusterCommand * cmd, sl_zcl_device_management_cluster_update_cin_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_device_management_cluster_update_cin_command, (uint8_t*)cmd_struct);
} 


// Signature for PublishSnapshot command
static const sl_signature_t sig_simple_metering_cluster_publish_snapshot_command = sl_zcl_simple_metering_cluster_publish_snapshot_command_signature;
// Command parser for PublishSnapshot command
EmberAfStatus zcl_decode_simple_metering_cluster_publish_snapshot_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_publish_snapshot_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_simple_metering_cluster_publish_snapshot_command, (uint8_t*)cmd_struct);
} 


// Signature for PublishPrepaySnapshot command
static const sl_signature_t sig_prepayment_cluster_publish_prepay_snapshot_command = sl_zcl_prepayment_cluster_publish_prepay_snapshot_command_signature;
// Command parser for PublishPrepaySnapshot command
EmberAfStatus zcl_decode_prepayment_cluster_publish_prepay_snapshot_command (EmberAfClusterCommand * cmd, sl_zcl_prepayment_cluster_publish_prepay_snapshot_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_prepayment_cluster_publish_prepay_snapshot_command, (uint8_t*)cmd_struct);
} 


// Signature for PowerProfileRequest command
static const sl_signature_t sig_power_profile_cluster_power_profile_request_command = sl_zcl_power_profile_cluster_power_profile_request_command_signature;
// Command parser for PowerProfileRequest command
EmberAfStatus zcl_decode_power_profile_cluster_power_profile_request_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_power_profile_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_power_profile_cluster_power_profile_request_command, (uint8_t*)cmd_struct);
} 


// Signature for PowerProfileScheduleConstraintsRequest command
static const sl_signature_t sig_power_profile_cluster_power_profile_schedule_constraints_request_command = sl_zcl_power_profile_cluster_power_profile_schedule_constraints_request_command_signature;
// Command parser for PowerProfileScheduleConstraintsRequest command
EmberAfStatus zcl_decode_power_profile_cluster_power_profile_schedule_constraints_request_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_power_profile_schedule_constraints_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_power_profile_cluster_power_profile_schedule_constraints_request_command, (uint8_t*)cmd_struct);
} 


// Signature for EnergyPhasesScheduleStateRequest command
static const sl_signature_t sig_power_profile_cluster_energy_phases_schedule_state_request_command = sl_zcl_power_profile_cluster_energy_phases_schedule_state_request_command_signature;
// Command parser for EnergyPhasesScheduleStateRequest command
EmberAfStatus zcl_decode_power_profile_cluster_energy_phases_schedule_state_request_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_energy_phases_schedule_state_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_power_profile_cluster_energy_phases_schedule_state_request_command, (uint8_t*)cmd_struct);
} 


// Signature for GetPowerProfilePrice command
static const sl_signature_t sig_power_profile_cluster_get_power_profile_price_command = sl_zcl_power_profile_cluster_get_power_profile_price_command_signature;
// Command parser for GetPowerProfilePrice command
EmberAfStatus zcl_decode_power_profile_cluster_get_power_profile_price_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_get_power_profile_price_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_power_profile_cluster_get_power_profile_price_command, (uint8_t*)cmd_struct);
} 


// Signature for EnergyPhasesScheduleRequest command
static const sl_signature_t sig_power_profile_cluster_energy_phases_schedule_request_command = sl_zcl_power_profile_cluster_energy_phases_schedule_request_command_signature;
// Command parser for EnergyPhasesScheduleRequest command
EmberAfStatus zcl_decode_power_profile_cluster_energy_phases_schedule_request_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_energy_phases_schedule_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_power_profile_cluster_energy_phases_schedule_request_command, (uint8_t*)cmd_struct);
} 


// Signature for GetHolidaySchedule command
static const sl_signature_t sig_door_lock_cluster_get_holiday_schedule_command = sl_zcl_door_lock_cluster_get_holiday_schedule_command_signature;
// Command parser for GetHolidaySchedule command
EmberAfStatus zcl_decode_door_lock_cluster_get_holiday_schedule_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_holiday_schedule_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_get_holiday_schedule_command, (uint8_t*)cmd_struct);
} 


// Signature for ClearHolidaySchedule command
static const sl_signature_t sig_door_lock_cluster_clear_holiday_schedule_command = sl_zcl_door_lock_cluster_clear_holiday_schedule_command_signature;
// Command parser for ClearHolidaySchedule command
EmberAfStatus zcl_decode_door_lock_cluster_clear_holiday_schedule_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_clear_holiday_schedule_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_clear_holiday_schedule_command, (uint8_t*)cmd_struct);
} 


// Signature for LockDoorResponse command
static const sl_signature_t sig_door_lock_cluster_lock_door_response_command = sl_zcl_door_lock_cluster_lock_door_response_command_signature;
// Command parser for LockDoorResponse command
EmberAfStatus zcl_decode_door_lock_cluster_lock_door_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_lock_door_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_lock_door_response_command, (uint8_t*)cmd_struct);
} 


// Signature for UnlockDoorResponse command
static const sl_signature_t sig_door_lock_cluster_unlock_door_response_command = sl_zcl_door_lock_cluster_unlock_door_response_command_signature;
// Command parser for UnlockDoorResponse command
EmberAfStatus zcl_decode_door_lock_cluster_unlock_door_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_unlock_door_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_unlock_door_response_command, (uint8_t*)cmd_struct);
} 


// Signature for ToggleResponse command
static const sl_signature_t sig_door_lock_cluster_toggle_response_command = sl_zcl_door_lock_cluster_toggle_response_command_signature;
// Command parser for ToggleResponse command
EmberAfStatus zcl_decode_door_lock_cluster_toggle_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_toggle_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_toggle_response_command, (uint8_t*)cmd_struct);
} 


// Signature for UnlockWithTimeoutResponse command
static const sl_signature_t sig_door_lock_cluster_unlock_with_timeout_response_command = sl_zcl_door_lock_cluster_unlock_with_timeout_response_command_signature;
// Command parser for UnlockWithTimeoutResponse command
EmberAfStatus zcl_decode_door_lock_cluster_unlock_with_timeout_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_unlock_with_timeout_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_unlock_with_timeout_response_command, (uint8_t*)cmd_struct);
} 


// Signature for ClearPinResponse command
static const sl_signature_t sig_door_lock_cluster_clear_pin_response_command = sl_zcl_door_lock_cluster_clear_pin_response_command_signature;
// Command parser for ClearPinResponse command
EmberAfStatus zcl_decode_door_lock_cluster_clear_pin_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_clear_pin_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_clear_pin_response_command, (uint8_t*)cmd_struct);
} 


// Signature for ClearAllPinsResponse command
static const sl_signature_t sig_door_lock_cluster_clear_all_pins_response_command = sl_zcl_door_lock_cluster_clear_all_pins_response_command_signature;
// Command parser for ClearAllPinsResponse command
EmberAfStatus zcl_decode_door_lock_cluster_clear_all_pins_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_clear_all_pins_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_clear_all_pins_response_command, (uint8_t*)cmd_struct);
} 


// Signature for SetUserStatusResponse command
static const sl_signature_t sig_door_lock_cluster_set_user_status_response_command = sl_zcl_door_lock_cluster_set_user_status_response_command_signature;
// Command parser for SetUserStatusResponse command
EmberAfStatus zcl_decode_door_lock_cluster_set_user_status_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_set_user_status_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_set_user_status_response_command, (uint8_t*)cmd_struct);
} 


// Signature for SetWeekdayScheduleResponse command
static const sl_signature_t sig_door_lock_cluster_set_weekday_schedule_response_command = sl_zcl_door_lock_cluster_set_weekday_schedule_response_command_signature;
// Command parser for SetWeekdayScheduleResponse command
EmberAfStatus zcl_decode_door_lock_cluster_set_weekday_schedule_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_set_weekday_schedule_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_set_weekday_schedule_response_command, (uint8_t*)cmd_struct);
} 


// Signature for ClearWeekdayScheduleResponse command
static const sl_signature_t sig_door_lock_cluster_clear_weekday_schedule_response_command = sl_zcl_door_lock_cluster_clear_weekday_schedule_response_command_signature;
// Command parser for ClearWeekdayScheduleResponse command
EmberAfStatus zcl_decode_door_lock_cluster_clear_weekday_schedule_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_clear_weekday_schedule_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_clear_weekday_schedule_response_command, (uint8_t*)cmd_struct);
} 


// Signature for SetYeardayScheduleResponse command
static const sl_signature_t sig_door_lock_cluster_set_yearday_schedule_response_command = sl_zcl_door_lock_cluster_set_yearday_schedule_response_command_signature;
// Command parser for SetYeardayScheduleResponse command
EmberAfStatus zcl_decode_door_lock_cluster_set_yearday_schedule_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_set_yearday_schedule_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_set_yearday_schedule_response_command, (uint8_t*)cmd_struct);
} 


// Signature for ClearYeardayScheduleResponse command
static const sl_signature_t sig_door_lock_cluster_clear_yearday_schedule_response_command = sl_zcl_door_lock_cluster_clear_yearday_schedule_response_command_signature;
// Command parser for ClearYeardayScheduleResponse command
EmberAfStatus zcl_decode_door_lock_cluster_clear_yearday_schedule_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_clear_yearday_schedule_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_clear_yearday_schedule_response_command, (uint8_t*)cmd_struct);
} 


// Signature for SetHolidayScheduleResponse command
static const sl_signature_t sig_door_lock_cluster_set_holiday_schedule_response_command = sl_zcl_door_lock_cluster_set_holiday_schedule_response_command_signature;
// Command parser for SetHolidayScheduleResponse command
EmberAfStatus zcl_decode_door_lock_cluster_set_holiday_schedule_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_set_holiday_schedule_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_set_holiday_schedule_response_command, (uint8_t*)cmd_struct);
} 


// Signature for ClearHolidayScheduleResponse command
static const sl_signature_t sig_door_lock_cluster_clear_holiday_schedule_response_command = sl_zcl_door_lock_cluster_clear_holiday_schedule_response_command_signature;
// Command parser for ClearHolidayScheduleResponse command
EmberAfStatus zcl_decode_door_lock_cluster_clear_holiday_schedule_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_clear_holiday_schedule_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_clear_holiday_schedule_response_command, (uint8_t*)cmd_struct);
} 


// Signature for SetUserTypeResponse command
static const sl_signature_t sig_door_lock_cluster_set_user_type_response_command = sl_zcl_door_lock_cluster_set_user_type_response_command_signature;
// Command parser for SetUserTypeResponse command
EmberAfStatus zcl_decode_door_lock_cluster_set_user_type_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_set_user_type_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_set_user_type_response_command, (uint8_t*)cmd_struct);
} 


// Signature for ClearRfidResponse command
static const sl_signature_t sig_door_lock_cluster_clear_rfid_response_command = sl_zcl_door_lock_cluster_clear_rfid_response_command_signature;
// Command parser for ClearRfidResponse command
EmberAfStatus zcl_decode_door_lock_cluster_clear_rfid_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_clear_rfid_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_clear_rfid_response_command, (uint8_t*)cmd_struct);
} 


// Signature for ClearAllRfidsResponse command
static const sl_signature_t sig_door_lock_cluster_clear_all_rfids_response_command = sl_zcl_door_lock_cluster_clear_all_rfids_response_command_signature;
// Command parser for ClearAllRfidsResponse command
EmberAfStatus zcl_decode_door_lock_cluster_clear_all_rfids_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_clear_all_rfids_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_clear_all_rfids_response_command, (uint8_t*)cmd_struct);
} 


// Signature for SetDisposableScheduleResponse command
static const sl_signature_t sig_door_lock_cluster_set_disposable_schedule_response_command = sl_zcl_door_lock_cluster_set_disposable_schedule_response_command_signature;
// Command parser for SetDisposableScheduleResponse command
EmberAfStatus zcl_decode_door_lock_cluster_set_disposable_schedule_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_set_disposable_schedule_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_set_disposable_schedule_response_command, (uint8_t*)cmd_struct);
} 


// Signature for ClearDisposableScheduleResponse command
static const sl_signature_t sig_door_lock_cluster_clear_disposable_schedule_response_command = sl_zcl_door_lock_cluster_clear_disposable_schedule_response_command_signature;
// Command parser for ClearDisposableScheduleResponse command
EmberAfStatus zcl_decode_door_lock_cluster_clear_disposable_schedule_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_clear_disposable_schedule_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_clear_disposable_schedule_response_command, (uint8_t*)cmd_struct);
} 


// Signature for ClearBiometricCredentialResponse command
static const sl_signature_t sig_door_lock_cluster_clear_biometric_credential_response_command = sl_zcl_door_lock_cluster_clear_biometric_credential_response_command_signature;
// Command parser for ClearBiometricCredentialResponse command
EmberAfStatus zcl_decode_door_lock_cluster_clear_biometric_credential_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_clear_biometric_credential_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_clear_biometric_credential_response_command, (uint8_t*)cmd_struct);
} 


// Signature for ClearAllBiometricCredentialsResponse command
static const sl_signature_t sig_door_lock_cluster_clear_all_biometric_credentials_response_command = sl_zcl_door_lock_cluster_clear_all_biometric_credentials_response_command_signature;
// Command parser for ClearAllBiometricCredentialsResponse command
EmberAfStatus zcl_decode_door_lock_cluster_clear_all_biometric_credentials_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_clear_all_biometric_credentials_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_clear_all_biometric_credentials_response_command, (uint8_t*)cmd_struct);
} 


// Signature for WindowCoveringGoToLiftPercentage command
static const sl_signature_t sig_window_covering_cluster_window_covering_go_to_lift_percentage_command = sl_zcl_window_covering_cluster_window_covering_go_to_lift_percentage_command_signature;
// Command parser for WindowCoveringGoToLiftPercentage command
EmberAfStatus zcl_decode_window_covering_cluster_window_covering_go_to_lift_percentage_command (EmberAfClusterCommand * cmd, sl_zcl_window_covering_cluster_window_covering_go_to_lift_percentage_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_window_covering_cluster_window_covering_go_to_lift_percentage_command, (uint8_t*)cmd_struct);
} 


// Signature for WindowCoveringGoToTiltPercentage command
static const sl_signature_t sig_window_covering_cluster_window_covering_go_to_tilt_percentage_command = sl_zcl_window_covering_cluster_window_covering_go_to_tilt_percentage_command_signature;
// Command parser for WindowCoveringGoToTiltPercentage command
EmberAfStatus zcl_decode_window_covering_cluster_window_covering_go_to_tilt_percentage_command (EmberAfClusterCommand * cmd, sl_zcl_window_covering_cluster_window_covering_go_to_tilt_percentage_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_window_covering_cluster_window_covering_go_to_tilt_percentage_command, (uint8_t*)cmd_struct);
} 


// Signature for BarrierControlGoToPercent command
static const sl_signature_t sig_barrier_control_cluster_barrier_control_go_to_percent_command = sl_zcl_barrier_control_cluster_barrier_control_go_to_percent_command_signature;
// Command parser for BarrierControlGoToPercent command
EmberAfStatus zcl_decode_barrier_control_cluster_barrier_control_go_to_percent_command (EmberAfClusterCommand * cmd, sl_zcl_barrier_control_cluster_barrier_control_go_to_percent_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_barrier_control_cluster_barrier_control_go_to_percent_command, (uint8_t*)cmd_struct);
} 


// Signature for GetZoneInformation command
static const sl_signature_t sig_ias_ace_cluster_get_zone_information_command = sl_zcl_ias_ace_cluster_get_zone_information_command_signature;
// Command parser for GetZoneInformation command
EmberAfStatus zcl_decode_ias_ace_cluster_get_zone_information_command (EmberAfClusterCommand * cmd, sl_zcl_ias_ace_cluster_get_zone_information_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_ias_ace_cluster_get_zone_information_command, (uint8_t*)cmd_struct);
} 


// Signature for GetSupportedTunnelProtocols command
static const sl_signature_t sig_tunneling_cluster_get_supported_tunnel_protocols_command = sl_zcl_tunneling_cluster_get_supported_tunnel_protocols_command_signature;
// Command parser for GetSupportedTunnelProtocols command
EmberAfStatus zcl_decode_tunneling_cluster_get_supported_tunnel_protocols_command (EmberAfClusterCommand * cmd, sl_zcl_tunneling_cluster_get_supported_tunnel_protocols_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_tunneling_cluster_get_supported_tunnel_protocols_command, (uint8_t*)cmd_struct);
} 


// Signature for SuspendZclMessages command
static const sl_signature_t sig_sub_g_hz_cluster_suspend_zcl_messages_command = sl_zcl_sub_g_hz_cluster_suspend_zcl_messages_command_signature;
// Command parser for SuspendZclMessages command
EmberAfStatus zcl_decode_sub_g_hz_cluster_suspend_zcl_messages_command (EmberAfClusterCommand * cmd, sl_zcl_sub_g_hz_cluster_suspend_zcl_messages_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_sub_g_hz_cluster_suspend_zcl_messages_command, (uint8_t*)cmd_struct);
} 


// Signature for GetGroupIdentifiersRequest command
static const sl_signature_t sig_zll_commissioning_cluster_get_group_identifiers_request_command = sl_zcl_zll_commissioning_cluster_get_group_identifiers_request_command_signature;
// Command parser for GetGroupIdentifiersRequest command
EmberAfStatus zcl_decode_zll_commissioning_cluster_get_group_identifiers_request_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_get_group_identifiers_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_zll_commissioning_cluster_get_group_identifiers_request_command, (uint8_t*)cmd_struct);
} 


// Signature for GetEndpointListRequest command
static const sl_signature_t sig_zll_commissioning_cluster_get_endpoint_list_request_command = sl_zcl_zll_commissioning_cluster_get_endpoint_list_request_command_signature;
// Command parser for GetEndpointListRequest command
EmberAfStatus zcl_decode_zll_commissioning_cluster_get_endpoint_list_request_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_get_endpoint_list_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_zll_commissioning_cluster_get_endpoint_list_request_command, (uint8_t*)cmd_struct);
} 


// Signature for CommandOne command
static const sl_signature_t sig_sample_mfg_specific_cluster_cluster_command_one_command = sl_zcl_sample_mfg_specific_cluster_cluster_command_one_command_signature;
// Command parser for CommandOne command
EmberAfStatus zcl_decode_sample_mfg_specific_cluster_cluster_command_one_command (EmberAfClusterCommand * cmd, sl_zcl_sample_mfg_specific_cluster_cluster_command_one_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_sample_mfg_specific_cluster_cluster_command_one_command, (uint8_t*)cmd_struct);
} 


// Signature for CommandTwo command
static const sl_signature_t sig_sample_mfg_specific_cluster_2_cluster_command_two_command = sl_zcl_sample_mfg_specific_cluster_2_cluster_command_two_command_signature;
// Command parser for CommandTwo command
EmberAfStatus zcl_decode_sample_mfg_specific_cluster_2_cluster_command_two_command (EmberAfClusterCommand * cmd, sl_zcl_sample_mfg_specific_cluster_2_cluster_command_two_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_sample_mfg_specific_cluster_2_cluster_command_two_command, (uint8_t*)cmd_struct);
} 


// Signature for SetMacPollFailureWaitTime command
static const sl_signature_t sig_sl_works_with_all_hubs_cluster_set_mac_poll_failure_wait_time_command = sl_zcl_sl_works_with_all_hubs_cluster_set_mac_poll_failure_wait_time_command_signature;
// Command parser for SetMacPollFailureWaitTime command
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_set_mac_poll_failure_wait_time_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_set_mac_poll_failure_wait_time_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_sl_works_with_all_hubs_cluster_set_mac_poll_failure_wait_time_command, (uint8_t*)cmd_struct);
} 


// Signature for DebugReportQuery command
static const sl_signature_t sig_sl_works_with_all_hubs_cluster_debug_report_query_command = sl_zcl_sl_works_with_all_hubs_cluster_debug_report_query_command_signature;
// Command parser for DebugReportQuery command
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_debug_report_query_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_debug_report_query_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_sl_works_with_all_hubs_cluster_debug_report_query_command, (uint8_t*)cmd_struct);
} 


// Signature for MirrorReportAttributeResponse command
static const sl_signature_t sig_simple_metering_cluster_mirror_report_attribute_response_command = sl_zcl_simple_metering_cluster_mirror_report_attribute_response_command_signature;
// Command parser for MirrorReportAttributeResponse command
EmberAfStatus zcl_decode_simple_metering_cluster_mirror_report_attribute_response_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_mirror_report_attribute_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_simple_metering_cluster_mirror_report_attribute_response_command, (uint8_t*)cmd_struct);
} 


// Signature for GeneralResponse command
static const sl_signature_t sig_gaming_cluster_general_response_command = sl_zcl_gaming_cluster_general_response_command_signature;
// Command parser for GeneralResponse command
EmberAfStatus zcl_decode_gaming_cluster_general_response_command (EmberAfClusterCommand * cmd, sl_zcl_gaming_cluster_general_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_gaming_cluster_general_response_command, (uint8_t*)cmd_struct);
} 


// Signature for LocationDataResponse command
static const sl_signature_t sig_rssi_location_cluster_location_data_response_command = sl_zcl_rssi_location_cluster_location_data_response_command_signature;
// Command parser for LocationDataResponse command
EmberAfStatus zcl_decode_rssi_location_cluster_location_data_response_command (EmberAfClusterCommand * cmd, sl_zcl_rssi_location_cluster_location_data_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_rssi_location_cluster_location_data_response_command, (uint8_t*)cmd_struct);
} 


// Signature for GetLocalesSupportedResponse command
static const sl_signature_t sig_basic_cluster_get_locales_supported_response_command = sl_zcl_basic_cluster_get_locales_supported_response_command_signature;
// Command parser for GetLocalesSupportedResponse command
EmberAfStatus zcl_decode_basic_cluster_get_locales_supported_response_command (EmberAfClusterCommand * cmd, sl_zcl_basic_cluster_get_locales_supported_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_basic_cluster_get_locales_supported_response_command, (uint8_t*)cmd_struct);
} 


// Signature for ZoneStatusChanged command
static const sl_signature_t sig_ias_ace_cluster_zone_status_changed_command = sl_zcl_ias_ace_cluster_zone_status_changed_command_signature;
// Command parser for ZoneStatusChanged command
EmberAfStatus zcl_decode_ias_ace_cluster_zone_status_changed_command (EmberAfClusterCommand * cmd, sl_zcl_ias_ace_cluster_zone_status_changed_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_ias_ace_cluster_zone_status_changed_command, (uint8_t*)cmd_struct);
} 


// Signature for GetZoneInformationResponse command
static const sl_signature_t sig_ias_ace_cluster_get_zone_information_response_command = sl_zcl_ias_ace_cluster_get_zone_information_response_command_signature;
// Command parser for GetZoneInformationResponse command
EmberAfStatus zcl_decode_ias_ace_cluster_get_zone_information_response_command (EmberAfClusterCommand * cmd, sl_zcl_ias_ace_cluster_get_zone_information_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_ias_ace_cluster_get_zone_information_response_command, (uint8_t*)cmd_struct);
} 


// Signature for VoiceTransmissionResponse command
static const sl_signature_t sig_voice_over_zig_bee_cluster_voice_transmission_response_command = sl_zcl_voice_over_zig_bee_cluster_voice_transmission_response_command_signature;
// Command parser for VoiceTransmissionResponse command
EmberAfStatus zcl_decode_voice_over_zig_bee_cluster_voice_transmission_response_command (EmberAfClusterCommand * cmd, sl_zcl_voice_over_zig_bee_cluster_voice_transmission_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_voice_over_zig_bee_cluster_voice_transmission_response_command, (uint8_t*)cmd_struct);
} 


// Signature for EventsNotification command
static const sl_signature_t sig_appliance_events_and_alert_cluster_events_notification_command = sl_zcl_appliance_events_and_alert_cluster_events_notification_command_signature;
// Command parser for EventsNotification command
EmberAfStatus zcl_decode_appliance_events_and_alert_cluster_events_notification_command (EmberAfClusterCommand * cmd, sl_zcl_appliance_events_and_alert_cluster_events_notification_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_appliance_events_and_alert_cluster_events_notification_command, (uint8_t*)cmd_struct);
} 


// Signature for GetAlarmResponse command
static const sl_signature_t sig_alarms_cluster_get_alarm_response_command = sl_zcl_alarms_cluster_get_alarm_response_command_signature;
// Command parser for GetAlarmResponse command
EmberAfStatus zcl_decode_alarms_cluster_get_alarm_response_command (EmberAfClusterCommand * cmd, sl_zcl_alarms_cluster_get_alarm_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_alarms_cluster_get_alarm_response_command, (uint8_t*)cmd_struct);
} 


// Signature for MoveToHue command
static const sl_signature_t sig_color_control_cluster_move_to_hue_command = sl_zcl_color_control_cluster_move_to_hue_command_signature;
// Command parser for MoveToHue command
EmberAfStatus zcl_decode_color_control_cluster_move_to_hue_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_move_to_hue_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_color_control_cluster_move_to_hue_command, (uint8_t*)cmd_struct);
} 


// Signature for ProgrammingEventNotification command
static const sl_signature_t sig_door_lock_cluster_programming_event_notification_command = sl_zcl_door_lock_cluster_programming_event_notification_command_signature;
// Command parser for ProgrammingEventNotification command
EmberAfStatus zcl_decode_door_lock_cluster_programming_event_notification_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_programming_event_notification_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_programming_event_notification_command, (uint8_t*)cmd_struct);
} 


// Signature for OperationEventNotification command
static const sl_signature_t sig_door_lock_cluster_operation_event_notification_command = sl_zcl_door_lock_cluster_operation_event_notification_command_signature;
// Command parser for OperationEventNotification command
EmberAfStatus zcl_decode_door_lock_cluster_operation_event_notification_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_operation_event_notification_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_operation_event_notification_command, (uint8_t*)cmd_struct);
} 


// Signature for GetProfileInfoResponseCommand command
static const sl_signature_t sig_electrical_measurement_cluster_get_profile_info_response_command_command = sl_zcl_electrical_measurement_cluster_get_profile_info_response_command_command_signature;
// Command parser for GetProfileInfoResponseCommand command
EmberAfStatus zcl_decode_electrical_measurement_cluster_get_profile_info_response_command_command (EmberAfClusterCommand * cmd, sl_zcl_electrical_measurement_cluster_get_profile_info_response_command_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_electrical_measurement_cluster_get_profile_info_response_command_command, (uint8_t*)cmd_struct);
} 


// Signature for BypassResponse command
static const sl_signature_t sig_ias_ace_cluster_bypass_response_command = sl_zcl_ias_ace_cluster_bypass_response_command_signature;
// Command parser for BypassResponse command
EmberAfStatus zcl_decode_ias_ace_cluster_bypass_response_command (EmberAfClusterCommand * cmd, sl_zcl_ias_ace_cluster_bypass_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_ias_ace_cluster_bypass_response_command, (uint8_t*)cmd_struct);
} 


// Signature for DeviceConfigurationResponse command
static const sl_signature_t sig_rssi_location_cluster_device_configuration_response_command = sl_zcl_rssi_location_cluster_device_configuration_response_command_signature;
// Command parser for DeviceConfigurationResponse command
EmberAfStatus zcl_decode_rssi_location_cluster_device_configuration_response_command (EmberAfClusterCommand * cmd, sl_zcl_rssi_location_cluster_device_configuration_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_rssi_location_cluster_device_configuration_response_command, (uint8_t*)cmd_struct);
} 


// Signature for AddGroupResponse command
static const sl_signature_t sig_groups_cluster_add_group_response_command = sl_zcl_groups_cluster_add_group_response_command_signature;
// Command parser for AddGroupResponse command
EmberAfStatus zcl_decode_groups_cluster_add_group_response_command (EmberAfClusterCommand * cmd, sl_zcl_groups_cluster_add_group_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_groups_cluster_add_group_response_command, (uint8_t*)cmd_struct);
} 


// Signature for RemoveGroupResponse command
static const sl_signature_t sig_groups_cluster_remove_group_response_command = sl_zcl_groups_cluster_remove_group_response_command_signature;
// Command parser for RemoveGroupResponse command
EmberAfStatus zcl_decode_groups_cluster_remove_group_response_command (EmberAfClusterCommand * cmd, sl_zcl_groups_cluster_remove_group_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_groups_cluster_remove_group_response_command, (uint8_t*)cmd_struct);
} 


// Signature for RemoveAllScenesResponse command
static const sl_signature_t sig_scenes_cluster_remove_all_scenes_response_command = sl_zcl_scenes_cluster_remove_all_scenes_response_command_signature;
// Command parser for RemoveAllScenesResponse command
EmberAfStatus zcl_decode_scenes_cluster_remove_all_scenes_response_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_remove_all_scenes_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_scenes_cluster_remove_all_scenes_response_command, (uint8_t*)cmd_struct);
} 


// Signature for MoveToLevelWithOnOff command
static const sl_signature_t sig_level_control_cluster_move_to_level_with_on_off_command = sl_zcl_level_control_cluster_move_to_level_with_on_off_command_signature;
// Command parser for MoveToLevelWithOnOff command
EmberAfStatus zcl_decode_level_control_cluster_move_to_level_with_on_off_command (EmberAfClusterCommand * cmd, sl_zcl_level_control_cluster_move_to_level_with_on_off_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_level_control_cluster_move_to_level_with_on_off_command, (uint8_t*)cmd_struct);
} 


// Signature for GetWeekdaySchedule command
static const sl_signature_t sig_door_lock_cluster_get_weekday_schedule_command = sl_zcl_door_lock_cluster_get_weekday_schedule_command_signature;
// Command parser for GetWeekdaySchedule command
EmberAfStatus zcl_decode_door_lock_cluster_get_weekday_schedule_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_weekday_schedule_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_get_weekday_schedule_command, (uint8_t*)cmd_struct);
} 


// Signature for ClearWeekdaySchedule command
static const sl_signature_t sig_door_lock_cluster_clear_weekday_schedule_command = sl_zcl_door_lock_cluster_clear_weekday_schedule_command_signature;
// Command parser for ClearWeekdaySchedule command
EmberAfStatus zcl_decode_door_lock_cluster_clear_weekday_schedule_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_clear_weekday_schedule_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_clear_weekday_schedule_command, (uint8_t*)cmd_struct);
} 


// Signature for GetYeardaySchedule command
static const sl_signature_t sig_door_lock_cluster_get_yearday_schedule_command = sl_zcl_door_lock_cluster_get_yearday_schedule_command_signature;
// Command parser for GetYeardaySchedule command
EmberAfStatus zcl_decode_door_lock_cluster_get_yearday_schedule_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_yearday_schedule_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_get_yearday_schedule_command, (uint8_t*)cmd_struct);
} 


// Signature for ClearYeardaySchedule command
static const sl_signature_t sig_door_lock_cluster_clear_yearday_schedule_command = sl_zcl_door_lock_cluster_clear_yearday_schedule_command_signature;
// Command parser for ClearYeardaySchedule command
EmberAfStatus zcl_decode_door_lock_cluster_clear_yearday_schedule_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_clear_yearday_schedule_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_clear_yearday_schedule_command, (uint8_t*)cmd_struct);
} 


// Signature for SetPendingNetworkUpdate command
static const sl_signature_t sig_sl_works_with_all_hubs_cluster_set_pending_network_update_command = sl_zcl_sl_works_with_all_hubs_cluster_set_pending_network_update_command_signature;
// Command parser for SetPendingNetworkUpdate command
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_set_pending_network_update_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_set_pending_network_update_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_sl_works_with_all_hubs_cluster_set_pending_network_update_command, (uint8_t*)cmd_struct);
} 


// Signature for GetNotifiedMessage command
static const sl_signature_t sig_simple_metering_cluster_get_notified_message_command = sl_zcl_simple_metering_cluster_get_notified_message_command_signature;
// Command parser for GetNotifiedMessage command
EmberAfStatus zcl_decode_simple_metering_cluster_get_notified_message_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_get_notified_message_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_simple_metering_cluster_get_notified_message_command, (uint8_t*)cmd_struct);
} 


// Signature for SetWeekdaySchedule command
static const sl_signature_t sig_door_lock_cluster_set_weekday_schedule_command = sl_zcl_door_lock_cluster_set_weekday_schedule_command_signature;
// Command parser for SetWeekdaySchedule command
EmberAfStatus zcl_decode_door_lock_cluster_set_weekday_schedule_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_set_weekday_schedule_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_set_weekday_schedule_command, (uint8_t*)cmd_struct);
} 


// Signature for MoveToLevel command
static const sl_signature_t sig_level_control_cluster_move_to_level_command = sl_zcl_level_control_cluster_move_to_level_command_signature;
// Command parser for MoveToLevel command
EmberAfStatus zcl_decode_level_control_cluster_move_to_level_command (EmberAfClusterCommand * cmd, sl_zcl_level_control_cluster_move_to_level_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_level_control_cluster_move_to_level_command, (uint8_t*)cmd_struct);
} 


// Signature for MoveToSaturation command
static const sl_signature_t sig_color_control_cluster_move_to_saturation_command = sl_zcl_color_control_cluster_move_to_saturation_command_signature;
// Command parser for MoveToSaturation command
EmberAfStatus zcl_decode_color_control_cluster_move_to_saturation_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_move_to_saturation_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_color_control_cluster_move_to_saturation_command, (uint8_t*)cmd_struct);
} 


// Signature for RequestTunnel command
static const sl_signature_t sig_tunneling_cluster_request_tunnel_command = sl_zcl_tunneling_cluster_request_tunnel_command_signature;
// Command parser for RequestTunnel command
EmberAfStatus zcl_decode_tunneling_cluster_request_tunnel_command (EmberAfClusterCommand * cmd, sl_zcl_tunneling_cluster_request_tunnel_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_tunneling_cluster_request_tunnel_command, (uint8_t*)cmd_struct);
} 


// Signature for ViewGroupResponse command
static const sl_signature_t sig_groups_cluster_view_group_response_command = sl_zcl_groups_cluster_view_group_response_command_signature;
// Command parser for ViewGroupResponse command
EmberAfStatus zcl_decode_groups_cluster_view_group_response_command (EmberAfClusterCommand * cmd, sl_zcl_groups_cluster_view_group_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_groups_cluster_view_group_response_command, (uint8_t*)cmd_struct);
} 


// Signature for PowerProfileScheduleConstraintsNotification command
static const sl_signature_t sig_power_profile_cluster_power_profile_schedule_constraints_notification_command = sl_zcl_power_profile_cluster_power_profile_schedule_constraints_notification_command_signature;
// Command parser for PowerProfileScheduleConstraintsNotification command
EmberAfStatus zcl_decode_power_profile_cluster_power_profile_schedule_constraints_notification_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_power_profile_schedule_constraints_notification_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_power_profile_cluster_power_profile_schedule_constraints_notification_command, (uint8_t*)cmd_struct);
} 


// Signature for PowerProfileScheduleConstraintsResponse command
static const sl_signature_t sig_power_profile_cluster_power_profile_schedule_constraints_response_command = sl_zcl_power_profile_cluster_power_profile_schedule_constraints_response_command_signature;
// Command parser for PowerProfileScheduleConstraintsResponse command
EmberAfStatus zcl_decode_power_profile_cluster_power_profile_schedule_constraints_response_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_power_profile_schedule_constraints_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_power_profile_cluster_power_profile_schedule_constraints_response_command, (uint8_t*)cmd_struct);
} 


// Signature for UpgradeEndRequest command
static const sl_signature_t sig_over_the_air_bootloading_cluster_upgrade_end_request_command = sl_zcl_over_the_air_bootloading_cluster_upgrade_end_request_command_signature;
// Command parser for UpgradeEndRequest command
EmberAfStatus zcl_decode_over_the_air_bootloading_cluster_upgrade_end_request_command (EmberAfClusterCommand * cmd, sl_zcl_over_the_air_bootloading_cluster_upgrade_end_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_over_the_air_bootloading_cluster_upgrade_end_request_command, (uint8_t*)cmd_struct);
} 


// Signature for QueryNextImageRequest command
static const sl_signature_t sig_over_the_air_bootloading_cluster_query_next_image_request_command = sl_zcl_over_the_air_bootloading_cluster_query_next_image_request_command_signature;
// Command parser for QueryNextImageRequest command
EmberAfStatus zcl_decode_over_the_air_bootloading_cluster_query_next_image_request_command (EmberAfClusterCommand * cmd, sl_zcl_over_the_air_bootloading_cluster_query_next_image_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_over_the_air_bootloading_cluster_query_next_image_request_command, (uint8_t*)cmd_struct);
} 


// Signature for QueryNextImageResponse command
static const sl_signature_t sig_over_the_air_bootloading_cluster_query_next_image_response_command = sl_zcl_over_the_air_bootloading_cluster_query_next_image_response_command_signature;
// Command parser for QueryNextImageResponse command
EmberAfStatus zcl_decode_over_the_air_bootloading_cluster_query_next_image_response_command (EmberAfClusterCommand * cmd, sl_zcl_over_the_air_bootloading_cluster_query_next_image_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_over_the_air_bootloading_cluster_query_next_image_response_command, (uint8_t*)cmd_struct);
} 


// Signature for QuerySpecificFileResponse command
static const sl_signature_t sig_over_the_air_bootloading_cluster_query_specific_file_response_command = sl_zcl_over_the_air_bootloading_cluster_query_specific_file_response_command_signature;
// Command parser for QuerySpecificFileResponse command
EmberAfStatus zcl_decode_over_the_air_bootloading_cluster_query_specific_file_response_command (EmberAfClusterCommand * cmd, sl_zcl_over_the_air_bootloading_cluster_query_specific_file_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_over_the_air_bootloading_cluster_query_specific_file_response_command, (uint8_t*)cmd_struct);
} 


// Signature for ImageBlockRequest command
static const sl_signature_t sig_over_the_air_bootloading_cluster_image_block_request_command = sl_zcl_over_the_air_bootloading_cluster_image_block_request_command_signature;
// Command parser for ImageBlockRequest command
EmberAfStatus zcl_decode_over_the_air_bootloading_cluster_image_block_request_command (EmberAfClusterCommand * cmd, sl_zcl_over_the_air_bootloading_cluster_image_block_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_over_the_air_bootloading_cluster_image_block_request_command, (uint8_t*)cmd_struct);
} 


// Signature for ImagePageRequest command
static const sl_signature_t sig_over_the_air_bootloading_cluster_image_page_request_command = sl_zcl_over_the_air_bootloading_cluster_image_page_request_command_signature;
// Command parser for ImagePageRequest command
EmberAfStatus zcl_decode_over_the_air_bootloading_cluster_image_page_request_command (EmberAfClusterCommand * cmd, sl_zcl_over_the_air_bootloading_cluster_image_page_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_over_the_air_bootloading_cluster_image_page_request_command, (uint8_t*)cmd_struct);
} 


// Signature for ImageBlockResponse command
static const sl_signature_t sig_over_the_air_bootloading_cluster_image_block_response_command = sl_zcl_over_the_air_bootloading_cluster_image_block_response_command_signature;
// Command parser for ImageBlockResponse command
EmberAfStatus zcl_decode_over_the_air_bootloading_cluster_image_block_response_command (EmberAfClusterCommand * cmd, sl_zcl_over_the_air_bootloading_cluster_image_block_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_over_the_air_bootloading_cluster_image_block_response_command, (uint8_t*)cmd_struct);
} 


// Signature for SetYeardaySchedule command
static const sl_signature_t sig_door_lock_cluster_set_yearday_schedule_command = sl_zcl_door_lock_cluster_set_yearday_schedule_command_signature;
// Command parser for SetYeardaySchedule command
EmberAfStatus zcl_decode_door_lock_cluster_set_yearday_schedule_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_set_yearday_schedule_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_set_yearday_schedule_command, (uint8_t*)cmd_struct);
} 


// Signature for GetPowerProfilePriceResponse command
static const sl_signature_t sig_power_profile_cluster_get_power_profile_price_response_command = sl_zcl_power_profile_cluster_get_power_profile_price_response_command_signature;
// Command parser for GetPowerProfilePriceResponse command
EmberAfStatus zcl_decode_power_profile_cluster_get_power_profile_price_response_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_get_power_profile_price_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_power_profile_cluster_get_power_profile_price_response_command, (uint8_t*)cmd_struct);
} 


// Signature for GetPowerProfilePriceExtendedResponse command
static const sl_signature_t sig_power_profile_cluster_get_power_profile_price_extended_response_command = sl_zcl_power_profile_cluster_get_power_profile_price_extended_response_command_signature;
// Command parser for GetPowerProfilePriceExtendedResponse command
EmberAfStatus zcl_decode_power_profile_cluster_get_power_profile_price_extended_response_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_get_power_profile_price_extended_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_power_profile_cluster_get_power_profile_price_extended_response_command, (uint8_t*)cmd_struct);
} 


// Signature for AddSceneResponse command
static const sl_signature_t sig_scenes_cluster_add_scene_response_command = sl_zcl_scenes_cluster_add_scene_response_command_signature;
// Command parser for AddSceneResponse command
EmberAfStatus zcl_decode_scenes_cluster_add_scene_response_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_add_scene_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_scenes_cluster_add_scene_response_command, (uint8_t*)cmd_struct);
} 


// Signature for RemoveSceneResponse command
static const sl_signature_t sig_scenes_cluster_remove_scene_response_command = sl_zcl_scenes_cluster_remove_scene_response_command_signature;
// Command parser for RemoveSceneResponse command
EmberAfStatus zcl_decode_scenes_cluster_remove_scene_response_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_remove_scene_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_scenes_cluster_remove_scene_response_command, (uint8_t*)cmd_struct);
} 


// Signature for StoreSceneResponse command
static const sl_signature_t sig_scenes_cluster_store_scene_response_command = sl_zcl_scenes_cluster_store_scene_response_command_signature;
// Command parser for StoreSceneResponse command
EmberAfStatus zcl_decode_scenes_cluster_store_scene_response_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_store_scene_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_scenes_cluster_store_scene_response_command, (uint8_t*)cmd_struct);
} 


// Signature for EnhancedAddSceneResponse command
static const sl_signature_t sig_scenes_cluster_enhanced_add_scene_response_command = sl_zcl_scenes_cluster_enhanced_add_scene_response_command_signature;
// Command parser for EnhancedAddSceneResponse command
EmberAfStatus zcl_decode_scenes_cluster_enhanced_add_scene_response_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_enhanced_add_scene_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_scenes_cluster_enhanced_add_scene_response_command, (uint8_t*)cmd_struct);
} 


// Signature for CopySceneResponse command
static const sl_signature_t sig_scenes_cluster_copy_scene_response_command = sl_zcl_scenes_cluster_copy_scene_response_command_signature;
// Command parser for CopySceneResponse command
EmberAfStatus zcl_decode_scenes_cluster_copy_scene_response_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_copy_scene_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_scenes_cluster_copy_scene_response_command, (uint8_t*)cmd_struct);
} 


// Signature for EnhancedViewSceneResponse command
static const sl_signature_t sig_scenes_cluster_enhanced_view_scene_response_command = sl_zcl_scenes_cluster_enhanced_view_scene_response_command_signature;
// Command parser for EnhancedViewSceneResponse command
EmberAfStatus zcl_decode_scenes_cluster_enhanced_view_scene_response_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_enhanced_view_scene_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_scenes_cluster_enhanced_view_scene_response_command, (uint8_t*)cmd_struct);
} 


// Signature for GetYeardayScheduleResponse command
static const sl_signature_t sig_door_lock_cluster_get_yearday_schedule_response_command = sl_zcl_door_lock_cluster_get_yearday_schedule_response_command_signature;
// Command parser for GetYeardayScheduleResponse command
EmberAfStatus zcl_decode_door_lock_cluster_get_yearday_schedule_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_yearday_schedule_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_get_yearday_schedule_response_command, (uint8_t*)cmd_struct);
} 


// Signature for GetWeekdayScheduleResponse command
static const sl_signature_t sig_door_lock_cluster_get_weekday_schedule_response_command = sl_zcl_door_lock_cluster_get_weekday_schedule_response_command_signature;
// Command parser for GetWeekdayScheduleResponse command
EmberAfStatus zcl_decode_door_lock_cluster_get_weekday_schedule_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_weekday_schedule_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_get_weekday_schedule_response_command, (uint8_t*)cmd_struct);
} 


// Signature for RequestPreferenceResponse command
static const sl_signature_t sig_information_cluster_request_preference_response_command = sl_zcl_information_cluster_request_preference_response_command_signature;
// Command parser for RequestPreferenceResponse command
EmberAfStatus zcl_decode_information_cluster_request_preference_response_command (EmberAfClusterCommand * cmd, sl_zcl_information_cluster_request_preference_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_information_cluster_request_preference_response_command, (uint8_t*)cmd_struct);
} 


// Signature for GetGroupMembership command
static const sl_signature_t sig_groups_cluster_get_group_membership_command = sl_zcl_groups_cluster_get_group_membership_command_signature;
// Command parser for GetGroupMembership command
EmberAfStatus zcl_decode_groups_cluster_get_group_membership_command (EmberAfClusterCommand * cmd, sl_zcl_groups_cluster_get_group_membership_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_groups_cluster_get_group_membership_command, (uint8_t*)cmd_struct);
} 


// Signature for NewDebugReportNotification command
static const sl_signature_t sig_sl_works_with_all_hubs_cluster_new_debug_report_notification_command = sl_zcl_sl_works_with_all_hubs_cluster_new_debug_report_notification_command_signature;
// Command parser for NewDebugReportNotification command
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_new_debug_report_notification_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_new_debug_report_notification_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_sl_works_with_all_hubs_cluster_new_debug_report_notification_command, (uint8_t*)cmd_struct);
} 


// Signature for SetHolidaySchedule command
static const sl_signature_t sig_door_lock_cluster_set_holiday_schedule_command = sl_zcl_door_lock_cluster_set_holiday_schedule_command_signature;
// Command parser for SetHolidaySchedule command
EmberAfStatus zcl_decode_door_lock_cluster_set_holiday_schedule_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_set_holiday_schedule_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_set_holiday_schedule_command, (uint8_t*)cmd_struct);
} 


// Signature for LogQueueResponse command
static const sl_signature_t sig_appliance_statistics_cluster_log_queue_response_command = sl_zcl_appliance_statistics_cluster_log_queue_response_command_signature;
// Command parser for LogQueueResponse command
EmberAfStatus zcl_decode_appliance_statistics_cluster_log_queue_response_command (EmberAfClusterCommand * cmd, sl_zcl_appliance_statistics_cluster_log_queue_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_appliance_statistics_cluster_log_queue_response_command, (uint8_t*)cmd_struct);
} 


// Signature for StatisticsAvailable command
static const sl_signature_t sig_appliance_statistics_cluster_statistics_available_command = sl_zcl_appliance_statistics_cluster_statistics_available_command_signature;
// Command parser for StatisticsAvailable command
EmberAfStatus zcl_decode_appliance_statistics_cluster_statistics_available_command (EmberAfClusterCommand * cmd, sl_zcl_appliance_statistics_cluster_statistics_available_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_appliance_statistics_cluster_statistics_available_command, (uint8_t*)cmd_struct);
} 


// Signature for stream command
static const sl_signature_t sig_mfglib_cluster_cluster_stream_command = sl_zcl_mfglib_cluster_cluster_stream_command_signature;
// Command parser for stream command
EmberAfStatus zcl_decode_mfglib_cluster_cluster_stream_command (EmberAfClusterCommand * cmd, sl_zcl_mfglib_cluster_cluster_stream_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_mfglib_cluster_cluster_stream_command, (uint8_t*)cmd_struct);
} 


// Signature for tone command
static const sl_signature_t sig_mfglib_cluster_cluster_tone_command = sl_zcl_mfglib_cluster_cluster_tone_command_signature;
// Command parser for tone command
EmberAfStatus zcl_decode_mfglib_cluster_cluster_tone_command (EmberAfClusterCommand * cmd, sl_zcl_mfglib_cluster_cluster_tone_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_mfglib_cluster_cluster_tone_command, (uint8_t*)cmd_struct);
} 


// Signature for rxMode command
static const sl_signature_t sig_mfglib_cluster_cluster_rx_mode_command = sl_zcl_mfglib_cluster_cluster_rx_mode_command_signature;
// Command parser for rxMode command
EmberAfStatus zcl_decode_mfglib_cluster_cluster_rx_mode_command (EmberAfClusterCommand * cmd, sl_zcl_mfglib_cluster_cluster_rx_mode_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_mfglib_cluster_cluster_rx_mode_command, (uint8_t*)cmd_struct);
} 


// Signature for InitiateTestMode command
static const sl_signature_t sig_ias_zone_cluster_initiate_test_mode_command = sl_zcl_ias_zone_cluster_initiate_test_mode_command_signature;
// Command parser for InitiateTestMode command
EmberAfStatus zcl_decode_ias_zone_cluster_initiate_test_mode_command (EmberAfClusterCommand * cmd, sl_zcl_ias_zone_cluster_initiate_test_mode_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_ias_zone_cluster_initiate_test_mode_command, (uint8_t*)cmd_struct);
} 


// Signature for RequestFastPollMode command
static const sl_signature_t sig_simple_metering_cluster_request_fast_poll_mode_command = sl_zcl_simple_metering_cluster_request_fast_poll_mode_command_signature;
// Command parser for RequestFastPollMode command
EmberAfStatus zcl_decode_simple_metering_cluster_request_fast_poll_mode_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_request_fast_poll_mode_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_simple_metering_cluster_request_fast_poll_mode_command, (uint8_t*)cmd_struct);
} 


// Signature for GetZoneStatus command
static const sl_signature_t sig_ias_ace_cluster_get_zone_status_command = sl_zcl_ias_ace_cluster_get_zone_status_command_signature;
// Command parser for GetZoneStatus command
EmberAfStatus zcl_decode_ias_ace_cluster_get_zone_status_command (EmberAfClusterCommand * cmd, sl_zcl_ias_ace_cluster_get_zone_status_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_ias_ace_cluster_get_zone_status_command, (uint8_t*)cmd_struct);
} 


// Signature for MoveToHueAndSaturation command
static const sl_signature_t sig_color_control_cluster_move_to_hue_and_saturation_command = sl_zcl_color_control_cluster_move_to_hue_and_saturation_command_signature;
// Command parser for MoveToHueAndSaturation command
EmberAfStatus zcl_decode_color_control_cluster_move_to_hue_and_saturation_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_move_to_hue_and_saturation_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_color_control_cluster_move_to_hue_and_saturation_command, (uint8_t*)cmd_struct);
} 


// Signature for GetGroupMembershipResponse command
static const sl_signature_t sig_groups_cluster_get_group_membership_response_command = sl_zcl_groups_cluster_get_group_membership_response_command_signature;
// Command parser for GetGroupMembershipResponse command
EmberAfStatus zcl_decode_groups_cluster_get_group_membership_response_command (EmberAfClusterCommand * cmd, sl_zcl_groups_cluster_get_group_membership_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_groups_cluster_get_group_membership_response_command, (uint8_t*)cmd_struct);
} 


// Signature for EnableWwahAppEventRetryAlgorithm command
static const sl_signature_t sig_sl_works_with_all_hubs_cluster_enable_wwah_app_event_retry_algorithm_command = sl_zcl_sl_works_with_all_hubs_cluster_enable_wwah_app_event_retry_algorithm_command_signature;
// Command parser for EnableWwahAppEventRetryAlgorithm command
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_enable_wwah_app_event_retry_algorithm_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_enable_wwah_app_event_retry_algorithm_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_sl_works_with_all_hubs_cluster_enable_wwah_app_event_retry_algorithm_command, (uint8_t*)cmd_struct);
} 


// Signature for GetHolidayScheduleResponse command
static const sl_signature_t sig_door_lock_cluster_get_holiday_schedule_response_command = sl_zcl_door_lock_cluster_get_holiday_schedule_response_command_signature;
// Command parser for GetHolidayScheduleResponse command
EmberAfStatus zcl_decode_door_lock_cluster_get_holiday_schedule_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_holiday_schedule_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_door_lock_cluster_get_holiday_schedule_response_command, (uint8_t*)cmd_struct);
} 


// Signature for PowerProfileNotification command
static const sl_signature_t sig_power_profile_cluster_power_profile_notification_command = sl_zcl_power_profile_cluster_power_profile_notification_command_signature;
// Command parser for PowerProfileNotification command
EmberAfStatus zcl_decode_power_profile_cluster_power_profile_notification_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_power_profile_notification_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_power_profile_cluster_power_profile_notification_command, (uint8_t*)cmd_struct);
} 


// Signature for PowerProfileResponse command
static const sl_signature_t sig_power_profile_cluster_power_profile_response_command = sl_zcl_power_profile_cluster_power_profile_response_command_signature;
// Command parser for PowerProfileResponse command
EmberAfStatus zcl_decode_power_profile_cluster_power_profile_response_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_power_profile_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_power_profile_cluster_power_profile_response_command, (uint8_t*)cmd_struct);
} 


// Signature for GetGroupIdentifiersResponse command
static const sl_signature_t sig_zll_commissioning_cluster_get_group_identifiers_response_command = sl_zcl_zll_commissioning_cluster_get_group_identifiers_response_command_signature;
// Command parser for GetGroupIdentifiersResponse command
EmberAfStatus zcl_decode_zll_commissioning_cluster_get_group_identifiers_response_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_get_group_identifiers_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_zll_commissioning_cluster_get_group_identifiers_response_command, (uint8_t*)cmd_struct);
} 


// Signature for GetEndpointListResponse command
static const sl_signature_t sig_zll_commissioning_cluster_get_endpoint_list_response_command = sl_zcl_zll_commissioning_cluster_get_endpoint_list_response_command_signature;
// Command parser for GetEndpointListResponse command
EmberAfStatus zcl_decode_zll_commissioning_cluster_get_endpoint_list_response_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_get_endpoint_list_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_zll_commissioning_cluster_get_endpoint_list_response_command, (uint8_t*)cmd_struct);
} 


// Signature for EnergyPhasesScheduleNotification command
static const sl_signature_t sig_power_profile_cluster_energy_phases_schedule_notification_command = sl_zcl_power_profile_cluster_energy_phases_schedule_notification_command_signature;
// Command parser for EnergyPhasesScheduleNotification command
EmberAfStatus zcl_decode_power_profile_cluster_energy_phases_schedule_notification_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_energy_phases_schedule_notification_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_power_profile_cluster_energy_phases_schedule_notification_command, (uint8_t*)cmd_struct);
} 


// Signature for EnergyPhasesScheduleResponse command
static const sl_signature_t sig_power_profile_cluster_energy_phases_schedule_response_command = sl_zcl_power_profile_cluster_energy_phases_schedule_response_command_signature;
// Command parser for EnergyPhasesScheduleResponse command
EmberAfStatus zcl_decode_power_profile_cluster_energy_phases_schedule_response_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_energy_phases_schedule_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_power_profile_cluster_energy_phases_schedule_response_command, (uint8_t*)cmd_struct);
} 


// Signature for EnergyPhasesScheduleStateResponse command
static const sl_signature_t sig_power_profile_cluster_energy_phases_schedule_state_response_command = sl_zcl_power_profile_cluster_energy_phases_schedule_state_response_command_signature;
// Command parser for EnergyPhasesScheduleStateResponse command
EmberAfStatus zcl_decode_power_profile_cluster_energy_phases_schedule_state_response_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_energy_phases_schedule_state_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_power_profile_cluster_energy_phases_schedule_state_response_command, (uint8_t*)cmd_struct);
} 


// Signature for EnergyPhasesScheduleStateNotification command
static const sl_signature_t sig_power_profile_cluster_energy_phases_schedule_state_notification_command = sl_zcl_power_profile_cluster_energy_phases_schedule_state_notification_command_signature;
// Command parser for EnergyPhasesScheduleStateNotification command
EmberAfStatus zcl_decode_power_profile_cluster_energy_phases_schedule_state_notification_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_energy_phases_schedule_state_notification_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_power_profile_cluster_energy_phases_schedule_state_notification_command, (uint8_t*)cmd_struct);
} 


// Signature for PublishTopUpLog command
static const sl_signature_t sig_prepayment_cluster_publish_top_up_log_command = sl_zcl_prepayment_cluster_publish_top_up_log_command_signature;
// Command parser for PublishTopUpLog command
EmberAfStatus zcl_decode_prepayment_cluster_publish_top_up_log_command (EmberAfClusterCommand * cmd, sl_zcl_prepayment_cluster_publish_top_up_log_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_prepayment_cluster_publish_top_up_log_command, (uint8_t*)cmd_struct);
} 


// Signature for PublishDebtLog command
static const sl_signature_t sig_prepayment_cluster_publish_debt_log_command = sl_zcl_prepayment_cluster_publish_debt_log_command_signature;
// Command parser for PublishDebtLog command
EmberAfStatus zcl_decode_prepayment_cluster_publish_debt_log_command (EmberAfClusterCommand * cmd, sl_zcl_prepayment_cluster_publish_debt_log_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_prepayment_cluster_publish_debt_log_command, (uint8_t*)cmd_struct);
} 


// Signature for ReportEventConfiguration command
static const sl_signature_t sig_device_management_cluster_report_event_configuration_command = sl_zcl_device_management_cluster_report_event_configuration_command_signature;
// Command parser for ReportEventConfiguration command
EmberAfStatus zcl_decode_device_management_cluster_report_event_configuration_command (EmberAfClusterCommand * cmd, sl_zcl_device_management_cluster_report_event_configuration_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_device_management_cluster_report_event_configuration_command, (uint8_t*)cmd_struct);
} 


// Signature for SetBypassedZoneList command
static const sl_signature_t sig_ias_ace_cluster_set_bypassed_zone_list_command = sl_zcl_ias_ace_cluster_set_bypassed_zone_list_command_signature;
// Command parser for SetBypassedZoneList command
EmberAfStatus zcl_decode_ias_ace_cluster_set_bypassed_zone_list_command (EmberAfClusterCommand * cmd, sl_zcl_ias_ace_cluster_set_bypassed_zone_list_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_ias_ace_cluster_set_bypassed_zone_list_command, (uint8_t*)cmd_struct);
} 


// Signature for RequestInformationResponse command
static const sl_signature_t sig_information_cluster_request_information_response_command = sl_zcl_information_cluster_request_information_response_command_signature;
// Command parser for RequestInformationResponse command
EmberAfStatus zcl_decode_information_cluster_request_information_response_command (EmberAfClusterCommand * cmd, sl_zcl_information_cluster_request_information_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_information_cluster_request_information_response_command, (uint8_t*)cmd_struct);
} 


// Signature for DebugReportQueryResponse command
static const sl_signature_t sig_sl_works_with_all_hubs_cluster_debug_report_query_response_command = sl_zcl_sl_works_with_all_hubs_cluster_debug_report_query_response_command_signature;
// Command parser for DebugReportQueryResponse command
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_debug_report_query_response_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_debug_report_query_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_sl_works_with_all_hubs_cluster_debug_report_query_response_command, (uint8_t*)cmd_struct);
} 


// Signature for PowerProfileStateResponse command
static const sl_signature_t sig_power_profile_cluster_power_profile_state_response_command = sl_zcl_power_profile_cluster_power_profile_state_response_command_signature;
// Command parser for PowerProfileStateResponse command
EmberAfStatus zcl_decode_power_profile_cluster_power_profile_state_response_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_power_profile_state_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_power_profile_cluster_power_profile_state_response_command, (uint8_t*)cmd_struct);
} 


// Signature for PowerProfilesStateNotification command
static const sl_signature_t sig_power_profile_cluster_power_profiles_state_notification_command = sl_zcl_power_profile_cluster_power_profiles_state_notification_command_signature;
// Command parser for PowerProfilesStateNotification command
EmberAfStatus zcl_decode_power_profile_cluster_power_profiles_state_notification_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_power_profiles_state_notification_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_power_profile_cluster_power_profiles_state_notification_command, (uint8_t*)cmd_struct);
} 


// Signature for RequestFastPollModeResponse command
static const sl_signature_t sig_simple_metering_cluster_request_fast_poll_mode_response_command = sl_zcl_simple_metering_cluster_request_fast_poll_mode_response_command_signature;
// Command parser for RequestFastPollModeResponse command
EmberAfStatus zcl_decode_simple_metering_cluster_request_fast_poll_mode_response_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_request_fast_poll_mode_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_simple_metering_cluster_request_fast_poll_mode_response_command, (uint8_t*)cmd_struct);
} 


// Signature for GpTranslationTableRequest command
static const sl_signature_t sig_green_power_cluster_gp_translation_table_request_command = sl_zcl_green_power_cluster_gp_translation_table_request_command_signature;
// Command parser for GpTranslationTableRequest command
EmberAfStatus zcl_decode_green_power_cluster_gp_translation_table_request_command (EmberAfClusterCommand * cmd, sl_zcl_green_power_cluster_gp_translation_table_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_green_power_cluster_gp_translation_table_request_command, (uint8_t*)cmd_struct);
} 


// Signature for PushInformation command
static const sl_signature_t sig_information_cluster_push_information_command = sl_zcl_information_cluster_push_information_command_signature;
// Command parser for PushInformation command
EmberAfStatus zcl_decode_information_cluster_push_information_command (EmberAfClusterCommand * cmd, sl_zcl_information_cluster_push_information_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_information_cluster_push_information_command, (uint8_t*)cmd_struct);
} 


// Signature for SendPreferenceResponse command
static const sl_signature_t sig_information_cluster_send_preference_response_command = sl_zcl_information_cluster_send_preference_response_command_signature;
// Command parser for SendPreferenceResponse command
EmberAfStatus zcl_decode_information_cluster_send_preference_response_command (EmberAfClusterCommand * cmd, sl_zcl_information_cluster_send_preference_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_information_cluster_send_preference_response_command, (uint8_t*)cmd_struct);
} 


// Signature for RequestPreferenceConfirmation command
static const sl_signature_t sig_information_cluster_request_preference_confirmation_command = sl_zcl_information_cluster_request_preference_confirmation_command_signature;
// Command parser for RequestPreferenceConfirmation command
EmberAfStatus zcl_decode_information_cluster_request_preference_confirmation_command (EmberAfClusterCommand * cmd, sl_zcl_information_cluster_request_preference_confirmation_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_information_cluster_request_preference_confirmation_command, (uint8_t*)cmd_struct);
} 


// Signature for VoiceTransmission command
static const sl_signature_t sig_voice_over_zig_bee_cluster_voice_transmission_command = sl_zcl_voice_over_zig_bee_cluster_voice_transmission_command_signature;
// Command parser for VoiceTransmission command
EmberAfStatus zcl_decode_voice_over_zig_bee_cluster_voice_transmission_command (EmberAfClusterCommand * cmd, sl_zcl_voice_over_zig_bee_cluster_voice_transmission_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_voice_over_zig_bee_cluster_voice_transmission_command, (uint8_t*)cmd_struct);
} 


// Signature for MatchProtocolAddress command
static const sl_signature_t sig_generic_tunnel_cluster_match_protocol_address_command = sl_zcl_generic_tunnel_cluster_match_protocol_address_command_signature;
// Command parser for MatchProtocolAddress command
EmberAfStatus zcl_decode_generic_tunnel_cluster_match_protocol_address_command (EmberAfClusterCommand * cmd, sl_zcl_generic_tunnel_cluster_match_protocol_address_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_generic_tunnel_cluster_match_protocol_address_command, (uint8_t*)cmd_struct);
} 


// Signature for AdvertiseProtocolAddress command
static const sl_signature_t sig_generic_tunnel_cluster_advertise_protocol_address_command = sl_zcl_generic_tunnel_cluster_advertise_protocol_address_command_signature;
// Command parser for AdvertiseProtocolAddress command
EmberAfStatus zcl_decode_generic_tunnel_cluster_advertise_protocol_address_command (EmberAfClusterCommand * cmd, sl_zcl_generic_tunnel_cluster_advertise_protocol_address_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_generic_tunnel_cluster_advertise_protocol_address_command, (uint8_t*)cmd_struct);
} 


// Signature for TransferAPDU command
static const sl_signature_t sig_11073_protocol_tunnel_cluster_transfer_apdu_command = sl_zcl_11073_protocol_tunnel_cluster_transfer_apdu_command_signature;
// Command parser for TransferAPDU command
EmberAfStatus zcl_decode_11073_protocol_tunnel_cluster_transfer_apdu_command (EmberAfClusterCommand * cmd, sl_zcl_11073_protocol_tunnel_cluster_transfer_apdu_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_11073_protocol_tunnel_cluster_transfer_apdu_command, (uint8_t*)cmd_struct);
} 


// Signature for TransferApduFromClient command
static const sl_signature_t sig_iso_7816_protocol_tunnel_cluster_transfer_apdu_from_client_command = sl_zcl_iso_7816_protocol_tunnel_cluster_transfer_apdu_from_client_command_signature;
// Command parser for TransferApduFromClient command
EmberAfStatus zcl_decode_iso_7816_protocol_tunnel_cluster_transfer_apdu_from_client_command (EmberAfClusterCommand * cmd, sl_zcl_iso_7816_protocol_tunnel_cluster_transfer_apdu_from_client_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_iso_7816_protocol_tunnel_cluster_transfer_apdu_from_client_command, (uint8_t*)cmd_struct);
} 


// Signature for TransferApduFromServer command
static const sl_signature_t sig_iso_7816_protocol_tunnel_cluster_transfer_apdu_from_server_command = sl_zcl_iso_7816_protocol_tunnel_cluster_transfer_apdu_from_server_command_signature;
// Command parser for TransferApduFromServer command
EmberAfStatus zcl_decode_iso_7816_protocol_tunnel_cluster_transfer_apdu_from_server_command (EmberAfClusterCommand * cmd, sl_zcl_iso_7816_protocol_tunnel_cluster_transfer_apdu_from_server_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_iso_7816_protocol_tunnel_cluster_transfer_apdu_from_server_command, (uint8_t*)cmd_struct);
} 


// Signature for UnlockTokens command
static const sl_signature_t sig_configuration_cluster_cluster_unlock_tokens_command = sl_zcl_configuration_cluster_cluster_unlock_tokens_command_signature;
// Command parser for UnlockTokens command
EmberAfStatus zcl_decode_configuration_cluster_cluster_unlock_tokens_command (EmberAfClusterCommand * cmd, sl_zcl_configuration_cluster_cluster_unlock_tokens_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_configuration_cluster_cluster_unlock_tokens_command, (uint8_t*)cmd_struct);
} 


// Signature for TransactionEnd command
static const sl_signature_t sig_payment_cluster_transaction_end_command = sl_zcl_payment_cluster_transaction_end_command_signature;
// Command parser for TransactionEnd command
EmberAfStatus zcl_decode_payment_cluster_transaction_end_command (EmberAfClusterCommand * cmd, sl_zcl_payment_cluster_transaction_end_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_payment_cluster_transaction_end_command, (uint8_t*)cmd_struct);
} 


// Signature for BillStatusNotification command
static const sl_signature_t sig_billing_cluster_bill_status_notification_command = sl_zcl_billing_cluster_bill_status_notification_command_signature;
// Command parser for BillStatusNotification command
EmberAfStatus zcl_decode_billing_cluster_bill_status_notification_command (EmberAfClusterCommand * cmd, sl_zcl_billing_cluster_bill_status_notification_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_billing_cluster_bill_status_notification_command, (uint8_t*)cmd_struct);
} 


// Signature for PaymentConfirm command
static const sl_signature_t sig_payment_cluster_payment_confirm_command = sl_zcl_payment_cluster_payment_confirm_command_signature;
// Command parser for PaymentConfirm command
EmberAfStatus zcl_decode_payment_cluster_payment_confirm_command (EmberAfClusterCommand * cmd, sl_zcl_payment_cluster_payment_confirm_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_payment_cluster_payment_confirm_command, (uint8_t*)cmd_struct);
} 


// Signature for Subscribe command
static const sl_signature_t sig_billing_cluster_subscribe_command = sl_zcl_billing_cluster_subscribe_command_signature;
// Command parser for Subscribe command
EmberAfStatus zcl_decode_billing_cluster_subscribe_command (EmberAfClusterCommand * cmd, sl_zcl_billing_cluster_subscribe_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_billing_cluster_subscribe_command, (uint8_t*)cmd_struct);
} 


// Signature for Unsubscribe command
static const sl_signature_t sig_billing_cluster_unsubscribe_command = sl_zcl_billing_cluster_unsubscribe_command_signature;
// Command parser for Unsubscribe command
EmberAfStatus zcl_decode_billing_cluster_unsubscribe_command (EmberAfClusterCommand * cmd, sl_zcl_billing_cluster_unsubscribe_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_billing_cluster_unsubscribe_command, (uint8_t*)cmd_struct);
} 


// Signature for StartBillingSession command
static const sl_signature_t sig_billing_cluster_start_billing_session_command = sl_zcl_billing_cluster_start_billing_session_command_signature;
// Command parser for StartBillingSession command
EmberAfStatus zcl_decode_billing_cluster_start_billing_session_command (EmberAfClusterCommand * cmd, sl_zcl_billing_cluster_start_billing_session_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_billing_cluster_start_billing_session_command, (uint8_t*)cmd_struct);
} 


// Signature for StopBillingSession command
static const sl_signature_t sig_billing_cluster_stop_billing_session_command = sl_zcl_billing_cluster_stop_billing_session_command_signature;
// Command parser for StopBillingSession command
EmberAfStatus zcl_decode_billing_cluster_stop_billing_session_command (EmberAfClusterCommand * cmd, sl_zcl_billing_cluster_stop_billing_session_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_billing_cluster_stop_billing_session_command, (uint8_t*)cmd_struct);
} 


// Signature for SessionKeepAlive command
static const sl_signature_t sig_billing_cluster_session_keep_alive_command = sl_zcl_billing_cluster_session_keep_alive_command_signature;
// Command parser for SessionKeepAlive command
EmberAfStatus zcl_decode_billing_cluster_session_keep_alive_command (EmberAfClusterCommand * cmd, sl_zcl_billing_cluster_session_keep_alive_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_billing_cluster_session_keep_alive_command, (uint8_t*)cmd_struct);
} 


// Signature for CheckBillStatus command
static const sl_signature_t sig_billing_cluster_check_bill_status_command = sl_zcl_billing_cluster_check_bill_status_command_signature;
// Command parser for CheckBillStatus command
EmberAfStatus zcl_decode_billing_cluster_check_bill_status_command (EmberAfClusterCommand * cmd, sl_zcl_billing_cluster_check_bill_status_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_billing_cluster_check_bill_status_command, (uint8_t*)cmd_struct);
} 


// Signature for BuyRequest command
static const sl_signature_t sig_payment_cluster_buy_request_command = sl_zcl_payment_cluster_buy_request_command_signature;
// Command parser for BuyRequest command
EmberAfStatus zcl_decode_payment_cluster_buy_request_command (EmberAfClusterCommand * cmd, sl_zcl_payment_cluster_buy_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_payment_cluster_buy_request_command, (uint8_t*)cmd_struct);
} 


// Signature for AcceptPayment command
static const sl_signature_t sig_payment_cluster_accept_payment_command = sl_zcl_payment_cluster_accept_payment_command_signature;
// Command parser for AcceptPayment command
EmberAfStatus zcl_decode_payment_cluster_accept_payment_command (EmberAfClusterCommand * cmd, sl_zcl_payment_cluster_accept_payment_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_payment_cluster_accept_payment_command, (uint8_t*)cmd_struct);
} 


// Signature for SendBillRecord command
static const sl_signature_t sig_billing_cluster_send_bill_record_command = sl_zcl_billing_cluster_send_bill_record_command_signature;
// Command parser for SendBillRecord command
EmberAfStatus zcl_decode_billing_cluster_send_bill_record_command (EmberAfClusterCommand * cmd, sl_zcl_billing_cluster_send_bill_record_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_billing_cluster_send_bill_record_command, (uint8_t*)cmd_struct);
} 


// Signature for ReceiptDelivery command
static const sl_signature_t sig_payment_cluster_receipt_delivery_command = sl_zcl_payment_cluster_receipt_delivery_command_signature;
// Command parser for ReceiptDelivery command
EmberAfStatus zcl_decode_payment_cluster_receipt_delivery_command (EmberAfClusterCommand * cmd, sl_zcl_payment_cluster_receipt_delivery_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_payment_cluster_receipt_delivery_command, (uint8_t*)cmd_struct);
} 


// Signature for BuyConfirm command
static const sl_signature_t sig_payment_cluster_buy_confirm_command = sl_zcl_payment_cluster_buy_confirm_command_signature;
// Command parser for BuyConfirm command
EmberAfStatus zcl_decode_payment_cluster_buy_confirm_command (EmberAfClusterCommand * cmd, sl_zcl_payment_cluster_buy_confirm_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_payment_cluster_buy_confirm_command, (uint8_t*)cmd_struct);
} 


// Signature for EphemeralDataRequest command
static const sl_signature_t sig_key_establishment_cluster_ephemeral_data_request_command = sl_zcl_key_establishment_cluster_ephemeral_data_request_command_signature;
// Command parser for EphemeralDataRequest command
EmberAfStatus zcl_decode_key_establishment_cluster_ephemeral_data_request_command (EmberAfClusterCommand * cmd, sl_zcl_key_establishment_cluster_ephemeral_data_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_key_establishment_cluster_ephemeral_data_request_command, (uint8_t*)cmd_struct);
} 


// Signature for ConfirmKeyDataRequest command
static const sl_signature_t sig_key_establishment_cluster_confirm_key_data_request_command = sl_zcl_key_establishment_cluster_confirm_key_data_request_command_signature;
// Command parser for ConfirmKeyDataRequest command
EmberAfStatus zcl_decode_key_establishment_cluster_confirm_key_data_request_command (EmberAfClusterCommand * cmd, sl_zcl_key_establishment_cluster_confirm_key_data_request_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_key_establishment_cluster_confirm_key_data_request_command, (uint8_t*)cmd_struct);
} 


// Signature for EphemeralDataResponse command
static const sl_signature_t sig_key_establishment_cluster_ephemeral_data_response_command = sl_zcl_key_establishment_cluster_ephemeral_data_response_command_signature;
// Command parser for EphemeralDataResponse command
EmberAfStatus zcl_decode_key_establishment_cluster_ephemeral_data_response_command (EmberAfClusterCommand * cmd, sl_zcl_key_establishment_cluster_ephemeral_data_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_key_establishment_cluster_ephemeral_data_response_command, (uint8_t*)cmd_struct);
} 


// Signature for ConfirmKeyDataResponse command
static const sl_signature_t sig_key_establishment_cluster_confirm_key_data_response_command = sl_zcl_key_establishment_cluster_confirm_key_data_response_command_signature;
// Command parser for ConfirmKeyDataResponse command
EmberAfStatus zcl_decode_key_establishment_cluster_confirm_key_data_response_command (EmberAfClusterCommand * cmd, sl_zcl_key_establishment_cluster_confirm_key_data_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_key_establishment_cluster_confirm_key_data_response_command, (uint8_t*)cmd_struct);
} 


// Signature for PushInformationResponse command
static const sl_signature_t sig_information_cluster_push_information_response_command = sl_zcl_information_cluster_push_information_response_command_signature;
// Command parser for PushInformationResponse command
EmberAfStatus zcl_decode_information_cluster_push_information_response_command (EmberAfClusterCommand * cmd, sl_zcl_information_cluster_push_information_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_information_cluster_push_information_response_command, (uint8_t*)cmd_struct);
} 


// Signature for UpdateResponse command
static const sl_signature_t sig_information_cluster_update_response_command = sl_zcl_information_cluster_update_response_command_signature;
// Command parser for UpdateResponse command
EmberAfStatus zcl_decode_information_cluster_update_response_command (EmberAfClusterCommand * cmd, sl_zcl_information_cluster_update_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_information_cluster_update_response_command, (uint8_t*)cmd_struct);
} 


// Signature for DeleteResponse command
static const sl_signature_t sig_information_cluster_delete_response_command = sl_zcl_information_cluster_delete_response_command_signature;
// Command parser for DeleteResponse command
EmberAfStatus zcl_decode_information_cluster_delete_response_command (EmberAfClusterCommand * cmd, sl_zcl_information_cluster_delete_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_information_cluster_delete_response_command, (uint8_t*)cmd_struct);
} 


// Signature for LogNotification command
static const sl_signature_t sig_appliance_statistics_cluster_log_notification_command = sl_zcl_appliance_statistics_cluster_log_notification_command_signature;
// Command parser for LogNotification command
EmberAfStatus zcl_decode_appliance_statistics_cluster_log_notification_command (EmberAfClusterCommand * cmd, sl_zcl_appliance_statistics_cluster_log_notification_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_appliance_statistics_cluster_log_notification_command, (uint8_t*)cmd_struct);
} 


// Signature for LogResponse command
static const sl_signature_t sig_appliance_statistics_cluster_log_response_command = sl_zcl_appliance_statistics_cluster_log_response_command_signature;
// Command parser for LogResponse command
EmberAfStatus zcl_decode_appliance_statistics_cluster_log_response_command (EmberAfClusterCommand * cmd, sl_zcl_appliance_statistics_cluster_log_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_appliance_statistics_cluster_log_response_command, (uint8_t*)cmd_struct);
} 


// Signature for GetMessageCancellation command
static const sl_signature_t sig_messaging_cluster_get_message_cancellation_command = sl_zcl_messaging_cluster_get_message_cancellation_command_signature;
// Command parser for GetMessageCancellation command
EmberAfStatus zcl_decode_messaging_cluster_get_message_cancellation_command (EmberAfClusterCommand * cmd, sl_zcl_messaging_cluster_get_message_cancellation_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_messaging_cluster_get_message_cancellation_command, (uint8_t*)cmd_struct);
} 


// Signature for CancelAllMessages command
static const sl_signature_t sig_messaging_cluster_cancel_all_messages_command = sl_zcl_messaging_cluster_cancel_all_messages_command_signature;
// Command parser for CancelAllMessages command
EmberAfStatus zcl_decode_messaging_cluster_cancel_all_messages_command (EmberAfClusterCommand * cmd, sl_zcl_messaging_cluster_cancel_all_messages_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_messaging_cluster_cancel_all_messages_command, (uint8_t*)cmd_struct);
} 


// Signature for SelectAvailableEmergencyCredit command
static const sl_signature_t sig_prepayment_cluster_select_available_emergency_credit_command = sl_zcl_prepayment_cluster_select_available_emergency_credit_command_signature;
// Command parser for SelectAvailableEmergencyCredit command
EmberAfStatus zcl_decode_prepayment_cluster_select_available_emergency_credit_command (EmberAfClusterCommand * cmd, sl_zcl_prepayment_cluster_select_available_emergency_credit_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_prepayment_cluster_select_available_emergency_credit_command, (uint8_t*)cmd_struct);
} 


// Signature for GetProfileResponse command
static const sl_signature_t sig_simple_metering_cluster_get_profile_response_command = sl_zcl_simple_metering_cluster_get_profile_response_command_signature;
// Command parser for GetProfileResponse command
EmberAfStatus zcl_decode_simple_metering_cluster_get_profile_response_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_get_profile_response_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_simple_metering_cluster_get_profile_response_command, (uint8_t*)cmd_struct);
} 


// Signature for GetConversionFactor command
static const sl_signature_t sig_price_cluster_get_conversion_factor_command = sl_zcl_price_cluster_get_conversion_factor_command_signature;
// Command parser for GetConversionFactor command
EmberAfStatus zcl_decode_price_cluster_get_conversion_factor_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_get_conversion_factor_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_price_cluster_get_conversion_factor_command, (uint8_t*)cmd_struct);
} 


// Signature for GetCalorificValue command
static const sl_signature_t sig_price_cluster_get_calorific_value_command = sl_zcl_price_cluster_get_calorific_value_command_signature;
// Command parser for GetCalorificValue command
EmberAfStatus zcl_decode_price_cluster_get_calorific_value_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_get_calorific_value_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_price_cluster_get_calorific_value_command, (uint8_t*)cmd_struct);
} 


// Signature for GetTariffInformation command
static const sl_signature_t sig_price_cluster_get_tariff_information_command = sl_zcl_price_cluster_get_tariff_information_command_signature;
// Command parser for GetTariffInformation command
EmberAfStatus zcl_decode_price_cluster_get_tariff_information_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_get_tariff_information_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_price_cluster_get_tariff_information_command, (uint8_t*)cmd_struct);
} 


// Signature for GetCO2Value command
static const sl_signature_t sig_price_cluster_get_co2_value_command = sl_zcl_price_cluster_get_co2_value_command_signature;
// Command parser for GetCO2Value command
EmberAfStatus zcl_decode_price_cluster_get_co2_value_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_get_co2_value_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_price_cluster_get_co2_value_command, (uint8_t*)cmd_struct);
} 


// Signature for GetBillingPeriod command
static const sl_signature_t sig_price_cluster_get_billing_period_command = sl_zcl_price_cluster_get_billing_period_command_signature;
// Command parser for GetBillingPeriod command
EmberAfStatus zcl_decode_price_cluster_get_billing_period_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_get_billing_period_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_price_cluster_get_billing_period_command, (uint8_t*)cmd_struct);
} 


// Signature for GetConsolidatedBill command
static const sl_signature_t sig_price_cluster_get_consolidated_bill_command = sl_zcl_price_cluster_get_consolidated_bill_command_signature;
// Command parser for GetConsolidatedBill command
EmberAfStatus zcl_decode_price_cluster_get_consolidated_bill_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_get_consolidated_bill_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_price_cluster_get_consolidated_bill_command, (uint8_t*)cmd_struct);
} 


// Signature for GetCalendar command
static const sl_signature_t sig_calendar_cluster_get_calendar_command = sl_zcl_calendar_cluster_get_calendar_command_signature;
// Command parser for GetCalendar command
EmberAfStatus zcl_decode_calendar_cluster_get_calendar_command (EmberAfClusterCommand * cmd, sl_zcl_calendar_cluster_get_calendar_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_calendar_cluster_get_calendar_command, (uint8_t*)cmd_struct);
} 


// Signature for GetScheduledPrices command
static const sl_signature_t sig_price_cluster_get_scheduled_prices_command = sl_zcl_price_cluster_get_scheduled_prices_command_signature;
// Command parser for GetScheduledPrices command
EmberAfStatus zcl_decode_price_cluster_get_scheduled_prices_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_get_scheduled_prices_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_price_cluster_get_scheduled_prices_command, (uint8_t*)cmd_struct);
} 


// Signature for GetCreditPayment command
static const sl_signature_t sig_price_cluster_get_credit_payment_command = sl_zcl_price_cluster_get_credit_payment_command_signature;
// Command parser for GetCreditPayment command
EmberAfStatus zcl_decode_price_cluster_get_credit_payment_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_get_credit_payment_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_price_cluster_get_credit_payment_command, (uint8_t*)cmd_struct);
} 


// Signature for GetTopUpLog command
static const sl_signature_t sig_prepayment_cluster_get_top_up_log_command = sl_zcl_prepayment_cluster_get_top_up_log_command_signature;
// Command parser for GetTopUpLog command
EmberAfStatus zcl_decode_prepayment_cluster_get_top_up_log_command (EmberAfClusterCommand * cmd, sl_zcl_prepayment_cluster_get_top_up_log_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_prepayment_cluster_get_top_up_log_command, (uint8_t*)cmd_struct);
} 


// Signature for GetBlockPeriods command
static const sl_signature_t sig_price_cluster_get_block_periods_command = sl_zcl_price_cluster_get_block_periods_command_signature;
// Command parser for GetBlockPeriods command
EmberAfStatus zcl_decode_price_cluster_get_block_periods_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_get_block_periods_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_price_cluster_get_block_periods_command, (uint8_t*)cmd_struct);
} 


// Signature for GetDebtRepaymentLog command
static const sl_signature_t sig_prepayment_cluster_get_debt_repayment_log_command = sl_zcl_prepayment_cluster_get_debt_repayment_log_command_signature;
// Command parser for GetDebtRepaymentLog command
EmberAfStatus zcl_decode_prepayment_cluster_get_debt_repayment_log_command (EmberAfClusterCommand * cmd, sl_zcl_prepayment_cluster_get_debt_repayment_log_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_prepayment_cluster_get_debt_repayment_log_command, (uint8_t*)cmd_struct);
} 


// Signature for GetSpecialDays command
static const sl_signature_t sig_calendar_cluster_get_special_days_command = sl_zcl_calendar_cluster_get_special_days_command_signature;
// Command parser for GetSpecialDays command
EmberAfStatus zcl_decode_calendar_cluster_get_special_days_command (EmberAfClusterCommand * cmd, sl_zcl_calendar_cluster_get_special_days_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_calendar_cluster_get_special_days_command, (uint8_t*)cmd_struct);
} 


// Signature for GetScheduledEvents command
static const sl_signature_t sig_demand_response_and_load_control_cluster_get_scheduled_events_command = sl_zcl_demand_response_and_load_control_cluster_get_scheduled_events_command_signature;
// Command parser for GetScheduledEvents command
EmberAfStatus zcl_decode_demand_response_and_load_control_cluster_get_scheduled_events_command (EmberAfClusterCommand * cmd, sl_zcl_demand_response_and_load_control_cluster_get_scheduled_events_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_demand_response_and_load_control_cluster_get_scheduled_events_command, (uint8_t*)cmd_struct);
} 


// Signature for GetSnapshot command
static const sl_signature_t sig_simple_metering_cluster_get_snapshot_command = sl_zcl_simple_metering_cluster_get_snapshot_command_signature;
// Command parser for GetSnapshot command
EmberAfStatus zcl_decode_simple_metering_cluster_get_snapshot_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_get_snapshot_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_simple_metering_cluster_get_snapshot_command, (uint8_t*)cmd_struct);
} 


// Signature for GetPrepaySnapshot command
static const sl_signature_t sig_prepayment_cluster_get_prepay_snapshot_command = sl_zcl_prepayment_cluster_get_prepay_snapshot_command_signature;
// Command parser for GetPrepaySnapshot command
EmberAfStatus zcl_decode_prepayment_cluster_get_prepay_snapshot_command (EmberAfClusterCommand * cmd, sl_zcl_prepayment_cluster_get_prepay_snapshot_command_t *cmd_struct) {
  // Use signature-decoding mechanism to parse this command.
  return sli_do_decode(cmd, sig_prepayment_cluster_get_prepay_snapshot_command, (uint8_t*)cmd_struct);
} 


// Command parser for GpPairingSearch command
EmberAfStatus zcl_decode_green_power_cluster_gp_pairing_search_command (EmberAfClusterCommand * cmd, sl_zcl_green_power_cluster_gp_pairing_search_command_t *cmd_struct) {
  // Use classic mechanism to parse this command.
  uint16_t payloadOffset = cmd->payloadStartIndex;
  // Not a fixed length command
            

  // - Argument is not present in all versions.
  if (cmd->bufLen < payloadOffset + 
2u)
  { 
    cmd_struct->options = 0xFFFF;
  } else {

        cmd_struct->options = sli_decode_parse_two_bytes(cmd, payloadOffset);


        payloadOffset+= 2u;
      }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0007) == 0x0000
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
4u ||
 !((cmd_struct->options & 0x0007) == 0x0000)) 
    { 
      cmd_struct->gpdSrcId = 0xFFFFFFFF;
    } 
    else 
    {
        cmd_struct->gpdSrcId = sli_decode_parse_four_bytes(cmd, payloadOffset);


        payloadOffset+= 4u;
    }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0007) == 0x0002
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
8u ||
 !((cmd_struct->options & 0x0007) == 0x0002)) 
    { 
      cmd_struct->gpdIeee = NULL;
    } 
    else 
    {
        cmd_struct->gpdIeee = sli_decode_bytes(cmd, payloadOffset);


        payloadOffset+= 8u;
    }
            


      // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0007) == 0x0002
      if (!((cmd_struct->options & 0x0007) == 0x0002)) { 
        cmd_struct->endpoint = 0xFF;
      } else {
        cmd_struct->endpoint = sli_decode_parse_one_byte(cmd, payloadOffset);


  }
  return EMBER_ZCL_STATUS_SUCCESS;
} 


// Command parser for GpNotification command
EmberAfStatus zcl_decode_green_power_cluster_gp_notification_command (EmberAfClusterCommand * cmd, sl_zcl_green_power_cluster_gp_notification_command_t *cmd_struct) {
  // Use classic mechanism to parse this command.
  uint16_t payloadOffset = cmd->payloadStartIndex;
  // Not a fixed length command
            

  // - Argument is not present in all versions.
  if (cmd->bufLen < payloadOffset + 
2u)
  { 
    cmd_struct->options = 0xFFFF;
  } else {

        cmd_struct->options = sli_decode_parse_two_bytes(cmd, payloadOffset);


        payloadOffset+= 2u;
      }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0007) == 0x0000
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
4u ||
 !((cmd_struct->options & 0x0007) == 0x0000)) 
    { 
      cmd_struct->gpdSrcId = 0xFFFFFFFF;
    } 
    else 
    {
        cmd_struct->gpdSrcId = sli_decode_parse_four_bytes(cmd, payloadOffset);


        payloadOffset+= 4u;
    }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0007) == 0x0002
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
8u ||
 !((cmd_struct->options & 0x0007) == 0x0002)) 
    { 
      cmd_struct->gpdIeee = NULL;
    } 
    else 
    {
        cmd_struct->gpdIeee = sli_decode_bytes(cmd, payloadOffset);


        payloadOffset+= 8u;
    }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0007) == 0x0002
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
1u ||
 !((cmd_struct->options & 0x0007) == 0x0002)) 
    { 
      cmd_struct->gpdEndpoint = 0xFF;
    } 
    else 
    {
        cmd_struct->gpdEndpoint = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
    }
            

  // - Argument is not present in all versions.
  if (cmd->bufLen < payloadOffset + 
4u)
  { 
    cmd_struct->gpdSecurityFrameCounter = 0xFFFFFFFF;
  } else {

        cmd_struct->gpdSecurityFrameCounter = sli_decode_parse_four_bytes(cmd, payloadOffset);


        payloadOffset+= 4u;
      }
            

  // - Argument is not present in all versions.
  if (cmd->bufLen < payloadOffset + 
1u)
  { 
    cmd_struct->gpdCommandId = 0xFF;
  } else {

        cmd_struct->gpdCommandId = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
      }
            

  // - Argument is not present in all versions.
  if (cmd->bufLen < payloadOffset + emberAfStringLength(cmd->buffer + payloadOffset) + 1u)
  { 
    cmd_struct->gpdCommandPayload = NULL;
  } else {

        cmd_struct->gpdCommandPayload = sli_decode_string(cmd, payloadOffset);


        payloadOffset+= emberAfStringLength(cmd->buffer + payloadOffset) + 1u;
      }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x4000) || (cmd_struct->options & 0x0800)
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
2u ||
 !((cmd_struct->options & 0x4000) || (cmd_struct->options & 0x0800))) 
    { 
      cmd_struct->gppShortAddress = 0xFFFF;
    } 
    else 
    {
        cmd_struct->gppShortAddress = sli_decode_parse_two_bytes(cmd, payloadOffset);


        payloadOffset+= 2u;
    }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x4000) || (cmd_struct->options & 0x0800)
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
1u ||
 !((cmd_struct->options & 0x4000) || (cmd_struct->options & 0x0800))) 
    { 
      cmd_struct->gppDistance = 0xFF;
    } 
    else 
    {
        cmd_struct->gppDistance = sli_decode_parse_one_byte(cmd, payloadOffset);


    }
  return EMBER_ZCL_STATUS_SUCCESS;
} 


// Command parser for GpCommissioningNotification command
EmberAfStatus zcl_decode_green_power_cluster_gp_commissioning_notification_command (EmberAfClusterCommand * cmd, sl_zcl_green_power_cluster_gp_commissioning_notification_command_t *cmd_struct) {
  // Use classic mechanism to parse this command.
  uint16_t payloadOffset = cmd->payloadStartIndex;
  // Not a fixed length command
            

  // - Argument is not present in all versions.
  if (cmd->bufLen < payloadOffset + 
2u)
  { 
    cmd_struct->options = 0xFFFF;
  } else {

        cmd_struct->options = sli_decode_parse_two_bytes(cmd, payloadOffset);


        payloadOffset+= 2u;
      }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0007) == 0x0000
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
4u ||
 !((cmd_struct->options & 0x0007) == 0x0000)) 
    { 
      cmd_struct->gpdSrcId = 0xFFFFFFFF;
    } 
    else 
    {
        cmd_struct->gpdSrcId = sli_decode_parse_four_bytes(cmd, payloadOffset);


        payloadOffset+= 4u;
    }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0007) == 0x0002
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
8u ||
 !((cmd_struct->options & 0x0007) == 0x0002)) 
    { 
      cmd_struct->gpdIeee = NULL;
    } 
    else 
    {
        cmd_struct->gpdIeee = sli_decode_bytes(cmd, payloadOffset);


        payloadOffset+= 8u;
    }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0007) == 0x0002
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
1u ||
 !((cmd_struct->options & 0x0007) == 0x0002)) 
    { 
      cmd_struct->endpoint = 0xFF;
    } 
    else 
    {
        cmd_struct->endpoint = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
    }
            

  // - Argument is not present in all versions.
  if (cmd->bufLen < payloadOffset + 
4u)
  { 
    cmd_struct->gpdSecurityFrameCounter = 0xFFFFFFFF;
  } else {

        cmd_struct->gpdSecurityFrameCounter = sli_decode_parse_four_bytes(cmd, payloadOffset);


        payloadOffset+= 4u;
      }
            

  // - Argument is not present in all versions.
  if (cmd->bufLen < payloadOffset + 
1u)
  { 
    cmd_struct->gpdCommandId = 0xFF;
  } else {

        cmd_struct->gpdCommandId = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
      }
            

  // - Argument is not present in all versions.
  if (cmd->bufLen < payloadOffset + emberAfStringLength(cmd->buffer + payloadOffset) + 1u)
  { 
    cmd_struct->gpdCommandPayload = NULL;
  } else {

        cmd_struct->gpdCommandPayload = sli_decode_string(cmd, payloadOffset);


        payloadOffset+= emberAfStringLength(cmd->buffer + payloadOffset) + 1u;
      }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0800) || (cmd_struct->options & 0x0008)
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
2u ||
 !((cmd_struct->options & 0x0800) || (cmd_struct->options & 0x0008))) 
    { 
      cmd_struct->gppShortAddress = 0xFFFF;
    } 
    else 
    {
        cmd_struct->gppShortAddress = sli_decode_parse_two_bytes(cmd, payloadOffset);


        payloadOffset+= 2u;
    }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0800) || (cmd_struct->options & 0x0008)
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
1u ||
 !((cmd_struct->options & 0x0800) || (cmd_struct->options & 0x0008))) 
    { 
      cmd_struct->gppLink = 0xFF;
    } 
    else 
    {
        cmd_struct->gppLink = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
    }
            


    // - Argument is conditionally present based on expression: cmd_struct->options & 0x0200
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
4u ||
 !(cmd_struct->options & 0x0200)) 
    { 
      cmd_struct->mic = 0xFFFFFFFF;
    } 
    else 
    {
        cmd_struct->mic = sli_decode_parse_four_bytes(cmd, payloadOffset);


    }
  return EMBER_ZCL_STATUS_SUCCESS;
} 


// Command parser for GpTranslationTableUpdate command
EmberAfStatus zcl_decode_green_power_cluster_gp_translation_table_update_command (EmberAfClusterCommand * cmd, sl_zcl_green_power_cluster_gp_translation_table_update_command_t *cmd_struct) {
  // Use classic mechanism to parse this command.
  uint16_t payloadOffset = cmd->payloadStartIndex;
  // Not a fixed length command
            

  // - Argument is not present in all versions.
  if (cmd->bufLen < payloadOffset + 
2u)
  { 
    cmd_struct->options = 0xFFFF;
  } else {

        cmd_struct->options = sli_decode_parse_two_bytes(cmd, payloadOffset);


        payloadOffset+= 2u;
      }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0007) == 0x0000
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
4u ||
 !((cmd_struct->options & 0x0007) == 0x0000)) 
    { 
      cmd_struct->gpdSrcId = 0xFFFFFFFF;
    } 
    else 
    {
        cmd_struct->gpdSrcId = sli_decode_parse_four_bytes(cmd, payloadOffset);


        payloadOffset+= 4u;
    }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0007) == 0x0002
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
8u ||
 !((cmd_struct->options & 0x0007) == 0x0002)) 
    { 
      cmd_struct->gpdIeee = NULL;
    } 
    else 
    {
        cmd_struct->gpdIeee = sli_decode_bytes(cmd, payloadOffset);


        payloadOffset+= 8u;
    }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0007) == 0x0002
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
1u ||
 !((cmd_struct->options & 0x0007) == 0x0002)) 
    { 
      cmd_struct->endpoint = 0xFF;
    } 
    else 
    {
        cmd_struct->endpoint = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
    }
    cmd_struct->translations = cmd->buffer + payloadOffset;
  return EMBER_ZCL_STATUS_SUCCESS;
} 


// Command parser for GpPairing command
EmberAfStatus zcl_decode_green_power_cluster_gp_pairing_command (EmberAfClusterCommand * cmd, sl_zcl_green_power_cluster_gp_pairing_command_t *cmd_struct) {
  // Use classic mechanism to parse this command.
  uint16_t payloadOffset = cmd->payloadStartIndex;
  // Not a fixed length command
            

  // - Argument is not present in all versions.
  if (cmd->bufLen < payloadOffset + 
3u)
  { 
    cmd_struct->options = 0xFFFFFFFF;
  } else {

        cmd_struct->options = sli_decode_parse_three_bytes(cmd, payloadOffset);


        payloadOffset+= 3u;
      }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x000007) == 0x000000
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
4u ||
 !((cmd_struct->options & 0x000007) == 0x000000)) 
    { 
      cmd_struct->gpdSrcId = 0xFFFFFFFF;
    } 
    else 
    {
        cmd_struct->gpdSrcId = sli_decode_parse_four_bytes(cmd, payloadOffset);


        payloadOffset+= 4u;
    }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x000007) == 0x000002
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
8u ||
 !((cmd_struct->options & 0x000007) == 0x000002)) 
    { 
      cmd_struct->gpdIeee = NULL;
    } 
    else 
    {
        cmd_struct->gpdIeee = sli_decode_bytes(cmd, payloadOffset);


        payloadOffset+= 8u;
    }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x000007) == 0x000002
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
1u ||
 !((cmd_struct->options & 0x000007) == 0x000002)) 
    { 
      cmd_struct->endpoint = 0xFF;
    } 
    else 
    {
        cmd_struct->endpoint = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
    }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x000010) == 0x000000 && ((cmd_struct->options & 0x000060) == 0x000000 || (cmd_struct->options & 0x000060) == 0x000060)
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
8u ||
 !((cmd_struct->options & 0x000010) == 0x000000 && ((cmd_struct->options & 0x000060) == 0x000000 || (cmd_struct->options & 0x000060) == 0x000060))) 
    { 
      cmd_struct->sinkIeeeAddress = NULL;
    } 
    else 
    {
        cmd_struct->sinkIeeeAddress = sli_decode_bytes(cmd, payloadOffset);


        payloadOffset+= 8u;
    }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x000010) == 0x000000 && ((cmd_struct->options & 0x000060) == 0x000000 || (cmd_struct->options & 0x000060) == 0x000060)
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
2u ||
 !((cmd_struct->options & 0x000010) == 0x000000 && ((cmd_struct->options & 0x000060) == 0x000000 || (cmd_struct->options & 0x000060) == 0x000060))) 
    { 
      cmd_struct->sinkNwkAddress = 0xFFFF;
    } 
    else 
    {
        cmd_struct->sinkNwkAddress = sli_decode_parse_two_bytes(cmd, payloadOffset);


        payloadOffset+= 2u;
    }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x000010) == 0x000000 && ((cmd_struct->options & 0x000060) == 0x000020 || (cmd_struct->options & 0x000060) == 0x000040)
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
2u ||
 !((cmd_struct->options & 0x000010) == 0x000000 && ((cmd_struct->options & 0x000060) == 0x000020 || (cmd_struct->options & 0x000060) == 0x000040))) 
    { 
      cmd_struct->sinkGroupId = 0xFFFF;
    } 
    else 
    {
        cmd_struct->sinkGroupId = sli_decode_parse_two_bytes(cmd, payloadOffset);


        payloadOffset+= 2u;
    }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x000008) == 0x000008 && (cmd_struct->options & 0x000010) != 0x000010
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
1u ||
 !((cmd_struct->options & 0x000008) == 0x000008 && (cmd_struct->options & 0x000010) != 0x000010)) 
    { 
      cmd_struct->deviceId = 0xFF;
    } 
    else 
    {
        cmd_struct->deviceId = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
    }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x004000) == 0x004000
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
4u ||
 !((cmd_struct->options & 0x004000) == 0x004000)) 
    { 
      cmd_struct->gpdSecurityFrameCounter = 0xFFFFFFFF;
    } 
    else 
    {
        cmd_struct->gpdSecurityFrameCounter = sli_decode_parse_four_bytes(cmd, payloadOffset);


        payloadOffset+= 4u;
    }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x008000) == 0x008000
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
16u ||
 !((cmd_struct->options & 0x008000) == 0x008000)) 
    { 
      cmd_struct->gpdKey = NULL;
    } 
    else 
    {
        cmd_struct->gpdKey = sli_decode_bytes(cmd, payloadOffset);


        payloadOffset+= 16u;
    }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x010000) == 0x010000
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
2u ||
 !((cmd_struct->options & 0x010000) == 0x010000)) 
    { 
      cmd_struct->assignedAlias = 0xFFFF;
    } 
    else 
    {
        cmd_struct->assignedAlias = sli_decode_parse_two_bytes(cmd, payloadOffset);


        payloadOffset+= 2u;
    }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x020000) == 0x020000
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
1u ||
 !((cmd_struct->options & 0x020000) == 0x020000)) 
    { 
      cmd_struct->groupcastRadius = 0xFF;
    } 
    else 
    {
        cmd_struct->groupcastRadius = sli_decode_parse_one_byte(cmd, payloadOffset);


    }
  return EMBER_ZCL_STATUS_SUCCESS;
} 


// Command parser for GpProxyTableRequest command
EmberAfStatus zcl_decode_green_power_cluster_gp_proxy_table_request_command (EmberAfClusterCommand * cmd, sl_zcl_green_power_cluster_gp_proxy_table_request_command_t *cmd_struct) {
  // Use classic mechanism to parse this command.
  uint16_t payloadOffset = cmd->payloadStartIndex;
  // Not a fixed length command
            
  if (cmd->bufLen < payloadOffset + 
1u)
  {
    return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
  }


        cmd_struct->options = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
            


      // - Argument is conditionally present based on expression: ((cmd_struct->options & 0x18) == 0x00) && ((cmd_struct->options & 0x07) == 0x00)
      if (!(((cmd_struct->options & 0x18) == 0x00) && ((cmd_struct->options & 0x07) == 0x00))) { 
        cmd_struct->gpdSrcId = 0xFFFFFFFF;
      } else {
        cmd_struct->gpdSrcId = sli_decode_parse_four_bytes(cmd, payloadOffset);


        payloadOffset+= 4u;
  }
            


      // - Argument is conditionally present based on expression: ((cmd_struct->options & 0x18) == 0x00) && ((cmd_struct->options & 0x07) == 0x02)
      if (!(((cmd_struct->options & 0x18) == 0x00) && ((cmd_struct->options & 0x07) == 0x02))) { 
        cmd_struct->gpdIeee = NULL;
      } else {
        cmd_struct->gpdIeee = sli_decode_bytes(cmd, payloadOffset);


        payloadOffset+= 8u;
  }
            


      // - Argument is conditionally present based on expression: (cmd_struct->options & 0x07) == 0x02
      if (!((cmd_struct->options & 0x07) == 0x02)) { 
        cmd_struct->endpoint = 0xFF;
      } else {
        cmd_struct->endpoint = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
  }
            


      // - Argument is conditionally present based on expression: (cmd_struct->options & 0x18) == 0x08
      if (!((cmd_struct->options & 0x18) == 0x08)) { 
        cmd_struct->index = 0xFF;
      } else {
        cmd_struct->index = sli_decode_parse_one_byte(cmd, payloadOffset);


  }
  return EMBER_ZCL_STATUS_SUCCESS;
} 


// Command parser for GpPairingConfiguration command
EmberAfStatus zcl_decode_green_power_cluster_gp_pairing_configuration_command (EmberAfClusterCommand * cmd, sl_zcl_green_power_cluster_gp_pairing_configuration_command_t *cmd_struct) {
  // Use classic mechanism to parse this command.
  uint16_t payloadOffset = cmd->payloadStartIndex;
  // Not a fixed length command
            

  // - Argument is not present in all versions.
  if (cmd->bufLen < payloadOffset + 
1u)
  { 
    cmd_struct->actions = 0xFF;
  } else {

        cmd_struct->actions = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
      }
            

  // - Argument is not present in all versions.
  if (cmd->bufLen < payloadOffset + 
2u)
  { 
    cmd_struct->options = 0xFFFF;
  } else {

        cmd_struct->options = sli_decode_parse_two_bytes(cmd, payloadOffset);


        payloadOffset+= 2u;
      }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0007) == 0x0000
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
4u ||
 !((cmd_struct->options & 0x0007) == 0x0000)) 
    { 
      cmd_struct->gpdSrcId = 0xFFFFFFFF;
    } 
    else 
    {
        cmd_struct->gpdSrcId = sli_decode_parse_four_bytes(cmd, payloadOffset);


        payloadOffset+= 4u;
    }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0007) == 0x0002
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
8u ||
 !((cmd_struct->options & 0x0007) == 0x0002)) 
    { 
      cmd_struct->gpdIeee = NULL;
    } 
    else 
    {
        cmd_struct->gpdIeee = sli_decode_bytes(cmd, payloadOffset);


        payloadOffset+= 8u;
    }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0007) == 0x0002
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
1u ||
 !((cmd_struct->options & 0x0007) == 0x0002)) 
    { 
      cmd_struct->endpoint = 0xFF;
    } 
    else 
    {
        cmd_struct->endpoint = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
    }
            

  // - Argument is not present in all versions.
  if (cmd->bufLen < payloadOffset + 
1u)
  { 
    cmd_struct->deviceId = 0xFF;
  } else {

        cmd_struct->deviceId = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
      }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0018) == 0x0010
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
1u ||
 !((cmd_struct->options & 0x0018) == 0x0010)) 
    { 
      cmd_struct->groupListCount = 0xFF;
    } 
    else 
    {
        cmd_struct->groupListCount = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
    }
  if ( (cmd_struct->options & 0x0018) == 0x0010 ) {
    // Array is conditionally present based on expression: (cmd_struct->options & 0x0018) == 0x0010
    cmd_struct->groupList = cmd->buffer + payloadOffset;
    payloadOffset += cmd_struct->groupListCount * sizeof(GpPairingConfigurationGroupList);
  } else {
    cmd_struct->groupList = NULL;
  } 
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0100)
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
2u ||
 !((cmd_struct->options & 0x0100))) 
    { 
      cmd_struct->gpdAssignedAlias = 0xFFFF;
    } 
    else 
    {
        cmd_struct->gpdAssignedAlias = sli_decode_parse_two_bytes(cmd, payloadOffset);


        payloadOffset+= 2u;
    }
            
  if (cmd->bufLen < payloadOffset + 
1u)
  {
    return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
  }


        cmd_struct->groupcastRadius = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0200)
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
1u ||
 !((cmd_struct->options & 0x0200))) 
    { 
      cmd_struct->securityOptions = 0xFF;
    } 
    else 
    {
        cmd_struct->securityOptions = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
    }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0200) || (cmd_struct->options & 0x0020)
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
4u ||
 !((cmd_struct->options & 0x0200) || (cmd_struct->options & 0x0020))) 
    { 
      cmd_struct->gpdSecurityFrameCounter = 0xFFFFFFFF;
    } 
    else 
    {
        cmd_struct->gpdSecurityFrameCounter = sli_decode_parse_four_bytes(cmd, payloadOffset);


        payloadOffset+= 4u;
    }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0200)
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
16u ||
 !((cmd_struct->options & 0x0200))) 
    { 
      cmd_struct->gpdSecurityKey = NULL;
    } 
    else 
    {
        cmd_struct->gpdSecurityKey = sli_decode_bytes(cmd, payloadOffset);


        payloadOffset+= 16u;
    }
            

  // - Argument is not present in all versions.
  if (cmd->bufLen < payloadOffset + 
1u)
  { 
    cmd_struct->numberOfPairedEndpoints = 0xFF;
  } else {

        cmd_struct->numberOfPairedEndpoints = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
      }
  if ( 0 < cmd_struct->numberOfPairedEndpoints && cmd_struct->numberOfPairedEndpoints < 0xFD ) {
    // Array is conditionally present based on expression: 0 < cmd_struct->numberOfPairedEndpoints && cmd_struct->numberOfPairedEndpoints < 0xFD
    cmd_struct->pairedEndpoints = cmd->buffer + payloadOffset;
    payloadOffset += cmd_struct->numberOfPairedEndpoints * sizeof(uint8_t);
  } else {
    cmd_struct->pairedEndpoints = NULL;
  } 
            


      // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0400)
      if (!((cmd_struct->options & 0x0400))) { 
        cmd_struct->applicationInformation = 0xFF;
      } else {
        cmd_struct->applicationInformation = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
  }
            


      // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0400) && (cmd_struct->applicationInformation & 0x01)
      if (!((cmd_struct->options & 0x0400) && (cmd_struct->applicationInformation & 0x01))) { 
        cmd_struct->manufacturerId = 0xFFFF;
      } else {
        cmd_struct->manufacturerId = sli_decode_parse_two_bytes(cmd, payloadOffset);


        payloadOffset+= 2u;
  }
            


      // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0400) && (cmd_struct->applicationInformation & 0x02)
      if (!((cmd_struct->options & 0x0400) && (cmd_struct->applicationInformation & 0x02))) { 
        cmd_struct->modeId = 0xFFFF;
      } else {
        cmd_struct->modeId = sli_decode_parse_two_bytes(cmd, payloadOffset);


        payloadOffset+= 2u;
  }
            


      // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0400) && (cmd_struct->applicationInformation & 0x04)
      if (!((cmd_struct->options & 0x0400) && (cmd_struct->applicationInformation & 0x04))) { 
        cmd_struct->numberOfGpdCommands = 0xFF;
      } else {
        cmd_struct->numberOfGpdCommands = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
  }
  if ( (cmd_struct->options & 0x0400) && (cmd_struct->applicationInformation & 0x04) ) {
    // Array is conditionally present based on expression: (cmd_struct->options & 0x0400) && (cmd_struct->applicationInformation & 0x04)
    cmd_struct->gpdCommandIdList = cmd->buffer + payloadOffset;
    payloadOffset += cmd_struct->numberOfGpdCommands * sizeof(uint8_t);
  } else {
    cmd_struct->gpdCommandIdList = NULL;
  } 
            


      // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0400) && (cmd_struct->applicationInformation & 0x08)
      if (!((cmd_struct->options & 0x0400) && (cmd_struct->applicationInformation & 0x08))) { 
        cmd_struct->clusterIdListCount = 0xFF;
      } else {
        cmd_struct->clusterIdListCount = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
  }
  if ( (cmd_struct->options & 0x0400) && (cmd_struct->applicationInformation & 0x08) && ((cmd_struct->clusterIdListCount & 0x0F) > 0) ) {
    // Array is conditionally present based on expression: (cmd_struct->options & 0x0400) && (cmd_struct->applicationInformation & 0x08) && ((cmd_struct->clusterIdListCount & 0x0F) > 0)
    cmd_struct->clusterListServer = cmd->buffer + payloadOffset;
    payloadOffset += (cmd_struct->clusterIdListCount & 0x0F) * sizeof(uint16_t);
  } else {
    cmd_struct->clusterListServer = NULL;
  } 
  if ( (cmd_struct->options & 0x0400) && (cmd_struct->applicationInformation & 0x08) && ((cmd_struct->clusterIdListCount >> 4) > 0) ) {
    // Array is conditionally present based on expression: (cmd_struct->options & 0x0400) && (cmd_struct->applicationInformation & 0x08) && ((cmd_struct->clusterIdListCount >> 4) > 0)
    cmd_struct->clusterListClient = cmd->buffer + payloadOffset;
    payloadOffset += (cmd_struct->clusterIdListCount >> 4) * sizeof(uint16_t);
  } else {
    cmd_struct->clusterListClient = NULL;
  } 
            


      // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0400) && (cmd_struct->applicationInformation & 0x10)
      if (!((cmd_struct->options & 0x0400) && (cmd_struct->applicationInformation & 0x10))) { 
        cmd_struct->switchInformationLength = 0xFF;
      } else {
        cmd_struct->switchInformationLength = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
  }
            


      // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0400) && (cmd_struct->applicationInformation & 0x10)
      if (!((cmd_struct->options & 0x0400) && (cmd_struct->applicationInformation & 0x10))) { 
        cmd_struct->switchConfiguration = 0xFF;
      } else {
        cmd_struct->switchConfiguration = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
  }
            


      // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0400) && (cmd_struct->applicationInformation & 0x10)
      if (!((cmd_struct->options & 0x0400) && (cmd_struct->applicationInformation & 0x10))) { 
        cmd_struct->currentContactStatus = 0xFF;
      } else {
        cmd_struct->currentContactStatus = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
  }
            


      // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0400) && (cmd_struct->applicationInformation & 0x20)
      if (!((cmd_struct->options & 0x0400) && (cmd_struct->applicationInformation & 0x20))) { 
        cmd_struct->totalNumberOfReports = 0xFF;
      } else {
        cmd_struct->totalNumberOfReports = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
  }
            


      // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0400) && (cmd_struct->applicationInformation & 0x20)
      if (!((cmd_struct->options & 0x0400) && (cmd_struct->applicationInformation & 0x20))) { 
        cmd_struct->numberOfReports = 0xFF;
      } else {
        cmd_struct->numberOfReports = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
  }
  if ( (cmd_struct->options & 0x0400) && (cmd_struct->applicationInformation & 0x20) ) {
    // Array is conditionally present based on expression: (cmd_struct->options & 0x0400) && (cmd_struct->applicationInformation & 0x20)
    cmd_struct->reportDescriptor = cmd->buffer + payloadOffset;
  } else {
    cmd_struct->reportDescriptor = NULL;
  } 
  return EMBER_ZCL_STATUS_SUCCESS;
} 


// Command parser for GpResponse command
EmberAfStatus zcl_decode_green_power_cluster_gp_response_command (EmberAfClusterCommand * cmd, sl_zcl_green_power_cluster_gp_response_command_t *cmd_struct) {
  // Use classic mechanism to parse this command.
  uint16_t payloadOffset = cmd->payloadStartIndex;
  // Not a fixed length command
            

  // - Argument is not present in all versions.
  if (cmd->bufLen < payloadOffset + 
1u)
  { 
    cmd_struct->options = 0xFF;
  } else {

        cmd_struct->options = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
      }
            

  // - Argument is not present in all versions.
  if (cmd->bufLen < payloadOffset + 
2u)
  { 
    cmd_struct->tempMasterShortAddress = 0xFFFF;
  } else {

        cmd_struct->tempMasterShortAddress = sli_decode_parse_two_bytes(cmd, payloadOffset);


        payloadOffset+= 2u;
      }
            

  // - Argument is not present in all versions.
  if (cmd->bufLen < payloadOffset + 
1u)
  { 
    cmd_struct->tempMasterTxChannel = 0xFF;
  } else {

        cmd_struct->tempMasterTxChannel = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
      }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x07) == 0x00
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
4u ||
 !((cmd_struct->options & 0x07) == 0x00)) 
    { 
      cmd_struct->gpdSrcId = 0xFFFFFFFF;
    } 
    else 
    {
        cmd_struct->gpdSrcId = sli_decode_parse_four_bytes(cmd, payloadOffset);


        payloadOffset+= 4u;
    }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x07) == 0x02
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
8u ||
 !((cmd_struct->options & 0x07) == 0x02)) 
    { 
      cmd_struct->gpdIeee = NULL;
    } 
    else 
    {
        cmd_struct->gpdIeee = sli_decode_bytes(cmd, payloadOffset);


        payloadOffset+= 8u;
    }
            


      // - Argument is conditionally present based on expression: (cmd_struct->options & 0x07) == 0x02
      if (!((cmd_struct->options & 0x07) == 0x02)) { 
        cmd_struct->endpoint = 0xFF;
      } else {
        cmd_struct->endpoint = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
  }
            

  // - Argument is not present in all versions.
  if (cmd->bufLen < payloadOffset + 
1u)
  { 
    cmd_struct->gpdCommandId = 0xFF;
  } else {

        cmd_struct->gpdCommandId = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
      }
            
  if (cmd->bufLen < payloadOffset + emberAfStringLength(cmd->buffer + payloadOffset) + 1u)
  {
    return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
  }


        cmd_struct->gpdCommandPayload = sli_decode_string(cmd, payloadOffset);


  return EMBER_ZCL_STATUS_SUCCESS;
} 


// Command parser for GpProxyCommissioningMode command
EmberAfStatus zcl_decode_green_power_cluster_gp_proxy_commissioning_mode_command (EmberAfClusterCommand * cmd, sl_zcl_green_power_cluster_gp_proxy_commissioning_mode_command_t *cmd_struct) {
  // Use classic mechanism to parse this command.
  uint16_t payloadOffset = cmd->payloadStartIndex;
  // Not a fixed length command
            

  // - Argument is not present in all versions.
  if (cmd->bufLen < payloadOffset + 
1u)
  { 
    cmd_struct->options = 0xFF;
  } else {

        cmd_struct->options = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
      }
            


      // - Argument is conditionally present based on expression: (cmd_struct->options & 0x02) == 0x02
      if (!((cmd_struct->options & 0x02) == 0x02)) { 
        cmd_struct->commissioningWindow = 0xFFFF;
      } else {
        cmd_struct->commissioningWindow = sli_decode_parse_two_bytes(cmd, payloadOffset);


        payloadOffset+= 2u;
  }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x10) == 0x10
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
1u ||
 !((cmd_struct->options & 0x10) == 0x10)) 
    { 
      cmd_struct->channel = 0xFF;
    } 
    else 
    {
        cmd_struct->channel = sli_decode_parse_one_byte(cmd, payloadOffset);


    }
  return EMBER_ZCL_STATUS_SUCCESS;
} 


// Command parser for GpNotificationResponse command
EmberAfStatus zcl_decode_green_power_cluster_gp_notification_response_command (EmberAfClusterCommand * cmd, sl_zcl_green_power_cluster_gp_notification_response_command_t *cmd_struct) {
  // Use classic mechanism to parse this command.
  uint16_t payloadOffset = cmd->payloadStartIndex;
  // Not a fixed length command
            

  // - Argument is not present in all versions.
  if (cmd->bufLen < payloadOffset + 
1u)
  { 
    cmd_struct->options = 0xFF;
  } else {

        cmd_struct->options = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
      }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0007) == 0x0000
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
4u ||
 !((cmd_struct->options & 0x0007) == 0x0000)) 
    { 
      cmd_struct->gpdSrcId = 0xFFFFFFFF;
    } 
    else 
    {
        cmd_struct->gpdSrcId = sli_decode_parse_four_bytes(cmd, payloadOffset);


        payloadOffset+= 4u;
    }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0007) == 0x0002
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
8u ||
 !((cmd_struct->options & 0x0007) == 0x0002)) 
    { 
      cmd_struct->gpdIeee = NULL;
    } 
    else 
    {
        cmd_struct->gpdIeee = sli_decode_bytes(cmd, payloadOffset);


        payloadOffset+= 8u;
    }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0007) == 0x0002
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
1u ||
 !((cmd_struct->options & 0x0007) == 0x0002)) 
    { 
      cmd_struct->endpoint = 0xFF;
    } 
    else 
    {
        cmd_struct->endpoint = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
    }
            

  // - Argument is not present in all versions.
  if (cmd->bufLen < payloadOffset + 
4u)
  { 
    cmd_struct->gpdSecurityFrameCounter = 0xFFFFFFFF;
  } else {

        cmd_struct->gpdSecurityFrameCounter = sli_decode_parse_four_bytes(cmd, payloadOffset);


      }
  return EMBER_ZCL_STATUS_SUCCESS;
} 


// Command parser for GpTunnelingStop command
EmberAfStatus zcl_decode_green_power_cluster_gp_tunneling_stop_command (EmberAfClusterCommand * cmd, sl_zcl_green_power_cluster_gp_tunneling_stop_command_t *cmd_struct) {
  // Use classic mechanism to parse this command.
  uint16_t payloadOffset = cmd->payloadStartIndex;
  // Not a fixed length command
            

  // - Argument is not present in all versions.
  if (cmd->bufLen < payloadOffset + 
1u)
  { 
    cmd_struct->options = 0xFF;
  } else {

        cmd_struct->options = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
      }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0007) == 0x0000
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
4u ||
 !((cmd_struct->options & 0x0007) == 0x0000)) 
    { 
      cmd_struct->gpdSrcId = 0xFFFFFFFF;
    } 
    else 
    {
        cmd_struct->gpdSrcId = sli_decode_parse_four_bytes(cmd, payloadOffset);


        payloadOffset+= 4u;
    }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0007) == 0x0002
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
8u ||
 !((cmd_struct->options & 0x0007) == 0x0002)) 
    { 
      cmd_struct->gpdIeee = NULL;
    } 
    else 
    {
        cmd_struct->gpdIeee = sli_decode_bytes(cmd, payloadOffset);


        payloadOffset+= 8u;
    }
            


    // - Argument is conditionally present based on expression: (cmd_struct->options & 0x0007) == 0x0002
    // - Argument is not present in all versions
    if (cmd->bufLen < payloadOffset + 
1u ||
 !((cmd_struct->options & 0x0007) == 0x0002)) 
    { 
      cmd_struct->endpoint = 0xFF;
    } 
    else 
    {
        cmd_struct->endpoint = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
    }
            

  // - Argument is not present in all versions.
  if (cmd->bufLen < payloadOffset + 
4u)
  { 
    cmd_struct->gpdSecurityFrameCounter = 0xFFFFFFFF;
  } else {

        cmd_struct->gpdSecurityFrameCounter = sli_decode_parse_four_bytes(cmd, payloadOffset);


        payloadOffset+= 4u;
      }
            

  // - Argument is not present in all versions.
  if (cmd->bufLen < payloadOffset + 
2u)
  { 
    cmd_struct->gppShortAddress = 0xFFFF;
  } else {

        cmd_struct->gppShortAddress = sli_decode_parse_two_bytes(cmd, payloadOffset);


        payloadOffset+= 2u;
      }
            

  // - Argument is not present in all versions.
  if (cmd->bufLen < payloadOffset + 
1u)
  { 
    cmd_struct->gppDistance = 0xFF;
  } else {

        cmd_struct->gppDistance = sli_decode_parse_one_byte(cmd, payloadOffset);


      }
  return EMBER_ZCL_STATUS_SUCCESS;
} 


// Command parser for PoweringOffNotification command
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_powering_off_notification_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_powering_off_notification_command_t *cmd_struct) {
  // Use classic mechanism to parse this command.
  uint16_t payloadOffset = cmd->payloadStartIndex;
  // Not a fixed length command
            
  if (cmd->bufLen < payloadOffset + 
1u)
  {
    return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
  }


        cmd_struct->powerNotificationReason = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
            
  if (cmd->bufLen < payloadOffset + 
2u)
  {
    return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
  }


        cmd_struct->manufacturerId = sli_decode_parse_two_bytes(cmd, payloadOffset);


        payloadOffset+= 2u;
            
  if (cmd->bufLen < payloadOffset + 
1u)
  {
    return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
  }


        cmd_struct->manufacturerReasonLength = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
  if ( cmd_struct->manufacturerReasonLength!=0 ) {
    // Array is conditionally present based on expression: cmd_struct->manufacturerReasonLength!=0
    cmd_struct->manufacturerReason = cmd->buffer + payloadOffset;
  } else {
    cmd_struct->manufacturerReason = NULL;
  } 
  return EMBER_ZCL_STATUS_SUCCESS;
} 


// Command parser for PoweringOnNotification command
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_powering_on_notification_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_powering_on_notification_command_t *cmd_struct) {
  // Use classic mechanism to parse this command.
  uint16_t payloadOffset = cmd->payloadStartIndex;
  // Not a fixed length command
            
  if (cmd->bufLen < payloadOffset + 
1u)
  {
    return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
  }


        cmd_struct->powerNotificationReason = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
            
  if (cmd->bufLen < payloadOffset + 
2u)
  {
    return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
  }


        cmd_struct->manufacturerId = sli_decode_parse_two_bytes(cmd, payloadOffset);


        payloadOffset+= 2u;
            
  if (cmd->bufLen < payloadOffset + 
1u)
  {
    return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
  }


        cmd_struct->manufacturerReasonLength = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
  if ( cmd_struct->manufacturerReasonLength!=0 ) {
    // Array is conditionally present based on expression: cmd_struct->manufacturerReasonLength!=0
    cmd_struct->manufacturerReason = cmd->buffer + payloadOffset;
  } else {
    cmd_struct->manufacturerReason = NULL;
  } 
  return EMBER_ZCL_STATUS_SUCCESS;
} 


// Command parser for GpSinkTableRequest command
EmberAfStatus zcl_decode_green_power_cluster_gp_sink_table_request_command (EmberAfClusterCommand * cmd, sl_zcl_green_power_cluster_gp_sink_table_request_command_t *cmd_struct) {
  // Use classic mechanism to parse this command.
  uint16_t payloadOffset = cmd->payloadStartIndex;
  // Not a fixed length command
            
  if (cmd->bufLen < payloadOffset + 
1u)
  {
    return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
  }


        cmd_struct->options = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
            


      // - Argument is conditionally present based on expression: ((cmd_struct->options & 0x07) == 0x00) && ((cmd_struct->options & 0x18) == 0x00)
      if (!(((cmd_struct->options & 0x07) == 0x00) && ((cmd_struct->options & 0x18) == 0x00))) { 
        cmd_struct->gpdSrcId = 0xFFFFFFFF;
      } else {
        cmd_struct->gpdSrcId = sli_decode_parse_four_bytes(cmd, payloadOffset);


        payloadOffset+= 4u;
  }
            


      // - Argument is conditionally present based on expression: ((cmd_struct->options & 0x07) == 0x02) && ((cmd_struct->options & 0x18) == 0x00)
      if (!(((cmd_struct->options & 0x07) == 0x02) && ((cmd_struct->options & 0x18) == 0x00))) { 
        cmd_struct->gpdIeee = NULL;
      } else {
        cmd_struct->gpdIeee = sli_decode_bytes(cmd, payloadOffset);


        payloadOffset+= 8u;
  }
            


      // - Argument is conditionally present based on expression: ((cmd_struct->options & 0x07) == 0x02) && ((cmd_struct->options & 0x18) == 0x00)
      if (!(((cmd_struct->options & 0x07) == 0x02) && ((cmd_struct->options & 0x18) == 0x00))) { 
        cmd_struct->endpoint = 0xFF;
      } else {
        cmd_struct->endpoint = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
  }
            


      // - Argument is conditionally present based on expression: (cmd_struct->options & 0x18) == 0x08
      if (!((cmd_struct->options & 0x18) == 0x08)) { 
        cmd_struct->index = 0xFF;
      } else {
        cmd_struct->index = sli_decode_parse_one_byte(cmd, payloadOffset);


  }
  return EMBER_ZCL_STATUS_SUCCESS;
} 


// Command parser for EnableApsLinkKeyAuthorization command
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_enable_aps_link_key_authorization_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_enable_aps_link_key_authorization_command_t *cmd_struct) {
  // Use classic mechanism to parse this command.
  uint16_t payloadOffset = cmd->payloadStartIndex;
  // Not a fixed length command
            
  if (cmd->bufLen < payloadOffset + 
1u)
  {
    return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
  }


        cmd_struct->numberExemptClusters = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
  if ( cmd_struct->numberExemptClusters!=0 ) {
    // Array is conditionally present based on expression: cmd_struct->numberExemptClusters!=0
    cmd_struct->clusterId = cmd->buffer + payloadOffset;
  } else {
    cmd_struct->clusterId = NULL;
  } 
  return EMBER_ZCL_STATUS_SUCCESS;
} 


// Command parser for DisableApsLinkKeyAuthorization command
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_disable_aps_link_key_authorization_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_disable_aps_link_key_authorization_command_t *cmd_struct) {
  // Use classic mechanism to parse this command.
  uint16_t payloadOffset = cmd->payloadStartIndex;
  // Not a fixed length command
            
  if (cmd->bufLen < payloadOffset + 
1u)
  {
    return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
  }


        cmd_struct->numberExemptClusters = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
  if ( cmd_struct->numberExemptClusters!=0 ) {
    // Array is conditionally present based on expression: cmd_struct->numberExemptClusters!=0
    cmd_struct->clusterId = cmd->buffer + payloadOffset;
  } else {
    cmd_struct->clusterId = NULL;
  } 
  return EMBER_ZCL_STATUS_SUCCESS;
} 


// Command parser for RequireApsAcksOnUnicasts command
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_require_aps_acks_on_unicasts_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_require_aps_acks_on_unicasts_command_t *cmd_struct) {
  // Use classic mechanism to parse this command.
  uint16_t payloadOffset = cmd->payloadStartIndex;
  // Not a fixed length command
            
  if (cmd->bufLen < payloadOffset + 
1u)
  {
    return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
  }


        cmd_struct->numberExemptClusters = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
  if ( cmd_struct->numberExemptClusters!=0 ) {
    // Array is conditionally present based on expression: cmd_struct->numberExemptClusters!=0
    cmd_struct->clusterId = cmd->buffer + payloadOffset;
  } else {
    cmd_struct->clusterId = NULL;
  } 
  return EMBER_ZCL_STATUS_SUCCESS;
} 


// Command parser for UseTrustCenterForClusterServer command
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_use_trust_center_for_cluster_server_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_use_trust_center_for_cluster_server_command_t *cmd_struct) {
  // Use classic mechanism to parse this command.
  uint16_t payloadOffset = cmd->payloadStartIndex;
  // Not a fixed length command
            
  if (cmd->bufLen < payloadOffset + 
1u)
  {
    return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
  }


        cmd_struct->numberOfClusters = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
  if ( cmd_struct->numberOfClusters!=0 ) {
    // Array is conditionally present based on expression: cmd_struct->numberOfClusters!=0
    cmd_struct->clusterId = cmd->buffer + payloadOffset;
  } else {
    cmd_struct->clusterId = NULL;
  } 
  return EMBER_ZCL_STATUS_SUCCESS;
} 


// Command parser for ApsAckEnablementQueryResponse command
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_aps_ack_enablement_query_response_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_aps_ack_enablement_query_response_command_t *cmd_struct) {
  // Use classic mechanism to parse this command.
  uint16_t payloadOffset = cmd->payloadStartIndex;
  // Not a fixed length command
            
  if (cmd->bufLen < payloadOffset + 
1u)
  {
    return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
  }


        cmd_struct->numberExemptClusters = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
  if ( cmd_struct->numberExemptClusters!=0 ) {
    // Array is conditionally present based on expression: cmd_struct->numberExemptClusters!=0
    cmd_struct->clusterId = cmd->buffer + payloadOffset;
  } else {
    cmd_struct->clusterId = NULL;
  } 
  return EMBER_ZCL_STATUS_SUCCESS;
} 


// Command parser for TrustCenterForClusterServerQueryResponse command
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_trust_center_for_cluster_server_query_response_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_trust_center_for_cluster_server_query_response_command_t *cmd_struct) {
  // Use classic mechanism to parse this command.
  uint16_t payloadOffset = cmd->payloadStartIndex;
  // Not a fixed length command
            
  if (cmd->bufLen < payloadOffset + 
1u)
  {
    return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
  }


        cmd_struct->numberOfClusters = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
  if ( cmd_struct->numberOfClusters!=0 ) {
    // Array is conditionally present based on expression: cmd_struct->numberOfClusters!=0
    cmd_struct->clusterId = cmd->buffer + payloadOffset;
  } else {
    cmd_struct->clusterId = NULL;
  } 
  return EMBER_ZCL_STATUS_SUCCESS;
} 


// Command parser for ViewSceneResponse command
EmberAfStatus zcl_decode_scenes_cluster_view_scene_response_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_view_scene_response_command_t *cmd_struct) {
  // Use classic mechanism to parse this command.
  uint16_t payloadOffset = cmd->payloadStartIndex;
  // Not a fixed length command
            
  if (cmd->bufLen < payloadOffset + 
1u)
  {
    return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
  }


        cmd_struct->status = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
            
  if (cmd->bufLen < payloadOffset + 
2u)
  {
    return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
  }


        cmd_struct->groupId = sli_decode_parse_two_bytes(cmd, payloadOffset);


        payloadOffset+= 2u;
            
  if (cmd->bufLen < payloadOffset + 
1u)
  {
    return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
  }


        cmd_struct->sceneId = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
            


      // - Argument is conditionally present based on expression: cmd_struct->status==0
      if (!(cmd_struct->status==0)) { 
        cmd_struct->transitionTime = 0xFFFF;
      } else {
        cmd_struct->transitionTime = sli_decode_parse_two_bytes(cmd, payloadOffset);


        payloadOffset+= 2u;
  }
            


      // - Argument is conditionally present based on expression: cmd_struct->status==0
      if (!(cmd_struct->status==0)) { 
        cmd_struct->sceneName = NULL;
      } else {
        cmd_struct->sceneName = sli_decode_string(cmd, payloadOffset);


        payloadOffset+= emberAfStringLength(cmd->buffer + payloadOffset) + 1u;
  }
  if ( cmd_struct->status==0 ) {
    // Array is conditionally present based on expression: cmd_struct->status==0
    cmd_struct->extensionFieldSets = cmd->buffer + payloadOffset;
  } else {
    cmd_struct->extensionFieldSets = NULL;
  } 
  return EMBER_ZCL_STATUS_SUCCESS;
} 


// Command parser for GetSceneMembershipResponse command
EmberAfStatus zcl_decode_scenes_cluster_get_scene_membership_response_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_get_scene_membership_response_command_t *cmd_struct) {
  // Use classic mechanism to parse this command.
  uint16_t payloadOffset = cmd->payloadStartIndex;
  // Not a fixed length command
            
  if (cmd->bufLen < payloadOffset + 
1u)
  {
    return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
  }


        cmd_struct->status = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
            
  if (cmd->bufLen < payloadOffset + 
1u)
  {
    return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
  }


        cmd_struct->capacity = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
            
  if (cmd->bufLen < payloadOffset + 
2u)
  {
    return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
  }


        cmd_struct->groupId = sli_decode_parse_two_bytes(cmd, payloadOffset);


        payloadOffset+= 2u;
            


      // - Argument is conditionally present based on expression: cmd_struct->status==0
      if (!(cmd_struct->status==0)) { 
        cmd_struct->sceneCount = 0xFF;
      } else {
        cmd_struct->sceneCount = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
  }
  if ( cmd_struct->status==0 ) {
    // Array is conditionally present based on expression: cmd_struct->status==0
    cmd_struct->sceneList = cmd->buffer + payloadOffset;
  } else {
    cmd_struct->sceneList = NULL;
  } 
  return EMBER_ZCL_STATUS_SUCCESS;
} 


// Command parser for UseTrustCenterForClusterServerResponse command
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_use_trust_center_for_cluster_server_response_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_use_trust_center_for_cluster_server_response_command_t *cmd_struct) {
  // Use classic mechanism to parse this command.
  uint16_t payloadOffset = cmd->payloadStartIndex;
  // Not a fixed length command
            
  if (cmd->bufLen < payloadOffset + 
1u)
  {
    return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
  }


        cmd_struct->status = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
            
  if (cmd->bufLen < payloadOffset + 
1u)
  {
    return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
  }


        cmd_struct->clusterStatusLength = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
  if ( cmd_struct->clusterStatusLength!=0 ) {
    // Array is conditionally present based on expression: cmd_struct->clusterStatusLength!=0
    cmd_struct->clusterStatus = cmd->buffer + payloadOffset;
  } else {
    cmd_struct->clusterStatus = NULL;
  } 
  return EMBER_ZCL_STATUS_SUCCESS;
} 


// Command parser for Bypass command
EmberAfStatus zcl_decode_ias_ace_cluster_bypass_command (EmberAfClusterCommand * cmd, sl_zcl_ias_ace_cluster_bypass_command_t *cmd_struct) {
  // Use classic mechanism to parse this command.
  uint16_t payloadOffset = cmd->payloadStartIndex;
  // Not a fixed length command
            
  if (cmd->bufLen < payloadOffset + 
1u)
  {
    return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
  }


        cmd_struct->numberOfZones = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
    cmd_struct->zoneIds = cmd->buffer + payloadOffset;
    payloadOffset += cmd_struct->numberOfZones * sizeof(uint8_t);
            

  // - Argument is not present in all versions.
  if (cmd->bufLen < payloadOffset + emberAfStringLength(cmd->buffer + payloadOffset) + 1u)
  { 
    cmd_struct->armDisarmCode = NULL;
  } else {

        cmd_struct->armDisarmCode = sli_decode_string(cmd, payloadOffset);


      }
  return EMBER_ZCL_STATUS_SUCCESS;
} 


// Command parser for SurveyBeaconsResponse command
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_survey_beacons_response_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_survey_beacons_response_command_t *cmd_struct) {
  // Use classic mechanism to parse this command.
  uint16_t payloadOffset = cmd->payloadStartIndex;
  // Not a fixed length command
            
  if (cmd->bufLen < payloadOffset + 
1u)
  {
    return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
  }


        cmd_struct->numberOfBeacons = sli_decode_parse_one_byte(cmd, payloadOffset);


        payloadOffset+= 1u;
  if ( cmd_struct->numberOfBeacons!=0 ) {
    // Array is conditionally present based on expression: cmd_struct->numberOfBeacons!=0
    cmd_struct->beacon = cmd->buffer + payloadOffset;
  } else {
    cmd_struct->beacon = NULL;
  } 
  return EMBER_ZCL_STATUS_SUCCESS;
} 



#endif //End of SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT