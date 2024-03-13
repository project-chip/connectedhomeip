# Copyright (c) 2023 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import uuid

from avh_api import ApiClient as AvhApiClient
from avh_api import Configuration as AvhApiConfiguration
from avh_api.api.arm_api import ArmApi as AvhApi
from avh_api.model.project_key import ProjectKey as AvhProjectKey


class AvhClient:
    def __init__(self, api_token, api_endpoint=None):
        avh_api_config = AvhApiConfiguration(host=api_endpoint)
        self.avh_api_client = AvhApiClient(avh_api_config)

        self.avh_api = AvhApi(self.avh_api_client)

        avh_api_config.access_token = self.avh_api.v1_auth_login(
            {"api_token": api_token}
        ).token

        self.default_project_id = self.avh_api.v1_get_projects()[0]["id"]

    def create_project(self, name, num_cores):
        return self.avh_api.v1_create_project(
            {
                "id": str(uuid.uuid4()),
                "name": name,
                "settings": {
                    "internet_access": True,
                    "dhcp": True,
                },
                "quotas": {"cores": num_cores},
            }
        )["id"]

    def delete_project(self, id):
        self.avh_api.v1_delete_project(id)

    def create_instance(self, name, flavor, os, osbuild):
        return self.avh_api.v1_create_instance(
            {
                "name": name,
                "project": self.default_project_id,
                "flavor": flavor,
                "os": os,
                "osbuild": osbuild,
            }
        )["id"]

    def instance_state(self, instance_id):
        return str(self.avh_api.v1_get_instance_state(instance_id))

    def instance_console_log(self, instance_id):
        return self.avh_api.v1_get_instance_console_log(instance_id)

    def instance_quick_connect_command(self, instance_id):
        return self.avh_api.v1_get_instance_quick_connect_command(instance_id)

    def create_ssh_project_key(self, label, key):
        return self.avh_api.v1_add_project_key(
            self.default_project_id,
            AvhProjectKey(kind="ssh", key=key, label=label),
        )["identifier"]

    def instance_console_url(self, instance_id):
        return self.avh_api.v1_get_instance_console(instance_id).url

    def delete_ssh_project_key(self, key_id):
        self.avh_api.v1_remove_project_key(self.default_project_id, key_id)

    def delete_instance(self, instance_id):
        self.avh_api.v1_delete_instance(instance_id)

    def close(self):
        self.avh_api_client.rest_client.pool_manager.clear()
        self.avh_api_client.close()
