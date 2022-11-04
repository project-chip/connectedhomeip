#!/usr/bin/env python3
"""Parses a ZAP input file and outputs directories to compile."""

import argparse
import pathlib
import json
import typing
import sys

# List of directories in src/app/clusters to build for server clusters.
SERVER_CLUSTERS: typing.Dict[str, typing.List[str]] = {
    'ACCESS_CONTROL_CLUSTER': ['access-control-server'],
    'ACCOUNT_LOGIN_CLUSTER': ['account-login-server'],
    'ADMINISTRATOR_COMMISSIONING_CLUSTER': ['administrator-commissioning-server'],
    'ALARM_CLUSTER': [],
    'APPLICATION_BASIC_CLUSTER': ['application-basic-server'],
    'APPLICATION_LAUNCHER_CLUSTER': ['application-launcher-server'],
    'AUDIO_OUTPUT_CLUSTER': ['audio-output-server'],
    'BALLAST_CONFIGURATION_CLUSTER': [],
    'BARRIER_CONTROL_CLUSTER': ['barrier-control-server'],
    'BASIC_CLUSTER': ['basic'],
    'BINARY_INPUT_BASIC_CLUSTER': [],
    'BINDING_CLUSTER': ['bindings'],
    'BOOLEAN_STATE_CLUSTER': [],
    'ACTIONS_CLUSTER': [],
    'BRIDGED_DEVICE_BASIC_CLUSTER': ['bridged-device-basic-information-server'],
    'CHANNEL_CLUSTER': ['channel-server'],
    'COLOR_CONTROL_CLUSTER': ['color-control-server'],
    'COMMISSIONING_CLUSTER': [],
    'CONTENT_LAUNCHER_CLUSTER': ['content-launch-server'],
    'DESCRIPTOR_CLUSTER': ['descriptor'],
    'DEVICE_TEMP_CLUSTER': [],
    'DIAGNOSTIC_LOGS_CLUSTER': ['diagnostic-logs-server'],
    'DOOR_LOCK_CLUSTER': ['door-lock-server'],
    'ELECTRICAL_MEASUREMENT_CLUSTER': [],
    'ETHERNET_NETWORK_DIAGNOSTICS_CLUSTER': ['ethernet-network-diagnostics-server'],
    'FAULT_INJECTION_CLUSTER': ['fault-injection-server'],
    'FAN_CONTROL_CLUSTER': ['fan-control-server'],
    'FIXED_LABEL_CLUSTER': ['fixed-label-server'],
    'FLOW_MEASUREMENT_CLUSTER': [],
    'GENERAL_COMMISSIONING_CLUSTER': ['general-commissioning-server'],
    'GENERAL_DIAGNOSTICS_CLUSTER': ['general-diagnostics-server'],
    'GROUP_KEY_MANAGEMENT_CLUSTER': ['group-key-mgmt-server'],
    'GROUPS_CLUSTER': ['groups-server'],
    'IAS_ZONE_CLUSTER': [],
    'IDENTIFY_CLUSTER': ['identify-server'],
    'ILLUMINANCE_MEASUREMENT_CLUSTER': [],
    'KEYPAD_INPUT_CLUSTER': ['keypad-input-server'],
    'LEVEL_CONTROL_CLUSTER': ['level-control'],
    'LOCALIZATION_CONFIGURATION_CLUSTER': ['localization-configuration-server'],
    'LOW_POWER_CLUSTER': ['low-power-server'],
    'MEDIA_INPUT_CLUSTER': ['media-input-server'],
    'MEDIA_PLAYBACK_CLUSTER': ['media-playback-server'],
    'MODE_SELECT_CLUSTER': ['mode-select-server'],
    'NETWORK_COMMISSIONING_CLUSTER': ['network-commissioning'],
    'OCCUPANCY_SENSING_CLUSTER': ['occupancy-sensor-server'],
    'ON_OFF_CLUSTER': ['on-off-server'],
    'ON_OFF_SWITCH_CONFIGURATION_CLUSTER': [],
    'OPERATIONAL_CREDENTIALS_CLUSTER': ['operational-credentials-server'],
    'OTA_BOOTLOAD_CLUSTER': [],
    'OTA_PROVIDER_CLUSTER': ['ota-provider'],
    'OTA_REQUESTOR_CLUSTER': ['ota-requestor'],
    'POWER_SOURCE_CLUSTER': ['power-source-server'],
    'POWER_SOURCE_CONFIGURATION_CLUSTER': ['power-source-configuration-server'],
    'POLL_CONTROL_CLUSTER': [],
    'POWER_CONFIG_CLUSTER': [],
    'POWER_PROFILE_CLUSTER': [],
    'PRESSURE_MEASUREMENT_CLUSTER': [],
    'PROXY_CONFIGURATION_CLUSTER': [],
    'PROXY_DISCOVERY_CLUSTER': [],
    'PROXY_VALID_CLUSTER': [],
    'PUMP_CONFIGURATION_AND_CONTROL_CLUSTER': ['pump-configuration-and-control-server'],
    'PWM_CLUSTER': [],
    'RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER': [],
    'SCENES_CLUSTER': ['scenes'],
    'SOFTWARE_DIAGNOSTICS_CLUSTER': ['software-diagnostics-server'],
    'SWITCH_CLUSTER': ['switch-server'],
    'TARGET_NAVIGATOR_CLUSTER': ['target-navigator-server'],
    'TEMPERATURE_MEASUREMENT_CLUSTER': [],
    'TEST_CLUSTER': ['test-cluster-server'],
    'THERMOSTAT_CLUSTER': ['thermostat-server'],
    'THERMOSTAT_USER_INTERFACE_CONFIGURATION_CLUSTER': ['thermostat-user-interface-configuration-server'],
    'THREAD_NETWORK_DIAGNOSTICS_CLUSTER': ['thread-network-diagnostics-server'],
    'TIME_CLUSTER': [],
    'TIME_FORMAT_LOCALIZATION_CLUSTER': ['time-format-localization-server'],
    'TIME_SYNCHRONIZATION_CLUSTER': [],
    'UNIT_LOCALIZATION_CLUSTER': [],
    'USER_LABEL_CLUSTER': ['user-label-server'],
    'WAKE_ON_LAN_CLUSTER': ['wake-on-lan-server'],
    'WIFI_NETWORK_DIAGNOSTICS_CLUSTER': ['wifi-network-diagnostics-server'],
    'WINDOW_COVERING_CLUSTER': ['window-covering-server'],
    'ZLL_COMMISSIONING_CLUSTER': []
}

# List of directories in src/app/clusters to build for client clusters.
CLIENT_CLUSTERS: typing.Dict[str, typing.List[str]] = {
    'ACCESS_CONTROL_CLUSTER': [],
    'ACCOUNT_LOGIN_CLUSTER': [],
    'ADMINISTRATOR_COMMISSIONING_CLUSTER': [],
    'ALARM_CLUSTER': [],
    'APPLICATION_BASIC_CLUSTER': [],
    'APPLICATION_LAUNCHER_CLUSTER': [],
    'AUDIO_OUTPUT_CLUSTER': [],
    'BALLAST_CONFIGURATION_CLUSTER': [],
    'BARRIER_CONTROL_CLUSTER': [],
    'BASIC_CLUSTER': [],
    'BINARY_INPUT_BASIC_CLUSTER': [],
    'BINDING_CLUSTER': [],
    'BOOLEAN_STATE_CLUSTER': [],
    'ACTIONS_CLUSTER': [],
    'BRIDGED_DEVICE_BASIC_CLUSTER': [],
    'CHANNEL_CLUSTER': [],
    'COLOR_CONTROL_CLUSTER': [],
    'COMMISSIONING_CLUSTER': [],
    'CONTENT_LAUNCHER_CLUSTER': [],
    'DESCRIPTOR_CLUSTER': [],
    'DEVICE_TEMP_CLUSTER': [],
    'DIAGNOSTIC_LOGS_CLUSTER': [],
    'DOOR_LOCK_CLUSTER': [],
    'ELECTRICAL_MEASUREMENT_CLUSTER': [],
    'ETHERNET_NETWORK_DIAGNOSTICS_CLUSTER': [],
    'FAULT_INJECTION_CLUSTER': [],
    'FAN_CONTROL_CLUSTER': [],
    'FIXED_LABEL_CLUSTER': [],
    'FLOW_MEASUREMENT_CLUSTER': [],
    'GENERAL_COMMISSIONING_CLUSTER': [],
    'GENERAL_DIAGNOSTICS_CLUSTER': [],
    'GROUP_KEY_MANAGEMENT_CLUSTER': [],
    'GROUPS_CLUSTER': [],
    'IAS_ZONE_CLUSTER': [],
    'IDENTIFY_CLUSTER': [],
    'ILLUMINANCE_MEASUREMENT_CLUSTER': [],
    'KEYPAD_INPUT_CLUSTER': [],
    'LEVEL_CONTROL_CLUSTER': [],
    'LOCALIZATION_CONFIGURATION_CLUSTER': [],
    'LOW_POWER_CLUSTER': [],
    'MEDIA_INPUT_CLUSTER': [],
    'MEDIA_PLAYBACK_CLUSTER': [],
    'MODE_SELECT_CLUSTER': [],
    'NETWORK_COMMISSIONING_CLUSTER': [],
    'OCCUPANCY_SENSING_CLUSTER': ['occupancy-sensor-server'],
    'ON_OFF_CLUSTER': [],
    'ON_OFF_SWITCH_CONFIGURATION_CLUSTER': [],
    'OPERATIONAL_CREDENTIALS_CLUSTER': [],
    'OTA_BOOTLOAD_CLUSTER': [],
    'OTA_PROVIDER_CLUSTER': [],
    'OTA_REQUESTOR_CLUSTER': [],
    'POLL_CONTROL_CLUSTER': [],
    'POWER_CONFIG_CLUSTER': [],
    'POWER_PROFILE_CLUSTER': [],
    'POWER_SOURCE_CLUSTER': [],
    'POWER_SOURCE_CONFIGURATION_CLUSTER': [],
    'PRESSURE_MEASUREMENT_CLUSTER': [],
    'PROXY_CONFIGURATION_CLUSTER': [],
    'PROXY_DISCOVERY_CLUSTER': [],
    'PROXY_VALID_CLUSTER': [],
    'PUMP_CONFIGURATION_AND_CONTROL_CLUSTER': ['pump-configuration-and-control-client'],
    'PWM_CLUSTER': [],
    'RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER': [],
    'SCENES_CLUSTER': [],
    'SOFTWARE_DIAGNOSTICS_CLUSTER': [],
    'SWITCH_CLUSTER': [],
    'TARGET_NAVIGATOR_CLUSTER': [],
    'TEMPERATURE_MEASUREMENT_CLUSTER': [],
    'TEST_CLUSTER': [],
    'THERMOSTAT_CLUSTER': ['thermostat-client'],
    'THERMOSTAT_USER_INTERFACE_CONFIGURATION_CLUSTER': [],
    'THREAD_NETWORK_DIAGNOSTICS_CLUSTER': [],
    'TIME_CLUSTER': [],
    'TIME_FORMAT_LOCALIZATION_CLUSTER': [],
    'TIME_SYNCHRONIZATION_CLUSTER': [],
    'TRUSTED_ROOT_CERTIFICATES_CLUSTER': [],
    'UNIT_LOCALIZATION_CLUSTER': [],
    'USER_LABEL_CLUSTER': [],
    'WAKE_ON_LAN_CLUSTER': [],
    'WIFI_NETWORK_DIAGNOSTICS_CLUSTER': [],
    'WINDOW_COVERING_CLUSTER': [],
    'ZLL_COMMISSIONING_CLUSTER': []
}


def get_cluster_sources(clusters: typing.Set[str],
                        source_map: typing.Dict[str,
                                                typing.List[str]], side: str):
    """Returns a list of cluster source directories for the given clusters.

    Returns:
      The set of source directories to build.
    """

    cluster_sources: typing.Set[str] = set()

    for cluster in clusters:
        if not cluster in source_map:
            raise ValueError("Unhandled %s cluster: %s"
                             " (hint: add to src/app/zap_cluster_list.py)" % (side, cluster))

        cluster_sources.update(source_map[cluster])

    return cluster_sources


def dump_zapfile_clusters(zap_file_path: pathlib.Path):
    """Prints all of the source directories to build for a given ZAP file.

    Arguments:
      zap_file_path - Path to the ZAP input file.
    """

    client_clusters: typing.Set[str] = set()
    server_clusters: typing.Set[str] = set()

    with open(zap_file_path, "r") as zap_file:
        zap_json = json.loads(zap_file.read())

        for endpoint_type in zap_json.get('endpointTypes'):
            for cluster in endpoint_type.get('clusters'):
                side: str = cluster.get('side')
                if side == 'client':
                    clusters_set = client_clusters
                elif side == 'server':
                    clusters_set = server_clusters
                else:
                    raise ValueError("Invalid side for cluster: %s" % side)

                if cluster.get('enabled') == 1:
                    clusters_set.add(cluster.get('define'))

    cluster_sources: typing.Set[str] = set()

    cluster_sources.update(
        get_cluster_sources(server_clusters, SERVER_CLUSTERS, 'server'))

    cluster_sources.update(
        get_cluster_sources(client_clusters, CLIENT_CLUSTERS, 'client'))

    for cluster in sorted(cluster_sources):
        print(cluster)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--zap_file',
                        help='Path to .zap file',
                        required=True,
                        type=pathlib.Path)

    args = parser.parse_args()

    dump_zapfile_clusters(args.zap_file)

    sys.exit(0)


if __name__ == '__main__':
    main()
