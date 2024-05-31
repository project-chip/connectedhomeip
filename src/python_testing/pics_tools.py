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

from spec_parsing_support import XmlCluster, build_xml_clusters

def attribute_pics(pics_base: str, id: int) -> str:
    return f'{pics_base}.S.A{id:04x}'


def accepted_cmd_pics(pics_base: str, id: int) -> str:
    return f'{pics_base}.S.C{id:02x}.Rsp'


def generated_cmd_pics(pics_base: str, id: int) -> str:
    return f'{pics_base}.S.C{id:02x}.Tx'


def feature_pics(pics_base: str, bit: int) -> str:
    return f'{pics_base}.S.F{bit:02x}'

def server_pics(pics_base: str) -> str:
    return f'{pics_base}.S'

def client_pics(pics_base: str) -> str:
    return f'{pics_base}.C'


def devimp() -> str:
    return "Does the device implement"
#TODO: handle adoc spacing
def _table_top()-> str:
    return ('|===\n'
            '|*Variable*   |*Description* |*Mandatory/Optional* |*Notes/Additional Constraints*)\n')
def _table_end() -> str:
    return '|==='

def _generate_generated_command_pics(cluster: XmlCluster) -> str:
    pass

def _generate_accepted_command_pics(cluster: XmlCluster) -> str:
    pass

def _generate_feature_pics(cluster: XmlCluster) -> str:
    pass

def _generate_attribute_pics(cluster: XmlCluster) -> str:
    # TODO: remove globals
    rows = [_table_top()]
    for id, attr in cluster.attributes.items():
        rows.append(f'|{attribute_pics(cluster.pics, id)}|{devimp()} {attr.name} attribute?| {attr.conformance.pics_str(cluster.pics)} ||\n')
    rows.append(_table_end())
    return ''.join(rows)

def _generate_server_cluster_pics(cluster: XmlCluster) -> str:
    pass

def _generate_role_pics(cluster: XmlCluster) -> str:
    rows = [_table_top(),
            f'|{server_pics(cluster.pics)}|{devimp()} {cluster.name} as a server?| O | |\n',
            f'|{client_pics(cluster.pics)}|{devimp()} {cluster.name} as a client?| O | |\n',
            _table_end()]
    return ''.join(rows)

def generate_pics_from_spec():
    #TODO: figure out how we actually want to output this - files? individual? All together?
    clusters, problems = build_xml_clusters()
    for id, cluster in clusters.items():
        print('=== Role')
        print(_generate_role_pics(cluster))
        print('=== Server')
        #TODO: add features
        print('==== Attributes')
        print(_generate_attribute_pics(cluster))
        # TODO: add accepted and generated commands
        # TODO: Add client (all optional)

if __name__ == "__main__":
    generate_pics_from_spec()
