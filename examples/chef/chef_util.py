# Copyright (c) 2022 Project CHIP Authors
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

import subprocess
import json
import os
import hashlib


ci_allowlist = ['lighting-app.zap']
cd_platforms_meta = {
    'linux': {
        'build_dir': 'linux/out',
        'platform_label': 'linux_x86',
    },
    'esp32': {
        'build_dir': 'esp32/build',
        'platform_label': 'esp32-m5stack',
    },
    'nrfconnect': {
        'build_dir': 'nrfconnect/build',
        'platform_label': 'nrf-nrf52840dk',
    },
}


def bundle(platform: str):
    if platform == 'linux':
        pass
    if platform == 'esp32':
        pass
    if platform == 'nrfconnect':
        pass


def check_zap_master(repo_base_path: str) -> str:
    """Produces hash of ZAP submodule in branch master"""
    git_cmd = ["git", "ls-tree", "master", "third_party/zap/repo"]
    zap_commit = str(subprocess.check_output(git_cmd, cwd=repo_base_path))
    zap_commit = zap_commit.split(" ")[2]
    zap_commit = zap_commit[:zap_commit.index("\\")]
    print(f"zap commit: {zap_commit}")
    return zap_commit


def generate_device_manifest(chef_devices_dir: str, include_zap_submod: bool = False, write_manifest_file: bool = False, ci_manifest_file_name: str = '', repo_base_path: str = '', chef_zzz_root: str = '') -> dict:
    """Produces dictionary containing md5 of device dir zap files"""
    ci_manifest = {}
    for device_dir_item in os.listdir(chef_devices_dir):
        target_file_ext = ".zap"
        if device_dir_item.endswith(target_file_ext):  # and device_dir_item in ci_allowlist:
            device_name = device_dir_item[:-len(target_file_ext)]
            device_file_path = os.path.join(chef_devices_dir, device_dir_item)
            with open(device_file_path, "rb") as device_file:
                device_file_data = device_file.read()
                device_file_md5 = hashlib.md5(device_file_data).hexdigest()
                ci_manifest[device_name] = device_file_md5
                print(f"Manifest for {device_name} : {device_file_md5}")
                if write_manifest_file:
                    device_zzz_dir_root = os.path.join(chef_zzz_root, device_name)
                    device_zzz_md5_file = os.path.join(device_zzz_dir_root, 'INPUTMD5.txt')
                    with open(device_zzz_md5_file, "w+", encoding="utf-8") as md5file:
                        md5file.write(device_file_md5)
    if include_zap_submod:
        ci_manifest["zap_commit"] = check_zap_master(repo_base_path)
    if write_manifest_file:
        with open(ci_manifest_file_name, "w+", encoding="utf-8") as ci_manifest_file:
            ci_manifest_file.write(json.dumps(ci_manifest, indent=4)+"\n")
    return ci_manifest
