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

#ifndef __ZAP_CLUSTER_COMMAND_PARSER_H__
#define __ZAP_CLUSTER_COMMAND_PARSER_H__ 1

/**
 * @defgroup zcl_command_parser ZCL Command Parsers
 * @ingroup command
 * @brief Application Framework command parsing functions reference
 * 
 * This document describes the generated parsing functions that 
 * are used to parse over-the-air payload into the corresponding structs.
 */

/**
 *
 * @addtogroup zcl_command_parser
 * @{
 */

#include "sl_component_catalog.h"
#ifdef SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT

#include "af.h"
#include "zap-command-structs.h"

/** @brief Parser function for "GetLocalesSupported" ZCL command from "Basic" cluster
 */
EmberAfStatus zcl_decode_basic_cluster_get_locales_supported_command (EmberAfClusterCommand * cmd, sl_zcl_basic_cluster_get_locales_supported_command_t *cmd_struct);
/** @brief Parser function for "GetLocalesSupportedResponse" ZCL command from "Basic" cluster
 */
EmberAfStatus zcl_decode_basic_cluster_get_locales_supported_response_command (EmberAfClusterCommand * cmd, sl_zcl_basic_cluster_get_locales_supported_response_command_t *cmd_struct);
/** @brief Parser function for "Identify" ZCL command from "Identify" cluster
 */
EmberAfStatus zcl_decode_identify_cluster_identify_command (EmberAfClusterCommand * cmd, sl_zcl_identify_cluster_identify_command_t *cmd_struct);
/** @brief Parser function for "EZModeInvoke" ZCL command from "Identify" cluster
 */
EmberAfStatus zcl_decode_identify_cluster_ez_mode_invoke_command (EmberAfClusterCommand * cmd, sl_zcl_identify_cluster_ez_mode_invoke_command_t *cmd_struct);
/** @brief Parser function for "UpdateCommissionState" ZCL command from "Identify" cluster
 */
EmberAfStatus zcl_decode_identify_cluster_update_commission_state_command (EmberAfClusterCommand * cmd, sl_zcl_identify_cluster_update_commission_state_command_t *cmd_struct);
/** @brief Parser function for "TriggerEffect" ZCL command from "Identify" cluster
 */
EmberAfStatus zcl_decode_identify_cluster_trigger_effect_command (EmberAfClusterCommand * cmd, sl_zcl_identify_cluster_trigger_effect_command_t *cmd_struct);
/** @brief Parser function for "IdentifyQueryResponse" ZCL command from "Identify" cluster
 */
EmberAfStatus zcl_decode_identify_cluster_identify_query_response_command (EmberAfClusterCommand * cmd, sl_zcl_identify_cluster_identify_query_response_command_t *cmd_struct);
/** @brief Parser function for "AddGroup" ZCL command from "Groups" cluster
 */
EmberAfStatus zcl_decode_groups_cluster_add_group_command (EmberAfClusterCommand * cmd, sl_zcl_groups_cluster_add_group_command_t *cmd_struct);
/** @brief Parser function for "ViewGroup" ZCL command from "Groups" cluster
 */
EmberAfStatus zcl_decode_groups_cluster_view_group_command (EmberAfClusterCommand * cmd, sl_zcl_groups_cluster_view_group_command_t *cmd_struct);
/** @brief Parser function for "GetGroupMembership" ZCL command from "Groups" cluster
 */
EmberAfStatus zcl_decode_groups_cluster_get_group_membership_command (EmberAfClusterCommand * cmd, sl_zcl_groups_cluster_get_group_membership_command_t *cmd_struct);
/** @brief Parser function for "RemoveGroup" ZCL command from "Groups" cluster
 */
EmberAfStatus zcl_decode_groups_cluster_remove_group_command (EmberAfClusterCommand * cmd, sl_zcl_groups_cluster_remove_group_command_t *cmd_struct);
/** @brief Parser function for "AddGroupIfIdentifying" ZCL command from "Groups" cluster
 */
EmberAfStatus zcl_decode_groups_cluster_add_group_if_identifying_command (EmberAfClusterCommand * cmd, sl_zcl_groups_cluster_add_group_if_identifying_command_t *cmd_struct);
/** @brief Parser function for "AddGroupResponse" ZCL command from "Groups" cluster
 */
EmberAfStatus zcl_decode_groups_cluster_add_group_response_command (EmberAfClusterCommand * cmd, sl_zcl_groups_cluster_add_group_response_command_t *cmd_struct);
/** @brief Parser function for "ViewGroupResponse" ZCL command from "Groups" cluster
 */
EmberAfStatus zcl_decode_groups_cluster_view_group_response_command (EmberAfClusterCommand * cmd, sl_zcl_groups_cluster_view_group_response_command_t *cmd_struct);
/** @brief Parser function for "GetGroupMembershipResponse" ZCL command from "Groups" cluster
 */
EmberAfStatus zcl_decode_groups_cluster_get_group_membership_response_command (EmberAfClusterCommand * cmd, sl_zcl_groups_cluster_get_group_membership_response_command_t *cmd_struct);
/** @brief Parser function for "RemoveGroupResponse" ZCL command from "Groups" cluster
 */
EmberAfStatus zcl_decode_groups_cluster_remove_group_response_command (EmberAfClusterCommand * cmd, sl_zcl_groups_cluster_remove_group_response_command_t *cmd_struct);
/** @brief Parser function for "AddScene" ZCL command from "Scenes" cluster
 */
EmberAfStatus zcl_decode_scenes_cluster_add_scene_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_add_scene_command_t *cmd_struct);
/** @brief Parser function for "ViewScene" ZCL command from "Scenes" cluster
 */
EmberAfStatus zcl_decode_scenes_cluster_view_scene_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_view_scene_command_t *cmd_struct);
/** @brief Parser function for "RemoveScene" ZCL command from "Scenes" cluster
 */
EmberAfStatus zcl_decode_scenes_cluster_remove_scene_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_remove_scene_command_t *cmd_struct);
/** @brief Parser function for "RemoveAllScenes" ZCL command from "Scenes" cluster
 */
EmberAfStatus zcl_decode_scenes_cluster_remove_all_scenes_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_remove_all_scenes_command_t *cmd_struct);
/** @brief Parser function for "StoreScene" ZCL command from "Scenes" cluster
 */
EmberAfStatus zcl_decode_scenes_cluster_store_scene_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_store_scene_command_t *cmd_struct);
/** @brief Parser function for "RecallScene" ZCL command from "Scenes" cluster
 */
EmberAfStatus zcl_decode_scenes_cluster_recall_scene_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_recall_scene_command_t *cmd_struct);
/** @brief Parser function for "GetSceneMembership" ZCL command from "Scenes" cluster
 */
EmberAfStatus zcl_decode_scenes_cluster_get_scene_membership_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_get_scene_membership_command_t *cmd_struct);
/** @brief Parser function for "EnhancedAddScene" ZCL command from "Scenes" cluster
 */
EmberAfStatus zcl_decode_scenes_cluster_enhanced_add_scene_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_enhanced_add_scene_command_t *cmd_struct);
/** @brief Parser function for "EnhancedViewScene" ZCL command from "Scenes" cluster
 */
EmberAfStatus zcl_decode_scenes_cluster_enhanced_view_scene_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_enhanced_view_scene_command_t *cmd_struct);
/** @brief Parser function for "CopyScene" ZCL command from "Scenes" cluster
 */
EmberAfStatus zcl_decode_scenes_cluster_copy_scene_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_copy_scene_command_t *cmd_struct);
/** @brief Parser function for "AddSceneResponse" ZCL command from "Scenes" cluster
 */
EmberAfStatus zcl_decode_scenes_cluster_add_scene_response_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_add_scene_response_command_t *cmd_struct);
/** @brief Parser function for "ViewSceneResponse" ZCL command from "Scenes" cluster
 */
EmberAfStatus zcl_decode_scenes_cluster_view_scene_response_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_view_scene_response_command_t *cmd_struct);
/** @brief Parser function for "RemoveSceneResponse" ZCL command from "Scenes" cluster
 */
EmberAfStatus zcl_decode_scenes_cluster_remove_scene_response_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_remove_scene_response_command_t *cmd_struct);
/** @brief Parser function for "RemoveAllScenesResponse" ZCL command from "Scenes" cluster
 */
EmberAfStatus zcl_decode_scenes_cluster_remove_all_scenes_response_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_remove_all_scenes_response_command_t *cmd_struct);
/** @brief Parser function for "StoreSceneResponse" ZCL command from "Scenes" cluster
 */
EmberAfStatus zcl_decode_scenes_cluster_store_scene_response_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_store_scene_response_command_t *cmd_struct);
/** @brief Parser function for "GetSceneMembershipResponse" ZCL command from "Scenes" cluster
 */
EmberAfStatus zcl_decode_scenes_cluster_get_scene_membership_response_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_get_scene_membership_response_command_t *cmd_struct);
/** @brief Parser function for "EnhancedAddSceneResponse" ZCL command from "Scenes" cluster
 */
EmberAfStatus zcl_decode_scenes_cluster_enhanced_add_scene_response_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_enhanced_add_scene_response_command_t *cmd_struct);
/** @brief Parser function for "EnhancedViewSceneResponse" ZCL command from "Scenes" cluster
 */
EmberAfStatus zcl_decode_scenes_cluster_enhanced_view_scene_response_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_enhanced_view_scene_response_command_t *cmd_struct);
/** @brief Parser function for "CopySceneResponse" ZCL command from "Scenes" cluster
 */
EmberAfStatus zcl_decode_scenes_cluster_copy_scene_response_command (EmberAfClusterCommand * cmd, sl_zcl_scenes_cluster_copy_scene_response_command_t *cmd_struct);
/** @brief Parser function for "OffWithEffect" ZCL command from "On/off" cluster
 */
EmberAfStatus zcl_decode_on_off_cluster_off_with_effect_command (EmberAfClusterCommand * cmd, sl_zcl_on_off_cluster_off_with_effect_command_t *cmd_struct);
/** @brief Parser function for "OnWithTimedOff" ZCL command from "On/off" cluster
 */
EmberAfStatus zcl_decode_on_off_cluster_on_with_timed_off_command (EmberAfClusterCommand * cmd, sl_zcl_on_off_cluster_on_with_timed_off_command_t *cmd_struct);
/** @brief Parser function for "MoveToLevel" ZCL command from "Level Control" cluster
 */
EmberAfStatus zcl_decode_level_control_cluster_move_to_level_command (EmberAfClusterCommand * cmd, sl_zcl_level_control_cluster_move_to_level_command_t *cmd_struct);
/** @brief Parser function for "Move" ZCL command from "Level Control" cluster
 */
EmberAfStatus zcl_decode_level_control_cluster_move_command (EmberAfClusterCommand * cmd, sl_zcl_level_control_cluster_move_command_t *cmd_struct);
/** @brief Parser function for "Step" ZCL command from "Level Control" cluster
 */
EmberAfStatus zcl_decode_level_control_cluster_step_command (EmberAfClusterCommand * cmd, sl_zcl_level_control_cluster_step_command_t *cmd_struct);
/** @brief Parser function for "Stop" ZCL command from "Level Control" cluster
 */
EmberAfStatus zcl_decode_level_control_cluster_stop_command (EmberAfClusterCommand * cmd, sl_zcl_level_control_cluster_stop_command_t *cmd_struct);
/** @brief Parser function for "MoveToLevelWithOnOff" ZCL command from "Level Control" cluster
 */
EmberAfStatus zcl_decode_level_control_cluster_move_to_level_with_on_off_command (EmberAfClusterCommand * cmd, sl_zcl_level_control_cluster_move_to_level_with_on_off_command_t *cmd_struct);
/** @brief Parser function for "MoveWithOnOff" ZCL command from "Level Control" cluster
 */
EmberAfStatus zcl_decode_level_control_cluster_move_with_on_off_command (EmberAfClusterCommand * cmd, sl_zcl_level_control_cluster_move_with_on_off_command_t *cmd_struct);
/** @brief Parser function for "StepWithOnOff" ZCL command from "Level Control" cluster
 */
EmberAfStatus zcl_decode_level_control_cluster_step_with_on_off_command (EmberAfClusterCommand * cmd, sl_zcl_level_control_cluster_step_with_on_off_command_t *cmd_struct);
/** @brief Parser function for "MoveToClosestFrequency" ZCL command from "Level Control" cluster
 */
EmberAfStatus zcl_decode_level_control_cluster_move_to_closest_frequency_command (EmberAfClusterCommand * cmd, sl_zcl_level_control_cluster_move_to_closest_frequency_command_t *cmd_struct);
/** @brief Parser function for "ResetAlarm" ZCL command from "Alarms" cluster
 */
EmberAfStatus zcl_decode_alarms_cluster_reset_alarm_command (EmberAfClusterCommand * cmd, sl_zcl_alarms_cluster_reset_alarm_command_t *cmd_struct);
/** @brief Parser function for "Alarm" ZCL command from "Alarms" cluster
 */
EmberAfStatus zcl_decode_alarms_cluster_alarm_command (EmberAfClusterCommand * cmd, sl_zcl_alarms_cluster_alarm_command_t *cmd_struct);
/** @brief Parser function for "GetAlarmResponse" ZCL command from "Alarms" cluster
 */
EmberAfStatus zcl_decode_alarms_cluster_get_alarm_response_command (EmberAfClusterCommand * cmd, sl_zcl_alarms_cluster_get_alarm_response_command_t *cmd_struct);
/** @brief Parser function for "SetAbsoluteLocation" ZCL command from "RSSI Location" cluster
 */
EmberAfStatus zcl_decode_rssi_location_cluster_set_absolute_location_command (EmberAfClusterCommand * cmd, sl_zcl_rssi_location_cluster_set_absolute_location_command_t *cmd_struct);
/** @brief Parser function for "SetDeviceConfiguration" ZCL command from "RSSI Location" cluster
 */
EmberAfStatus zcl_decode_rssi_location_cluster_set_device_configuration_command (EmberAfClusterCommand * cmd, sl_zcl_rssi_location_cluster_set_device_configuration_command_t *cmd_struct);
/** @brief Parser function for "GetDeviceConfiguration" ZCL command from "RSSI Location" cluster
 */
EmberAfStatus zcl_decode_rssi_location_cluster_get_device_configuration_command (EmberAfClusterCommand * cmd, sl_zcl_rssi_location_cluster_get_device_configuration_command_t *cmd_struct);
/** @brief Parser function for "GetLocationData" ZCL command from "RSSI Location" cluster
 */
EmberAfStatus zcl_decode_rssi_location_cluster_get_location_data_command (EmberAfClusterCommand * cmd, sl_zcl_rssi_location_cluster_get_location_data_command_t *cmd_struct);
/** @brief Parser function for "RssiResponse" ZCL command from "RSSI Location" cluster
 */
EmberAfStatus zcl_decode_rssi_location_cluster_rssi_response_command (EmberAfClusterCommand * cmd, sl_zcl_rssi_location_cluster_rssi_response_command_t *cmd_struct);
/** @brief Parser function for "SendPings" ZCL command from "RSSI Location" cluster
 */
EmberAfStatus zcl_decode_rssi_location_cluster_send_pings_command (EmberAfClusterCommand * cmd, sl_zcl_rssi_location_cluster_send_pings_command_t *cmd_struct);
/** @brief Parser function for "AnchorNodeAnnounce" ZCL command from "RSSI Location" cluster
 */
EmberAfStatus zcl_decode_rssi_location_cluster_anchor_node_announce_command (EmberAfClusterCommand * cmd, sl_zcl_rssi_location_cluster_anchor_node_announce_command_t *cmd_struct);
/** @brief Parser function for "DeviceConfigurationResponse" ZCL command from "RSSI Location" cluster
 */
EmberAfStatus zcl_decode_rssi_location_cluster_device_configuration_response_command (EmberAfClusterCommand * cmd, sl_zcl_rssi_location_cluster_device_configuration_response_command_t *cmd_struct);
/** @brief Parser function for "LocationDataResponse" ZCL command from "RSSI Location" cluster
 */
EmberAfStatus zcl_decode_rssi_location_cluster_location_data_response_command (EmberAfClusterCommand * cmd, sl_zcl_rssi_location_cluster_location_data_response_command_t *cmd_struct);
/** @brief Parser function for "LocationDataNotification" ZCL command from "RSSI Location" cluster
 */
EmberAfStatus zcl_decode_rssi_location_cluster_location_data_notification_command (EmberAfClusterCommand * cmd, sl_zcl_rssi_location_cluster_location_data_notification_command_t *cmd_struct);
/** @brief Parser function for "CompactLocationDataNotification" ZCL command from "RSSI Location" cluster
 */
EmberAfStatus zcl_decode_rssi_location_cluster_compact_location_data_notification_command (EmberAfClusterCommand * cmd, sl_zcl_rssi_location_cluster_compact_location_data_notification_command_t *cmd_struct);
/** @brief Parser function for "RssiPing" ZCL command from "RSSI Location" cluster
 */
EmberAfStatus zcl_decode_rssi_location_cluster_rssi_ping_command (EmberAfClusterCommand * cmd, sl_zcl_rssi_location_cluster_rssi_ping_command_t *cmd_struct);
/** @brief Parser function for "ReportRssiMeasurements" ZCL command from "RSSI Location" cluster
 */
EmberAfStatus zcl_decode_rssi_location_cluster_report_rssi_measurements_command (EmberAfClusterCommand * cmd, sl_zcl_rssi_location_cluster_report_rssi_measurements_command_t *cmd_struct);
/** @brief Parser function for "RequestOwnLocation" ZCL command from "RSSI Location" cluster
 */
EmberAfStatus zcl_decode_rssi_location_cluster_request_own_location_command (EmberAfClusterCommand * cmd, sl_zcl_rssi_location_cluster_request_own_location_command_t *cmd_struct);
/** @brief Parser function for "RestartDevice" ZCL command from "Commissioning" cluster
 */
EmberAfStatus zcl_decode_commissioning_cluster_restart_device_command (EmberAfClusterCommand * cmd, sl_zcl_commissioning_cluster_restart_device_command_t *cmd_struct);
/** @brief Parser function for "SaveStartupParameters" ZCL command from "Commissioning" cluster
 */
EmberAfStatus zcl_decode_commissioning_cluster_save_startup_parameters_command (EmberAfClusterCommand * cmd, sl_zcl_commissioning_cluster_save_startup_parameters_command_t *cmd_struct);
/** @brief Parser function for "RestoreStartupParameters" ZCL command from "Commissioning" cluster
 */
EmberAfStatus zcl_decode_commissioning_cluster_restore_startup_parameters_command (EmberAfClusterCommand * cmd, sl_zcl_commissioning_cluster_restore_startup_parameters_command_t *cmd_struct);
/** @brief Parser function for "ResetStartupParameters" ZCL command from "Commissioning" cluster
 */
EmberAfStatus zcl_decode_commissioning_cluster_reset_startup_parameters_command (EmberAfClusterCommand * cmd, sl_zcl_commissioning_cluster_reset_startup_parameters_command_t *cmd_struct);
/** @brief Parser function for "RestartDeviceResponse" ZCL command from "Commissioning" cluster
 */
EmberAfStatus zcl_decode_commissioning_cluster_restart_device_response_command (EmberAfClusterCommand * cmd, sl_zcl_commissioning_cluster_restart_device_response_command_t *cmd_struct);
/** @brief Parser function for "SaveStartupParametersResponse" ZCL command from "Commissioning" cluster
 */
EmberAfStatus zcl_decode_commissioning_cluster_save_startup_parameters_response_command (EmberAfClusterCommand * cmd, sl_zcl_commissioning_cluster_save_startup_parameters_response_command_t *cmd_struct);
/** @brief Parser function for "RestoreStartupParametersResponse" ZCL command from "Commissioning" cluster
 */
EmberAfStatus zcl_decode_commissioning_cluster_restore_startup_parameters_response_command (EmberAfClusterCommand * cmd, sl_zcl_commissioning_cluster_restore_startup_parameters_response_command_t *cmd_struct);
/** @brief Parser function for "ResetStartupParametersResponse" ZCL command from "Commissioning" cluster
 */
EmberAfStatus zcl_decode_commissioning_cluster_reset_startup_parameters_response_command (EmberAfClusterCommand * cmd, sl_zcl_commissioning_cluster_reset_startup_parameters_response_command_t *cmd_struct);
/** @brief Parser function for "TransferPartitionedFrame" ZCL command from "Partition" cluster
 */
EmberAfStatus zcl_decode_partition_cluster_transfer_partitioned_frame_command (EmberAfClusterCommand * cmd, sl_zcl_partition_cluster_transfer_partitioned_frame_command_t *cmd_struct);
/** @brief Parser function for "ReadHandshakeParam" ZCL command from "Partition" cluster
 */
EmberAfStatus zcl_decode_partition_cluster_read_handshake_param_command (EmberAfClusterCommand * cmd, sl_zcl_partition_cluster_read_handshake_param_command_t *cmd_struct);
/** @brief Parser function for "WriteHandshakeParam" ZCL command from "Partition" cluster
 */
EmberAfStatus zcl_decode_partition_cluster_write_handshake_param_command (EmberAfClusterCommand * cmd, sl_zcl_partition_cluster_write_handshake_param_command_t *cmd_struct);
/** @brief Parser function for "MultipleAck" ZCL command from "Partition" cluster
 */
EmberAfStatus zcl_decode_partition_cluster_multiple_ack_command (EmberAfClusterCommand * cmd, sl_zcl_partition_cluster_multiple_ack_command_t *cmd_struct);
/** @brief Parser function for "ReadHandshakeParamResponse" ZCL command from "Partition" cluster
 */
EmberAfStatus zcl_decode_partition_cluster_read_handshake_param_response_command (EmberAfClusterCommand * cmd, sl_zcl_partition_cluster_read_handshake_param_response_command_t *cmd_struct);
/** @brief Parser function for "QueryNextImageRequest" ZCL command from "Over the Air Bootloading" cluster
 */
EmberAfStatus zcl_decode_over_the_air_bootloading_cluster_query_next_image_request_command (EmberAfClusterCommand * cmd, sl_zcl_over_the_air_bootloading_cluster_query_next_image_request_command_t *cmd_struct);
/** @brief Parser function for "ImageBlockRequest" ZCL command from "Over the Air Bootloading" cluster
 */
EmberAfStatus zcl_decode_over_the_air_bootloading_cluster_image_block_request_command (EmberAfClusterCommand * cmd, sl_zcl_over_the_air_bootloading_cluster_image_block_request_command_t *cmd_struct);
/** @brief Parser function for "ImagePageRequest" ZCL command from "Over the Air Bootloading" cluster
 */
EmberAfStatus zcl_decode_over_the_air_bootloading_cluster_image_page_request_command (EmberAfClusterCommand * cmd, sl_zcl_over_the_air_bootloading_cluster_image_page_request_command_t *cmd_struct);
/** @brief Parser function for "UpgradeEndRequest" ZCL command from "Over the Air Bootloading" cluster
 */
EmberAfStatus zcl_decode_over_the_air_bootloading_cluster_upgrade_end_request_command (EmberAfClusterCommand * cmd, sl_zcl_over_the_air_bootloading_cluster_upgrade_end_request_command_t *cmd_struct);
/** @brief Parser function for "QuerySpecificFileRequest" ZCL command from "Over the Air Bootloading" cluster
 */
EmberAfStatus zcl_decode_over_the_air_bootloading_cluster_query_specific_file_request_command (EmberAfClusterCommand * cmd, sl_zcl_over_the_air_bootloading_cluster_query_specific_file_request_command_t *cmd_struct);
/** @brief Parser function for "ImageNotify" ZCL command from "Over the Air Bootloading" cluster
 */
EmberAfStatus zcl_decode_over_the_air_bootloading_cluster_image_notify_command (EmberAfClusterCommand * cmd, sl_zcl_over_the_air_bootloading_cluster_image_notify_command_t *cmd_struct);
/** @brief Parser function for "QueryNextImageResponse" ZCL command from "Over the Air Bootloading" cluster
 */
EmberAfStatus zcl_decode_over_the_air_bootloading_cluster_query_next_image_response_command (EmberAfClusterCommand * cmd, sl_zcl_over_the_air_bootloading_cluster_query_next_image_response_command_t *cmd_struct);
/** @brief Parser function for "ImageBlockResponse" ZCL command from "Over the Air Bootloading" cluster
 */
EmberAfStatus zcl_decode_over_the_air_bootloading_cluster_image_block_response_command (EmberAfClusterCommand * cmd, sl_zcl_over_the_air_bootloading_cluster_image_block_response_command_t *cmd_struct);
/** @brief Parser function for "UpgradeEndResponse" ZCL command from "Over the Air Bootloading" cluster
 */
EmberAfStatus zcl_decode_over_the_air_bootloading_cluster_upgrade_end_response_command (EmberAfClusterCommand * cmd, sl_zcl_over_the_air_bootloading_cluster_upgrade_end_response_command_t *cmd_struct);
/** @brief Parser function for "QuerySpecificFileResponse" ZCL command from "Over the Air Bootloading" cluster
 */
EmberAfStatus zcl_decode_over_the_air_bootloading_cluster_query_specific_file_response_command (EmberAfClusterCommand * cmd, sl_zcl_over_the_air_bootloading_cluster_query_specific_file_response_command_t *cmd_struct);
/** @brief Parser function for "PowerProfileRequest" ZCL command from "Power Profile" cluster
 */
EmberAfStatus zcl_decode_power_profile_cluster_power_profile_request_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_power_profile_request_command_t *cmd_struct);
/** @brief Parser function for "GetPowerProfilePriceResponse" ZCL command from "Power Profile" cluster
 */
EmberAfStatus zcl_decode_power_profile_cluster_get_power_profile_price_response_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_get_power_profile_price_response_command_t *cmd_struct);
/** @brief Parser function for "GetOverallSchedulePriceResponse" ZCL command from "Power Profile" cluster
 */
EmberAfStatus zcl_decode_power_profile_cluster_get_overall_schedule_price_response_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_get_overall_schedule_price_response_command_t *cmd_struct);
/** @brief Parser function for "EnergyPhasesScheduleNotification" ZCL command from "Power Profile" cluster
 */
EmberAfStatus zcl_decode_power_profile_cluster_energy_phases_schedule_notification_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_energy_phases_schedule_notification_command_t *cmd_struct);
/** @brief Parser function for "EnergyPhasesScheduleResponse" ZCL command from "Power Profile" cluster
 */
EmberAfStatus zcl_decode_power_profile_cluster_energy_phases_schedule_response_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_energy_phases_schedule_response_command_t *cmd_struct);
/** @brief Parser function for "PowerProfileScheduleConstraintsRequest" ZCL command from "Power Profile" cluster
 */
EmberAfStatus zcl_decode_power_profile_cluster_power_profile_schedule_constraints_request_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_power_profile_schedule_constraints_request_command_t *cmd_struct);
/** @brief Parser function for "EnergyPhasesScheduleStateRequest" ZCL command from "Power Profile" cluster
 */
EmberAfStatus zcl_decode_power_profile_cluster_energy_phases_schedule_state_request_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_energy_phases_schedule_state_request_command_t *cmd_struct);
/** @brief Parser function for "GetPowerProfilePriceExtendedResponse" ZCL command from "Power Profile" cluster
 */
EmberAfStatus zcl_decode_power_profile_cluster_get_power_profile_price_extended_response_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_get_power_profile_price_extended_response_command_t *cmd_struct);
/** @brief Parser function for "PowerProfileNotification" ZCL command from "Power Profile" cluster
 */
EmberAfStatus zcl_decode_power_profile_cluster_power_profile_notification_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_power_profile_notification_command_t *cmd_struct);
/** @brief Parser function for "PowerProfileResponse" ZCL command from "Power Profile" cluster
 */
EmberAfStatus zcl_decode_power_profile_cluster_power_profile_response_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_power_profile_response_command_t *cmd_struct);
/** @brief Parser function for "PowerProfileStateResponse" ZCL command from "Power Profile" cluster
 */
EmberAfStatus zcl_decode_power_profile_cluster_power_profile_state_response_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_power_profile_state_response_command_t *cmd_struct);
/** @brief Parser function for "GetPowerProfilePrice" ZCL command from "Power Profile" cluster
 */
EmberAfStatus zcl_decode_power_profile_cluster_get_power_profile_price_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_get_power_profile_price_command_t *cmd_struct);
/** @brief Parser function for "PowerProfilesStateNotification" ZCL command from "Power Profile" cluster
 */
EmberAfStatus zcl_decode_power_profile_cluster_power_profiles_state_notification_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_power_profiles_state_notification_command_t *cmd_struct);
/** @brief Parser function for "EnergyPhasesScheduleRequest" ZCL command from "Power Profile" cluster
 */
EmberAfStatus zcl_decode_power_profile_cluster_energy_phases_schedule_request_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_energy_phases_schedule_request_command_t *cmd_struct);
/** @brief Parser function for "EnergyPhasesScheduleStateResponse" ZCL command from "Power Profile" cluster
 */
EmberAfStatus zcl_decode_power_profile_cluster_energy_phases_schedule_state_response_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_energy_phases_schedule_state_response_command_t *cmd_struct);
/** @brief Parser function for "EnergyPhasesScheduleStateNotification" ZCL command from "Power Profile" cluster
 */
EmberAfStatus zcl_decode_power_profile_cluster_energy_phases_schedule_state_notification_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_energy_phases_schedule_state_notification_command_t *cmd_struct);
/** @brief Parser function for "PowerProfileScheduleConstraintsNotification" ZCL command from "Power Profile" cluster
 */
EmberAfStatus zcl_decode_power_profile_cluster_power_profile_schedule_constraints_notification_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_power_profile_schedule_constraints_notification_command_t *cmd_struct);
/** @brief Parser function for "PowerProfileScheduleConstraintsResponse" ZCL command from "Power Profile" cluster
 */
EmberAfStatus zcl_decode_power_profile_cluster_power_profile_schedule_constraints_response_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_power_profile_schedule_constraints_response_command_t *cmd_struct);
/** @brief Parser function for "GetPowerProfilePriceExtended" ZCL command from "Power Profile" cluster
 */
EmberAfStatus zcl_decode_power_profile_cluster_get_power_profile_price_extended_command (EmberAfClusterCommand * cmd, sl_zcl_power_profile_cluster_get_power_profile_price_extended_command_t *cmd_struct);
/** @brief Parser function for "ExecutionOfACommand" ZCL command from "Appliance Control" cluster
 */
EmberAfStatus zcl_decode_appliance_control_cluster_execution_of_a_command_command (EmberAfClusterCommand * cmd, sl_zcl_appliance_control_cluster_execution_of_a_command_command_t *cmd_struct);
/** @brief Parser function for "WriteFunctions" ZCL command from "Appliance Control" cluster
 */
EmberAfStatus zcl_decode_appliance_control_cluster_write_functions_command (EmberAfClusterCommand * cmd, sl_zcl_appliance_control_cluster_write_functions_command_t *cmd_struct);
/** @brief Parser function for "OverloadWarning" ZCL command from "Appliance Control" cluster
 */
EmberAfStatus zcl_decode_appliance_control_cluster_overload_warning_command (EmberAfClusterCommand * cmd, sl_zcl_appliance_control_cluster_overload_warning_command_t *cmd_struct);
/** @brief Parser function for "SignalStateResponse" ZCL command from "Appliance Control" cluster
 */
EmberAfStatus zcl_decode_appliance_control_cluster_signal_state_response_command (EmberAfClusterCommand * cmd, sl_zcl_appliance_control_cluster_signal_state_response_command_t *cmd_struct);
/** @brief Parser function for "SignalStateNotification" ZCL command from "Appliance Control" cluster
 */
EmberAfStatus zcl_decode_appliance_control_cluster_signal_state_notification_command (EmberAfClusterCommand * cmd, sl_zcl_appliance_control_cluster_signal_state_notification_command_t *cmd_struct);
/** @brief Parser function for "CheckInResponse" ZCL command from "Poll Control" cluster
 */
EmberAfStatus zcl_decode_poll_control_cluster_check_in_response_command (EmberAfClusterCommand * cmd, sl_zcl_poll_control_cluster_check_in_response_command_t *cmd_struct);
/** @brief Parser function for "SetLongPollInterval" ZCL command from "Poll Control" cluster
 */
EmberAfStatus zcl_decode_poll_control_cluster_set_long_poll_interval_command (EmberAfClusterCommand * cmd, sl_zcl_poll_control_cluster_set_long_poll_interval_command_t *cmd_struct);
/** @brief Parser function for "SetShortPollInterval" ZCL command from "Poll Control" cluster
 */
EmberAfStatus zcl_decode_poll_control_cluster_set_short_poll_interval_command (EmberAfClusterCommand * cmd, sl_zcl_poll_control_cluster_set_short_poll_interval_command_t *cmd_struct);
/** @brief Parser function for "GpNotification" ZCL command from "Green Power" cluster
 */
EmberAfStatus zcl_decode_green_power_cluster_gp_notification_command (EmberAfClusterCommand * cmd, sl_zcl_green_power_cluster_gp_notification_command_t *cmd_struct);
/** @brief Parser function for "GpPairingSearch" ZCL command from "Green Power" cluster
 */
EmberAfStatus zcl_decode_green_power_cluster_gp_pairing_search_command (EmberAfClusterCommand * cmd, sl_zcl_green_power_cluster_gp_pairing_search_command_t *cmd_struct);
/** @brief Parser function for "GpTunnelingStop" ZCL command from "Green Power" cluster
 */
EmberAfStatus zcl_decode_green_power_cluster_gp_tunneling_stop_command (EmberAfClusterCommand * cmd, sl_zcl_green_power_cluster_gp_tunneling_stop_command_t *cmd_struct);
/** @brief Parser function for "GpCommissioningNotification" ZCL command from "Green Power" cluster
 */
EmberAfStatus zcl_decode_green_power_cluster_gp_commissioning_notification_command (EmberAfClusterCommand * cmd, sl_zcl_green_power_cluster_gp_commissioning_notification_command_t *cmd_struct);
/** @brief Parser function for "GpSinkCommissioningMode" ZCL command from "Green Power" cluster
 */
EmberAfStatus zcl_decode_green_power_cluster_gp_sink_commissioning_mode_command (EmberAfClusterCommand * cmd, sl_zcl_green_power_cluster_gp_sink_commissioning_mode_command_t *cmd_struct);
/** @brief Parser function for "GpTranslationTableUpdate" ZCL command from "Green Power" cluster
 */
EmberAfStatus zcl_decode_green_power_cluster_gp_translation_table_update_command (EmberAfClusterCommand * cmd, sl_zcl_green_power_cluster_gp_translation_table_update_command_t *cmd_struct);
/** @brief Parser function for "GpTranslationTableRequest" ZCL command from "Green Power" cluster
 */
EmberAfStatus zcl_decode_green_power_cluster_gp_translation_table_request_command (EmberAfClusterCommand * cmd, sl_zcl_green_power_cluster_gp_translation_table_request_command_t *cmd_struct);
/** @brief Parser function for "GpPairingConfiguration" ZCL command from "Green Power" cluster
 */
EmberAfStatus zcl_decode_green_power_cluster_gp_pairing_configuration_command (EmberAfClusterCommand * cmd, sl_zcl_green_power_cluster_gp_pairing_configuration_command_t *cmd_struct);
/** @brief Parser function for "GpSinkTableRequest" ZCL command from "Green Power" cluster
 */
EmberAfStatus zcl_decode_green_power_cluster_gp_sink_table_request_command (EmberAfClusterCommand * cmd, sl_zcl_green_power_cluster_gp_sink_table_request_command_t *cmd_struct);
/** @brief Parser function for "GpProxyTableResponse" ZCL command from "Green Power" cluster
 */
EmberAfStatus zcl_decode_green_power_cluster_gp_proxy_table_response_command (EmberAfClusterCommand * cmd, sl_zcl_green_power_cluster_gp_proxy_table_response_command_t *cmd_struct);
/** @brief Parser function for "GpNotificationResponse" ZCL command from "Green Power" cluster
 */
EmberAfStatus zcl_decode_green_power_cluster_gp_notification_response_command (EmberAfClusterCommand * cmd, sl_zcl_green_power_cluster_gp_notification_response_command_t *cmd_struct);
/** @brief Parser function for "GpPairing" ZCL command from "Green Power" cluster
 */
EmberAfStatus zcl_decode_green_power_cluster_gp_pairing_command (EmberAfClusterCommand * cmd, sl_zcl_green_power_cluster_gp_pairing_command_t *cmd_struct);
/** @brief Parser function for "GpProxyCommissioningMode" ZCL command from "Green Power" cluster
 */
EmberAfStatus zcl_decode_green_power_cluster_gp_proxy_commissioning_mode_command (EmberAfClusterCommand * cmd, sl_zcl_green_power_cluster_gp_proxy_commissioning_mode_command_t *cmd_struct);
/** @brief Parser function for "GpResponse" ZCL command from "Green Power" cluster
 */
EmberAfStatus zcl_decode_green_power_cluster_gp_response_command (EmberAfClusterCommand * cmd, sl_zcl_green_power_cluster_gp_response_command_t *cmd_struct);
/** @brief Parser function for "GpTranslationTableResponse" ZCL command from "Green Power" cluster
 */
EmberAfStatus zcl_decode_green_power_cluster_gp_translation_table_response_command (EmberAfClusterCommand * cmd, sl_zcl_green_power_cluster_gp_translation_table_response_command_t *cmd_struct);
/** @brief Parser function for "GpSinkTableResponse" ZCL command from "Green Power" cluster
 */
EmberAfStatus zcl_decode_green_power_cluster_gp_sink_table_response_command (EmberAfClusterCommand * cmd, sl_zcl_green_power_cluster_gp_sink_table_response_command_t *cmd_struct);
/** @brief Parser function for "GpProxyTableRequest" ZCL command from "Green Power" cluster
 */
EmberAfStatus zcl_decode_green_power_cluster_gp_proxy_table_request_command (EmberAfClusterCommand * cmd, sl_zcl_green_power_cluster_gp_proxy_table_request_command_t *cmd_struct);
/** @brief Parser function for "LockDoor" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_lock_door_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_lock_door_command_t *cmd_struct);
/** @brief Parser function for "UnlockDoor" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_unlock_door_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_unlock_door_command_t *cmd_struct);
/** @brief Parser function for "Toggle" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_toggle_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_toggle_command_t *cmd_struct);
/** @brief Parser function for "UnlockWithTimeout" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_unlock_with_timeout_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_unlock_with_timeout_command_t *cmd_struct);
/** @brief Parser function for "GetLogRecord" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_get_log_record_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_log_record_command_t *cmd_struct);
/** @brief Parser function for "SetPin" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_set_pin_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_set_pin_command_t *cmd_struct);
/** @brief Parser function for "GetPin" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_get_pin_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_pin_command_t *cmd_struct);
/** @brief Parser function for "ClearPin" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_clear_pin_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_clear_pin_command_t *cmd_struct);
/** @brief Parser function for "SetUserStatus" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_set_user_status_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_set_user_status_command_t *cmd_struct);
/** @brief Parser function for "GetUserStatus" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_get_user_status_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_user_status_command_t *cmd_struct);
/** @brief Parser function for "SetWeekdaySchedule" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_set_weekday_schedule_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_set_weekday_schedule_command_t *cmd_struct);
/** @brief Parser function for "GetWeekdaySchedule" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_get_weekday_schedule_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_weekday_schedule_command_t *cmd_struct);
/** @brief Parser function for "ClearWeekdaySchedule" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_clear_weekday_schedule_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_clear_weekday_schedule_command_t *cmd_struct);
/** @brief Parser function for "SetYeardaySchedule" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_set_yearday_schedule_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_set_yearday_schedule_command_t *cmd_struct);
/** @brief Parser function for "GetYeardaySchedule" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_get_yearday_schedule_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_yearday_schedule_command_t *cmd_struct);
/** @brief Parser function for "ClearYeardaySchedule" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_clear_yearday_schedule_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_clear_yearday_schedule_command_t *cmd_struct);
/** @brief Parser function for "SetHolidaySchedule" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_set_holiday_schedule_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_set_holiday_schedule_command_t *cmd_struct);
/** @brief Parser function for "GetHolidaySchedule" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_get_holiday_schedule_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_holiday_schedule_command_t *cmd_struct);
/** @brief Parser function for "ClearHolidaySchedule" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_clear_holiday_schedule_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_clear_holiday_schedule_command_t *cmd_struct);
/** @brief Parser function for "SetUserType" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_set_user_type_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_set_user_type_command_t *cmd_struct);
/** @brief Parser function for "GetUserType" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_get_user_type_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_user_type_command_t *cmd_struct);
/** @brief Parser function for "SetRfid" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_set_rfid_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_set_rfid_command_t *cmd_struct);
/** @brief Parser function for "GetRfid" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_get_rfid_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_rfid_command_t *cmd_struct);
/** @brief Parser function for "ClearRfid" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_clear_rfid_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_clear_rfid_command_t *cmd_struct);
/** @brief Parser function for "SetDisposableSchedule" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_set_disposable_schedule_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_set_disposable_schedule_command_t *cmd_struct);
/** @brief Parser function for "GetDisposableSchedule" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_get_disposable_schedule_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_disposable_schedule_command_t *cmd_struct);
/** @brief Parser function for "ClearDisposableSchedule" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_clear_disposable_schedule_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_clear_disposable_schedule_command_t *cmd_struct);
/** @brief Parser function for "ClearBiometricCredential" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_clear_biometric_credential_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_clear_biometric_credential_command_t *cmd_struct);
/** @brief Parser function for "LockDoorResponse" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_lock_door_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_lock_door_response_command_t *cmd_struct);
/** @brief Parser function for "UnlockDoorResponse" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_unlock_door_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_unlock_door_response_command_t *cmd_struct);
/** @brief Parser function for "ToggleResponse" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_toggle_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_toggle_response_command_t *cmd_struct);
/** @brief Parser function for "UnlockWithTimeoutResponse" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_unlock_with_timeout_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_unlock_with_timeout_response_command_t *cmd_struct);
/** @brief Parser function for "GetLogRecordResponse" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_get_log_record_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_log_record_response_command_t *cmd_struct);
/** @brief Parser function for "SetPinResponse" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_set_pin_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_set_pin_response_command_t *cmd_struct);
/** @brief Parser function for "GetPinResponse" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_get_pin_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_pin_response_command_t *cmd_struct);
/** @brief Parser function for "ClearPinResponse" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_clear_pin_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_clear_pin_response_command_t *cmd_struct);
/** @brief Parser function for "ClearAllPinsResponse" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_clear_all_pins_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_clear_all_pins_response_command_t *cmd_struct);
/** @brief Parser function for "SetUserStatusResponse" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_set_user_status_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_set_user_status_response_command_t *cmd_struct);
/** @brief Parser function for "GetUserStatusResponse" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_get_user_status_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_user_status_response_command_t *cmd_struct);
/** @brief Parser function for "SetWeekdayScheduleResponse" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_set_weekday_schedule_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_set_weekday_schedule_response_command_t *cmd_struct);
/** @brief Parser function for "GetWeekdayScheduleResponse" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_get_weekday_schedule_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_weekday_schedule_response_command_t *cmd_struct);
/** @brief Parser function for "ClearWeekdayScheduleResponse" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_clear_weekday_schedule_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_clear_weekday_schedule_response_command_t *cmd_struct);
/** @brief Parser function for "SetYeardayScheduleResponse" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_set_yearday_schedule_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_set_yearday_schedule_response_command_t *cmd_struct);
/** @brief Parser function for "GetYeardayScheduleResponse" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_get_yearday_schedule_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_yearday_schedule_response_command_t *cmd_struct);
/** @brief Parser function for "ClearYeardayScheduleResponse" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_clear_yearday_schedule_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_clear_yearday_schedule_response_command_t *cmd_struct);
/** @brief Parser function for "SetHolidayScheduleResponse" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_set_holiday_schedule_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_set_holiday_schedule_response_command_t *cmd_struct);
/** @brief Parser function for "GetHolidayScheduleResponse" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_get_holiday_schedule_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_holiday_schedule_response_command_t *cmd_struct);
/** @brief Parser function for "ClearHolidayScheduleResponse" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_clear_holiday_schedule_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_clear_holiday_schedule_response_command_t *cmd_struct);
/** @brief Parser function for "SetUserTypeResponse" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_set_user_type_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_set_user_type_response_command_t *cmd_struct);
/** @brief Parser function for "GetUserTypeResponse" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_get_user_type_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_user_type_response_command_t *cmd_struct);
/** @brief Parser function for "SetRfidResponse" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_set_rfid_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_set_rfid_response_command_t *cmd_struct);
/** @brief Parser function for "GetRfidResponse" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_get_rfid_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_rfid_response_command_t *cmd_struct);
/** @brief Parser function for "ClearRfidResponse" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_clear_rfid_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_clear_rfid_response_command_t *cmd_struct);
/** @brief Parser function for "ClearAllRfidsResponse" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_clear_all_rfids_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_clear_all_rfids_response_command_t *cmd_struct);
/** @brief Parser function for "SetDisposableScheduleResponse" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_set_disposable_schedule_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_set_disposable_schedule_response_command_t *cmd_struct);
/** @brief Parser function for "GetDisposableScheduleResponse" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_get_disposable_schedule_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_get_disposable_schedule_response_command_t *cmd_struct);
/** @brief Parser function for "ClearDisposableScheduleResponse" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_clear_disposable_schedule_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_clear_disposable_schedule_response_command_t *cmd_struct);
/** @brief Parser function for "ClearBiometricCredentialResponse" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_clear_biometric_credential_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_clear_biometric_credential_response_command_t *cmd_struct);
/** @brief Parser function for "ClearAllBiometricCredentialsResponse" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_clear_all_biometric_credentials_response_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_clear_all_biometric_credentials_response_command_t *cmd_struct);
/** @brief Parser function for "OperationEventNotification" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_operation_event_notification_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_operation_event_notification_command_t *cmd_struct);
/** @brief Parser function for "ProgrammingEventNotification" ZCL command from "Door Lock" cluster
 */
EmberAfStatus zcl_decode_door_lock_cluster_programming_event_notification_command (EmberAfClusterCommand * cmd, sl_zcl_door_lock_cluster_programming_event_notification_command_t *cmd_struct);
/** @brief Parser function for "WindowCoveringGoToLiftValue" ZCL command from "Window Covering" cluster
 */
EmberAfStatus zcl_decode_window_covering_cluster_window_covering_go_to_lift_value_command (EmberAfClusterCommand * cmd, sl_zcl_window_covering_cluster_window_covering_go_to_lift_value_command_t *cmd_struct);
/** @brief Parser function for "WindowCoveringGoToLiftPercentage" ZCL command from "Window Covering" cluster
 */
EmberAfStatus zcl_decode_window_covering_cluster_window_covering_go_to_lift_percentage_command (EmberAfClusterCommand * cmd, sl_zcl_window_covering_cluster_window_covering_go_to_lift_percentage_command_t *cmd_struct);
/** @brief Parser function for "WindowCoveringGoToTiltValue" ZCL command from "Window Covering" cluster
 */
EmberAfStatus zcl_decode_window_covering_cluster_window_covering_go_to_tilt_value_command (EmberAfClusterCommand * cmd, sl_zcl_window_covering_cluster_window_covering_go_to_tilt_value_command_t *cmd_struct);
/** @brief Parser function for "WindowCoveringGoToTiltPercentage" ZCL command from "Window Covering" cluster
 */
EmberAfStatus zcl_decode_window_covering_cluster_window_covering_go_to_tilt_percentage_command (EmberAfClusterCommand * cmd, sl_zcl_window_covering_cluster_window_covering_go_to_tilt_percentage_command_t *cmd_struct);
/** @brief Parser function for "BarrierControlGoToPercent" ZCL command from "Barrier Control" cluster
 */
EmberAfStatus zcl_decode_barrier_control_cluster_barrier_control_go_to_percent_command (EmberAfClusterCommand * cmd, sl_zcl_barrier_control_cluster_barrier_control_go_to_percent_command_t *cmd_struct);
/** @brief Parser function for "SetpointRaiseLower" ZCL command from "Thermostat" cluster
 */
EmberAfStatus zcl_decode_thermostat_cluster_setpoint_raise_lower_command (EmberAfClusterCommand * cmd, sl_zcl_thermostat_cluster_setpoint_raise_lower_command_t *cmd_struct);
/** @brief Parser function for "SetWeeklySchedule" ZCL command from "Thermostat" cluster
 */
EmberAfStatus zcl_decode_thermostat_cluster_set_weekly_schedule_command (EmberAfClusterCommand * cmd, sl_zcl_thermostat_cluster_set_weekly_schedule_command_t *cmd_struct);
/** @brief Parser function for "GetWeeklySchedule" ZCL command from "Thermostat" cluster
 */
EmberAfStatus zcl_decode_thermostat_cluster_get_weekly_schedule_command (EmberAfClusterCommand * cmd, sl_zcl_thermostat_cluster_get_weekly_schedule_command_t *cmd_struct);
/** @brief Parser function for "CurrentWeeklySchedule" ZCL command from "Thermostat" cluster
 */
EmberAfStatus zcl_decode_thermostat_cluster_current_weekly_schedule_command (EmberAfClusterCommand * cmd, sl_zcl_thermostat_cluster_current_weekly_schedule_command_t *cmd_struct);
/** @brief Parser function for "RelayStatusLog" ZCL command from "Thermostat" cluster
 */
EmberAfStatus zcl_decode_thermostat_cluster_relay_status_log_command (EmberAfClusterCommand * cmd, sl_zcl_thermostat_cluster_relay_status_log_command_t *cmd_struct);
/** @brief Parser function for "MoveToHue" ZCL command from "Color Control" cluster
 */
EmberAfStatus zcl_decode_color_control_cluster_move_to_hue_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_move_to_hue_command_t *cmd_struct);
/** @brief Parser function for "MoveHue" ZCL command from "Color Control" cluster
 */
EmberAfStatus zcl_decode_color_control_cluster_move_hue_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_move_hue_command_t *cmd_struct);
/** @brief Parser function for "StepHue" ZCL command from "Color Control" cluster
 */
EmberAfStatus zcl_decode_color_control_cluster_step_hue_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_step_hue_command_t *cmd_struct);
/** @brief Parser function for "MoveToSaturation" ZCL command from "Color Control" cluster
 */
EmberAfStatus zcl_decode_color_control_cluster_move_to_saturation_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_move_to_saturation_command_t *cmd_struct);
/** @brief Parser function for "MoveSaturation" ZCL command from "Color Control" cluster
 */
EmberAfStatus zcl_decode_color_control_cluster_move_saturation_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_move_saturation_command_t *cmd_struct);
/** @brief Parser function for "StepSaturation" ZCL command from "Color Control" cluster
 */
EmberAfStatus zcl_decode_color_control_cluster_step_saturation_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_step_saturation_command_t *cmd_struct);
/** @brief Parser function for "MoveToHueAndSaturation" ZCL command from "Color Control" cluster
 */
EmberAfStatus zcl_decode_color_control_cluster_move_to_hue_and_saturation_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_move_to_hue_and_saturation_command_t *cmd_struct);
/** @brief Parser function for "MoveToColor" ZCL command from "Color Control" cluster
 */
EmberAfStatus zcl_decode_color_control_cluster_move_to_color_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_move_to_color_command_t *cmd_struct);
/** @brief Parser function for "MoveColor" ZCL command from "Color Control" cluster
 */
EmberAfStatus zcl_decode_color_control_cluster_move_color_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_move_color_command_t *cmd_struct);
/** @brief Parser function for "StepColor" ZCL command from "Color Control" cluster
 */
EmberAfStatus zcl_decode_color_control_cluster_step_color_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_step_color_command_t *cmd_struct);
/** @brief Parser function for "MoveToColorTemperature" ZCL command from "Color Control" cluster
 */
EmberAfStatus zcl_decode_color_control_cluster_move_to_color_temperature_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_move_to_color_temperature_command_t *cmd_struct);
/** @brief Parser function for "EnhancedMoveToHue" ZCL command from "Color Control" cluster
 */
EmberAfStatus zcl_decode_color_control_cluster_enhanced_move_to_hue_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_enhanced_move_to_hue_command_t *cmd_struct);
/** @brief Parser function for "EnhancedMoveHue" ZCL command from "Color Control" cluster
 */
EmberAfStatus zcl_decode_color_control_cluster_enhanced_move_hue_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_enhanced_move_hue_command_t *cmd_struct);
/** @brief Parser function for "EnhancedStepHue" ZCL command from "Color Control" cluster
 */
EmberAfStatus zcl_decode_color_control_cluster_enhanced_step_hue_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_enhanced_step_hue_command_t *cmd_struct);
/** @brief Parser function for "EnhancedMoveToHueAndSaturation" ZCL command from "Color Control" cluster
 */
EmberAfStatus zcl_decode_color_control_cluster_enhanced_move_to_hue_and_saturation_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_enhanced_move_to_hue_and_saturation_command_t *cmd_struct);
/** @brief Parser function for "ColorLoopSet" ZCL command from "Color Control" cluster
 */
EmberAfStatus zcl_decode_color_control_cluster_color_loop_set_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_color_loop_set_command_t *cmd_struct);
/** @brief Parser function for "StopMoveStep" ZCL command from "Color Control" cluster
 */
EmberAfStatus zcl_decode_color_control_cluster_stop_move_step_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_stop_move_step_command_t *cmd_struct);
/** @brief Parser function for "MoveColorTemperature" ZCL command from "Color Control" cluster
 */
EmberAfStatus zcl_decode_color_control_cluster_move_color_temperature_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_move_color_temperature_command_t *cmd_struct);
/** @brief Parser function for "StepColorTemperature" ZCL command from "Color Control" cluster
 */
EmberAfStatus zcl_decode_color_control_cluster_step_color_temperature_command (EmberAfClusterCommand * cmd, sl_zcl_color_control_cluster_step_color_temperature_command_t *cmd_struct);
/** @brief Parser function for "ZoneEnrollResponse" ZCL command from "IAS Zone" cluster
 */
EmberAfStatus zcl_decode_ias_zone_cluster_zone_enroll_response_command (EmberAfClusterCommand * cmd, sl_zcl_ias_zone_cluster_zone_enroll_response_command_t *cmd_struct);
/** @brief Parser function for "InitiateTestMode" ZCL command from "IAS Zone" cluster
 */
EmberAfStatus zcl_decode_ias_zone_cluster_initiate_test_mode_command (EmberAfClusterCommand * cmd, sl_zcl_ias_zone_cluster_initiate_test_mode_command_t *cmd_struct);
/** @brief Parser function for "ZoneStatusChangeNotification" ZCL command from "IAS Zone" cluster
 */
EmberAfStatus zcl_decode_ias_zone_cluster_zone_status_change_notification_command (EmberAfClusterCommand * cmd, sl_zcl_ias_zone_cluster_zone_status_change_notification_command_t *cmd_struct);
/** @brief Parser function for "ZoneEnrollRequest" ZCL command from "IAS Zone" cluster
 */
EmberAfStatus zcl_decode_ias_zone_cluster_zone_enroll_request_command (EmberAfClusterCommand * cmd, sl_zcl_ias_zone_cluster_zone_enroll_request_command_t *cmd_struct);
/** @brief Parser function for "Arm" ZCL command from "IAS ACE" cluster
 */
EmberAfStatus zcl_decode_ias_ace_cluster_arm_command (EmberAfClusterCommand * cmd, sl_zcl_ias_ace_cluster_arm_command_t *cmd_struct);
/** @brief Parser function for "Bypass" ZCL command from "IAS ACE" cluster
 */
EmberAfStatus zcl_decode_ias_ace_cluster_bypass_command (EmberAfClusterCommand * cmd, sl_zcl_ias_ace_cluster_bypass_command_t *cmd_struct);
/** @brief Parser function for "GetZoneInformation" ZCL command from "IAS ACE" cluster
 */
EmberAfStatus zcl_decode_ias_ace_cluster_get_zone_information_command (EmberAfClusterCommand * cmd, sl_zcl_ias_ace_cluster_get_zone_information_command_t *cmd_struct);
/** @brief Parser function for "GetZoneStatus" ZCL command from "IAS ACE" cluster
 */
EmberAfStatus zcl_decode_ias_ace_cluster_get_zone_status_command (EmberAfClusterCommand * cmd, sl_zcl_ias_ace_cluster_get_zone_status_command_t *cmd_struct);
/** @brief Parser function for "ArmResponse" ZCL command from "IAS ACE" cluster
 */
EmberAfStatus zcl_decode_ias_ace_cluster_arm_response_command (EmberAfClusterCommand * cmd, sl_zcl_ias_ace_cluster_arm_response_command_t *cmd_struct);
/** @brief Parser function for "GetZoneIdMapResponse" ZCL command from "IAS ACE" cluster
 */
EmberAfStatus zcl_decode_ias_ace_cluster_get_zone_id_map_response_command (EmberAfClusterCommand * cmd, sl_zcl_ias_ace_cluster_get_zone_id_map_response_command_t *cmd_struct);
/** @brief Parser function for "GetZoneInformationResponse" ZCL command from "IAS ACE" cluster
 */
EmberAfStatus zcl_decode_ias_ace_cluster_get_zone_information_response_command (EmberAfClusterCommand * cmd, sl_zcl_ias_ace_cluster_get_zone_information_response_command_t *cmd_struct);
/** @brief Parser function for "ZoneStatusChanged" ZCL command from "IAS ACE" cluster
 */
EmberAfStatus zcl_decode_ias_ace_cluster_zone_status_changed_command (EmberAfClusterCommand * cmd, sl_zcl_ias_ace_cluster_zone_status_changed_command_t *cmd_struct);
/** @brief Parser function for "PanelStatusChanged" ZCL command from "IAS ACE" cluster
 */
EmberAfStatus zcl_decode_ias_ace_cluster_panel_status_changed_command (EmberAfClusterCommand * cmd, sl_zcl_ias_ace_cluster_panel_status_changed_command_t *cmd_struct);
/** @brief Parser function for "GetPanelStatusResponse" ZCL command from "IAS ACE" cluster
 */
EmberAfStatus zcl_decode_ias_ace_cluster_get_panel_status_response_command (EmberAfClusterCommand * cmd, sl_zcl_ias_ace_cluster_get_panel_status_response_command_t *cmd_struct);
/** @brief Parser function for "SetBypassedZoneList" ZCL command from "IAS ACE" cluster
 */
EmberAfStatus zcl_decode_ias_ace_cluster_set_bypassed_zone_list_command (EmberAfClusterCommand * cmd, sl_zcl_ias_ace_cluster_set_bypassed_zone_list_command_t *cmd_struct);
/** @brief Parser function for "BypassResponse" ZCL command from "IAS ACE" cluster
 */
EmberAfStatus zcl_decode_ias_ace_cluster_bypass_response_command (EmberAfClusterCommand * cmd, sl_zcl_ias_ace_cluster_bypass_response_command_t *cmd_struct);
/** @brief Parser function for "GetZoneStatusResponse" ZCL command from "IAS ACE" cluster
 */
EmberAfStatus zcl_decode_ias_ace_cluster_get_zone_status_response_command (EmberAfClusterCommand * cmd, sl_zcl_ias_ace_cluster_get_zone_status_response_command_t *cmd_struct);
/** @brief Parser function for "StartWarning" ZCL command from "IAS WD" cluster
 */
EmberAfStatus zcl_decode_ias_wd_cluster_start_warning_command (EmberAfClusterCommand * cmd, sl_zcl_ias_wd_cluster_start_warning_command_t *cmd_struct);
/** @brief Parser function for "Squawk" ZCL command from "IAS WD" cluster
 */
EmberAfStatus zcl_decode_ias_wd_cluster_squawk_command (EmberAfClusterCommand * cmd, sl_zcl_ias_wd_cluster_squawk_command_t *cmd_struct);
/** @brief Parser function for "MatchProtocolAddress" ZCL command from "Generic Tunnel" cluster
 */
EmberAfStatus zcl_decode_generic_tunnel_cluster_match_protocol_address_command (EmberAfClusterCommand * cmd, sl_zcl_generic_tunnel_cluster_match_protocol_address_command_t *cmd_struct);
/** @brief Parser function for "MatchProtocolAddressResponse" ZCL command from "Generic Tunnel" cluster
 */
EmberAfStatus zcl_decode_generic_tunnel_cluster_match_protocol_address_response_command (EmberAfClusterCommand * cmd, sl_zcl_generic_tunnel_cluster_match_protocol_address_response_command_t *cmd_struct);
/** @brief Parser function for "AdvertiseProtocolAddress" ZCL command from "Generic Tunnel" cluster
 */
EmberAfStatus zcl_decode_generic_tunnel_cluster_advertise_protocol_address_command (EmberAfClusterCommand * cmd, sl_zcl_generic_tunnel_cluster_advertise_protocol_address_command_t *cmd_struct);
/** @brief Parser function for "TransferNpdu" ZCL command from "BACnet Protocol Tunnel" cluster
 */
EmberAfStatus zcl_decode_ba_cnet_protocol_tunnel_cluster_transfer_npdu_command (EmberAfClusterCommand * cmd, sl_zcl_ba_cnet_protocol_tunnel_cluster_transfer_npdu_command_t *cmd_struct);
/** @brief Parser function for "TransferAPDU" ZCL command from "11073 Protocol Tunnel" cluster
 */
EmberAfStatus zcl_decode_11073_protocol_tunnel_cluster_transfer_apdu_command (EmberAfClusterCommand * cmd, sl_zcl_11073_protocol_tunnel_cluster_transfer_apdu_command_t *cmd_struct);
/** @brief Parser function for "ConnectRequest" ZCL command from "11073 Protocol Tunnel" cluster
 */
EmberAfStatus zcl_decode_11073_protocol_tunnel_cluster_connect_request_command (EmberAfClusterCommand * cmd, sl_zcl_11073_protocol_tunnel_cluster_connect_request_command_t *cmd_struct);
/** @brief Parser function for "DisconnectRequest" ZCL command from "11073 Protocol Tunnel" cluster
 */
EmberAfStatus zcl_decode_11073_protocol_tunnel_cluster_disconnect_request_command (EmberAfClusterCommand * cmd, sl_zcl_11073_protocol_tunnel_cluster_disconnect_request_command_t *cmd_struct);
/** @brief Parser function for "ConnectStatusNotification" ZCL command from "11073 Protocol Tunnel" cluster
 */
EmberAfStatus zcl_decode_11073_protocol_tunnel_cluster_connect_status_notification_command (EmberAfClusterCommand * cmd, sl_zcl_11073_protocol_tunnel_cluster_connect_status_notification_command_t *cmd_struct);
/** @brief Parser function for "TransferApduFromClient" ZCL command from "ISO 7816 Protocol Tunnel" cluster
 */
EmberAfStatus zcl_decode_iso_7816_protocol_tunnel_cluster_transfer_apdu_from_client_command (EmberAfClusterCommand * cmd, sl_zcl_iso_7816_protocol_tunnel_cluster_transfer_apdu_from_client_command_t *cmd_struct);
/** @brief Parser function for "TransferApduFromServer" ZCL command from "ISO 7816 Protocol Tunnel" cluster
 */
EmberAfStatus zcl_decode_iso_7816_protocol_tunnel_cluster_transfer_apdu_from_server_command (EmberAfClusterCommand * cmd, sl_zcl_iso_7816_protocol_tunnel_cluster_transfer_apdu_from_server_command_t *cmd_struct);
/** @brief Parser function for "GetCurrentPrice" ZCL command from "Price" cluster
 */
EmberAfStatus zcl_decode_price_cluster_get_current_price_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_get_current_price_command_t *cmd_struct);
/** @brief Parser function for "GetScheduledPrices" ZCL command from "Price" cluster
 */
EmberAfStatus zcl_decode_price_cluster_get_scheduled_prices_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_get_scheduled_prices_command_t *cmd_struct);
/** @brief Parser function for "PriceAcknowledgement" ZCL command from "Price" cluster
 */
EmberAfStatus zcl_decode_price_cluster_price_acknowledgement_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_price_acknowledgement_command_t *cmd_struct);
/** @brief Parser function for "GetBlockPeriods" ZCL command from "Price" cluster
 */
EmberAfStatus zcl_decode_price_cluster_get_block_periods_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_get_block_periods_command_t *cmd_struct);
/** @brief Parser function for "GetConversionFactor" ZCL command from "Price" cluster
 */
EmberAfStatus zcl_decode_price_cluster_get_conversion_factor_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_get_conversion_factor_command_t *cmd_struct);
/** @brief Parser function for "GetCalorificValue" ZCL command from "Price" cluster
 */
EmberAfStatus zcl_decode_price_cluster_get_calorific_value_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_get_calorific_value_command_t *cmd_struct);
/** @brief Parser function for "GetTariffInformation" ZCL command from "Price" cluster
 */
EmberAfStatus zcl_decode_price_cluster_get_tariff_information_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_get_tariff_information_command_t *cmd_struct);
/** @brief Parser function for "GetPriceMatrix" ZCL command from "Price" cluster
 */
EmberAfStatus zcl_decode_price_cluster_get_price_matrix_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_get_price_matrix_command_t *cmd_struct);
/** @brief Parser function for "GetBlockThresholds" ZCL command from "Price" cluster
 */
EmberAfStatus zcl_decode_price_cluster_get_block_thresholds_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_get_block_thresholds_command_t *cmd_struct);
/** @brief Parser function for "GetCO2Value" ZCL command from "Price" cluster
 */
EmberAfStatus zcl_decode_price_cluster_get_co2_value_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_get_co2_value_command_t *cmd_struct);
/** @brief Parser function for "GetTierLabels" ZCL command from "Price" cluster
 */
EmberAfStatus zcl_decode_price_cluster_get_tier_labels_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_get_tier_labels_command_t *cmd_struct);
/** @brief Parser function for "GetBillingPeriod" ZCL command from "Price" cluster
 */
EmberAfStatus zcl_decode_price_cluster_get_billing_period_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_get_billing_period_command_t *cmd_struct);
/** @brief Parser function for "GetConsolidatedBill" ZCL command from "Price" cluster
 */
EmberAfStatus zcl_decode_price_cluster_get_consolidated_bill_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_get_consolidated_bill_command_t *cmd_struct);
/** @brief Parser function for "CppEventResponse" ZCL command from "Price" cluster
 */
EmberAfStatus zcl_decode_price_cluster_cpp_event_response_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_cpp_event_response_command_t *cmd_struct);
/** @brief Parser function for "GetCreditPayment" ZCL command from "Price" cluster
 */
EmberAfStatus zcl_decode_price_cluster_get_credit_payment_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_get_credit_payment_command_t *cmd_struct);
/** @brief Parser function for "PublishPrice" ZCL command from "Price" cluster
 */
EmberAfStatus zcl_decode_price_cluster_publish_price_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_publish_price_command_t *cmd_struct);
/** @brief Parser function for "PublishBlockPeriod" ZCL command from "Price" cluster
 */
EmberAfStatus zcl_decode_price_cluster_publish_block_period_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_publish_block_period_command_t *cmd_struct);
/** @brief Parser function for "PublishConversionFactor" ZCL command from "Price" cluster
 */
EmberAfStatus zcl_decode_price_cluster_publish_conversion_factor_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_publish_conversion_factor_command_t *cmd_struct);
/** @brief Parser function for "PublishCalorificValue" ZCL command from "Price" cluster
 */
EmberAfStatus zcl_decode_price_cluster_publish_calorific_value_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_publish_calorific_value_command_t *cmd_struct);
/** @brief Parser function for "PublishTariffInformation" ZCL command from "Price" cluster
 */
EmberAfStatus zcl_decode_price_cluster_publish_tariff_information_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_publish_tariff_information_command_t *cmd_struct);
/** @brief Parser function for "PublishPriceMatrix" ZCL command from "Price" cluster
 */
EmberAfStatus zcl_decode_price_cluster_publish_price_matrix_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_publish_price_matrix_command_t *cmd_struct);
/** @brief Parser function for "PublishBlockThresholds" ZCL command from "Price" cluster
 */
EmberAfStatus zcl_decode_price_cluster_publish_block_thresholds_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_publish_block_thresholds_command_t *cmd_struct);
/** @brief Parser function for "PublishCO2Value" ZCL command from "Price" cluster
 */
EmberAfStatus zcl_decode_price_cluster_publish_co2_value_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_publish_co2_value_command_t *cmd_struct);
/** @brief Parser function for "PublishTierLabels" ZCL command from "Price" cluster
 */
EmberAfStatus zcl_decode_price_cluster_publish_tier_labels_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_publish_tier_labels_command_t *cmd_struct);
/** @brief Parser function for "PublishBillingPeriod" ZCL command from "Price" cluster
 */
EmberAfStatus zcl_decode_price_cluster_publish_billing_period_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_publish_billing_period_command_t *cmd_struct);
/** @brief Parser function for "PublishConsolidatedBill" ZCL command from "Price" cluster
 */
EmberAfStatus zcl_decode_price_cluster_publish_consolidated_bill_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_publish_consolidated_bill_command_t *cmd_struct);
/** @brief Parser function for "PublishCppEvent" ZCL command from "Price" cluster
 */
EmberAfStatus zcl_decode_price_cluster_publish_cpp_event_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_publish_cpp_event_command_t *cmd_struct);
/** @brief Parser function for "PublishCreditPayment" ZCL command from "Price" cluster
 */
EmberAfStatus zcl_decode_price_cluster_publish_credit_payment_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_publish_credit_payment_command_t *cmd_struct);
/** @brief Parser function for "PublishCurrencyConversion" ZCL command from "Price" cluster
 */
EmberAfStatus zcl_decode_price_cluster_publish_currency_conversion_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_publish_currency_conversion_command_t *cmd_struct);
/** @brief Parser function for "CancelTariff" ZCL command from "Price" cluster
 */
EmberAfStatus zcl_decode_price_cluster_cancel_tariff_command (EmberAfClusterCommand * cmd, sl_zcl_price_cluster_cancel_tariff_command_t *cmd_struct);
/** @brief Parser function for "ReportEventStatus" ZCL command from "Demand Response and Load Control" cluster
 */
EmberAfStatus zcl_decode_demand_response_and_load_control_cluster_report_event_status_command (EmberAfClusterCommand * cmd, sl_zcl_demand_response_and_load_control_cluster_report_event_status_command_t *cmd_struct);
/** @brief Parser function for "GetScheduledEvents" ZCL command from "Demand Response and Load Control" cluster
 */
EmberAfStatus zcl_decode_demand_response_and_load_control_cluster_get_scheduled_events_command (EmberAfClusterCommand * cmd, sl_zcl_demand_response_and_load_control_cluster_get_scheduled_events_command_t *cmd_struct);
/** @brief Parser function for "LoadControlEvent" ZCL command from "Demand Response and Load Control" cluster
 */
EmberAfStatus zcl_decode_demand_response_and_load_control_cluster_load_control_event_command (EmberAfClusterCommand * cmd, sl_zcl_demand_response_and_load_control_cluster_load_control_event_command_t *cmd_struct);
/** @brief Parser function for "CancelLoadControlEvent" ZCL command from "Demand Response and Load Control" cluster
 */
EmberAfStatus zcl_decode_demand_response_and_load_control_cluster_cancel_load_control_event_command (EmberAfClusterCommand * cmd, sl_zcl_demand_response_and_load_control_cluster_cancel_load_control_event_command_t *cmd_struct);
/** @brief Parser function for "CancelAllLoadControlEvents" ZCL command from "Demand Response and Load Control" cluster
 */
EmberAfStatus zcl_decode_demand_response_and_load_control_cluster_cancel_all_load_control_events_command (EmberAfClusterCommand * cmd, sl_zcl_demand_response_and_load_control_cluster_cancel_all_load_control_events_command_t *cmd_struct);
/** @brief Parser function for "GetProfile" ZCL command from "Simple Metering" cluster
 */
EmberAfStatus zcl_decode_simple_metering_cluster_get_profile_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_get_profile_command_t *cmd_struct);
/** @brief Parser function for "RequestMirrorResponse" ZCL command from "Simple Metering" cluster
 */
EmberAfStatus zcl_decode_simple_metering_cluster_request_mirror_response_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_request_mirror_response_command_t *cmd_struct);
/** @brief Parser function for "MirrorRemoved" ZCL command from "Simple Metering" cluster
 */
EmberAfStatus zcl_decode_simple_metering_cluster_mirror_removed_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_mirror_removed_command_t *cmd_struct);
/** @brief Parser function for "RequestFastPollMode" ZCL command from "Simple Metering" cluster
 */
EmberAfStatus zcl_decode_simple_metering_cluster_request_fast_poll_mode_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_request_fast_poll_mode_command_t *cmd_struct);
/** @brief Parser function for "ScheduleSnapshot" ZCL command from "Simple Metering" cluster
 */
EmberAfStatus zcl_decode_simple_metering_cluster_schedule_snapshot_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_schedule_snapshot_command_t *cmd_struct);
/** @brief Parser function for "TakeSnapshot" ZCL command from "Simple Metering" cluster
 */
EmberAfStatus zcl_decode_simple_metering_cluster_take_snapshot_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_take_snapshot_command_t *cmd_struct);
/** @brief Parser function for "GetSnapshot" ZCL command from "Simple Metering" cluster
 */
EmberAfStatus zcl_decode_simple_metering_cluster_get_snapshot_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_get_snapshot_command_t *cmd_struct);
/** @brief Parser function for "StartSampling" ZCL command from "Simple Metering" cluster
 */
EmberAfStatus zcl_decode_simple_metering_cluster_start_sampling_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_start_sampling_command_t *cmd_struct);
/** @brief Parser function for "GetSampledData" ZCL command from "Simple Metering" cluster
 */
EmberAfStatus zcl_decode_simple_metering_cluster_get_sampled_data_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_get_sampled_data_command_t *cmd_struct);
/** @brief Parser function for "MirrorReportAttributeResponse" ZCL command from "Simple Metering" cluster
 */
EmberAfStatus zcl_decode_simple_metering_cluster_mirror_report_attribute_response_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_mirror_report_attribute_response_command_t *cmd_struct);
/** @brief Parser function for "ResetLoadLimitCounter" ZCL command from "Simple Metering" cluster
 */
EmberAfStatus zcl_decode_simple_metering_cluster_reset_load_limit_counter_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_reset_load_limit_counter_command_t *cmd_struct);
/** @brief Parser function for "ChangeSupply" ZCL command from "Simple Metering" cluster
 */
EmberAfStatus zcl_decode_simple_metering_cluster_change_supply_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_change_supply_command_t *cmd_struct);
/** @brief Parser function for "LocalChangeSupply" ZCL command from "Simple Metering" cluster
 */
EmberAfStatus zcl_decode_simple_metering_cluster_local_change_supply_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_local_change_supply_command_t *cmd_struct);
/** @brief Parser function for "SetSupplyStatus" ZCL command from "Simple Metering" cluster
 */
EmberAfStatus zcl_decode_simple_metering_cluster_set_supply_status_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_set_supply_status_command_t *cmd_struct);
/** @brief Parser function for "SetUncontrolledFlowThreshold" ZCL command from "Simple Metering" cluster
 */
EmberAfStatus zcl_decode_simple_metering_cluster_set_uncontrolled_flow_threshold_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_set_uncontrolled_flow_threshold_command_t *cmd_struct);
/** @brief Parser function for "GetProfileResponse" ZCL command from "Simple Metering" cluster
 */
EmberAfStatus zcl_decode_simple_metering_cluster_get_profile_response_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_get_profile_response_command_t *cmd_struct);
/** @brief Parser function for "RequestFastPollModeResponse" ZCL command from "Simple Metering" cluster
 */
EmberAfStatus zcl_decode_simple_metering_cluster_request_fast_poll_mode_response_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_request_fast_poll_mode_response_command_t *cmd_struct);
/** @brief Parser function for "ScheduleSnapshotResponse" ZCL command from "Simple Metering" cluster
 */
EmberAfStatus zcl_decode_simple_metering_cluster_schedule_snapshot_response_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_schedule_snapshot_response_command_t *cmd_struct);
/** @brief Parser function for "TakeSnapshotResponse" ZCL command from "Simple Metering" cluster
 */
EmberAfStatus zcl_decode_simple_metering_cluster_take_snapshot_response_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_take_snapshot_response_command_t *cmd_struct);
/** @brief Parser function for "PublishSnapshot" ZCL command from "Simple Metering" cluster
 */
EmberAfStatus zcl_decode_simple_metering_cluster_publish_snapshot_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_publish_snapshot_command_t *cmd_struct);
/** @brief Parser function for "GetSampledDataResponse" ZCL command from "Simple Metering" cluster
 */
EmberAfStatus zcl_decode_simple_metering_cluster_get_sampled_data_response_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_get_sampled_data_response_command_t *cmd_struct);
/** @brief Parser function for "ConfigureMirror" ZCL command from "Simple Metering" cluster
 */
EmberAfStatus zcl_decode_simple_metering_cluster_configure_mirror_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_configure_mirror_command_t *cmd_struct);
/** @brief Parser function for "ConfigureNotificationScheme" ZCL command from "Simple Metering" cluster
 */
EmberAfStatus zcl_decode_simple_metering_cluster_configure_notification_scheme_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_configure_notification_scheme_command_t *cmd_struct);
/** @brief Parser function for "ConfigureNotificationFlags" ZCL command from "Simple Metering" cluster
 */
EmberAfStatus zcl_decode_simple_metering_cluster_configure_notification_flags_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_configure_notification_flags_command_t *cmd_struct);
/** @brief Parser function for "GetNotifiedMessage" ZCL command from "Simple Metering" cluster
 */
EmberAfStatus zcl_decode_simple_metering_cluster_get_notified_message_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_get_notified_message_command_t *cmd_struct);
/** @brief Parser function for "SupplyStatusResponse" ZCL command from "Simple Metering" cluster
 */
EmberAfStatus zcl_decode_simple_metering_cluster_supply_status_response_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_supply_status_response_command_t *cmd_struct);
/** @brief Parser function for "StartSamplingResponse" ZCL command from "Simple Metering" cluster
 */
EmberAfStatus zcl_decode_simple_metering_cluster_start_sampling_response_command (EmberAfClusterCommand * cmd, sl_zcl_simple_metering_cluster_start_sampling_response_command_t *cmd_struct);
/** @brief Parser function for "MessageConfirmation" ZCL command from "Messaging" cluster
 */
EmberAfStatus zcl_decode_messaging_cluster_message_confirmation_command (EmberAfClusterCommand * cmd, sl_zcl_messaging_cluster_message_confirmation_command_t *cmd_struct);
/** @brief Parser function for "GetMessageCancellation" ZCL command from "Messaging" cluster
 */
EmberAfStatus zcl_decode_messaging_cluster_get_message_cancellation_command (EmberAfClusterCommand * cmd, sl_zcl_messaging_cluster_get_message_cancellation_command_t *cmd_struct);
/** @brief Parser function for "DisplayMessage" ZCL command from "Messaging" cluster
 */
EmberAfStatus zcl_decode_messaging_cluster_display_message_command (EmberAfClusterCommand * cmd, sl_zcl_messaging_cluster_display_message_command_t *cmd_struct);
/** @brief Parser function for "CancelMessage" ZCL command from "Messaging" cluster
 */
EmberAfStatus zcl_decode_messaging_cluster_cancel_message_command (EmberAfClusterCommand * cmd, sl_zcl_messaging_cluster_cancel_message_command_t *cmd_struct);
/** @brief Parser function for "DisplayProtectedMessage" ZCL command from "Messaging" cluster
 */
EmberAfStatus zcl_decode_messaging_cluster_display_protected_message_command (EmberAfClusterCommand * cmd, sl_zcl_messaging_cluster_display_protected_message_command_t *cmd_struct);
/** @brief Parser function for "CancelAllMessages" ZCL command from "Messaging" cluster
 */
EmberAfStatus zcl_decode_messaging_cluster_cancel_all_messages_command (EmberAfClusterCommand * cmd, sl_zcl_messaging_cluster_cancel_all_messages_command_t *cmd_struct);
/** @brief Parser function for "RequestTunnel" ZCL command from "Tunneling" cluster
 */
EmberAfStatus zcl_decode_tunneling_cluster_request_tunnel_command (EmberAfClusterCommand * cmd, sl_zcl_tunneling_cluster_request_tunnel_command_t *cmd_struct);
/** @brief Parser function for "CloseTunnel" ZCL command from "Tunneling" cluster
 */
EmberAfStatus zcl_decode_tunneling_cluster_close_tunnel_command (EmberAfClusterCommand * cmd, sl_zcl_tunneling_cluster_close_tunnel_command_t *cmd_struct);
/** @brief Parser function for "TransferDataClientToServer" ZCL command from "Tunneling" cluster
 */
EmberAfStatus zcl_decode_tunneling_cluster_transfer_data_client_to_server_command (EmberAfClusterCommand * cmd, sl_zcl_tunneling_cluster_transfer_data_client_to_server_command_t *cmd_struct);
/** @brief Parser function for "TransferDataErrorClientToServer" ZCL command from "Tunneling" cluster
 */
EmberAfStatus zcl_decode_tunneling_cluster_transfer_data_error_client_to_server_command (EmberAfClusterCommand * cmd, sl_zcl_tunneling_cluster_transfer_data_error_client_to_server_command_t *cmd_struct);
/** @brief Parser function for "AckTransferDataClientToServer" ZCL command from "Tunneling" cluster
 */
EmberAfStatus zcl_decode_tunneling_cluster_ack_transfer_data_client_to_server_command (EmberAfClusterCommand * cmd, sl_zcl_tunneling_cluster_ack_transfer_data_client_to_server_command_t *cmd_struct);
/** @brief Parser function for "ReadyDataClientToServer" ZCL command from "Tunneling" cluster
 */
EmberAfStatus zcl_decode_tunneling_cluster_ready_data_client_to_server_command (EmberAfClusterCommand * cmd, sl_zcl_tunneling_cluster_ready_data_client_to_server_command_t *cmd_struct);
/** @brief Parser function for "GetSupportedTunnelProtocols" ZCL command from "Tunneling" cluster
 */
EmberAfStatus zcl_decode_tunneling_cluster_get_supported_tunnel_protocols_command (EmberAfClusterCommand * cmd, sl_zcl_tunneling_cluster_get_supported_tunnel_protocols_command_t *cmd_struct);
/** @brief Parser function for "RequestTunnelResponse" ZCL command from "Tunneling" cluster
 */
EmberAfStatus zcl_decode_tunneling_cluster_request_tunnel_response_command (EmberAfClusterCommand * cmd, sl_zcl_tunneling_cluster_request_tunnel_response_command_t *cmd_struct);
/** @brief Parser function for "TransferDataServerToClient" ZCL command from "Tunneling" cluster
 */
EmberAfStatus zcl_decode_tunneling_cluster_transfer_data_server_to_client_command (EmberAfClusterCommand * cmd, sl_zcl_tunneling_cluster_transfer_data_server_to_client_command_t *cmd_struct);
/** @brief Parser function for "TransferDataErrorServerToClient" ZCL command from "Tunneling" cluster
 */
EmberAfStatus zcl_decode_tunneling_cluster_transfer_data_error_server_to_client_command (EmberAfClusterCommand * cmd, sl_zcl_tunneling_cluster_transfer_data_error_server_to_client_command_t *cmd_struct);
/** @brief Parser function for "AckTransferDataServerToClient" ZCL command from "Tunneling" cluster
 */
EmberAfStatus zcl_decode_tunneling_cluster_ack_transfer_data_server_to_client_command (EmberAfClusterCommand * cmd, sl_zcl_tunneling_cluster_ack_transfer_data_server_to_client_command_t *cmd_struct);
/** @brief Parser function for "ReadyDataServerToClient" ZCL command from "Tunneling" cluster
 */
EmberAfStatus zcl_decode_tunneling_cluster_ready_data_server_to_client_command (EmberAfClusterCommand * cmd, sl_zcl_tunneling_cluster_ready_data_server_to_client_command_t *cmd_struct);
/** @brief Parser function for "SupportedTunnelProtocolsResponse" ZCL command from "Tunneling" cluster
 */
EmberAfStatus zcl_decode_tunneling_cluster_supported_tunnel_protocols_response_command (EmberAfClusterCommand * cmd, sl_zcl_tunneling_cluster_supported_tunnel_protocols_response_command_t *cmd_struct);
/** @brief Parser function for "TunnelClosureNotification" ZCL command from "Tunneling" cluster
 */
EmberAfStatus zcl_decode_tunneling_cluster_tunnel_closure_notification_command (EmberAfClusterCommand * cmd, sl_zcl_tunneling_cluster_tunnel_closure_notification_command_t *cmd_struct);
/** @brief Parser function for "SelectAvailableEmergencyCredit" ZCL command from "Prepayment" cluster
 */
EmberAfStatus zcl_decode_prepayment_cluster_select_available_emergency_credit_command (EmberAfClusterCommand * cmd, sl_zcl_prepayment_cluster_select_available_emergency_credit_command_t *cmd_struct);
/** @brief Parser function for "ChangeDebt" ZCL command from "Prepayment" cluster
 */
EmberAfStatus zcl_decode_prepayment_cluster_change_debt_command (EmberAfClusterCommand * cmd, sl_zcl_prepayment_cluster_change_debt_command_t *cmd_struct);
/** @brief Parser function for "EmergencyCreditSetup" ZCL command from "Prepayment" cluster
 */
EmberAfStatus zcl_decode_prepayment_cluster_emergency_credit_setup_command (EmberAfClusterCommand * cmd, sl_zcl_prepayment_cluster_emergency_credit_setup_command_t *cmd_struct);
/** @brief Parser function for "ConsumerTopUp" ZCL command from "Prepayment" cluster
 */
EmberAfStatus zcl_decode_prepayment_cluster_consumer_top_up_command (EmberAfClusterCommand * cmd, sl_zcl_prepayment_cluster_consumer_top_up_command_t *cmd_struct);
/** @brief Parser function for "CreditAdjustment" ZCL command from "Prepayment" cluster
 */
EmberAfStatus zcl_decode_prepayment_cluster_credit_adjustment_command (EmberAfClusterCommand * cmd, sl_zcl_prepayment_cluster_credit_adjustment_command_t *cmd_struct);
/** @brief Parser function for "ChangePaymentMode" ZCL command from "Prepayment" cluster
 */
EmberAfStatus zcl_decode_prepayment_cluster_change_payment_mode_command (EmberAfClusterCommand * cmd, sl_zcl_prepayment_cluster_change_payment_mode_command_t *cmd_struct);
/** @brief Parser function for "GetPrepaySnapshot" ZCL command from "Prepayment" cluster
 */
EmberAfStatus zcl_decode_prepayment_cluster_get_prepay_snapshot_command (EmberAfClusterCommand * cmd, sl_zcl_prepayment_cluster_get_prepay_snapshot_command_t *cmd_struct);
/** @brief Parser function for "GetTopUpLog" ZCL command from "Prepayment" cluster
 */
EmberAfStatus zcl_decode_prepayment_cluster_get_top_up_log_command (EmberAfClusterCommand * cmd, sl_zcl_prepayment_cluster_get_top_up_log_command_t *cmd_struct);
/** @brief Parser function for "SetLowCreditWarningLevel" ZCL command from "Prepayment" cluster
 */
EmberAfStatus zcl_decode_prepayment_cluster_set_low_credit_warning_level_command (EmberAfClusterCommand * cmd, sl_zcl_prepayment_cluster_set_low_credit_warning_level_command_t *cmd_struct);
/** @brief Parser function for "GetDebtRepaymentLog" ZCL command from "Prepayment" cluster
 */
EmberAfStatus zcl_decode_prepayment_cluster_get_debt_repayment_log_command (EmberAfClusterCommand * cmd, sl_zcl_prepayment_cluster_get_debt_repayment_log_command_t *cmd_struct);
/** @brief Parser function for "SetMaximumCreditLimit" ZCL command from "Prepayment" cluster
 */
EmberAfStatus zcl_decode_prepayment_cluster_set_maximum_credit_limit_command (EmberAfClusterCommand * cmd, sl_zcl_prepayment_cluster_set_maximum_credit_limit_command_t *cmd_struct);
/** @brief Parser function for "SetOverallDebtCap" ZCL command from "Prepayment" cluster
 */
EmberAfStatus zcl_decode_prepayment_cluster_set_overall_debt_cap_command (EmberAfClusterCommand * cmd, sl_zcl_prepayment_cluster_set_overall_debt_cap_command_t *cmd_struct);
/** @brief Parser function for "PublishPrepaySnapshot" ZCL command from "Prepayment" cluster
 */
EmberAfStatus zcl_decode_prepayment_cluster_publish_prepay_snapshot_command (EmberAfClusterCommand * cmd, sl_zcl_prepayment_cluster_publish_prepay_snapshot_command_t *cmd_struct);
/** @brief Parser function for "ChangePaymentModeResponse" ZCL command from "Prepayment" cluster
 */
EmberAfStatus zcl_decode_prepayment_cluster_change_payment_mode_response_command (EmberAfClusterCommand * cmd, sl_zcl_prepayment_cluster_change_payment_mode_response_command_t *cmd_struct);
/** @brief Parser function for "ConsumerTopUpResponse" ZCL command from "Prepayment" cluster
 */
EmberAfStatus zcl_decode_prepayment_cluster_consumer_top_up_response_command (EmberAfClusterCommand * cmd, sl_zcl_prepayment_cluster_consumer_top_up_response_command_t *cmd_struct);
/** @brief Parser function for "PublishTopUpLog" ZCL command from "Prepayment" cluster
 */
EmberAfStatus zcl_decode_prepayment_cluster_publish_top_up_log_command (EmberAfClusterCommand * cmd, sl_zcl_prepayment_cluster_publish_top_up_log_command_t *cmd_struct);
/** @brief Parser function for "PublishDebtLog" ZCL command from "Prepayment" cluster
 */
EmberAfStatus zcl_decode_prepayment_cluster_publish_debt_log_command (EmberAfClusterCommand * cmd, sl_zcl_prepayment_cluster_publish_debt_log_command_t *cmd_struct);
/** @brief Parser function for "ManageEvent" ZCL command from "Energy Management" cluster
 */
EmberAfStatus zcl_decode_energy_management_cluster_manage_event_command (EmberAfClusterCommand * cmd, sl_zcl_energy_management_cluster_manage_event_command_t *cmd_struct);
/** @brief Parser function for "ReportEventStatus" ZCL command from "Energy Management" cluster
 */
EmberAfStatus zcl_decode_energy_management_cluster_report_event_status_command (EmberAfClusterCommand * cmd, sl_zcl_energy_management_cluster_report_event_status_command_t *cmd_struct);
/** @brief Parser function for "GetCalendar" ZCL command from "Calendar" cluster
 */
EmberAfStatus zcl_decode_calendar_cluster_get_calendar_command (EmberAfClusterCommand * cmd, sl_zcl_calendar_cluster_get_calendar_command_t *cmd_struct);
/** @brief Parser function for "GetDayProfiles" ZCL command from "Calendar" cluster
 */
EmberAfStatus zcl_decode_calendar_cluster_get_day_profiles_command (EmberAfClusterCommand * cmd, sl_zcl_calendar_cluster_get_day_profiles_command_t *cmd_struct);
/** @brief Parser function for "GetWeekProfiles" ZCL command from "Calendar" cluster
 */
EmberAfStatus zcl_decode_calendar_cluster_get_week_profiles_command (EmberAfClusterCommand * cmd, sl_zcl_calendar_cluster_get_week_profiles_command_t *cmd_struct);
/** @brief Parser function for "GetSeasons" ZCL command from "Calendar" cluster
 */
EmberAfStatus zcl_decode_calendar_cluster_get_seasons_command (EmberAfClusterCommand * cmd, sl_zcl_calendar_cluster_get_seasons_command_t *cmd_struct);
/** @brief Parser function for "GetSpecialDays" ZCL command from "Calendar" cluster
 */
EmberAfStatus zcl_decode_calendar_cluster_get_special_days_command (EmberAfClusterCommand * cmd, sl_zcl_calendar_cluster_get_special_days_command_t *cmd_struct);
/** @brief Parser function for "PublishCalendar" ZCL command from "Calendar" cluster
 */
EmberAfStatus zcl_decode_calendar_cluster_publish_calendar_command (EmberAfClusterCommand * cmd, sl_zcl_calendar_cluster_publish_calendar_command_t *cmd_struct);
/** @brief Parser function for "PublishDayProfile" ZCL command from "Calendar" cluster
 */
EmberAfStatus zcl_decode_calendar_cluster_publish_day_profile_command (EmberAfClusterCommand * cmd, sl_zcl_calendar_cluster_publish_day_profile_command_t *cmd_struct);
/** @brief Parser function for "PublishWeekProfile" ZCL command from "Calendar" cluster
 */
EmberAfStatus zcl_decode_calendar_cluster_publish_week_profile_command (EmberAfClusterCommand * cmd, sl_zcl_calendar_cluster_publish_week_profile_command_t *cmd_struct);
/** @brief Parser function for "PublishSeasons" ZCL command from "Calendar" cluster
 */
EmberAfStatus zcl_decode_calendar_cluster_publish_seasons_command (EmberAfClusterCommand * cmd, sl_zcl_calendar_cluster_publish_seasons_command_t *cmd_struct);
/** @brief Parser function for "PublishSpecialDays" ZCL command from "Calendar" cluster
 */
EmberAfStatus zcl_decode_calendar_cluster_publish_special_days_command (EmberAfClusterCommand * cmd, sl_zcl_calendar_cluster_publish_special_days_command_t *cmd_struct);
/** @brief Parser function for "CancelCalendar" ZCL command from "Calendar" cluster
 */
EmberAfStatus zcl_decode_calendar_cluster_cancel_calendar_command (EmberAfClusterCommand * cmd, sl_zcl_calendar_cluster_cancel_calendar_command_t *cmd_struct);
/** @brief Parser function for "RequestNewPassword" ZCL command from "Device Management" cluster
 */
EmberAfStatus zcl_decode_device_management_cluster_request_new_password_command (EmberAfClusterCommand * cmd, sl_zcl_device_management_cluster_request_new_password_command_t *cmd_struct);
/** @brief Parser function for "ReportEventConfiguration" ZCL command from "Device Management" cluster
 */
EmberAfStatus zcl_decode_device_management_cluster_report_event_configuration_command (EmberAfClusterCommand * cmd, sl_zcl_device_management_cluster_report_event_configuration_command_t *cmd_struct);
/** @brief Parser function for "PublishChangeOfTenancy" ZCL command from "Device Management" cluster
 */
EmberAfStatus zcl_decode_device_management_cluster_publish_change_of_tenancy_command (EmberAfClusterCommand * cmd, sl_zcl_device_management_cluster_publish_change_of_tenancy_command_t *cmd_struct);
/** @brief Parser function for "PublishChangeOfSupplier" ZCL command from "Device Management" cluster
 */
EmberAfStatus zcl_decode_device_management_cluster_publish_change_of_supplier_command (EmberAfClusterCommand * cmd, sl_zcl_device_management_cluster_publish_change_of_supplier_command_t *cmd_struct);
/** @brief Parser function for "RequestNewPasswordResponse" ZCL command from "Device Management" cluster
 */
EmberAfStatus zcl_decode_device_management_cluster_request_new_password_response_command (EmberAfClusterCommand * cmd, sl_zcl_device_management_cluster_request_new_password_response_command_t *cmd_struct);
/** @brief Parser function for "UpdateSiteId" ZCL command from "Device Management" cluster
 */
EmberAfStatus zcl_decode_device_management_cluster_update_site_id_command (EmberAfClusterCommand * cmd, sl_zcl_device_management_cluster_update_site_id_command_t *cmd_struct);
/** @brief Parser function for "SetEventConfiguration" ZCL command from "Device Management" cluster
 */
EmberAfStatus zcl_decode_device_management_cluster_set_event_configuration_command (EmberAfClusterCommand * cmd, sl_zcl_device_management_cluster_set_event_configuration_command_t *cmd_struct);
/** @brief Parser function for "GetEventConfiguration" ZCL command from "Device Management" cluster
 */
EmberAfStatus zcl_decode_device_management_cluster_get_event_configuration_command (EmberAfClusterCommand * cmd, sl_zcl_device_management_cluster_get_event_configuration_command_t *cmd_struct);
/** @brief Parser function for "UpdateCIN" ZCL command from "Device Management" cluster
 */
EmberAfStatus zcl_decode_device_management_cluster_update_cin_command (EmberAfClusterCommand * cmd, sl_zcl_device_management_cluster_update_cin_command_t *cmd_struct);
/** @brief Parser function for "GetEventLog" ZCL command from "Events" cluster
 */
EmberAfStatus zcl_decode_events_cluster_get_event_log_command (EmberAfClusterCommand * cmd, sl_zcl_events_cluster_get_event_log_command_t *cmd_struct);
/** @brief Parser function for "ClearEventLogRequest" ZCL command from "Events" cluster
 */
EmberAfStatus zcl_decode_events_cluster_clear_event_log_request_command (EmberAfClusterCommand * cmd, sl_zcl_events_cluster_clear_event_log_request_command_t *cmd_struct);
/** @brief Parser function for "PublishEvent" ZCL command from "Events" cluster
 */
EmberAfStatus zcl_decode_events_cluster_publish_event_command (EmberAfClusterCommand * cmd, sl_zcl_events_cluster_publish_event_command_t *cmd_struct);
/** @brief Parser function for "PublishEventLog" ZCL command from "Events" cluster
 */
EmberAfStatus zcl_decode_events_cluster_publish_event_log_command (EmberAfClusterCommand * cmd, sl_zcl_events_cluster_publish_event_log_command_t *cmd_struct);
/** @brief Parser function for "ClearEventLogResponse" ZCL command from "Events" cluster
 */
EmberAfStatus zcl_decode_events_cluster_clear_event_log_response_command (EmberAfClusterCommand * cmd, sl_zcl_events_cluster_clear_event_log_response_command_t *cmd_struct);
/** @brief Parser function for "PairingRequest" ZCL command from "MDU Pairing" cluster
 */
EmberAfStatus zcl_decode_mdu_pairing_cluster_pairing_request_command (EmberAfClusterCommand * cmd, sl_zcl_mdu_pairing_cluster_pairing_request_command_t *cmd_struct);
/** @brief Parser function for "PairingResponse" ZCL command from "MDU Pairing" cluster
 */
EmberAfStatus zcl_decode_mdu_pairing_cluster_pairing_response_command (EmberAfClusterCommand * cmd, sl_zcl_mdu_pairing_cluster_pairing_response_command_t *cmd_struct);
/** @brief Parser function for "SuspendZclMessages" ZCL command from "Sub-GHz" cluster
 */
EmberAfStatus zcl_decode_sub_g_hz_cluster_suspend_zcl_messages_command (EmberAfClusterCommand * cmd, sl_zcl_sub_g_hz_cluster_suspend_zcl_messages_command_t *cmd_struct);
/** @brief Parser function for "InitiateKeyEstablishmentRequest" ZCL command from "Key Establishment" cluster
 */
EmberAfStatus zcl_decode_key_establishment_cluster_initiate_key_establishment_request_command (EmberAfClusterCommand * cmd, sl_zcl_key_establishment_cluster_initiate_key_establishment_request_command_t *cmd_struct);
/** @brief Parser function for "EphemeralDataRequest" ZCL command from "Key Establishment" cluster
 */
EmberAfStatus zcl_decode_key_establishment_cluster_ephemeral_data_request_command (EmberAfClusterCommand * cmd, sl_zcl_key_establishment_cluster_ephemeral_data_request_command_t *cmd_struct);
/** @brief Parser function for "ConfirmKeyDataRequest" ZCL command from "Key Establishment" cluster
 */
EmberAfStatus zcl_decode_key_establishment_cluster_confirm_key_data_request_command (EmberAfClusterCommand * cmd, sl_zcl_key_establishment_cluster_confirm_key_data_request_command_t *cmd_struct);
/** @brief Parser function for "TerminateKeyEstablishmentFromClient" ZCL command from "Key Establishment" cluster
 */
EmberAfStatus zcl_decode_key_establishment_cluster_terminate_key_establishment_from_client_command (EmberAfClusterCommand * cmd, sl_zcl_key_establishment_cluster_terminate_key_establishment_from_client_command_t *cmd_struct);
/** @brief Parser function for "InitiateKeyEstablishmentResponse" ZCL command from "Key Establishment" cluster
 */
EmberAfStatus zcl_decode_key_establishment_cluster_initiate_key_establishment_response_command (EmberAfClusterCommand * cmd, sl_zcl_key_establishment_cluster_initiate_key_establishment_response_command_t *cmd_struct);
/** @brief Parser function for "EphemeralDataResponse" ZCL command from "Key Establishment" cluster
 */
EmberAfStatus zcl_decode_key_establishment_cluster_ephemeral_data_response_command (EmberAfClusterCommand * cmd, sl_zcl_key_establishment_cluster_ephemeral_data_response_command_t *cmd_struct);
/** @brief Parser function for "ConfirmKeyDataResponse" ZCL command from "Key Establishment" cluster
 */
EmberAfStatus zcl_decode_key_establishment_cluster_confirm_key_data_response_command (EmberAfClusterCommand * cmd, sl_zcl_key_establishment_cluster_confirm_key_data_response_command_t *cmd_struct);
/** @brief Parser function for "TerminateKeyEstablishmentFromServer" ZCL command from "Key Establishment" cluster
 */
EmberAfStatus zcl_decode_key_establishment_cluster_terminate_key_establishment_from_server_command (EmberAfClusterCommand * cmd, sl_zcl_key_establishment_cluster_terminate_key_establishment_from_server_command_t *cmd_struct);
/** @brief Parser function for "RequestInformation" ZCL command from "Information" cluster
 */
EmberAfStatus zcl_decode_information_cluster_request_information_command (EmberAfClusterCommand * cmd, sl_zcl_information_cluster_request_information_command_t *cmd_struct);
/** @brief Parser function for "PushInformationResponse" ZCL command from "Information" cluster
 */
EmberAfStatus zcl_decode_information_cluster_push_information_response_command (EmberAfClusterCommand * cmd, sl_zcl_information_cluster_push_information_response_command_t *cmd_struct);
/** @brief Parser function for "SendPreference" ZCL command from "Information" cluster
 */
EmberAfStatus zcl_decode_information_cluster_send_preference_command (EmberAfClusterCommand * cmd, sl_zcl_information_cluster_send_preference_command_t *cmd_struct);
/** @brief Parser function for "RequestPreferenceResponse" ZCL command from "Information" cluster
 */
EmberAfStatus zcl_decode_information_cluster_request_preference_response_command (EmberAfClusterCommand * cmd, sl_zcl_information_cluster_request_preference_response_command_t *cmd_struct);
/** @brief Parser function for "Update" ZCL command from "Information" cluster
 */
EmberAfStatus zcl_decode_information_cluster_update_command (EmberAfClusterCommand * cmd, sl_zcl_information_cluster_update_command_t *cmd_struct);
/** @brief Parser function for "Delete" ZCL command from "Information" cluster
 */
EmberAfStatus zcl_decode_information_cluster_delete_command (EmberAfClusterCommand * cmd, sl_zcl_information_cluster_delete_command_t *cmd_struct);
/** @brief Parser function for "ConfigureNodeDescription" ZCL command from "Information" cluster
 */
EmberAfStatus zcl_decode_information_cluster_configure_node_description_command (EmberAfClusterCommand * cmd, sl_zcl_information_cluster_configure_node_description_command_t *cmd_struct);
/** @brief Parser function for "ConfigureDeliveryEnable" ZCL command from "Information" cluster
 */
EmberAfStatus zcl_decode_information_cluster_configure_delivery_enable_command (EmberAfClusterCommand * cmd, sl_zcl_information_cluster_configure_delivery_enable_command_t *cmd_struct);
/** @brief Parser function for "ConfigurePushInformationTimer" ZCL command from "Information" cluster
 */
EmberAfStatus zcl_decode_information_cluster_configure_push_information_timer_command (EmberAfClusterCommand * cmd, sl_zcl_information_cluster_configure_push_information_timer_command_t *cmd_struct);
/** @brief Parser function for "ConfigureSetRootId" ZCL command from "Information" cluster
 */
EmberAfStatus zcl_decode_information_cluster_configure_set_root_id_command (EmberAfClusterCommand * cmd, sl_zcl_information_cluster_configure_set_root_id_command_t *cmd_struct);
/** @brief Parser function for "RequestInformationResponse" ZCL command from "Information" cluster
 */
EmberAfStatus zcl_decode_information_cluster_request_information_response_command (EmberAfClusterCommand * cmd, sl_zcl_information_cluster_request_information_response_command_t *cmd_struct);
/** @brief Parser function for "PushInformation" ZCL command from "Information" cluster
 */
EmberAfStatus zcl_decode_information_cluster_push_information_command (EmberAfClusterCommand * cmd, sl_zcl_information_cluster_push_information_command_t *cmd_struct);
/** @brief Parser function for "SendPreferenceResponse" ZCL command from "Information" cluster
 */
EmberAfStatus zcl_decode_information_cluster_send_preference_response_command (EmberAfClusterCommand * cmd, sl_zcl_information_cluster_send_preference_response_command_t *cmd_struct);
/** @brief Parser function for "RequestPreferenceConfirmation" ZCL command from "Information" cluster
 */
EmberAfStatus zcl_decode_information_cluster_request_preference_confirmation_command (EmberAfClusterCommand * cmd, sl_zcl_information_cluster_request_preference_confirmation_command_t *cmd_struct);
/** @brief Parser function for "UpdateResponse" ZCL command from "Information" cluster
 */
EmberAfStatus zcl_decode_information_cluster_update_response_command (EmberAfClusterCommand * cmd, sl_zcl_information_cluster_update_response_command_t *cmd_struct);
/** @brief Parser function for "DeleteResponse" ZCL command from "Information" cluster
 */
EmberAfStatus zcl_decode_information_cluster_delete_response_command (EmberAfClusterCommand * cmd, sl_zcl_information_cluster_delete_response_command_t *cmd_struct);
/** @brief Parser function for "ReadFileRequest" ZCL command from "Data Sharing" cluster
 */
EmberAfStatus zcl_decode_data_sharing_cluster_read_file_request_command (EmberAfClusterCommand * cmd, sl_zcl_data_sharing_cluster_read_file_request_command_t *cmd_struct);
/** @brief Parser function for "ReadRecordRequest" ZCL command from "Data Sharing" cluster
 */
EmberAfStatus zcl_decode_data_sharing_cluster_read_record_request_command (EmberAfClusterCommand * cmd, sl_zcl_data_sharing_cluster_read_record_request_command_t *cmd_struct);
/** @brief Parser function for "WriteFileResponse" ZCL command from "Data Sharing" cluster
 */
EmberAfStatus zcl_decode_data_sharing_cluster_write_file_response_command (EmberAfClusterCommand * cmd, sl_zcl_data_sharing_cluster_write_file_response_command_t *cmd_struct);
/** @brief Parser function for "WriteFileRequest" ZCL command from "Data Sharing" cluster
 */
EmberAfStatus zcl_decode_data_sharing_cluster_write_file_request_command (EmberAfClusterCommand * cmd, sl_zcl_data_sharing_cluster_write_file_request_command_t *cmd_struct);
/** @brief Parser function for "ModifyFileRequest" ZCL command from "Data Sharing" cluster
 */
EmberAfStatus zcl_decode_data_sharing_cluster_modify_file_request_command (EmberAfClusterCommand * cmd, sl_zcl_data_sharing_cluster_modify_file_request_command_t *cmd_struct);
/** @brief Parser function for "ModifyRecordRequest" ZCL command from "Data Sharing" cluster
 */
EmberAfStatus zcl_decode_data_sharing_cluster_modify_record_request_command (EmberAfClusterCommand * cmd, sl_zcl_data_sharing_cluster_modify_record_request_command_t *cmd_struct);
/** @brief Parser function for "FileTransmission" ZCL command from "Data Sharing" cluster
 */
EmberAfStatus zcl_decode_data_sharing_cluster_file_transmission_command (EmberAfClusterCommand * cmd, sl_zcl_data_sharing_cluster_file_transmission_command_t *cmd_struct);
/** @brief Parser function for "RecordTransmission" ZCL command from "Data Sharing" cluster
 */
EmberAfStatus zcl_decode_data_sharing_cluster_record_transmission_command (EmberAfClusterCommand * cmd, sl_zcl_data_sharing_cluster_record_transmission_command_t *cmd_struct);
/** @brief Parser function for "SearchGame" ZCL command from "Gaming" cluster
 */
EmberAfStatus zcl_decode_gaming_cluster_search_game_command (EmberAfClusterCommand * cmd, sl_zcl_gaming_cluster_search_game_command_t *cmd_struct);
/** @brief Parser function for "JoinGame" ZCL command from "Gaming" cluster
 */
EmberAfStatus zcl_decode_gaming_cluster_join_game_command (EmberAfClusterCommand * cmd, sl_zcl_gaming_cluster_join_game_command_t *cmd_struct);
/** @brief Parser function for "ActionControl" ZCL command from "Gaming" cluster
 */
EmberAfStatus zcl_decode_gaming_cluster_action_control_command (EmberAfClusterCommand * cmd, sl_zcl_gaming_cluster_action_control_command_t *cmd_struct);
/** @brief Parser function for "GameAnnouncement" ZCL command from "Gaming" cluster
 */
EmberAfStatus zcl_decode_gaming_cluster_game_announcement_command (EmberAfClusterCommand * cmd, sl_zcl_gaming_cluster_game_announcement_command_t *cmd_struct);
/** @brief Parser function for "GeneralResponse" ZCL command from "Gaming" cluster
 */
EmberAfStatus zcl_decode_gaming_cluster_general_response_command (EmberAfClusterCommand * cmd, sl_zcl_gaming_cluster_general_response_command_t *cmd_struct);
/** @brief Parser function for "PathCreation" ZCL command from "Data Rate Control" cluster
 */
EmberAfStatus zcl_decode_data_rate_control_cluster_path_creation_command (EmberAfClusterCommand * cmd, sl_zcl_data_rate_control_cluster_path_creation_command_t *cmd_struct);
/** @brief Parser function for "DataRateNotification" ZCL command from "Data Rate Control" cluster
 */
EmberAfStatus zcl_decode_data_rate_control_cluster_data_rate_notification_command (EmberAfClusterCommand * cmd, sl_zcl_data_rate_control_cluster_data_rate_notification_command_t *cmd_struct);
/** @brief Parser function for "PathDeletion" ZCL command from "Data Rate Control" cluster
 */
EmberAfStatus zcl_decode_data_rate_control_cluster_path_deletion_command (EmberAfClusterCommand * cmd, sl_zcl_data_rate_control_cluster_path_deletion_command_t *cmd_struct);
/** @brief Parser function for "DataRateControl" ZCL command from "Data Rate Control" cluster
 */
EmberAfStatus zcl_decode_data_rate_control_cluster_data_rate_control_command (EmberAfClusterCommand * cmd, sl_zcl_data_rate_control_cluster_data_rate_control_command_t *cmd_struct);
/** @brief Parser function for "EstablishmentRequest" ZCL command from "Voice over ZigBee" cluster
 */
EmberAfStatus zcl_decode_voice_over_zig_bee_cluster_establishment_request_command (EmberAfClusterCommand * cmd, sl_zcl_voice_over_zig_bee_cluster_establishment_request_command_t *cmd_struct);
/** @brief Parser function for "VoiceTransmission" ZCL command from "Voice over ZigBee" cluster
 */
EmberAfStatus zcl_decode_voice_over_zig_bee_cluster_voice_transmission_command (EmberAfClusterCommand * cmd, sl_zcl_voice_over_zig_bee_cluster_voice_transmission_command_t *cmd_struct);
/** @brief Parser function for "ControlResponse" ZCL command from "Voice over ZigBee" cluster
 */
EmberAfStatus zcl_decode_voice_over_zig_bee_cluster_control_response_command (EmberAfClusterCommand * cmd, sl_zcl_voice_over_zig_bee_cluster_control_response_command_t *cmd_struct);
/** @brief Parser function for "EstablishmentResponse" ZCL command from "Voice over ZigBee" cluster
 */
EmberAfStatus zcl_decode_voice_over_zig_bee_cluster_establishment_response_command (EmberAfClusterCommand * cmd, sl_zcl_voice_over_zig_bee_cluster_establishment_response_command_t *cmd_struct);
/** @brief Parser function for "VoiceTransmissionResponse" ZCL command from "Voice over ZigBee" cluster
 */
EmberAfStatus zcl_decode_voice_over_zig_bee_cluster_voice_transmission_response_command (EmberAfClusterCommand * cmd, sl_zcl_voice_over_zig_bee_cluster_voice_transmission_response_command_t *cmd_struct);
/** @brief Parser function for "Control" ZCL command from "Voice over ZigBee" cluster
 */
EmberAfStatus zcl_decode_voice_over_zig_bee_cluster_control_command (EmberAfClusterCommand * cmd, sl_zcl_voice_over_zig_bee_cluster_control_command_t *cmd_struct);
/** @brief Parser function for "JoinChatRequest" ZCL command from "Chatting" cluster
 */
EmberAfStatus zcl_decode_chatting_cluster_join_chat_request_command (EmberAfClusterCommand * cmd, sl_zcl_chatting_cluster_join_chat_request_command_t *cmd_struct);
/** @brief Parser function for "LeaveChatRequest" ZCL command from "Chatting" cluster
 */
EmberAfStatus zcl_decode_chatting_cluster_leave_chat_request_command (EmberAfClusterCommand * cmd, sl_zcl_chatting_cluster_leave_chat_request_command_t *cmd_struct);
/** @brief Parser function for "SwitchChairmanResponse" ZCL command from "Chatting" cluster
 */
EmberAfStatus zcl_decode_chatting_cluster_switch_chairman_response_command (EmberAfClusterCommand * cmd, sl_zcl_chatting_cluster_switch_chairman_response_command_t *cmd_struct);
/** @brief Parser function for "StartChatRequest" ZCL command from "Chatting" cluster
 */
EmberAfStatus zcl_decode_chatting_cluster_start_chat_request_command (EmberAfClusterCommand * cmd, sl_zcl_chatting_cluster_start_chat_request_command_t *cmd_struct);
/** @brief Parser function for "ChatMessage" ZCL command from "Chatting" cluster
 */
EmberAfStatus zcl_decode_chatting_cluster_chat_message_command (EmberAfClusterCommand * cmd, sl_zcl_chatting_cluster_chat_message_command_t *cmd_struct);
/** @brief Parser function for "GetNodeInformationRequest" ZCL command from "Chatting" cluster
 */
EmberAfStatus zcl_decode_chatting_cluster_get_node_information_request_command (EmberAfClusterCommand * cmd, sl_zcl_chatting_cluster_get_node_information_request_command_t *cmd_struct);
/** @brief Parser function for "StartChatResponse" ZCL command from "Chatting" cluster
 */
EmberAfStatus zcl_decode_chatting_cluster_start_chat_response_command (EmberAfClusterCommand * cmd, sl_zcl_chatting_cluster_start_chat_response_command_t *cmd_struct);
/** @brief Parser function for "JoinChatResponse" ZCL command from "Chatting" cluster
 */
EmberAfStatus zcl_decode_chatting_cluster_join_chat_response_command (EmberAfClusterCommand * cmd, sl_zcl_chatting_cluster_join_chat_response_command_t *cmd_struct);
/** @brief Parser function for "UserLeft" ZCL command from "Chatting" cluster
 */
EmberAfStatus zcl_decode_chatting_cluster_user_left_command (EmberAfClusterCommand * cmd, sl_zcl_chatting_cluster_user_left_command_t *cmd_struct);
/** @brief Parser function for "UserJoined" ZCL command from "Chatting" cluster
 */
EmberAfStatus zcl_decode_chatting_cluster_user_joined_command (EmberAfClusterCommand * cmd, sl_zcl_chatting_cluster_user_joined_command_t *cmd_struct);
/** @brief Parser function for "SearchChatResponse" ZCL command from "Chatting" cluster
 */
EmberAfStatus zcl_decode_chatting_cluster_search_chat_response_command (EmberAfClusterCommand * cmd, sl_zcl_chatting_cluster_search_chat_response_command_t *cmd_struct);
/** @brief Parser function for "SwitchChairmanRequest" ZCL command from "Chatting" cluster
 */
EmberAfStatus zcl_decode_chatting_cluster_switch_chairman_request_command (EmberAfClusterCommand * cmd, sl_zcl_chatting_cluster_switch_chairman_request_command_t *cmd_struct);
/** @brief Parser function for "SwitchChairmanConfirm" ZCL command from "Chatting" cluster
 */
EmberAfStatus zcl_decode_chatting_cluster_switch_chairman_confirm_command (EmberAfClusterCommand * cmd, sl_zcl_chatting_cluster_switch_chairman_confirm_command_t *cmd_struct);
/** @brief Parser function for "SwitchChairmanNotification" ZCL command from "Chatting" cluster
 */
EmberAfStatus zcl_decode_chatting_cluster_switch_chairman_notification_command (EmberAfClusterCommand * cmd, sl_zcl_chatting_cluster_switch_chairman_notification_command_t *cmd_struct);
/** @brief Parser function for "GetNodeInformationResponse" ZCL command from "Chatting" cluster
 */
EmberAfStatus zcl_decode_chatting_cluster_get_node_information_response_command (EmberAfClusterCommand * cmd, sl_zcl_chatting_cluster_get_node_information_response_command_t *cmd_struct);
/** @brief Parser function for "BuyRequest" ZCL command from "Payment" cluster
 */
EmberAfStatus zcl_decode_payment_cluster_buy_request_command (EmberAfClusterCommand * cmd, sl_zcl_payment_cluster_buy_request_command_t *cmd_struct);
/** @brief Parser function for "AcceptPayment" ZCL command from "Payment" cluster
 */
EmberAfStatus zcl_decode_payment_cluster_accept_payment_command (EmberAfClusterCommand * cmd, sl_zcl_payment_cluster_accept_payment_command_t *cmd_struct);
/** @brief Parser function for "PaymentConfirm" ZCL command from "Payment" cluster
 */
EmberAfStatus zcl_decode_payment_cluster_payment_confirm_command (EmberAfClusterCommand * cmd, sl_zcl_payment_cluster_payment_confirm_command_t *cmd_struct);
/** @brief Parser function for "BuyConfirm" ZCL command from "Payment" cluster
 */
EmberAfStatus zcl_decode_payment_cluster_buy_confirm_command (EmberAfClusterCommand * cmd, sl_zcl_payment_cluster_buy_confirm_command_t *cmd_struct);
/** @brief Parser function for "ReceiptDelivery" ZCL command from "Payment" cluster
 */
EmberAfStatus zcl_decode_payment_cluster_receipt_delivery_command (EmberAfClusterCommand * cmd, sl_zcl_payment_cluster_receipt_delivery_command_t *cmd_struct);
/** @brief Parser function for "TransactionEnd" ZCL command from "Payment" cluster
 */
EmberAfStatus zcl_decode_payment_cluster_transaction_end_command (EmberAfClusterCommand * cmd, sl_zcl_payment_cluster_transaction_end_command_t *cmd_struct);
/** @brief Parser function for "Subscribe" ZCL command from "Billing" cluster
 */
EmberAfStatus zcl_decode_billing_cluster_subscribe_command (EmberAfClusterCommand * cmd, sl_zcl_billing_cluster_subscribe_command_t *cmd_struct);
/** @brief Parser function for "Unsubscribe" ZCL command from "Billing" cluster
 */
EmberAfStatus zcl_decode_billing_cluster_unsubscribe_command (EmberAfClusterCommand * cmd, sl_zcl_billing_cluster_unsubscribe_command_t *cmd_struct);
/** @brief Parser function for "StartBillingSession" ZCL command from "Billing" cluster
 */
EmberAfStatus zcl_decode_billing_cluster_start_billing_session_command (EmberAfClusterCommand * cmd, sl_zcl_billing_cluster_start_billing_session_command_t *cmd_struct);
/** @brief Parser function for "StopBillingSession" ZCL command from "Billing" cluster
 */
EmberAfStatus zcl_decode_billing_cluster_stop_billing_session_command (EmberAfClusterCommand * cmd, sl_zcl_billing_cluster_stop_billing_session_command_t *cmd_struct);
/** @brief Parser function for "BillStatusNotification" ZCL command from "Billing" cluster
 */
EmberAfStatus zcl_decode_billing_cluster_bill_status_notification_command (EmberAfClusterCommand * cmd, sl_zcl_billing_cluster_bill_status_notification_command_t *cmd_struct);
/** @brief Parser function for "SessionKeepAlive" ZCL command from "Billing" cluster
 */
EmberAfStatus zcl_decode_billing_cluster_session_keep_alive_command (EmberAfClusterCommand * cmd, sl_zcl_billing_cluster_session_keep_alive_command_t *cmd_struct);
/** @brief Parser function for "CheckBillStatus" ZCL command from "Billing" cluster
 */
EmberAfStatus zcl_decode_billing_cluster_check_bill_status_command (EmberAfClusterCommand * cmd, sl_zcl_billing_cluster_check_bill_status_command_t *cmd_struct);
/** @brief Parser function for "SendBillRecord" ZCL command from "Billing" cluster
 */
EmberAfStatus zcl_decode_billing_cluster_send_bill_record_command (EmberAfClusterCommand * cmd, sl_zcl_billing_cluster_send_bill_record_command_t *cmd_struct);
/** @brief Parser function for "GetAlertsResponse" ZCL command from "Appliance Events and Alert" cluster
 */
EmberAfStatus zcl_decode_appliance_events_and_alert_cluster_get_alerts_response_command (EmberAfClusterCommand * cmd, sl_zcl_appliance_events_and_alert_cluster_get_alerts_response_command_t *cmd_struct);
/** @brief Parser function for "AlertsNotification" ZCL command from "Appliance Events and Alert" cluster
 */
EmberAfStatus zcl_decode_appliance_events_and_alert_cluster_alerts_notification_command (EmberAfClusterCommand * cmd, sl_zcl_appliance_events_and_alert_cluster_alerts_notification_command_t *cmd_struct);
/** @brief Parser function for "EventsNotification" ZCL command from "Appliance Events and Alert" cluster
 */
EmberAfStatus zcl_decode_appliance_events_and_alert_cluster_events_notification_command (EmberAfClusterCommand * cmd, sl_zcl_appliance_events_and_alert_cluster_events_notification_command_t *cmd_struct);
/** @brief Parser function for "LogRequest" ZCL command from "Appliance Statistics" cluster
 */
EmberAfStatus zcl_decode_appliance_statistics_cluster_log_request_command (EmberAfClusterCommand * cmd, sl_zcl_appliance_statistics_cluster_log_request_command_t *cmd_struct);
/** @brief Parser function for "LogNotification" ZCL command from "Appliance Statistics" cluster
 */
EmberAfStatus zcl_decode_appliance_statistics_cluster_log_notification_command (EmberAfClusterCommand * cmd, sl_zcl_appliance_statistics_cluster_log_notification_command_t *cmd_struct);
/** @brief Parser function for "LogResponse" ZCL command from "Appliance Statistics" cluster
 */
EmberAfStatus zcl_decode_appliance_statistics_cluster_log_response_command (EmberAfClusterCommand * cmd, sl_zcl_appliance_statistics_cluster_log_response_command_t *cmd_struct);
/** @brief Parser function for "LogQueueResponse" ZCL command from "Appliance Statistics" cluster
 */
EmberAfStatus zcl_decode_appliance_statistics_cluster_log_queue_response_command (EmberAfClusterCommand * cmd, sl_zcl_appliance_statistics_cluster_log_queue_response_command_t *cmd_struct);
/** @brief Parser function for "StatisticsAvailable" ZCL command from "Appliance Statistics" cluster
 */
EmberAfStatus zcl_decode_appliance_statistics_cluster_statistics_available_command (EmberAfClusterCommand * cmd, sl_zcl_appliance_statistics_cluster_statistics_available_command_t *cmd_struct);
/** @brief Parser function for "GetMeasurementProfileCommand" ZCL command from "Electrical Measurement" cluster
 */
EmberAfStatus zcl_decode_electrical_measurement_cluster_get_measurement_profile_command_command (EmberAfClusterCommand * cmd, sl_zcl_electrical_measurement_cluster_get_measurement_profile_command_command_t *cmd_struct);
/** @brief Parser function for "GetProfileInfoResponseCommand" ZCL command from "Electrical Measurement" cluster
 */
EmberAfStatus zcl_decode_electrical_measurement_cluster_get_profile_info_response_command_command (EmberAfClusterCommand * cmd, sl_zcl_electrical_measurement_cluster_get_profile_info_response_command_command_t *cmd_struct);
/** @brief Parser function for "GetMeasurementProfileResponseCommand" ZCL command from "Electrical Measurement" cluster
 */
EmberAfStatus zcl_decode_electrical_measurement_cluster_get_measurement_profile_response_command_command (EmberAfClusterCommand * cmd, sl_zcl_electrical_measurement_cluster_get_measurement_profile_response_command_command_t *cmd_struct);
/** @brief Parser function for "ScanRequest" ZCL command from "ZLL Commissioning" cluster
 */
EmberAfStatus zcl_decode_zll_commissioning_cluster_scan_request_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_scan_request_command_t *cmd_struct);
/** @brief Parser function for "DeviceInformationRequest" ZCL command from "ZLL Commissioning" cluster
 */
EmberAfStatus zcl_decode_zll_commissioning_cluster_device_information_request_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_device_information_request_command_t *cmd_struct);
/** @brief Parser function for "IdentifyRequest" ZCL command from "ZLL Commissioning" cluster
 */
EmberAfStatus zcl_decode_zll_commissioning_cluster_identify_request_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_identify_request_command_t *cmd_struct);
/** @brief Parser function for "ResetToFactoryNewRequest" ZCL command from "ZLL Commissioning" cluster
 */
EmberAfStatus zcl_decode_zll_commissioning_cluster_reset_to_factory_new_request_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_reset_to_factory_new_request_command_t *cmd_struct);
/** @brief Parser function for "NetworkStartRequest" ZCL command from "ZLL Commissioning" cluster
 */
EmberAfStatus zcl_decode_zll_commissioning_cluster_network_start_request_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_network_start_request_command_t *cmd_struct);
/** @brief Parser function for "NetworkJoinRouterRequest" ZCL command from "ZLL Commissioning" cluster
 */
EmberAfStatus zcl_decode_zll_commissioning_cluster_network_join_router_request_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_network_join_router_request_command_t *cmd_struct);
/** @brief Parser function for "NetworkJoinEndDeviceRequest" ZCL command from "ZLL Commissioning" cluster
 */
EmberAfStatus zcl_decode_zll_commissioning_cluster_network_join_end_device_request_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_network_join_end_device_request_command_t *cmd_struct);
/** @brief Parser function for "NetworkUpdateRequest" ZCL command from "ZLL Commissioning" cluster
 */
EmberAfStatus zcl_decode_zll_commissioning_cluster_network_update_request_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_network_update_request_command_t *cmd_struct);
/** @brief Parser function for "GetGroupIdentifiersRequest" ZCL command from "ZLL Commissioning" cluster
 */
EmberAfStatus zcl_decode_zll_commissioning_cluster_get_group_identifiers_request_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_get_group_identifiers_request_command_t *cmd_struct);
/** @brief Parser function for "GetEndpointListRequest" ZCL command from "ZLL Commissioning" cluster
 */
EmberAfStatus zcl_decode_zll_commissioning_cluster_get_endpoint_list_request_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_get_endpoint_list_request_command_t *cmd_struct);
/** @brief Parser function for "ScanResponse" ZCL command from "ZLL Commissioning" cluster
 */
EmberAfStatus zcl_decode_zll_commissioning_cluster_scan_response_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_scan_response_command_t *cmd_struct);
/** @brief Parser function for "DeviceInformationResponse" ZCL command from "ZLL Commissioning" cluster
 */
EmberAfStatus zcl_decode_zll_commissioning_cluster_device_information_response_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_device_information_response_command_t *cmd_struct);
/** @brief Parser function for "NetworkStartResponse" ZCL command from "ZLL Commissioning" cluster
 */
EmberAfStatus zcl_decode_zll_commissioning_cluster_network_start_response_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_network_start_response_command_t *cmd_struct);
/** @brief Parser function for "NetworkJoinRouterResponse" ZCL command from "ZLL Commissioning" cluster
 */
EmberAfStatus zcl_decode_zll_commissioning_cluster_network_join_router_response_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_network_join_router_response_command_t *cmd_struct);
/** @brief Parser function for "NetworkJoinEndDeviceResponse" ZCL command from "ZLL Commissioning" cluster
 */
EmberAfStatus zcl_decode_zll_commissioning_cluster_network_join_end_device_response_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_network_join_end_device_response_command_t *cmd_struct);
/** @brief Parser function for "EndpointInformation" ZCL command from "ZLL Commissioning" cluster
 */
EmberAfStatus zcl_decode_zll_commissioning_cluster_endpoint_information_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_endpoint_information_command_t *cmd_struct);
/** @brief Parser function for "GetGroupIdentifiersResponse" ZCL command from "ZLL Commissioning" cluster
 */
EmberAfStatus zcl_decode_zll_commissioning_cluster_get_group_identifiers_response_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_get_group_identifiers_response_command_t *cmd_struct);
/** @brief Parser function for "GetEndpointListResponse" ZCL command from "ZLL Commissioning" cluster
 */
EmberAfStatus zcl_decode_zll_commissioning_cluster_get_endpoint_list_response_command (EmberAfClusterCommand * cmd, sl_zcl_zll_commissioning_cluster_get_endpoint_list_response_command_t *cmd_struct);
/** @brief Parser function for "CommandOne" ZCL command from "Sample Mfg Specific Cluster" cluster
 */
EmberAfStatus zcl_decode_sample_mfg_specific_cluster_cluster_command_one_command (EmberAfClusterCommand * cmd, sl_zcl_sample_mfg_specific_cluster_cluster_command_one_command_t *cmd_struct);
/** @brief Parser function for "CommandTwo" ZCL command from "Sample Mfg Specific Cluster 2" cluster
 */
EmberAfStatus zcl_decode_sample_mfg_specific_cluster_2_cluster_command_two_command (EmberAfClusterCommand * cmd, sl_zcl_sample_mfg_specific_cluster_2_cluster_command_two_command_t *cmd_struct);
/** @brief Parser function for "SetToken" ZCL command from "Configuration Cluster" cluster
 */
EmberAfStatus zcl_decode_configuration_cluster_cluster_set_token_command (EmberAfClusterCommand * cmd, sl_zcl_configuration_cluster_cluster_set_token_command_t *cmd_struct);
/** @brief Parser function for "ReadTokens" ZCL command from "Configuration Cluster" cluster
 */
EmberAfStatus zcl_decode_configuration_cluster_cluster_read_tokens_command (EmberAfClusterCommand * cmd, sl_zcl_configuration_cluster_cluster_read_tokens_command_t *cmd_struct);
/** @brief Parser function for "UnlockTokens" ZCL command from "Configuration Cluster" cluster
 */
EmberAfStatus zcl_decode_configuration_cluster_cluster_unlock_tokens_command (EmberAfClusterCommand * cmd, sl_zcl_configuration_cluster_cluster_unlock_tokens_command_t *cmd_struct);
/** @brief Parser function for "ReturnToken" ZCL command from "Configuration Cluster" cluster
 */
EmberAfStatus zcl_decode_configuration_cluster_cluster_return_token_command (EmberAfClusterCommand * cmd, sl_zcl_configuration_cluster_cluster_return_token_command_t *cmd_struct);
/** @brief Parser function for "stream" ZCL command from "MFGLIB Cluster" cluster
 */
EmberAfStatus zcl_decode_mfglib_cluster_cluster_stream_command (EmberAfClusterCommand * cmd, sl_zcl_mfglib_cluster_cluster_stream_command_t *cmd_struct);
/** @brief Parser function for "tone" ZCL command from "MFGLIB Cluster" cluster
 */
EmberAfStatus zcl_decode_mfglib_cluster_cluster_tone_command (EmberAfClusterCommand * cmd, sl_zcl_mfglib_cluster_cluster_tone_command_t *cmd_struct);
/** @brief Parser function for "rxMode" ZCL command from "MFGLIB Cluster" cluster
 */
EmberAfStatus zcl_decode_mfglib_cluster_cluster_rx_mode_command (EmberAfClusterCommand * cmd, sl_zcl_mfglib_cluster_cluster_rx_mode_command_t *cmd_struct);
/** @brief Parser function for "EnableApsLinkKeyAuthorization" ZCL command from "SL Works With All Hubs" cluster
 */
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_enable_aps_link_key_authorization_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_enable_aps_link_key_authorization_command_t *cmd_struct);
/** @brief Parser function for "DisableApsLinkKeyAuthorization" ZCL command from "SL Works With All Hubs" cluster
 */
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_disable_aps_link_key_authorization_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_disable_aps_link_key_authorization_command_t *cmd_struct);
/** @brief Parser function for "ApsLinkKeyAuthorizationQuery" ZCL command from "SL Works With All Hubs" cluster
 */
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_aps_link_key_authorization_query_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_aps_link_key_authorization_query_command_t *cmd_struct);
/** @brief Parser function for "EnableWwahAppEventRetryAlgorithm" ZCL command from "SL Works With All Hubs" cluster
 */
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_enable_wwah_app_event_retry_algorithm_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_enable_wwah_app_event_retry_algorithm_command_t *cmd_struct);
/** @brief Parser function for "EnableWwahRejoinAlgorithm" ZCL command from "SL Works With All Hubs" cluster
 */
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_enable_wwah_rejoin_algorithm_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_enable_wwah_rejoin_algorithm_command_t *cmd_struct);
/** @brief Parser function for "SetIasZoneEnrollmentMethod" ZCL command from "SL Works With All Hubs" cluster
 */
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_set_ias_zone_enrollment_method_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_set_ias_zone_enrollment_method_command_t *cmd_struct);
/** @brief Parser function for "EnablePeriodicRouterCheckIns" ZCL command from "SL Works With All Hubs" cluster
 */
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_enable_periodic_router_check_ins_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_enable_periodic_router_check_ins_command_t *cmd_struct);
/** @brief Parser function for "SetMacPollFailureWaitTime" ZCL command from "SL Works With All Hubs" cluster
 */
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_set_mac_poll_failure_wait_time_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_set_mac_poll_failure_wait_time_command_t *cmd_struct);
/** @brief Parser function for "SetPendingNetworkUpdate" ZCL command from "SL Works With All Hubs" cluster
 */
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_set_pending_network_update_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_set_pending_network_update_command_t *cmd_struct);
/** @brief Parser function for "RequireApsAcksOnUnicasts" ZCL command from "SL Works With All Hubs" cluster
 */
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_require_aps_acks_on_unicasts_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_require_aps_acks_on_unicasts_command_t *cmd_struct);
/** @brief Parser function for "DebugReportQuery" ZCL command from "SL Works With All Hubs" cluster
 */
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_debug_report_query_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_debug_report_query_command_t *cmd_struct);
/** @brief Parser function for "SurveyBeacons" ZCL command from "SL Works With All Hubs" cluster
 */
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_survey_beacons_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_survey_beacons_command_t *cmd_struct);
/** @brief Parser function for "UseTrustCenterForClusterServer" ZCL command from "SL Works With All Hubs" cluster
 */
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_use_trust_center_for_cluster_server_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_use_trust_center_for_cluster_server_command_t *cmd_struct);
/** @brief Parser function for "ApsLinkKeyAuthorizationQueryResponse" ZCL command from "SL Works With All Hubs" cluster
 */
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_aps_link_key_authorization_query_response_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_aps_link_key_authorization_query_response_command_t *cmd_struct);
/** @brief Parser function for "PoweringOffNotification" ZCL command from "SL Works With All Hubs" cluster
 */
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_powering_off_notification_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_powering_off_notification_command_t *cmd_struct);
/** @brief Parser function for "PoweringOnNotification" ZCL command from "SL Works With All Hubs" cluster
 */
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_powering_on_notification_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_powering_on_notification_command_t *cmd_struct);
/** @brief Parser function for "ShortAddressChange" ZCL command from "SL Works With All Hubs" cluster
 */
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_short_address_change_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_short_address_change_command_t *cmd_struct);
/** @brief Parser function for "ApsAckEnablementQueryResponse" ZCL command from "SL Works With All Hubs" cluster
 */
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_aps_ack_enablement_query_response_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_aps_ack_enablement_query_response_command_t *cmd_struct);
/** @brief Parser function for "PowerDescriptorChange" ZCL command from "SL Works With All Hubs" cluster
 */
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_power_descriptor_change_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_power_descriptor_change_command_t *cmd_struct);
/** @brief Parser function for "NewDebugReportNotification" ZCL command from "SL Works With All Hubs" cluster
 */
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_new_debug_report_notification_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_new_debug_report_notification_command_t *cmd_struct);
/** @brief Parser function for "DebugReportQueryResponse" ZCL command from "SL Works With All Hubs" cluster
 */
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_debug_report_query_response_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_debug_report_query_response_command_t *cmd_struct);
/** @brief Parser function for "TrustCenterForClusterServerQueryResponse" ZCL command from "SL Works With All Hubs" cluster
 */
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_trust_center_for_cluster_server_query_response_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_trust_center_for_cluster_server_query_response_command_t *cmd_struct);
/** @brief Parser function for "SurveyBeaconsResponse" ZCL command from "SL Works With All Hubs" cluster
 */
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_survey_beacons_response_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_survey_beacons_response_command_t *cmd_struct);
/** @brief Parser function for "UseTrustCenterForClusterServerResponse" ZCL command from "SL Works With All Hubs" cluster
 */
EmberAfStatus zcl_decode_sl_works_with_all_hubs_cluster_use_trust_center_for_cluster_server_response_command (EmberAfClusterCommand * cmd, sl_zcl_sl_works_with_all_hubs_cluster_use_trust_center_for_cluster_server_response_command_t *cmd_struct);

#endif //End of SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT

/** @} END addtogroup */
#endif // __ZAP_CLUSTER_COMMAND_PARSER_H__