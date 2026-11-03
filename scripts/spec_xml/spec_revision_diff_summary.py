#
#    Copyright (c) 2024 Project CHIP Authors
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
# The output is formatted in clean GitHub-flavored Markdown.

import click

from matter.testing.conformance import EMPTY_CLUSTER_GLOBAL_ATTRIBUTES, ConformanceDecision
from matter.testing.spec_parsing import PrebuiltDataModelDirectory, build_xml_clusters, build_xml_device_types


def get_changes(old, new):
    added = [e.name for k, e in new.items() if k not in old]
    removed = [e.name for k, e in old.items() if k not in new]
    same_ids = set(new.keys()).intersection(set(old.keys()))

    return added, removed, same_ids


def str_changes(element, added, removed, change_ids, old, new):
    if not added and not removed and not change_ids:
        return []

    ret = []
    if added:
        ret.append(f'- **{element} added:** {", ".join(f"`{a}`" for a in added)}')
    if removed:
        ret.append(f'- **{element} removed:** {", ".join(f"`{r}`" for r in removed)}')
    if change_ids:
        ret.append(f'- **{element} changed:**')
    for change_id in change_ids:
        name = old[change_id].name if old[change_id].name == new[
            change_id].name else f'{new[change_id].name} (previously {old[change_id].name})'
        ret.append(f'  - `{name}`:')
        ret.append(f'    - Old: `{old[change_id]}`')
        ret.append(f'    - New: `{new[change_id]}`')
    return ret


def str_element_changes(element, old, new):
    added, removed, same_ids = get_changes(old, new)
    change_ids = [change_id for change_id in same_ids if old[change_id] != new[change_id]
                  or str(old[change_id].conformance) != str(new[change_id].conformance)]
    return str_changes(element, added, removed, change_ids, old, new)


def diff_clusters(prior_revision: PrebuiltDataModelDirectory, new_revision: PrebuiltDataModelDirectory) -> None:
    prior_clusters, _ = build_xml_clusters(prior_revision)
    new_clusters, _ = build_xml_clusters(new_revision)

    additional_clusters, removed_clusters, same_cluster_ids = get_changes(prior_clusters, new_clusters)

    print(f'# Data Model Comparison: {prior_revision.dirname} -> {new_revision.dirname}\n')
    print('## Cluster Changes\n')
    print('### Newly Added Clusters')
    if additional_clusters:
        for c in sorted(additional_clusters):
            print(f'- `{c}`')
    else:
        print('- *(None)*')
    print()

    print(f'### Removed Clusters (since {prior_revision.dirname})')
    if removed_clusters:
        for c in sorted(removed_clusters):
            print(f'- `{c}`')
    else:
        print('- *(None)*')
    print()

    modified_clusters_header_printed = False
    for cid in sorted(same_cluster_ids):
        new = new_clusters[cid]
        old = prior_clusters[cid]

        name = old.name if old.name == new.name else f'{new.name} (previously {old.name})'

        changes = []
        if old.revision != new.revision:
            changes.append(f'- **Revision change**: {old.revision} -> {new.revision}')
            for r in range(old.revision + 1, new.revision + 1):
                try:
                    changes.append(f'  - Revision {r}: {new.revision_desc[r]}')
                except KeyError:
                    changes.append(f'  - Revision {r}: NOT PRESENT IN SPEC')
        changes.extend(str_element_changes('Features', old.features, new.features))
        changes.extend(str_element_changes('Attributes', old.attributes, new.attributes))
        changes.extend(str_element_changes('Accepted Commands', old.accepted_commands, new.accepted_commands))
        changes.extend(str_element_changes('Generated Commands', old.generated_commands, new.generated_commands))
        changes.extend(str_element_changes('Events', old.events, new.events))

        if changes:
            if not modified_clusters_header_printed:
                print('### Modified Clusters\n')
                modified_clusters_header_printed = True
            print(f'#### Cluster: `{name}` (0x{cid:04X})')
            print('\n'.join(changes))
            print()


def diff_device_types(prior_revision: PrebuiltDataModelDirectory, new_revision: PrebuiltDataModelDirectory) -> None:
    prior_device_types, _ = build_xml_device_types(prior_revision)
    new_device_types, _ = build_xml_device_types(new_revision)

    additional_device_types, removed_device_types, same_device_type_ids = get_changes(prior_device_types, new_device_types)

    print('## Device Type Changes\n')
    print('### Newly Added Device Types')
    if additional_device_types:
        for dt in sorted(additional_device_types):
            print(f'- `{dt}`')
    else:
        print('- *(None)*')
    print()

    print(f'### Removed Device Types (since {prior_revision.dirname})')
    if removed_device_types:
        for dt in sorted(removed_device_types):
            print(f'- `{dt}`')
    else:
        print('- *(None)*')
    print()

    modified_dt_header_printed = False
    for cid in sorted(same_device_type_ids):
        new = new_device_types[cid]
        old = prior_device_types[cid]

        name = old.name if old.name == new.name else f'{new.name} (previously {old.name})'

        changes = []
        if old.revision != new.revision:
            changes.append(f'- **Revision change**: {old.revision} -> {new.revision}')
            for r in range(old.revision + 1, new.revision + 1):
                try:
                    changes.append(f'  - Revision {r}: {new.revision_desc[r]}')
                except KeyError:
                    changes.append(f'  - Revision {r}: NOT PRESENT IN SPEC')
        changes.extend(str_element_changes('Server Clusters', old.server_clusters, new.server_clusters))
        changes.extend(str_element_changes('Client Clusters', old.client_clusters, new.client_clusters))

        if changes:
            if not modified_dt_header_printed:
                print('### Modified Device Types\n')
                modified_dt_header_printed = True
            print(f'#### Device Type: `{name}` (0x{cid:04X})')
            print('\n'.join(changes))
            print()


def _get_provisional(items):
    return {e.name for e in items if e.conformance(EMPTY_CLUSTER_GLOBAL_ATTRIBUTES).decision == ConformanceDecision.PROVISIONAL}


def get_provisional_diff(rev1: PrebuiltDataModelDirectory, rev2: PrebuiltDataModelDirectory):
    clusters_rev1, _ = build_xml_clusters(rev1)
    clusters_rev2, _ = build_xml_clusters(rev2)

    provisional_clusters_rev1 = [c.name for c in clusters_rev1.values() if c.is_provisional]
    provisional_clusters_rev2 = [c.name for c in clusters_rev2.values() if c.is_provisional]

    rev2_additional_provisional_clusters = set(provisional_clusters_rev2) - set(provisional_clusters_rev1)
    print(f'### Provisional Clusters in {rev2.dirname} not in {rev1.dirname}')
    if rev2_additional_provisional_clusters:
        for c in sorted(rev2_additional_provisional_clusters):
            print(f'- `{c}`')
    else:
        print('- *(None)*')
    print()

    printed_cluster_elements_header = False
    for k in sorted(clusters_rev2.keys()):
        if k not in clusters_rev1:
            continue
        c2 = clusters_rev2[k]
        c1 = clusters_rev1[k]
        rev2_provisional_features = _get_provisional(c2.features.values())
        rev1_provisional_features = _get_provisional(c1.features.values())
        features = rev2_provisional_features - rev1_provisional_features

        rev2_provisional_attributes = _get_provisional(c2.attributes.values())
        rev1_provisional_attributes = _get_provisional(c1.attributes.values())
        attributes = rev2_provisional_attributes - rev1_provisional_attributes

        rev2_provisional_accepted_commands = _get_provisional(c2.accepted_commands.values())
        rev1_provisional_accepted_commands = _get_provisional(c1.accepted_commands.values())
        accepted_commands = rev2_provisional_accepted_commands - rev1_provisional_accepted_commands

        rev2_provisional_generated_commands = _get_provisional(c2.generated_commands.values())
        rev1_provisional_generated_commands = _get_provisional(c1.generated_commands.values())
        generated_commands = rev2_provisional_generated_commands - rev1_provisional_generated_commands

        rev2_provisional_events = _get_provisional(c2.events.values())
        rev1_provisional_events = _get_provisional(c1.events.values())
        events = rev2_provisional_events - rev1_provisional_events

        if not features and not attributes and not accepted_commands and not generated_commands and not events:
            continue

        if not printed_cluster_elements_header:
            print(f'### Provisional Elements in {rev2.dirname} that are not provisional in {rev1.dirname}\n')
            printed_cluster_elements_header = True

        print(f'#### Cluster: `{c2.name}`')
        if features:
            print(f'- **Features:** {", ".join(f"`{f}`" for f in sorted(features))}')
        if attributes:
            print(f'- **Attributes:** {", ".join(f"`{a}`" for a in sorted(attributes))}')
        if accepted_commands:
            print(f'- **Accepted commands:** {", ".join(f"`{c}`" for c in sorted(accepted_commands))}')
        if generated_commands:
            print(f'- **Generated commands:** {", ".join(f"`{c}`" for c in sorted(generated_commands))}')
        if events:
            print(f'- **Events:** {", ".join(f"`{e}`" for e in sorted(events))}')
        print()


def get_all_provisional_clusters(prior_revision: PrebuiltDataModelDirectory, new_revision: PrebuiltDataModelDirectory):
    get_provisional_diff(prior_revision, new_revision)
    get_provisional_diff(new_revision, prior_revision)


def get_all_provisional_device_types(new_revision: PrebuiltDataModelDirectory):
    device_types, _ = build_xml_device_types(new_revision)

    print(f'### Provisional Device Types in {new_revision.dirname}\n')
    found_any = False
    for cid in sorted(device_types.keys()):
        d = device_types[cid]
        server_clusters = _get_provisional(d.server_clusters.values())
        client_clusters = _get_provisional(d.client_clusters.values())
        if not server_clusters and not client_clusters:
            continue

        found_any = True
        print(f'#### Device Type: `{d.name}`')
        if server_clusters:
            print(f'- **Provisional server clusters:** {", ".join(f"`{c}`" for c in sorted(server_clusters))}')
        if client_clusters:
            print(f'- **Provisional client clusters:** {", ".join(f"`{c}`" for c in sorted(client_clusters))}')
        print()
    if not found_any:
        print('- *(None)*\n')


REVISIONS = {
    '1.3': PrebuiltDataModelDirectory.k1_3,
    '1.4': PrebuiltDataModelDirectory.k1_4,
    '1.4.1': PrebuiltDataModelDirectory.k1_4_1,
    '1.4.2': PrebuiltDataModelDirectory.k1_4_2,
    '1.5': PrebuiltDataModelDirectory.k1_5,
    '1.5.1': PrebuiltDataModelDirectory.k1_5_1,
    '1.6': PrebuiltDataModelDirectory.k1_6,
    '1.6.1': PrebuiltDataModelDirectory.k1_6_1,
    '1.7': PrebuiltDataModelDirectory.k1_7,
}


@click.command()
@click.argument('prior_revision', type=click.Choice(list(REVISIONS.keys())))
@click.argument('new_revision', type=click.Choice(list(REVISIONS.keys())))
def main(prior_revision: str, new_revision: str):
    diff_clusters(REVISIONS[prior_revision], REVISIONS[new_revision])
    diff_device_types(REVISIONS[prior_revision], REVISIONS[new_revision])
    print('## Provisional Checks\n')
    get_all_provisional_clusters(REVISIONS[prior_revision], REVISIONS[new_revision])
    get_all_provisional_device_types(REVISIONS[new_revision])


if __name__ == "__main__":
    main()
