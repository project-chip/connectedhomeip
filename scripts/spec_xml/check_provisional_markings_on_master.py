#
#    Copyright (c) 2025 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#
# This script gives a print out of the differences between the two specified spec
# versions and a description of the provisional elements in the later version.
# Right now, this is just in print form. The intent is to use this for new
# data model XML drops to show the differences. This was also used to double-check
# spec expectations before the 1.4 release and we should continue to do so going forward.
import chip.clusters as Clusters
from chip.testing.conformance import is_provisional
from chip.testing.spec_parsing import PrebuiltDataModelDirectory, build_xml_clusters


def check_cluster_provisional_markings_on_master():
    # TODO: Do we want a way to mark the most recent certified?
    prior_clusters, _ = build_xml_clusters(PrebuiltDataModelDirectory.k1_4)
    new_clusters, _ = build_xml_clusters(PrebuiltDataModelDirectory.kMaster)

    additional_clusters = set(new_clusters.keys()) - set(prior_clusters.keys())

    unmarked_new_clusters = [new_clusters[c].name for c in additional_clusters if not new_clusters[c].is_provisional]

    print("New Clusters in TOT that are not marked as provisional")
    if unmarked_new_clusters:
        for c in sorted(unmarked_new_clusters):
            print(c)
    else:
        print("None")

    same_ids = set(new_clusters.keys()).intersection(set(prior_clusters.keys()))
    print("Old clusters with new elements that are not marked as provisional")
    for c in same_ids:
        def print_unmarked(new, old, element_type):
            new_ids = set(new.keys()) - set(old.keys())
            unmarked = [new[e].name for e in new_ids if not is_provisional(new[e].conformance)]
            if unmarked:
                print(f"New unmarked {element_type} in cluster {new_clusters[c].name}")
                for e in unmarked:
                    print(f'\t{e}')

        print_unmarked(new_clusters[c].features, prior_clusters[c].features, "features")
        print_unmarked(new_clusters[c].attributes, prior_clusters[c].attributes, "attributes")
        print_unmarked(new_clusters[c].accepted_commands, prior_clusters[c].accepted_commands, "accepted commands")
        print_unmarked(new_clusters[c].generated_commands, prior_clusters[c].generated_commands, "generated commands")
        print_unmarked(new_clusters[c].events, prior_clusters[c].events, "events")


if __name__ == "__main__":
    check_cluster_provisional_markings_on_master()
