#!/usr/bin/env python3
"""Parses a ZAP input file and outputs directories to compile."""

import argparse
import json
import os
import pathlib
import sys
import typing


def get_cluster_sources(clusters: typing.Set[str],
                        source_map: typing.Dict[str,
                                                typing.List[str]], side: str):
    """Returns a list of cluster source directories for the given clusters.

    Returns:
      The set of source directories to build.
    """

    cluster_sources: typing.Set[str] = set()

    for cluster in clusters:
        if cluster not in source_map:
            raise ValueError("Unhandled %s cluster: %s"
                             " (hint: add to src/app/zap_cluster_list.json)" % (side, cluster))

        cluster_sources.update(source_map[cluster])

    return cluster_sources


def dump_zapfile_clusters(zap_file_path: pathlib.Path,
                          implementation_data_path: pathlib.Path,
                          external_clusters: typing.List[str]):
    """Prints all of the source directories to build for a given ZAP file.

    Arguments:
      zap_file_path - Path to the ZAP input file.
    """

    # List of directories in src/app/clusters to build for server clusters.
    SERVER_CLUSTERS: typing.Dict[str, typing.List[str]] = {}

    # List of directories in src/app/clusters to build for client clusters.
    CLIENT_CLUSTERS: typing.Dict[str, typing.List[str]] = {}

    with open(implementation_data_path, "r") as implementation_data_file:
        implementation_data = json.load(implementation_data_file)
        SERVER_CLUSTERS = implementation_data["ServerDirectories"]
        CLIENT_CLUSTERS = implementation_data["ClientDirectories"]

    client_clusters: typing.Set[str] = set()
    server_clusters: typing.Set[str] = set()

    with open(zap_file_path, "r") as zap_file:
        zap_json = json.loads(zap_file.read())

        for endpoint_type in zap_json.get('endpointTypes'):
            for cluster in endpoint_type.get('clusters'):
                if cluster.get('define') in external_clusters:
                    continue
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
    parser.add_argument('--cluster-implementation-data',
                        help='Path to .json file which lists the directories cluster implementations live in',
                        required=False,
                        type=pathlib.Path,
                        default=os.path.join(os.path.dirname(__file__), "zap_cluster_list.json"))
    parser.add_argument('--external-clusters',
                        help='Clusters with external implementations. ' +
                             'The default implementations will not be used nor required for these clusters. ' +
                             'Format: MY_CUSTOM_CLUSTER',
                        nargs='+',
                        metavar='EXTERNAL_CLUSTER',
                        default=[])

    args = parser.parse_args()

    dump_zapfile_clusters(args.zap_file, args.cluster_implementation_data, args.external_clusters)

    sys.exit(0)


if __name__ == '__main__':
    main()
