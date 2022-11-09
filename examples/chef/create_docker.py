#!/usr/bin/env python3

# Copyright (c) 2020 Project CHIP Authors
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

import os
import sys
import optparse
import tarfile
import shutil
import re
import docker

client = docker.from_env()

parser = optparse.OptionParser()
parser.add_option("-c", "--commit_sha", type='string',
                  action='store',
                  dest="commit_sha")
parser.add_option("-s", "--short_sha", type='string',
                  action='store',
                  dest="short_sha")
parser.add_option("-r", "--revision_id", type='string',
                  action='store',
                  dest="revision_id")
parser.add_option("-b", "--build_id", type='string',
                  action='store',
                  dest="build_id")
parser.add_option("-i", "--image_name", type='string',
                  action='store',
                  dest="image_name")
parser.add_option("-w", "--working_dir", type='string',
                  action='store',
                  dest="working_dir")
parser.add_option("-t", "--tar_path", type='string',
                  action='store',
                  dest="tar_path")
parser.add_option("-p", "--platform", type='string',
                  action='store',
                  dest="platform")

options, _ = parser.parse_args(sys.argv[1:])

out_directory = f'{options.working_dir}/out'

# Fetch list of devices
files = os.listdir(f'{options.working_dir}/devices')
devices = []
for file in files:
    device = re.search("rootnode.+(?=.zap)", file)
    if device is not None and device.group(0) not in devices:
        devices.append(device.group(0))

for device in devices:
    # Clean up the out directory before extracting device files
    shutil.rmtree(out_directory)
    os.mkdir(out_directory)

    print(f'Extracting {options.platform} files of {device}')
    my_tar = tarfile.open(
        f'{options.tar_path}/{options.platform}-{device}.tar.gz')
    my_tar.extractall(out_directory)
    my_tar.close()

    docker_image_name = f'{options.image_name}/{options.platform}/{device}'.lower()

    print(f'Building {options.platform} docker image for {device}')
    image = client.images.build(path=options.working_dir, buildargs={
        'DEVICE_NAME': f'{device}'})
    image[0].tag(docker_image_name, tag='latest')
    image[0].tag(docker_image_name, tag=f'short-sha_{options.short_sha}')
    image[0].tag(docker_image_name, tag=f'build-id_{options.build_id}')
    image[0].tag(docker_image_name, tag=f'commit-sha_{options.commit_sha}')
    image[0].tag(docker_image_name, tag=f'revision-id_{options.revision_id}')

    print(f'Pushing image: {docker_image_name}')
    client.images.push(docker_image_name)
