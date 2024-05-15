#
#    Copyright (c) 2023 Project CHIP Authors
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

import functools
from collections import defaultdict
from dataclasses import dataclass, field
from typing import Any

import chip.clusters as Clusters


@dataclass
class TagProblem:
    root: int
    missing_attribute: bool
    missing_feature: bool
    duplicates: set[int]
    same_tag: set[int] = field(default_factory=set)


def separate_endpoint_types(endpoint_dict: dict[int, Any]) -> tuple[list[int], list[int]]:
    """Returns a tuple containing the list of flat endpoints and a list of tree endpoints"""
    flat = []
    tree = []
    for endpoint_id, endpoint in endpoint_dict.items():
        if endpoint_id == 0:
            continue
        aggregator_id = 0x000e
        content_app_id = 0x0024
        device_types = [d.deviceType for d in endpoint[Clusters.Descriptor][Clusters.Descriptor.Attributes.DeviceTypeList]]
        if aggregator_id in device_types:
            flat.append(endpoint_id)
        else:
            if content_app_id in device_types:
                continue
            tree.append(endpoint_id)
    return (flat, tree)


def get_all_children(endpoint_id, endpoint_dict: dict[int, Any]) -> set[int]:
    """Returns all the children (include subchildren) of the given endpoint
       This assumes we've already checked that there are no cycles, so we can do the dumb things and just trace the tree
    """
    children = set()

    def add_children(endpoint_id, children):
        immediate_children = endpoint_dict[endpoint_id][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList]
        if not immediate_children:
            return
        children.update(set(immediate_children))
        for child in immediate_children:
            add_children(child, children)

    add_children(endpoint_id, children)
    return children


def find_tree_roots(tree_endpoints: list[int], endpoint_dict: dict[int, Any]) -> set[int]:
    """Returns a set of all the endpoints in tree_endpoints that are roots for a tree (not include singletons)"""
    tree_roots = set()

    def find_tree_root(current_id):
        for endpoint_id, endpoint in endpoint_dict.items():
            if endpoint_id not in tree_endpoints:
                continue
            if current_id in endpoint[Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList]:
                # this is not the root, move up
                return find_tree_root(endpoint_id)
        return current_id

    for endpoint_id in tree_endpoints:
        root = find_tree_root(endpoint_id)
        if root != endpoint_id:
            tree_roots.add(root)
    return tree_roots


def parts_list_cycles(tree_endpoints: list[int], endpoint_dict: dict[int, Any]) -> list[int]:
    """Returns a list of all the endpoints in the tree_endpoints list that contain cycles"""
    def parts_list_cycle_detect(visited: set, current_id: int) -> bool:
        if current_id in visited:
            return True
        visited.add(current_id)
        for child in endpoint_dict[current_id][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList]:
            child_has_cycles = parts_list_cycle_detect(visited, child)
            if child_has_cycles:
                return True
        return False

    cycles = []
    # This is quick enough that we can do all the endpoints without searching for the roots
    for endpoint_id in tree_endpoints:
        visited = set()
        if parts_list_cycle_detect(visited, endpoint_id):
            cycles.append(endpoint_id)
    return cycles


def create_device_type_lists(roots: list[int], endpoint_dict: dict[int, Any]) -> dict[int, dict[int, set[int]]]:
    """Returns a list of endpoints per device type for each root in the list"""
    device_types = {}
    for root in roots:
        tree_device_types = defaultdict(set)
        eps = get_all_children(root, endpoint_dict)
        eps.add(root)
        for ep in eps:
            for d in endpoint_dict[ep][Clusters.Descriptor][Clusters.Descriptor.Attributes.DeviceTypeList]:
                tree_device_types[d.deviceType].add(ep)
        device_types[root] = tree_device_types

    return device_types


def get_direct_children_of_root(endpoint_dict: dict[int, Any]) -> set[int]:
    root_children = set(endpoint_dict[0][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList])
    direct_children = root_children
    for ep in root_children:
        ep_children = set(endpoint_dict[ep][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList])
        direct_children = direct_children - ep_children
    return direct_children


def create_device_type_list_for_root(direct_children, endpoint_dict: dict[int, Any]) -> dict[int, set[int]]:
    device_types = defaultdict(set)
    for ep in direct_children:
        for d in endpoint_dict[ep][Clusters.Descriptor][Clusters.Descriptor.Attributes.DeviceTypeList]:
            device_types[d.deviceType].add(ep)
    return device_types


def cmp_tag_list(a: Clusters.Descriptor.Structs.SemanticTagStruct, b: Clusters.Descriptor.Structs.SemanticTagStruct):
    if a.mfgCode != b.mfgCode:
        return -1 if a.mfgCode < b.mfgCode else 1
    if a.namespaceID != b.namespaceID:
        return -1 if a.namespaceID < b.namespaceID else 1
    if a.tag != b.tag:
        return -1 if a.tag < b.tag else 1
    if a.label != b.label:
        return -1 if a.label < b.label else 1
    return 0


def find_tag_list_problems(roots: list[int], device_types: dict[int, dict[int, set[int]]], endpoint_dict: dict[int, Any]) -> dict[int, TagProblem]:
    """Checks for non-spec compliant tag lists"""
    tag_problems = {}
    for root in roots:
        for _, endpoints in device_types[root].items():
            if len(endpoints) < 2:
                continue
            for endpoint in endpoints:
                missing_feature = not bool(endpoint_dict[endpoint][Clusters.Descriptor]
                                           [Clusters.Descriptor.Attributes.FeatureMap] & Clusters.Descriptor.Bitmaps.Feature.kTagList)
                if Clusters.Descriptor.Attributes.TagList not in endpoint_dict[endpoint][Clusters.Descriptor] or endpoint_dict[endpoint][Clusters.Descriptor][Clusters.Descriptor.Attributes.TagList] == []:
                    tag_problems[endpoint] = TagProblem(root=root, missing_attribute=True,
                                                        missing_feature=missing_feature, duplicates=endpoints)
                    continue
                # Check that this tag isn't the same as the other tags in the endpoint list
                duplicate_tags = set()
                for other in endpoints:
                    if other == endpoint:
                        continue
                    # The OTHER endpoint is missing a tag list attribute - ignore this here, we'll catch that when we assess this endpoint as the primary
                    if Clusters.Descriptor.Attributes.TagList not in endpoint_dict[other][Clusters.Descriptor]:
                        continue

                    if sorted(endpoint_dict[endpoint][Clusters.Descriptor][Clusters.Descriptor.Attributes.TagList], key=functools.cmp_to_key(cmp_tag_list)) == sorted(endpoint_dict[other][Clusters.Descriptor][Clusters.Descriptor.Attributes.TagList], key=functools.cmp_to_key(cmp_tag_list)):
                        duplicate_tags.add(other)
                if len(duplicate_tags) != 0:
                    duplicate_tags.add(endpoint)
                    tag_problems[endpoint] = TagProblem(root=root, missing_attribute=False, missing_feature=missing_feature,
                                                        duplicates=endpoints, same_tag=duplicate_tags)
                    continue
                if missing_feature:
                    tag_problems[endpoint] = TagProblem(root=root, missing_attribute=False,
                                                        missing_feature=missing_feature, duplicates=endpoints)

    return tag_problems


def flat_list_ok(flat_endpoint_id_to_check: int, endpoints_dict: dict[int, Any]) -> bool:
    '''Checks if the (flat) PartsList on the supplied endpoint contains all the sub-children of its parts.'''
    sub_children = set()
    for child in endpoints_dict[flat_endpoint_id_to_check][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList]:
        sub_children.update(get_all_children(child, endpoints_dict))
    return all(item in endpoints_dict[flat_endpoint_id_to_check][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList] for item in sub_children)
