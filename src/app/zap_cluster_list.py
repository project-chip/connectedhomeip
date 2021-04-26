#!/usr/bin/env python3
"""Parses a ZAP input file and outputs directories to compile."""

import argparse
import pathlib
import json
import typing
import sys

# List of directories in src/app/clusters to build for server clusters.
SERVER_CLUSTERS: typing.Dict[str, typing.List[str]] = {
    'ALARM_CLUSTER': [],
    'APPLICATION_BASIC_CLUSTER': [],
    'BARRIER_CONTROL_CLUSTER': ['barrier-control-server'],
    'BASIC_CLUSTER': ['basic'],
    'BINDING_CLUSTER': ['bindings'],
    'COLOR_CONTROL_CLUSTER': ['color-control-server'],
    'COMMISSIONING_CLUSTER': [],
    'CONTENT_LAUNCH_CLUSTER': ['content-launch-server'],
    'DESCRIPTOR_CLUSTER': ['descriptor'],
    'DEVICE_TEMP_CLUSTER': [],
    'DOOR_LOCK_CLUSTER': ['door-lock-server'],
    'GENERAL_COMMISSIONING_CLUSTER': ['general-commissioning-server'],
    'GROUPS_CLUSTER': ['groups-server'],
    'GROUP_KEY_MANAGEMENT_CLUSTER': [],
    'IAS_ZONE_CLUSTER': ['ias-zone-server'],
    'IDENTIFY_CLUSTER': ['identify'],
    'LEVEL_CONTROL_CLUSTER': ['level-control'],
    'LOW_POWER_CLUSTER': ['low-power-server'],
    'MEDIA_PLAYBACK_CLUSTER': ['media-playback-server'],
    'NETWORK_COMMISSIONING_CLUSTER': ['network-commissioning'],
    'OCCUPANCY_SENSING_CLUSTER': [],
    'ON_OFF_CLUSTER': ['on-off-server'],
    'OPERATIONAL_CREDENTIALS_CLUSTER': ['operational-credentials'],
    'OTA_BOOTLOAD_CLUSTER': [],
    'OTA_SERVER_CLUSTER': ['ota-server'],
    'OTA_CLIENT_CLUSTER': [],
    'POWER_CONFIG_CLUSTER': [],
    'PUMP_CONFIG_CONTROL_CLUSTER': [],
    'SCENES_CLUSTER': ['scenes'],
    'TEMP_MEASUREMENT_CLUSTER': ['temperature-measurement-server'],
    'THERMOSTAT_CLUSTER': [],
    'WINDOW_COVERING_CLUSTER': [],
    'ZLL_COMMISSIONING_CLUSTER': []
}

# List of directories in src/app/clusters to build for client clusters.
CLIENT_CLUSTERS: typing.Dict[str, typing.List[str]] = {
    'ALARM_CLUSTER': [],
    'APPLICATION_BASIC_CLUSTER': [],
    'BARRIER_CONTROL_CLUSTER': [],
    'BASIC_CLUSTER': [],
    'BINDING_CLUSTER': [],
    'COLOR_CONTROL_CLUSTER': [],
    'COMMISSIONING_CLUSTER': [],
    'CONTENT_LAUNCH_CLUSTER': [],
    'DEVICE_TEMP_CLUSTER': [],
    'DESCRIPTOR_CLUSTER': [],
    'DOOR_LOCK_CLUSTER': [],
    'GENERAL_COMMISSIONING_CLUSTER': [],
    'GROUPS_CLUSTER': [],
    'GROUP_KEY_MANAGEMENT_CLUSTER': [],
    'IAS_ZONE_CLUSTER': ['ias-zone-client'],
    'IDENTIFY_CLUSTER': [],
    'LEVEL_CONTROL_CLUSTER': [],
    'LOW_POWER_CLUSTER': [],
    'MEDIA_PLAYBACK_CLUSTER': ['media-playback-client'],
    'NETWORK_COMMISSIONING_CLUSTER': [],
    'OCCUPANCY_SENSING_CLUSTER': [],
    'ON_OFF_CLUSTER': [],
    'OPERATIONAL_CREDENTIALS_CLUSTER': ['operational-credentials'],
    'OTA_BOOTLOAD_CLUSTER': [],
    'OTA_SERVER_CLUSTER': ['ota-server'],
    'OTA_CLIENT_CLUSTER': [],
    'POWER_CONFIG_CLUSTER': [],
    'PUMP_CONFIG_CONTROL_CLUSTER': [],
    'SCENES_CLUSTER': [],
    'TEMP_MEASUREMENT_CLUSTER': [],
    'THERMOSTAT_CLUSTER': ['thermostat-client'],
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
                        type=pathlib.Path)

    args = parser.parse_args()

    dump_zapfile_clusters(args.zap_file)

    sys.exit(0)


if __name__ == '__main__':
    main()
