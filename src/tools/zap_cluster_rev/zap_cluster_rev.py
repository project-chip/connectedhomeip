#!/usr/bin/env python

import argparse
import json
from dataclasses import dataclass

from chip.testing.spec_parsing import build_xml_clusters


@dataclass
class ClusterInfo():
    endpoint_id: int
    cluster_code: int
    cluster_spec_revision: int
    cluster_name: str
    json_attribute: object

    def cluster_revision(self):
        return int(self.json_attribute["defaultValue"])

    def __str__(self):
        return ('Endpoint: %d cluster_code: %d cluster_revision: %d cluster_spec_revision: %d name: %s' % (self.endpoint_id, self.cluster_code, self.cluster_revision(), self.cluster_spec_revision, self.cluster_name))

    def update_cluster_revision(self):
        self.json_attribute["defaultValue"] = self.cluster_spec_revision


def load_zap(filename: str):
    with open(filename, "rt") as infile:
        return json.load(infile)


def save_zap(body: object, filename: str):
    with open(filename, "wt+") as outfile:
        return json.dump(body, outfile, indent=2)


def get_outdated_clusters(body: object, xml_clusters: dict) -> list[ClusterInfo]:
    result = []
    for endpoint in body.get("endpointTypes", []):
        endpoint_id = endpoint.get("id") - 1
        for cluster in endpoint.get("clusters", []):
            for attribute in cluster.get("attributes", []):
                if attribute.get("name") != "ClusterRevision" or attribute.get("storageOption") != "RAM":
                    continue
                cluster_revision = int(attribute.get("defaultValue"))
                spec_revision = xml_clusters[cluster.get("code")].revision
                # Filter in outdated clusters only
                if (cluster_revision == spec_revision):
                    break
                cluster_info = ClusterInfo(endpoint_id=endpoint_id, cluster_code=cluster.get("code"),
                                           cluster_spec_revision=spec_revision, cluster_name=cluster.get("name"), json_attribute=attribute)
                result.append(cluster_info)
    return result


def main():
    parser = argparse.ArgumentParser(
        description="Process ZAP Files and update outdated cluster revisions according to the spec")
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument("--print-only", action="store_true", help="Print outdated cluster information.")
    group.add_argument("--update", action="store_true", help="Update outdated cluster revisions on ZAP file.")
    parser.add_argument("filenames", nargs="+", help="A sequence of ZAP filenames.")
    args = parser.parse_args()

    print("**WARNING**: This tool only updates the revision number. Please ensure any new attributes, events or commands are implemented accordingly.")
    outdated_count = 0
    for zap_filename in args.filenames:
        print("Checking for outdated cluster revisions on: %s" % zap_filename)
        body = load_zap(zap_filename)
        spec_xml_clusters, problems = build_xml_clusters()

        outdated_clusters = get_outdated_clusters(body, spec_xml_clusters)
        print("%d found!" % len(outdated_clusters))
        outdated_count += len(outdated_clusters)
        print(*outdated_clusters, sep='\n')

        if args.print_only:
            continue

        # Update outdated cluster revisions according to the spec
        for cluster in outdated_clusters:
            cluster.update_cluster_revision()
        if (outdated_clusters):
            print('Cluster revisions updated successfully!\n')

        # Check there's no longer any outdated cluster
        assert (not get_outdated_clusters(body, spec_xml_clusters))

        save_zap(body, zap_filename)

    # If it's printing only, return the number of outdated clusters, so it can be used as a test
    if args.print_only:
        return outdated_count


if __name__ == "__main__":
    main()
