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

import argparse
import os
import shutil
import tarfile

import docker

client = docker.from_env()

_CREATE_DOCKER_SCRIPT_PATH = os.path.dirname(__file__)
_SUPPORTED_PLATFORM = 'linux_x86'

parser = argparse.ArgumentParser()
parser.add_argument('-c', '--commit_sha', required=True)
parser.add_argument('-s', '--short_sha', required=True)
parser.add_argument('-r', '--revision_id', required=True)
parser.add_argument('-b', '--build_id', required=True)
parser.add_argument('-i', '--image_name', required=True)
parser.add_argument('-t', '--tar_path', required=True)

args = parser.parse_args()

out_directory = f'{_CREATE_DOCKER_SCRIPT_PATH}/out'

for device_file_name in os.listdir(args.tar_path):
    platform, device = device_file_name.split('-')
    if _SUPPORTED_PLATFORM not in platform:
        continue

    # Clean up the out directory before extracting device files
    shutil.rmtree(out_directory, ignore_errors=True)
    os.mkdir(out_directory)

    device = device.replace('.tar.gz', '')

    print(f'Extracting {platform} files of {device}')
    my_tar = tarfile.open(f'{args.tar_path}/{device_file_name}')
    my_tar.extractall(out_directory)
    my_tar.close()

    docker_image_name = f'{args.image_name}/{platform}/{device}'.lower()

    print(f'Building {platform} docker image for {device}')
    image = client.images.build(path=_CREATE_DOCKER_SCRIPT_PATH, buildargs={
        'DEVICE_NAME': f'{device}'})
    image[0].tag(docker_image_name, tag='latest')
    image[0].tag(docker_image_name, tag=f'short-sha_{args.short_sha}')
    image[0].tag(docker_image_name, tag=f'build-id_{args.build_id}')
    image[0].tag(docker_image_name, tag=f'commit-sha_{args.commit_sha}')
    image[0].tag(docker_image_name, tag=f'revision-id_{args.revision_id}')

    print(f'Pushing image: {docker_image_name}')
    response = client.images.push(docker_image_name, stream=True, decode=True)
    for line in response:
        print(line)
