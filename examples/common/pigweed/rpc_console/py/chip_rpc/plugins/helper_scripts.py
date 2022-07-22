#
#    Copyright (c) 2022 Project CHIP Authors
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
"""Helper scripts for interacting with Chip devices."""

import json


class HelperScripts:
    """Helper scripts for interacting with Chip devices."""

    def __init__(self, rpcs):
        self.rpcs = rpcs

    def get_descriptor(self) -> str:
        """Use the Descriptor RPC service to build a dictionary with descriptor
           values. Currently this reads the root node's DeviceTypeList and
           PartsList, and the server/client list for every endpoint in the
           PartsList."""
        out = {}

        (status, device_types) = self.rpcs.chip.rpc.Descriptor.DeviceTypeList(endpoint=0)
        if not status.ok():
            raise Exception("Failed to read ep0 device type list: %s", status)

        out["device_types"] = list(
            map(lambda dt: dt.device_type, device_types))
        (status, parts_list) = self.rpcs.chip.rpc.Descriptor.PartsList(endpoint=0)
        if not status.ok():
            raise Exception("Failed to read ep0 parts list: %s", status)

        out["parts_list"] = list(map(lambda ep: ep.endpoint, parts_list))
        out["endpoints"] = {}
        for ep in out["parts_list"]:
            out["endpoints"][str(ep)] = {}
            (status, clusters) = self.rpcs.chip.rpc.Descriptor.ClientList(endpoint=ep)
            if not status.ok():
                raise Exception("Failed to read ep0 parts list: %s", status)
            out["endpoints"][str(ep)]["client_list"] = list(
                map(lambda c: c.cluster_id, clusters))
            (status, clusters) = self.rpcs.chip.rpc.Descriptor.ServerList(endpoint=ep)
            if not status.ok():
                raise Exception("Failed to read ep0 parts list: %s", status)
            out["endpoints"][str(ep)]["server_list"] = list(
                map(lambda c: c.cluster_id, clusters))

        return out

    def print_descriptor(self) -> None:
        """ Pretty print the results of get_descriptor. """
        print(json.dumps(self.get_descriptor(), indent=4))
