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
import docker

client = docker.from_env()

_CREATE_DOCKER_SCRIPT_PATH = os.path.dirname(__file__)
_SUPPORTED_PLATFORM = 'linux_x86'

parser = optparse.OptionParser()
parser.add_option('-c', '--commit_sha', type='string',
                  action='store',
                  dest='commit_sha')
parser.add_option('-s', '--short_sha', type='string',
                  action='store',
                  dest='short_sha')
parser.add_option('-r', '--revision_id', type='string',
                  action='store',
                  dest='revision_id')
parser.add_option('-b', '--build_id', type='string',
                  action='store',
                  dest='build_id')
parser.add_option('-i', '--image_name', type='string',
                  action='store',
                  dest='image_name')
parser.add_option('-t', '--tar_path', type='string',
                  action='store',
                  dest='tar_path')

options, args = parser.parse_args(sys.argv[1:])

# Check that all options are set
for option, value in options.__dict__.items():
    if (value is None):
        parser.error(f'--{option} is not set')

out_directory = f'{_CREATE_DOCKER_SCRIPT_PATH}/out'

for device_file_name in os.listdir(options.tar_path):
    # Clean up the out directory before extracting device files
    shutil.rmtree(out_directory)
    os.mkdir(out_directory)

    platform, device = device_file_name.split('-')
    if _SUPPORTED_PLATFORM not in platform:
        continue

    device = device.replace('.tar.gz', '')

    print(f'Extracting {platform} files of {device}')
    my_tar = tarfile.open(f'{options.tar_path}/{device_file_name}')
    my_tar.extractall(out_directory)
    my_tar.close()

    docker_image_name = f'{options.image_name}/{platform}/{device}'.lower()

    print(f'Building {platform} docker image for {device}')
    image = client.images.build(path=_CREATE_DOCKER_SCRIPT_PATH, buildargs={
        'DEVICE_NAME': f'{device}'})
    image[0].tag(docker_image_name, tag='latest')
    image[0].tag(docker_image_name, tag=f'short-sha_{options.short_sha}')
    image[0].tag(docker_image_name, tag=f'build-id_{options.build_id}')
    image[0].tag(docker_image_name, tag=f'commit-sha_{options.commit_sha}')
    image[0].tag(docker_image_name, tag=f'revision-id_{options.revision_id}')

    print(f'Pushing image: {docker_image_name}')
    client.images.push(docker_image_name)
